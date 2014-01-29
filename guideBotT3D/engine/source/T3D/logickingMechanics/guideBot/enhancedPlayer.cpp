//-----------------------------------------------------------------------------
// Guide Bot
// Copyright (C) LogicKing.com, Inc.
//-----------------------------------------------------------------------------

#include "guideBot/actorState.h"
#include "guideBot/perceptionEvent.h"
#include "guideBot/stringUtils.h"

#include "T3D/logickingMechanics/guideBot/enhancedPlayer.h"
#include "T3D/logickingMechanics/guideBot/scriptAction.h"
#include "T3D/logickingMechanics/guideBot/sceneWorldObject.h"
#include "scene/simPath.h"
#include "T3D/gameBase/gameConnection.h"
#include "core/stream/bitStream.h"
#include "ts/tsShapeInstance.h"
#include "T3D/physics/physicsPlayer.h"
#include "math/mathTypes.h"
#include "math/mathIO.h"
#include "sfx/sfxSystem.h"
#include "sfx/sfxTrack.h"
#include "sfx/sfxSource.h"
#include "sfx/sfxTypes.h"

#include "console/consoleTypes.h"

#include <sstream>

////////////////////////////////////////////////////
//////***********VisualDataBlock************/////////
////////////////////////////////////////////////////
IMPLEMENT_CO_DATABLOCK_V1( VisualDataBlock );

VisualDataBlock::VisualDataBlock()
{
	m_visualAnimStr  = NULL;
	m_visualSoundTrack = NULL;
	m_playTime = GuideBot::INVALID_TIME;

	for (size_t i=0; i<MaxCallbacks; i++)
	{
		m_callbackTime[i] = 0.f;
		m_callbackFunc[i] = NULL;
	}

}

void VisualDataBlock::initPersistFields()
{
	addField("visualAnim", TypeCaseString, Offset(m_visualAnimStr, VisualDataBlock));
	addField("visualSound", TypeSFXTrackName, Offset(m_visualSoundTrack, VisualDataBlock));
	addField("visualEffectors", TypeCaseString, Offset(m_visualEffectorsStr, VisualDataBlock));
	addField("time", TypeS32, Offset(m_playTime, VisualDataBlock));

	addField("callbackTime", TypeF32, Offset(m_callbackTime,VisualDataBlock), MaxCallbacks);
	addField("callbackFunction", TypeCaseString, Offset(m_callbackFunc,VisualDataBlock), MaxCallbacks);
}

VisualDataBlock::CallbackCaller::CallbackCaller(const char* name) : m_funcName(name)
{

}

void VisualDataBlock::CallbackCaller::execute(GuideBot::Actor* owner)
{
	EnhancedPlayer* enhancedPlayer = static_cast<EnhancedPlayer*>(owner);
	const char* argv[2];
	argv[0] = m_funcName.c_str();
	argv[1] = enhancedPlayer->getIdString();
	Con::execute(2, argv);
}

bool VisualDataBlock::onAdd()
{
	if (!Parent::onAdd())
		return false;

	const char* name = Parent::getName();
	if (name)
	{
		//setup visual
		
		if (m_visualEffectorsStr && GuideBot::Effector::convertFromString(m_visualEffectorsStr,m_effectorIDs))
		{
			m_name = name;
			for (size_t i=0; i<MaxCallbacks; i++)
			{
				if (m_callbackFunc[i] != NULL)
				{
					CallbackCaller*	caller = new CallbackCaller(m_callbackFunc[i]);
					m_callbackCallers.push_back(caller);
					GuideBot::Visual::AnimCallbackFunction callback;
					callback.bind(caller,&CallbackCaller::execute);
					addCallback(m_callbackTime[i],callback);
				}
			}
		}
		else
		{
			Con::errorf( "VisualDataBlock: visual %s doesn't have effectors.It can't be created",name);
			return false;
		}
	}
	return true;
}

void VisualDataBlock::onRemove()
{
	//destroy callers
	for (size_t i=0;i<m_callbackCallers.size(); i++)
	{
		CallbackCaller*	caller = m_callbackCallers[i];
		GB_SAFE_DELETE(caller);
	}
	m_callbackCallers.clear();

	Parent::onRemove();

	
}

void VisualDataBlock::packData(BitStream* stream)
{
	Parent::packData(stream);

	stream->writeString(m_name.c_str());

	stream->writeString(m_visualAnimStr);
	sfxWrite(stream, m_visualSoundTrack);
	stream->writeString(m_visualEffectorsStr);
	

	for (size_t i=0; i<MaxCallbacks; i++)
	{
		if (stream->writeFlag(m_callbackFunc[i]!=NULL))
		{
			stream->writeString(m_callbackFunc[i]);
			stream->write(m_callbackTime[i]);
		}
	}
}

void VisualDataBlock::unpackData(BitStream* stream)
{
	Parent::unpackData(stream);

	const char* object = stream->readSTString();
	assignName(object);	


	m_visualAnimStr = stream->readSTString();
	sfxRead(stream, &m_visualSoundTrack);
	m_visualEffectorsStr = stream->readSTString();
	
	for (size_t i=0; i<MaxCallbacks; i++)
	{
		if (stream->readFlag())
		{
			m_callbackFunc[i] = stream->readSTString();
			stream->read(&m_callbackTime[i]);
		}
	}
}

bool VisualDataBlock::preload(bool server, String &errorStr)
{
	if(!Parent::preload(server, errorStr))
		return false;

	// Resolve objects transmitted from server
	if( !server )
	{
		String errorStr;
		if( !sfxResolve( &m_visualSoundTrack, errorStr ) )
			Con::errorf( "VisualDataBlock::preload: %s", errorStr.c_str() );
	}

	return true;
}


////////////////////////////////////////////////////
//////***********StateDataBlock************/////////
////////////////////////////////////////////////////
IMPLEMENT_CO_DATABLOCK_V1( StateDataBlock );

StateDataBlock::StateDataBlock()
{
	m_state = NULL;
	m_pose = NULL;
	for (size_t i=0; i<MaxMoveDirs; i++)
	{
		m_movementDir[i] = VectorF::Zero;
		m_movementVisual[i] = NULL;
		m_movementVel[i] = 0.f;
	}

	for (size_t i=0; i<MaxVisuals; i++)
	{
		m_alias[i] = NULL;
		m_aliasVisual[i] = NULL;
	}

	for (size_t i=0; i<MaxChanges; i++)
	{
		m_changeState[i] = NULL;
		m_changeVisual[i] = NULL;
	}
	
	m_enableCallback = NULL;
	m_disableCallback = NULL;
}


void StateDataBlock::initPersistFields()
{
	addField("movementDir", TypePoint3F, Offset(m_movementDir, StateDataBlock), MaxMoveDirs);
	addField("movementVisual", TypeCaseString, Offset(m_movementVisual, StateDataBlock), MaxMoveDirs);
	addField("movementVel", TypeF32, Offset(m_movementVel, StateDataBlock), MaxMoveDirs);

	addField("alias", TypeCaseString, Offset(m_alias, StateDataBlock), MaxVisuals);
	addField("aliasVisual", TypeCaseString, Offset(m_aliasVisual, StateDataBlock), MaxVisuals);

	addField("changeState", TypeCaseString, Offset(m_changeState, StateDataBlock), MaxChanges);
	addField("changeVisual", TypeCaseString, Offset(m_changeVisual, StateDataBlock), MaxChanges);

	addField("enableCallback", TypeCaseString, Offset(m_enableCallback, StateDataBlock));
	addField("disableCallback", TypeCaseString, Offset(m_disableCallback, StateDataBlock));

	addField("pose", TypeCaseString, Offset(m_pose, StateDataBlock));
}

bool StateDataBlock::onAdd()
{
	if (!Parent::onAdd())
		return false;
	const char* name = getName();
	if (name)
	{
		//create state
		m_state = new GuideBot::State(name);
		m_state->setUserData(this);

		for (int i = 0;i<MaxMoveDirs; i++)
		{
			if (m_movementVisual[i]!=NULL && dStrcmp(m_movementVisual[i],""))
			{
				m_state->addMovementDesc(GuideBot::MovementDesc(GuideBot::toGbVector(m_movementDir[i]), 
					m_movementVel[i], m_movementVisual[i]));
			}
		}

		for (int i = 0;i<MaxVisuals; i++)
		{
			if (m_alias[i]!=NULL && m_aliasVisual[i]!=NULL)
			{
				std::string alias = m_alias[i];
				std::string visual = m_aliasVisual[i];
				if (!alias.empty() && !visual.empty())
					m_state->addVisualAlias(alias,visual);
			}
		}
		
		for (int i = 0;i<MaxChanges; i++)
		{
			if (m_changeState[i]!=NULL && m_changeVisual[i]!=NULL)
			{
				std::string stateName = m_changeState[i];
				std::string visual = m_changeVisual[i];
				m_state->addStateChange(stateName,visual);
			}
		}

		std::string enableCallback = m_enableCallback ? m_enableCallback : "";
		std::string disableCallback =  m_disableCallback ? m_disableCallback : "";
		m_state->setCallbacks(enableCallback,disableCallback);
	}
	return true;
}

void StateDataBlock::onRemove()
{
	SAFE_DELETE(m_state);
	Parent::onRemove();
}

void StateDataBlock::packData(BitStream* stream)
{
	Parent::packData(stream);
	
	stream->writeString(getName());

	for (int i = 0; i < MaxMoveDirs; i++)
	{
		if (stream->writeFlag(m_movementVisual[i]!=NULL))
		{
			stream->writeString(m_movementVisual[i]);
			mathWrite(*stream,m_movementDir[i]);
			stream->write(m_movementVel[i]);
		}
	}

	for (int i = 0; i < MaxVisuals; i++)
	{
		if (stream->writeFlag(m_alias[i]!=NULL && m_aliasVisual[i]!=NULL))
		{
			stream->writeString(m_alias[i]);
			stream->writeString(m_aliasVisual[i]);
		}
	}

	for (int i = 0; i < MaxChanges; i++)
	{
		if (stream->writeFlag(m_changeState[i]!=NULL && m_changeVisual[i]!=NULL))
		{
			stream->writeString(m_changeState[i]);
			stream->writeString(m_changeVisual[i]);
		}
	}

	if (stream->writeFlag(m_enableCallback))
		stream->writeString(m_enableCallback);
	if (stream->writeFlag(m_disableCallback))
		stream->writeString(m_disableCallback);

	stream->writeString(m_pose);
}

void StateDataBlock::unpackData(BitStream* stream)
{
	Parent::unpackData(stream);

	const char* object = stream->readSTString();
	assignName(object);	

	for (int i = 0;i<MaxMoveDirs; i++)
	{
		if (stream->readFlag())
		{
			m_movementVisual[i] = stream->readSTString();
			mathRead(*stream,&m_movementDir[i]);
			stream->read(&m_movementVel[i]);
		}
	}

	for (int i = 0;i<MaxVisuals; i++)
	{
		if (stream->readFlag())
		{
			m_alias[i] = stream->readSTString();
			m_aliasVisual[i] = stream->readSTString();
		}
	}

	for (int i = 0;i<MaxChanges; i++)
	{
		if (stream->readFlag())
		{
			m_changeState[i] = stream->readSTString();
			m_changeVisual[i] = stream->readSTString();
		}
	}

	if (stream->readFlag())
		m_enableCallback = stream->readSTString();
	if (stream->readFlag())
		m_disableCallback = stream->readSTString();

	m_pose = stream->readSTString();
}

////////////////////////////////////////////////////
//////********AttackInfoDataBlock**********/////////
////////////////////////////////////////////////////
IMPLEMENT_CO_DATABLOCK_V1(AttackInfoDataBlock);

AttackInfoDataBlock::AttackInfoDataBlock(): m_minDist(0.f), m_maxDist(0.f), m_fovAngle(0), m_periodicity(0),m_action(NULL), m_pause(0),
											m_evaluator(NULL), m_weapon(NULL), m_state(NULL), m_coverState(NULL),
											m_manoeuvreMinDist(2.f), m_manoeuvreMaxDist(10.f), m_strafe(true), 
											m_lookAtEnemy(true), m_attackState(NULL), m_blockMovements(false)
{

}

void AttackInfoDataBlock::initPersistFields()
{
	addField("minDist", TypeF32, Offset(m_minDist, AttackInfoDataBlock));
	addField("maxDist", TypeF32, Offset(m_maxDist, AttackInfoDataBlock));
	addField("fov",  TypeS32, Offset(m_fovAngle, AttackInfoDataBlock));
	addField("periodicity",  TypeS32, Offset(m_periodicity, AttackInfoDataBlock));
	addField("manoeuvreMinDist", TypeF32, Offset(m_manoeuvreMinDist, AttackInfoDataBlock));
	addField("manoeuvreMaxDist", TypeF32, Offset(m_manoeuvreMaxDist, AttackInfoDataBlock));
	addField("strafe", TypeBool, Offset(m_strafe, AttackInfoDataBlock));
	addField("pause",  TypeS32, Offset(m_pause, AttackInfoDataBlock));
	addField("blockMovements",  TypeS32, Offset(m_blockMovements, AttackInfoDataBlock));
	addField("evaluator",  TypeCaseString, Offset(m_evaluator, AttackInfoDataBlock));
	addField("action",  TypeCaseString, Offset(m_action, AttackInfoDataBlock));
	addField("state",  TypeCaseString, Offset(m_state, AttackInfoDataBlock));
	addField("weapon",  TypeCaseString, Offset(m_weapon, AttackInfoDataBlock));	
	addField("coverState",  TypeCaseString, Offset(m_coverState, AttackInfoDataBlock));
	addField("attackState",  TypeCaseString, Offset(m_attackState, AttackInfoDataBlock));
	addField("lookAtEnemy",  TypeBool, Offset(m_lookAtEnemy, AttackInfoDataBlock));
}

bool AttackInfoDataBlock::onAdd()
{
	if (!Parent::onAdd())
		return false;
	const char* name = getName();
	if (name)	
	{
		m_attackInfo.minDistSq = m_minDist*m_minDist;
		m_attackInfo.maxDistSq = m_maxDist*m_maxDist;
		m_attackInfo.fov = (float) m_fovAngle/180.f* M_PI;
		m_attackInfo.periodicity = (unsigned int) m_periodicity;
		m_attackInfo.manoeuvreMinDist = m_manoeuvreMinDist;
		m_attackInfo.manoeuvreMaxDist = m_manoeuvreMaxDist;
		m_attackInfo.strafe = m_strafe;
		m_attackInfo.pause = (unsigned int) m_pause;
		m_attackInfo.blockMovements = m_blockMovements;
		if (m_evaluator)
			m_attackInfo.evaluator.bind(this,&AttackInfoDataBlock::evaluatorCaller);
		m_attackInfo.action = m_action ? m_action : "";
		m_attackInfo.weapon = m_weapon ? m_weapon : "";
		m_attackInfo.state = m_state ? m_state : "";
		m_attackInfo.coverState = m_coverState ? m_coverState : "";
		m_attackInfo.attackState = m_attackState ? m_attackState : "";
		m_attackInfo.name = name;
		m_attackInfo.lookAtEnemy = m_lookAtEnemy;
		m_attackInfo.setUserData(this);
	}
	return true;
}

float AttackInfoDataBlock::evaluatorCaller(GuideBot::Actor* actor,GuideBot::WorldObject* enemyObj, float distSq, 
										   float fov,GuideBot::AttackInfo* info, unsigned int lastExecutionTime, 
										   bool isActive)
{
	EnhancedPlayer* player = static_cast<EnhancedPlayer*>(actor);
	SceneObject*	object = static_cast<SceneObject*>(enemyObj->getUserData());
	GB_ASSERT(object,"AttackInfoDataBlock::evaluatorCaller: world object cannot be casted to scene object");
	char playerID[64];
	dSprintf(playerID, sizeof(playerID), "%d",player ->getId());
	char objectID[64];
	dSprintf(objectID, sizeof(objectID), "%d", object->getId());
	char infoID[64];
	dSprintf(infoID, sizeof(infoID), "%d", getId());

	const char* argv[8];
	argv[0] = m_evaluator;
	argv[1] = playerID;
	argv[2] = objectID;
	argv[3] = Con::getFloatArg(distSq);
	argv[4] = Con::getFloatArg(fov);
	argv[5] = infoID;
	argv[6] = Con::getIntArg(lastExecutionTime);
	argv[7] = isActive ? "1" : "0";
	

	const char* buf = Con::execute(8, argv);
	return buf ? dAtof(buf) : 0.f;

}

////////////////////////////////////////////////////
//////*******Waypoint object and datablock*******///
////////////////////////////////////////////////////
IMPLEMENT_CO_DATABLOCK_V1(WaypointDataBlock);
WaypointDataBlock::WaypointDataBlock():	m_radius(0.f)
{

}

void WaypointDataBlock::initPersistFields()
{
	Parent::initPersistFields();
	addField("radius", TypeF32, Offset(m_radius, AnchorDataBlock));
}

IMPLEMENT_CO_NETOBJECT_V1(WaypointObject);

WaypointObject::WaypointObject():	m_waypoint(NULL)
									, mDataBlock(NULL)
{
}

bool WaypointObject::onNewDataBlock(GameBaseData* dptr, bool reload)
{
	mDataBlock = dynamic_cast<WaypointDataBlock*>(dptr);
	if (!mDataBlock || !Parent::onNewDataBlock(mDataBlock, reload))
		return false;
	return true;
}

bool WaypointObject::onAdd()
{
	if (!Parent::onAdd())
		return false;

	if (isServerObject() && mDataBlock)
	{
		m_waypoint = new GuideBot::Waypoint(GuideBot::toGbMatrix(getTransform()), mDataBlock->m_radius);
		m_waypoint->registerObject();
		m_waypoint->setUserData(this);
		//for correct world object representation
		m_worldObject = m_waypoint;
	}

	return true;
}

void WaypointObject::onRemove()
{
	SAFE_DELETE(m_waypoint);

	Parent::onRemove();
}


////////////////////////////////////////////////////
//////***********ObstacleObject************/////////
////////////////////////////////////////////////////

ImplementEnumType( ObstacleTypes, "" )
   { GuideBot::Obstacle::OT_LOW, "low" },
   { GuideBot::Obstacle::OT_MEDIUM, "medium" },
   { GuideBot::Obstacle::OT_HIGH, "high" }
EndImplementEnumType;

IMPLEMENT_CO_NETOBJECT_V1(ObstacleObject);

ObstacleObject::ObstacleObject(): m_type(GuideBot::Obstacle::OT_HIGH),m_radius(0.f), m_obstacle(NULL)
{

}

void ObstacleObject::initPersistFields()
{
	Parent::initPersistFields();
	addField("radius", TypeF32, Offset(m_radius, ObstacleObject));
	addField("type",  TYPEID< ObstacleTypes >(), Offset(m_type, ObstacleObject));
}

bool ObstacleObject::onAdd()
{
	if (!Parent::onAdd())
		return false;

	if (isServerObject())
	{
		m_obstacle = new GuideBot::Obstacle(m_type, GuideBot::toGbMatrix(getTransform()), m_radius);
	}

	return true;
}

void ObstacleObject::onRemove()
{
	Parent::onRemove();
	SAFE_DELETE(m_obstacle);
}

////////////////////////////////////////////////////
//////**********AnchorObject***************/////////
////////////////////////////////////////////////////
IMPLEMENT_CO_DATABLOCK_V1(AnchorDataBlock);
AnchorDataBlock::AnchorDataBlock():	m_radius(0.f)
									, m_type(GuideBot::Obstacle::OT_HIGH)
									, m_hasObstacle(false)
{
	
}

void AnchorDataBlock::initPersistFields()
{
	Parent::initPersistFields();
	addField("radius", TypeF32, Offset(m_radius, AnchorDataBlock));
	addField("type",  TYPEID< ObstacleTypes >(), Offset(m_type, AnchorDataBlock));
	addField("hasObstacle",  TypeBool, Offset(m_hasObstacle, AnchorDataBlock));
}

//IMPLEMENT_CONSOLETYPE(AnchorDataBlock);
//IMPLEMENT_SETDATATYPE(AnchorDataBlock);
//IMPLEMENT_GETDATATYPE(AnchorDataBlock);


IMPLEMENT_CO_NETOBJECT_V1(AnchorObject);

AnchorObject::AnchorObject():	m_anchor(NULL)
								, m_obstacle(NULL)
								, mDataBlock(NULL)
{
}

bool AnchorObject::onNewDataBlock(GameBaseData* dptr, bool reload)
{
	mDataBlock = dynamic_cast<AnchorDataBlock*>(dptr);
	if (!mDataBlock || !Parent::onNewDataBlock(mDataBlock, reload))
		return false;
	return true;

}

bool AnchorObject::onAdd()
{
	if (!Parent::onAdd())
		return false;

	if (isServerObject() && mDataBlock)
	{
		m_anchor = new GuideBot::Anchor(GuideBot::toGbMatrix(getTransform()), mDataBlock->m_radius);
		if (mDataBlock->m_hasObstacle)
			m_obstacle = new GuideBot::Obstacle(mDataBlock->m_type, 
					GuideBot::toGbMatrix(getTransform()), mDataBlock->m_radius);

		//for correct world object representation
		m_worldObject = m_anchor;
	}

	return true;
}

void AnchorObject::onRemove()
{
	Parent::onRemove();
	SAFE_DELETE(m_anchor);
	SAFE_DELETE(m_obstacle);	
}

////////////////////////////////////////////////////
//////***********EnhancedPlayer data*******/////////
////////////////////////////////////////////////////

IMPLEMENT_CO_DATABLOCK_V1(EnhancedPlayerData);
EnhancedPlayerData::EnhancedPlayerData()
{
	for (size_t i=0; i<MaxAttackInfosNum; i++)
	{
		m_attackInfoNames[i] = NULL;
	}
}

void EnhancedPlayerData::initPersistFields()
{
	Parent::initPersistFields();
	addField("attackInfo", TypeCaseString, Offset(m_attackInfoNames, EnhancedPlayerData), MaxAttackInfosNum);
	addField("enableSlopes", TypeBool, Offset(m_enableSlopes, EnhancedPlayerData));
	addField("viewDistance", TypeF32, Offset(m_viewDistance, EnhancedPlayerData));
	addField("fov", TypeF32, Offset(m_fov, EnhancedPlayerData));
	addField("forgetTime", TypeS32, Offset(m_forgetTime, EnhancedPlayerData));	
}

void EnhancedPlayerData::packData(BitStream* stream)
{
	Parent::packData(stream);
}

void EnhancedPlayerData::unpackData(BitStream* stream)
{
	Parent::unpackData(stream);
	
}

////////////////////////////////////////////////////
//////***********EnhancedPlayer************/////////
////////////////////////////////////////////////////

static F32 sMinWarpTicks = 0.5f;       // Fraction of tick at which instant warp occurs
static S32 sMaxWarpTicks = 3;          // Max warp duration in ticks
static S32 sMaxPredictionTicks = 30;

const GuideBot::EnumDescription::Record EnhancedPlayer::poseEnums[] =
{
	{ EnhancedPlayer::StandPose, "stand" },
	{ EnhancedPlayer::CrouchPose, "crouch" },
	{ EnhancedPlayer::PronePose, "prone" },
	{ EnhancedPlayer::SwimPose, "swim" },
};

GuideBot::EnumDescription EnhancedPlayer::poseEnumDescription(4, poseEnums);


IMPLEMENT_CO_NETOBJECT_V1(EnhancedPlayer);

EnhancedPlayer::EnhancedPlayer():	mDataBlock(NULL)
									, m_actorVelocity(VectorF::Zero)
									, m_initState(NULL)
									, m_enablePerception(false)
{
	//m_lookDir = VectorF(0.f,1.f,0.f);
	for(size_t i=0; i<MaxTriggerKeys; i++)
	{
		m_prevTrigger[i] = false;
	}
	
}

EnhancedPlayer::~EnhancedPlayer()
{

}
void EnhancedPlayer::initPersistFields()
{
	Parent::initPersistFields();
	addField("initState",    TypeCaseString,	Offset(m_initState,      EnhancedPlayer));
	addField("enablePerception", TypeBool, Offset(m_enablePerception, EnhancedPlayer));	
}

bool EnhancedPlayer::onAdd()
{
	if (!Parent::onAdd())
		return false;


	setTm(GuideBot::toGbMatrix(getTransform()));
	m_initState = m_initState ? m_initState : "SoldierNormal";
	GuideBot::State* state = getState(m_initState);
	AssertFatal(state, "EnhancedPlayer::onAdd: can't init starting state");
	setActorState(state);
	
	if (isServerObject())
	{
		//set action idle
		GuideBot::ActionIdle* idlePrototype = static_cast<GuideBot::ActionIdle*>
			(GuideBot::ActionLibrary::instance().getActionPrototype("ActionIdle"));
		idlePrototype->setParams(mDataBlock->m_enableSlopes);
		setActionByPrototype(idlePrototype);		

		enablePerception(m_enablePerception);

		setFov(mDegToRad(mDataBlock->m_fov));
		setViewDistance(mDataBlock->m_viewDistance);
		setForgetTime((unsigned int)mDataBlock->m_forgetTime);

		for(size_t i = 0;i<EnhancedPlayerData::MaxAttackInfosNum; i++)
		{
			if (mDataBlock->m_attackInfoNames[i])
			{
				AttackInfoDataBlock* dataBlock = static_cast<AttackInfoDataBlock*>(Sim::findObject(mDataBlock->m_attackInfoNames[i]));
				if (dataBlock)
				{
					m_attackInfos.push_back(dataBlock->getAttackInfo());
				}
				else
					Con::warnf("EnhancedPlayer::onAdd: can't find attack info datablock %s",mDataBlock->m_attackInfoNames[i]);
			}
		}
	}
	
	mShapeInstance->destroyThread(mActionAnimation.thread);
	mActionAnimation.thread = NULL;

	if (isServerObject())
	{
		mShapeInstance->setDirty(TSShapeInstance::TransformDirty);
		mShapeInstance->animate();		
	}
	//m_thread = mShapeInstance->addThread();

	for (U32 i = 0; i < PlayerData::NumSpineNodes; i++)
		if (mDataBlock->spineNode[i] != -1)
			mShapeInstance->setNodeAnimationState(mDataBlock->spineNode[i],0);

	
	return true;
}

void EnhancedPlayer::onRemove()
{
	if (isServerObject())
	{
		//GB_INFO("EnhancedPlayer::onRemove: %p time: %d",this,GuideBot::Platform::getTime());
	}
	Parent::onRemove();

	destroyAllActions();
}

bool EnhancedPlayer::onNewDataBlock(GameBaseData* dptr, bool reload)
{
	mDataBlock = dynamic_cast<EnhancedPlayerData*>(dptr);
	if (!mDataBlock || !Parent::onNewDataBlock(mDataBlock, reload))
		return false;
	return true;
}

void EnhancedPlayer::moveUpdate()
{
	//update look dir from actor tm
	//if (!isControlledByPlayer())
	{
		F32 prevZRot = mRot.z;
		VectorF lookDir = GuideBot::fromGbVector(getLookDir());
		mRot.z = mAtan2(lookDir.x, lookDir.y);

		while (mRot.z < 0.0f)
			mRot.z += M_2PI_F;
		while (mRot.z > M_2PI_F)
			mRot.z -= M_2PI_F;

		if (isClientObject())
		{
			delta.rot = mRot;
			delta.rotVec.x = delta.rotVec.y = 0.0f;
			delta.rotVec.z = prevZRot - mRot.z;
			if (delta.rotVec.z > M_PI_F)
				delta.rotVec.z -= M_2PI_F;
			else if (delta.rotVec.z < -M_PI_F)
				delta.rotVec.z += M_2PI_F;
			delta.headVec = delta.head;
			delta.head = mHead;
			delta.headVec-=delta.head;
		}
	}
	

	//update position, set position and orientation
	m_actorVelocity = GuideBot::fromGbVector(getVelocityVector());
	mVelocity = m_actorVelocity;
	mVelocity.z = -9.8f;//.hack - add some gravity

	VectorF contactNormal(0,0,0);
	bool jumpSurface = false, runSurface = false;
	findContact( &runSurface, &jumpSurface, &contactNormal );
	VectorF prevPos = getPosition();
	updatePos();

	m_actualVelocity = GuideBot::toGbVector((getPosition() - prevPos)/TickSec); 
}

void EnhancedPlayer::setPosition(const Point3F& pos,const Point3F& viewRot)
{
	Parent::setPosition(pos,viewRot);
	if (isServerObject())
	{
		//set tm to guide bot actor
		MatrixF tm;
		tm.set(EulerF(0.0f, 0.0f, viewRot.z));
		tm.setColumn(3,pos);
		setTm(GuideBot::toGbMatrix(tm));
	}
}

GuideBot::Vector3 EnhancedPlayer::getEyesLookDir()
{
	GuideBot::Vector3 lookDir;
	if (mDataBlock->eyeNode!=-1)
	{
		MatrixF mat;
		ShapeBase::getEyeTransform(&mat);
		lookDir = GuideBot::toGbVector(mat.getColumn3F(1));
	}
	else
		lookDir = Actor::getEyesLookDir();
	return lookDir;
}

void EnhancedPlayer::processTick(const Move *move)
{
	//GB_INFO("EnhancedPlayer::processTick(%d)",isClientObject());
	Move aiMove;
	if (!move && isServerObject() && getAIMove(&aiMove))
		move = &aiMove;
	
	ShapeBase::processTick(move);


	if (!getControllingClient() && isControlledByPlayer())
	{
		//GB_INFO("processTick(%d): control false",isClientObject());
		setControlledByPlayer(false);

	}

	// Warp to catch up to server
	if (delta.warpTicks > 0) {
		delta.warpTicks--;

		// Set new pos.
		getTransform().getColumn(3,&delta.pos);
		delta.pos += delta.warpOffset;
		delta.rot += delta.rotOffset;
		setPosition(delta.pos,delta.rot);
		setRenderPosition(delta.pos,delta.rot);
		// Backstepping
		delta.posVec.x = -delta.warpOffset.x;
		delta.posVec.y = -delta.warpOffset.y;
		delta.posVec.z = -delta.warpOffset.z;
		delta.rotVec.x = -delta.rotOffset.x;
		delta.rotVec.y = -delta.rotOffset.y;
		delta.rotVec.z = -delta.rotOffset.z;
	}
	else {
		if (!move) {
			if (isGhost()) {
				// If we haven't run out of prediction time,
				// predict using the last known move.
				if (mPredictionCount-- <= 0)
					return;
				move = &delta.move;
			}
			else
				move = &NullMove;
		}
			
		if (!isGhost())
			updateVisuals(TickSec);

		if (getControllingClient())
		{
			if (!isDead())
			{

			if (!m_isControlledByPlayer)
			{
				//GB_INFO("processTick(%d): control true",isClientObject());
				setControlledByPlayer(true);

				if (isClientObject())
				{
					//reset action idle
					terminateAllActions();
					GuideBot::ActionIdle* idlePrototype = static_cast<GuideBot::ActionIdle*>
						(GuideBot::ActionLibrary::instance().getActionPrototype("ActionIdle"));
					idlePrototype->setParams(mDataBlock->m_enableSlopes);
					setActionByPrototype(idlePrototype);
				}

				m_controlledLookAt = static_cast<GuideBot::ActionLookAt*>(setLookDirMode(GuideBot::ActionLookAt::L_DIR,
					&GuideBot::Vector3::X,NULL,10));
			}

			//update pitch (head x)
			F32 p = move->pitch;
			if (p > M_PI_F) 
				p -= M_2PI_F;
			mHead.x = mClampF(mHead.x + p,mDataBlock->minLookAngle,
				mDataBlock->maxLookAngle);

			//update yaw
			VectorF rot = mRot;
			float y = move->yaw;
			if (y > M_PI_F)
				y -= M_2PI_F;
			rot.z += y;

			//form move vector from move x,y
			MatrixF zRot;
			zRot.set(EulerF(0.0f, 0.0f, rot.z));
			VectorF moveVec;
			zRot.getColumn(0,&moveVec);
			moveVec *= move->x;
			VectorF tv;
			zRot.getColumn(1,&tv);
			moveVec += tv * move->y;

			//set look and move dir to actor
			GB_ASSERT(m_controlledLookAt,"EnhancedPlayer::processTick: controlled actor must have special lookAt action");
			VectorF lookDir = tv;
			lookDir.z =  mTan(-mHead.x);
			m_controlledLookAt->setDirection(GuideBot::toGbVector(lookDir));
			setMoveDir(GuideBot::toGbVector(moveVec));


			//handle triggers
			if (isServerObject())
			{
				if (move->trigger[0])
				{
					if(!m_prevTrigger[0])
						setActionByName("ActionShot");
				}
				else if(m_prevTrigger[0])
				{
					terminateAction("ActionShot");
				}
				else if (move->trigger[1]  && !m_prevTrigger[1] && !isActionSetted("ActionThrowGrenade"))
				{
					setActionByName("ActionThrowGrenade");
				}
				else if (move->trigger[3] && !m_prevTrigger[3] && !isActionSetted("ActionCrouch"))
				{
					setActionByName("ActionCrouch");
				}
				else if (move->trigger[4] && !m_prevTrigger[4] && !isActionSetted("ActionStandUp"))
				{
					setActionByName("ActionStandUp");
				}

				for(size_t i=0; i<MaxTriggerKeys; i++)
				{
					m_prevTrigger[i] = move->trigger[i];
				}
			}

			}
			else if (isClientObject() && m_controlledLookAt)
			{
				terminateAllActions();
				m_controlledLookAt = false;
			}
		}

		if(isServerObject() || isControlledByPlayer())
		{
			mVelocity = VectorF(0.f,0.f,-9.8f); //default velocity
			//actor update
			update();
	
			m_wallCollision = false;
			if (!mPhysicsRep)
				updateWorkingCollisionSet();
			moveUpdate();

			if (!m_wallCollision)
				m_wallAvoidance = GuideBot::Vector3::ZERO;
		}
		else
		{
			delta.posVec = VectorF::Zero;
			delta.rotVec = VectorF::Zero;
		}
	}
}

void EnhancedPlayer::advanceTime(F32 dt)
{
	ShapeBase::advanceTime(dt);
	updateVisuals(dt);
}

void EnhancedPlayer::updateLookAnimation(F32 dT)
{
	if (isControlledByPlayer())
	{
		Point3F renderHead = delta.head + delta.headVec * dT;
		GuideBot::ActionSlope* slopeAction = static_cast<GuideBot::ActionSlope*>(
						getActiveAction("ActionSlope", GuideBot::E_SPINE));
		if (slopeAction)
		{
			GuideBot::ActionPlayVisual* slopeVisual = slopeAction->getSlopeVisual();
			if (slopeVisual && slopeVisual->getVisualIdx()!=GuideBot::INVALID_VISUAL_IDX)
			{
				VisualInfoMap::iterator it = m_visualInfoMap.find(slopeVisual->getVisualIdx());
				if (it != m_visualInfoMap.end())
				{
					TSThread* thread = it->second.thread;
					float pitch  = mTan(-renderHead.x);
					float animationPos = 0.5f*(1.f - pitch);
					mShapeInstance->setPos(thread, animationPos);
				}
			}
		}
	}
}

void EnhancedPlayer::writePacketData(GameConnection *connection, BitStream *stream)
{
	Parent::writePacketData(connection, stream);//ShapeBase
}

void EnhancedPlayer::readPacketData(GameConnection *connection, BitStream *stream)
{
	Parent::readPacketData(connection, stream);//ShapeBase
}

U32 EnhancedPlayer::packUpdate(NetConnection *con, U32 mask, BitStream *stream)
{
	U32 retMask = ShapeBase::packUpdate(con, mask, stream);
	
	if (stream->writeFlag(mask & InitialUpdateMask))
	{
		stream->writeString(m_initState);
	}

	if (stream->writeFlag(mask & ActionMask))
	{
		U16 clientAnimNum = 0;
		for(VisualInfoMap::iterator it = m_visualInfoMap.begin(),end = m_visualInfoMap.end(); it!=end; it++)
		{
			if (m_visualToStop.find(it->first)!=m_visualToStop.end())
				continue;
			if (!it->second.clientControlled  || getControllingClient()!=con)
				clientAnimNum++;
		}

		stream->write(clientAnimNum);
		for (VisualInfoMap::iterator it = m_visualInfoMap.begin(),end = m_visualInfoMap.end(); it!=end; it++)
		{
			if ((it->second.clientControlled && getControllingClient()==con) ||
					(m_visualToStop.find(it->first)!=m_visualToStop.end()))
				continue;
			//visual id
			stream->write(it->first);

			//animation
			if (it->second.animIdx!=-1)
			{
				//pos (actual server position)
				float pos = mShapeInstance->getSequencePos(it->second.thread);
				stream->write(pos);
				//time scale
				stream->write(it->second.timeScale);
			}			
		}
	}
	
	//for setting proper pose on client and for client controlled actors
	if (stream->writeFlag(mask & ActorStateMask))
	{
		StateDataBlock* stateDB = static_cast<StateDataBlock*>(getActorState()->getUserData());
		stream->write(stateDB->getId());
	}

	stream->writeFlag(isDead());

	if(stream->writeFlag(getControllingClient() == con && !(mask & InitialUpdateMask)))
	{
		stream->writeFlag(isMovingActive());
		return(retMask);
	}
		

	////
	if (stream->writeFlag(mask & MoveMask))
	{
		stream->writeFlag(mFalling);

		stream->writeInt(mState,NumStateBits);
		if (stream->writeFlag(mState == RecoverState))
			stream->writeInt(mRecoverTicks,PlayerData::RecoverDelayBits);

		Point3F pos;
		getTransform().getColumn(3,&pos);
		stream->writeCompressedPoint(pos);
		F32 len = mVelocity.len();
		if(stream->writeFlag(len > 0.02f))
		{
			Point3F outVel = mVelocity;
			outVel *= 1.0f/len;
			stream->writeNormalVector(outVel, 10);
			len *= 32.0f;  // 5 bits of fraction
			if(len > 8191)
				len = 8191;
			stream->writeInt((S32)len, 13);
		}
		stream->write(mRot.z);
		stream->write(mHead.x);
		stream->write(mHead.z);
		//stream->writeFloat(mRot.z / M_2PI_F, 7);
		//stream->writeSignedFloat(mHead.x / mDataBlock->maxLookAngle, 6);
		//stream->writeSignedFloat(mHead.z / mDataBlock->maxLookAngle, 6);
		delta.move.pack(stream);
		stream->writeFlag(!(mask & NoWarpMask));
	}

	return retMask;
}


void EnhancedPlayer::unpackUpdate(NetConnection *con, BitStream *stream)
{
	//GB_INFO("EnhancedPlayer::unpackUpdate");
	ShapeBase::unpackUpdate(con,stream);

	if (stream->readFlag())
	{
		m_initState = stream->readSTString();
	}

	if (stream->readFlag())
	{
		//update visuals from server
		VisualInfoIdxSet currentVisuals;
		U16 animNum;
		stream->read(&animNum);
		for (U16 i=0; i<animNum;i++)
		{
			VisualInfo visualInfo; 
			stream->read(&visualInfo.visualIdx);
			
			Sim::findObject( visualInfo.visualIdx, visualInfo.visual);
			GB_ASSERT(visualInfo.visual, GuideBot::format("EnhancedPlayer::unpackUpdate: can't find visual datablock: id %d", visualInfo.visualIdx));

			prepareVisualInfo(visualInfo.visual,visualInfo);

			//animation
			if (visualInfo.animIdx!=-1)
			{
				stream->read(&visualInfo.position);
				stream->read(&visualInfo.timeScale);
			}			

			startVisual(visualInfo);
			currentVisuals.insert(visualInfo.visualIdx);
		}
		// stop animations which were not updated
		for (VisualInfoMap::iterator it = m_visualInfoMap.begin(),end = m_visualInfoMap.end(); it!=end; it++)
		{
			//if (it->second.clientControlled && isControlledByPlayer())
			//	continue;
			if (currentVisuals.find(it->first)==currentVisuals.end() && !it->second.clientControlled)
			{
				//GB_INFO("unpackUpdate:: stop anim %d ",it->first);
				m_visualToStop.insert(it->first);
			}
		}
	}

	//state change
	if (stream->readFlag())
	{
		SimObjectId stateId;
		stream->read(&stateId);
		StateDataBlock* stateDB = dynamic_cast<StateDataBlock*>(Sim::findObject(stateId));
		GuideBot::State* state = stateDB ? stateDB->getState() : NULL;
		GB_ASSERT(state,GuideBot::format("EnhancedPlayer::unpackUpdate: Can't find state by id %d",stateId));
		setActorState(state);
	}

	if (stream->readFlag() && !isDead())
	{
		setDead(true);
	}

	// controlled by the client?
	if(stream->readFlag())
	{
		//moving activity
		bool moving = stream->readFlag();
		if (moving !=isMovingActive())
		{
			GuideBot::EffectorIDVector effectors;
			effectors.push_back(GuideBot::E_LEGS);
			if (moving)
				resetEffectors(effectors);
			else
				blockEffectors(effectors);
		}

		return;
	}


	/////
	if (stream->readFlag()) {
		mPredictionCount = sMaxPredictionTicks;
		mFalling = stream->readFlag();

		ActionState actionState = (ActionState)stream->readInt(NumStateBits);
		if (stream->readFlag()) {
			mRecoverTicks = stream->readInt(PlayerData::RecoverDelayBits);
			setState(actionState, mRecoverTicks);
		}
		else
			setState(actionState);

		Point3F pos,rot;
		stream->readCompressedPoint(&pos);
		//F32 speed = mVelocity.len();
		if(stream->readFlag())
		{
			stream->readNormalVector(&mVelocity, 10);
			mVelocity *= stream->readInt(13) / 32.0f;
		}
		else
		{
			mVelocity.set(0.0f, 0.0f, 0.0f);
		}
		F32 speed = mVelocity.len();
		rot.y = rot.x = 0.0f;
		stream->read(&rot.z);
		stream->read(&mHead.x);
		stream->read(&mHead.z);
		//rot.z = stream->readFloat(7) * M_2PI_F;
		/*if(rot.z < - M_PI_F)
			rot.z += M_2PI_F;
		else if(rot.z > M_PI_F)
			rot.z -= M_2PI_F;*/
		//mHead.x = stream->readSignedFloat(6) * mDataBlock->maxLookAngle;
		//mHead.z = stream->readSignedFloat(6) * mDataBlock->maxLookAngle;
		delta.move.unpack(stream);

		delta.head = mHead;
		delta.headVec.set(0.0f, 0.0f, 0.0f);

		if (stream->readFlag() && isProperlyAdded())
		{
			// Determine number of ticks to warp based on the average
			// of the client and server velocities.
			delta.warpOffset = pos - delta.pos;
			F32 as = (speed + mVelocity.len()) * 0.5f * TickSec;
			F32 dt = (as > 0.00001f) ? delta.warpOffset.len() / as: sMaxWarpTicks;
			delta.warpTicks = (S32)((dt > sMinWarpTicks) ? getMax(mFloor(dt + 0.5f), 1.0f) : 0.0f);

			if (delta.warpTicks)
			{
				// Setup the warp to start on the next tick.
				if (delta.warpTicks > sMaxWarpTicks)
					delta.warpTicks = sMaxWarpTicks;
				delta.warpOffset /= (F32)delta.warpTicks;

				delta.rotOffset = rot - delta.rot;
				while (delta.rotOffset.z > M_PI_F || delta.rotOffset.z < -M_PI_F)
				if(delta.rotOffset.z < - M_PI_F)
					delta.rotOffset.z += M_2PI_F;
				else if(delta.rotOffset.z > M_PI_F)
					delta.rotOffset.z -= M_2PI_F;
				delta.rotOffset /= (F32)delta.warpTicks;
			}
			else
			{
				// Going to skip the warp, server and client are real close.
				// Adjust the frame interpolation to move smoothly to the
				// new position within the current tick.
				Point3F cp = delta.pos + delta.posVec * delta.dt;
				if (delta.dt == 0) 
				{
					delta.posVec.set(0.0f, 0.0f, 0.0f);
					delta.rotVec.set(0.0f, 0.0f, 0.0f);
				}
				else
				{
					F32 dti = 1.0f / delta.dt;
					delta.posVec = (cp - pos) * dti;
					delta.rotVec.z = mRot.z - rot.z;
					
					while (delta.rotVec.z > M_PI_F || delta.rotVec.z < -M_PI_F)
					if(delta.rotVec.z > M_PI_F)
						delta.rotVec.z -= M_2PI_F;
					else if(delta.rotVec.z < -M_PI_F)
						delta.rotVec.z += M_2PI_F;

					delta.rotVec.z *= dti;
				}
				delta.pos = pos;
				delta.rot = rot;
				setPosition(pos,rot);
			}
		}
		else 
		{
			// Set the player to the server position
			delta.pos = pos;
			delta.rot = rot;
			delta.posVec.set(0.0f, 0.0f, 0.0f);
			delta.rotVec.set(0.0f, 0.0f, 0.0f);
			delta.warpTicks = 0;
			delta.dt = 0.0f;
			setPosition(pos,rot);
		}
	}
}

/*
const MatrixF& EnhancedPlayer::getTm() const
{
	return getTransform();
}*/


void EnhancedPlayer::getAABB(GuideBot::Vector3& _min, GuideBot::Vector3& _max) const
{
	const Box3F worldBox = getWorldBox();
	_min = GuideBot::toGbVector(worldBox.minExtents);
	_max = GuideBot::toGbVector(worldBox.maxExtents);
}

void EnhancedPlayer::prepareVisualInfo(VisualDataBlock* visualData, VisualInfo& info)
{
	//animation
	const char* animName = visualData->getAnimation(); 
	if (animName && animName[0])
	{
		info.animIdx = (S32)getShape()->findSequence(animName);
	}
}

GuideBot::VisualIdx EnhancedPlayer::startVisual(GuideBot::Visual* visual)
{
	VisualDataBlock* visualData = static_cast<VisualDataBlock*>(visual);

	VisualInfo info;
	info.visual = visualData;
	info.visualIdx = visualData->getId();
	prepareVisualInfo(visualData, info);
	
	if (info.animIdx==-1 && !visualData->getSound())
	{
		return GuideBot::INVALID_VISUAL_IDX;
	}

	startVisual(info);
	return info.visualIdx;
}

static F32 sAnimationTransitionTime = 0.25f;
static bool usingTransition = true;//false;

void EnhancedPlayer::startVisual(VisualInfo& visualInfo)
{
	//GB_INFO("startVisual(%d): anim %d isCl: %d",isClientObject(),animInfo.idx,animInfo.clientControlled);
	
	VisualInfoMap::iterator it = m_visualInfoMap.find(visualInfo.visualIdx);
	if (it!=m_visualInfoMap.end())
	{
		//such visual was just stopped or it's client update, so set params for it 
		VisualInfoIdxSet::iterator setIt;
		if ((setIt=m_visualToStop.find(visualInfo.visualIdx))!=m_visualToStop.end() || (isClientObject()))
		{
			bool updating = true;
			if (setIt!=m_visualToStop.end())
			{
				updating = false;
				m_visualToStop.erase(setIt);
			}

			visualInfo.thread = it->second.thread;
			visualInfo.soundSource = it->second.soundSource;
			it->second = visualInfo;

			//animation
			if (visualInfo.animIdx!=-1)
			{
				mShapeInstance->setTimeScale(visualInfo.thread,visualInfo.timeScale);
				float pos = mShapeInstance->getPos(visualInfo.thread);
				if (!(isClientObject() && visualInfo.timeScale!=0.f && 
							(mShapeInstance->isInTransition(visualInfo.thread) || mFabs(pos - visualInfo.position)<0.2f)))
					mShapeInstance->setPos(visualInfo.thread,visualInfo.position);			

				if (!isClientObject())
					it->second.lastUpdateTime = visualInfo.position;
			}

			//sound
			if (visualInfo.soundSource && !updating && !visualInfo.soundSource->isPlaying())
			{
				visualInfo.soundSource->play();
			}
		}
	}
	else
	{
		//start new visual

		//animation
		if (visualInfo.animIdx!=-1)
		{
			//create new thread and set sequence
			GB_ASSERT(visualInfo.animIdx>=0 && visualInfo.animIdx<mShapeInstance->getShape()->sequences.size(),\
				"EnhancedPlayer::playAnim: Wrong animation idx");

			visualInfo.thread = mShapeInstance->addThread();
			bool isBlend = mShapeInstance->getShape()->sequences[visualInfo.animIdx].isBlend();
			mShapeInstance->setBlendEnabled(visualInfo.thread,false);
			if (usingTransition && !isBlend)
			{
				mShapeInstance->transitionToSequence(visualInfo.thread,visualInfo.animIdx,
					visualInfo.position, sAnimationTransitionTime, true);
			}
			else
				mShapeInstance->setSequence(visualInfo.thread,visualInfo.animIdx,visualInfo.position);
			mShapeInstance->setBlendEnabled(visualInfo.thread,true);

			if (visualInfo.timeScale!=1.f)
				mShapeInstance->setTimeScale(visualInfo.thread, visualInfo.timeScale);
		}
		

		//sound
		if (visualInfo.visual->getSound())
		{
			visualInfo.soundSource = SFX->createSource(visualInfo.visual->getSound(), &getTransform());
			visualInfo.soundSource->play();
		}

		m_visualInfoMap.insert(std::make_pair(visualInfo.visualIdx, visualInfo));
	}

	setMaskBits(ActionMask);
};

void EnhancedPlayer::stopVisual(GuideBot::VisualIdx visualIdx)
{
	//GB_INFO("stopAnim(%d): anim %d",isClientObject(),animIdx);
	//GB_INFO("EnhancedPlayer::stopAnim(%d) %s - %d",isClientObject(),getShape()->getName(getShape()->sequences[animIdx].nameIndex).c_str(),animIdx);
	
	if (isClientObject())
	{
		VisualInfoMap::iterator it = m_visualInfoMap.find(visualIdx);
		if (it!=m_visualInfoMap.end())
		{
			VisualInfo& visualInfo = it->second;
			if (!visualInfo.clientControlled) //stop on client only client controlled animations
				return;
		}
		
	}		
	m_visualToStop.insert(visualIdx);

	setMaskBits(ActionMask);
}

void EnhancedPlayer::updateVisuals(F32 dt)
{
	//stop animations
	VisualInfoIdxSet::iterator its = m_visualToStop.begin(),ends = m_visualToStop.end();
	while (its!=ends)
	{
		VisualInfoMap::iterator it = m_visualInfoMap.find(*its);
		if (it!=m_visualInfoMap.end())
		{
			if (it->second.thread)
				mShapeInstance->destroyThread(it->second.thread);
	
			if (it->second.soundSource)
			{
				it->second.soundSource->stop();
			}

			m_visualInfoMap.erase(it);
		}
		its++;
	}
	m_visualToStop.clear();
	
	bool needAnimateServer = true;	//animate server shape instance before any callback executing
	/*
	if (m_visualInfoMap.empty())
	{
		GB_ERROR("EnhancedPlayer::updateAnim(%d): ERROR! Actor doesn't have animations!",isClientObject());
	}*/

	//update animations
	VisualInfoMap::iterator it = m_visualInfoMap.begin();
	VisualInfoMap::iterator end = m_visualInfoMap.end();

	while(it!=end)
	{
		if (it->second.thread)
			mShapeInstance->advanceTime(dt,it->second.thread);
		it++;
	}

	if (isServerObject())
	{
		//mShapeInstance->animate();

		it = m_visualInfoMap.begin();
		while(it!=end)
		{
			//check animation callback
			if (it->second.thread)
			{
				float pos =	mShapeInstance->getSequencePos(it->second.thread);//getPos
				CallbackMap& callbackMap = it->second.callbackMap;
				for (CallbackMap::iterator jt = callbackMap.begin(), jend = callbackMap.end();
					jt!=jend; jt++)
				{
					if (jt->first > it->second.lastUpdateTime && jt->first<=pos)
					{
						if (needAnimateServer)
						{
							mShapeInstance->animate();
							needAnimateServer = false;
						}

						jt->second(this);
					}
				}
				it->second.lastUpdateTime = pos;
			}
			
			//check visual end callback(fired on animation end or sound end, if visual doesn't have animation)
			if (it->second.callback && 
					((it->second.thread && mShapeInstance->getPos(it->second.thread) == 1.f &&  it->second.timeScale!=0.f) ||
					(it->second.animIdx==-1 && it->second.soundSource && it->second.soundSource->isStopped())))
			{
				if (needAnimateServer && it->second.thread)
				{
					mShapeInstance->animate();
					needAnimateServer = false;
				}

				VisualEndCallback* callback = it->second.callback;
				(*callback)(1.f);		
			}

			it++;	
		}
	}
}

bool EnhancedPlayer::isCyclicVisual(GuideBot::Visual* visual)
{
	VisualDataBlock* visualData = static_cast<VisualDataBlock*>(visual);
	bool isCyclic = false;

	const char* anim_name = visualData->getAnimation();
	if (anim_name && anim_name[0])
	{
		const TSShape* shape = getShape();
		S32 seqIdx = shape->findSequence(anim_name); 
		if (seqIdx < 0) 
		{ 
			Con::errorf("Could not find sequence named '%s'", anim_name); 
			return false; 
		} 
		const TSShape::Sequence* var = &shape->sequences[seqIdx];
		
		isCyclic = var->isCyclic();
	}
	else if (visualData->getSound())
	{
		isCyclic = visualData->getSound()->getDescription()->mIsLooping;
	}

	return isCyclic;
}

void EnhancedPlayer::setVisualEndCallback(VisualIdx viusalIdx,VisualEndCallback* callback)
{
	VisualInfoMap::iterator it = m_visualInfoMap.find(viusalIdx);
	if (it!=m_visualInfoMap.end())
	{
		it->second.callback = callback;
	}
}

void EnhancedPlayer::setAnimCallbacks(VisualIdx visualIdx,CallbackMap& callbacks)
{
	VisualInfoMap::iterator it = m_visualInfoMap.find(visualIdx);
	if (it!=m_visualInfoMap.end())
	{
		it->second.callbackMap = callbacks;
	}
}

void EnhancedPlayer::setAnimationTimeScale(VisualIdx visualIdx, float scale)
{
	VisualInfoMap::iterator it = m_visualInfoMap.find(visualIdx);
	if (it!=m_visualInfoMap.end())
	{
		it->second.timeScale = scale;
		mShapeInstance->setTimeScale(it->second.thread, scale);
		setMaskBits(ActionMask);
	}
}

void EnhancedPlayer::setClientControlled(int animIdx)
{
	VisualInfoMap::iterator it = m_visualInfoMap.find(animIdx);
	if (it!=m_visualInfoMap.end())
	{
		it->second.clientControlled = true;
	}
}

void EnhancedPlayer::setAnimationPosition(int animIdx, float pos)
{
	VisualInfoMap::iterator it = m_visualInfoMap.find(animIdx);
	if (it!=m_visualInfoMap.end())
	{
		it->second.position = pos;
		mShapeInstance->setPos(it->second.thread, pos);
		setMaskBits(ActionMask);
	}
}

GuideBot::Visual* EnhancedPlayer::getVisual(const std::string& visualName)
{
	VisualDataBlock* visualDB = dynamic_cast<VisualDataBlock*>( Sim::findObject( visualName.c_str() ) );
	return static_cast<GuideBot::Visual*>(visualDB);
}

GuideBot::State* EnhancedPlayer::getState (const std::string& stateName)
{
	StateDataBlock* stateDB = dynamic_cast<StateDataBlock*>( Sim::findObject( stateName.c_str() ) );

	GuideBot::State* state = stateDB ? stateDB->getState() : NULL;

	return state;
}

void EnhancedPlayer::setActorState(GuideBot::State* state)
{
	if (m_state != state)
	{
		setMaskBits(ActorStateMask);

		//handle player pose
		StateDataBlock* visualData = static_cast<StateDataBlock*>(state->getUserData());
		const char* poseStr = visualData->getPose();
		Pose pose = StandPose;
		if  (poseStr)
			poseEnumDescription.getValueByName(poseStr,pose);
		setPose(pose);
	}
	GuideBot::Actor::setActorState(state);
}

void	EnhancedPlayer::call(const std::string& callbackName)
{
	const char* argv[2];
	argv[0] = callbackName.c_str();
	argv[1] = getIdString();
	Con::execute(2, argv);
}

GuideBot::Action* EnhancedPlayer::takeWeapon(const std::string& weapon, const std::string& state,
											 GuideBot::Action* parent, bool onlyPrototype)
{
	const char* argv[3];
	argv[0] = "setActionTakeWeapon";
	argv[1] = weapon.c_str();
	argv[2] = state.c_str();
	const char* buf = Con::execute(3, argv);
	ScriptAction* prototype = NULL;
	if (buf)
		prototype = dynamic_cast<ScriptAction*>(Sim::findObject(buf));
	if (!prototype)
		return NULL;
	
	if (onlyPrototype)
		return prototype;

	return setActionByPrototype(prototype,parent);
}


void EnhancedPlayer::updateVision()
{
	VectorF pos = getPosition();

    SceneContainer* container;
      container = &gClientContainer;

	if (container)
	{
		SimpleQueryList queryList;
		container->findObjects( PlayerObjectType | ProjectileObjectType, SimpleQueryList::insertionCallback, &queryList);
		for (size_t i=0; i < queryList.mList.size(); i++)
		{
			SceneObject* sceneObject = queryList.mList[i];
			WorldObject* object = sceneObject->getWorldObject();
			if (object && object != this)
				addVisualFrameToMemory(object);
		}
	}
}

bool EnhancedPlayer::checkObjectLos(GuideBot::WorldObject* object)
{
	MatrixF eyeMat;
	getEyeTransform(&eyeMat);
	VectorF eyePos = eyeMat.getPosition();
	GuideBot::Vector3 aabbMin, aabbMax;
	object->getAABB(aabbMin, aabbMax);
	Point3F targetLoc = GuideBot::fromGbVector(0.5f*(aabbMin + aabbMax));
	targetLoc.z = GuideBot::lerp(aabbMin.z,aabbMax.z,0.8f);

	RayInfo dummy;
	bool res = !getContainer()->castRay( eyePos, targetLoc,
		InteriorObjectType | StaticShapeObjectType | StaticObjectType |	TerrainObjectType, &dummy);
	return res;
}

void EnhancedPlayer::createWorldObject()
{
	//there is no need to create representation as EnhancedPlayer is inherited from WorldObject
	m_worldObject = this;
	m_worldObject->registerObject();
}

void EnhancedPlayer::destroyWorldObject()
{
	m_worldObject = NULL;
}
static float lerpFactor = 0.01f;
void EnhancedPlayer::wallAvoindance(const VectorF& wallNormal)
{
	if(!m_isControlledByPlayer &&  mFabs(wallNormal.z) < 0.4f)
	{
		mVelocity.z = 0.f;
		float vel = mVelocity.magnitudeSafe();
		VectorF dir = mVelocity;
		VectorF normal = wallNormal;
		normal.z = 0.f;
		normal.normalizeSafe();
		dir.z = 0.f;
		dir.normalizeSafe();
		float dotValue = mDot(dir, normal);
		if (dotValue < 0.f)
		{
			if (mFabs(dotValue+1.f) < POINT_EPSILON)
			{
				//rotate to small angle
				MatrixF rotMat;
				rotMat.set(EulerF(0.f,0.f,Float_Pi/10.f));
				rotMat.mulP(dir);
			}

			VectorF vec;
			mCross(dir, normal, &vec);
			mCross(vec, normal, &vec);
			vec.z = 0.f;
			vec.normalizeSafe();
			dir = vec;
			dir.z = 0;
			dir.normalizeSafe();
			dir = -dir;

			m_wallCollision = true;
			VectorF wallAvoidance = GuideBot::fromGbVector(m_wallAvoidance);
			//interpolation avoidance direction
			if (wallAvoidance.isZero())
				wallAvoidance = dir;
			else
				wallAvoidance = wallAvoidance + lerpFactor*(dir - wallAvoidance);
			dir = wallAvoidance;
			dir.normalizeSafe();

			//update moveDir
			m_wallAvoidance = GuideBot::toGbVector(wallAvoidance);
			m_lastMoveDir = GuideBot::toGbVector(dir);

			dir *= vel;
			mVelocity = dir;
			mVelocity.z = -9.8f;
			return;
		}
	}
}

float EnhancedPlayer::getTeamRelationship(WorldObject* object)
{
	//call script function to calculate team relationship factor
	SceneObject* sceneObject = static_cast<SceneObject*>(object->getUserData());
	GB_ASSERT(sceneObject,"EnhancedPlayer::evaluator: world object cannot be casted to scene object");
	char playerID[64], objectID[64];
	dSprintf(playerID, sizeof(playerID), "%d",getId());
	dSprintf(objectID, sizeof(objectID), "%d", sceneObject->getId());
	const char* argv[3];
	argv[0] = "getTeamRelationship";
	argv[1] = playerID;
	argv[2] = objectID;
	const char* buf = Con::execute(3, argv);
	return buf ? dAtof(buf) : 1.f;
}

void EnhancedPlayer::getMuzzleVector(U32 imageSlot,VectorF* vec)
{
	MountedImage& image = mMountedImageList[imageSlot];
	//for first person construct muzzle vector by eye transform
	if (image.dataBlock->correctMuzzleVector)
	{
		if (GameConnection * gc = getControllingClient())
		{
			if (gc->isFirstPerson() && !gc->isAIControlled())
			{
				MatrixF mat;
				getMuzzleTransform(imageSlot,&mat);
				if (getCorrectedAim(mat, vec))
					return;
			}
		}
	}

	//otherwise, by pitch and lookdir
	VectorF lookDir = GuideBot::fromGbVector(getLookDir());
	lookDir.z = getPitch();
	lookDir.normalize();
	*vec = lookDir;
}
///////////////////////////////////////////////////////////////////
class ScriptEnemyEvaluatorCaller
{
public:
	static ScriptEnemyEvaluatorCaller* getInstance() 
	{
		static ScriptEnemyEvaluatorCaller self;
		return &self;
	}
	void init(const char* functionName)
	{
		m_functionName = functionName; 
	};
	float evaluator(GuideBot::Actor* actor, GuideBot::WorldObject* enemy)
	{
		GB_ASSERT(!m_functionName.empty(),"ScriptEnemyEvaluatorCaller::evaluator: function name must be defined");
		EnhancedPlayer* player = static_cast<EnhancedPlayer*>(actor);
		SceneObject*	object = static_cast<SceneObject*>(enemy->getUserData());
		GB_ASSERT(object,"EnhancedPlayer::evaluator: world object cannot be casted to scene object");
		const GuideBot::Actor::MemoryFrame* enemyFrame = actor->getMemoryFrame(enemy);
		GB_ASSERT(enemyFrame,"EnhancedPlayer::evaluator: can't find memory frame for enemy");
		
		char playerID[64];
		dSprintf(playerID, sizeof(playerID), "%d",player->getId());
		char objectID[64];
		dSprintf(objectID, sizeof(objectID), "%d", object->getId());

		const char* argv[5];
		argv[0] = m_functionName.c_str();
		argv[1] = playerID;
		argv[2] = objectID;
		argv[3] = Con::getFloatArg(enemyFrame->distSq);
		argv[4] = Con::getFloatArg(enemyFrame->fov);

		const char* buf = Con::execute(5, argv);
		return buf ? dAtof(buf) : 0.f;
	}
protected:
	ScriptEnemyEvaluatorCaller() {};
	std::string m_functionName;
};

ConsoleFunction( setEnemyEvaluator, void, 1, 2, "()")
{
	GuideBot::Actor::EnemyEvaluator evaluator; 

	const char* customEnemyEvaluator = argc==2 ? argv[1] : NULL;
	if (customEnemyEvaluator)
	{
		ScriptEnemyEvaluatorCaller* caller =  ScriptEnemyEvaluatorCaller::getInstance();
		caller->init(customEnemyEvaluator);
		evaluator.bind(caller,&ScriptEnemyEvaluatorCaller::evaluator);
	}
	GuideBot::Actor::setEnemyEvaluator(evaluator);

}

ConsoleMethod( EnhancedPlayer, setAction, void, 3, 4, "(actionName [,parentAction])")
{
	if (!GuideBot::ActionLibrary::instance().checkActionPrototype(argv[2]))
	{
		Con::errorf("EnhancedPlayer::setAction: can't find action %s", argv[2] ? argv[2] : "");
		return;
	}
	
	ScriptAction* parentAction = NULL;
	if (argc == 4)
	{
		parentAction = dynamic_cast<ScriptAction*>(Sim::findObject(argv[3]));
		if (!parentAction)
		{
			Con::errorf("ScriptAction::addConsecutiveAction: can't register action %s",argv[3]);
			return;
		}
	}
	object->setActionByName(argv[2],parentAction);
}

ConsoleMethod( EnhancedPlayer, setActionByPrototype, void, 3, 5, "(action [,parentAction, direclty])")
{
	ScriptAction * action = dynamic_cast<ScriptAction*>(Sim::findObject(argv[2]));
	if (!action)
	{
		Con::errorf("EnhancedPlayer::setActionByPrototype: can't find prototype %s",argv[2]);
		return;
	}
	
	ScriptAction* parentAction = NULL;
	if (argc >= 4 && dStrcmp(argv[3],""))
	{
		parentAction = dynamic_cast<ScriptAction*>(Sim::findObject(argv[3]));
		if (!parentAction)
		{
			Con::errorf("ScriptAction::addConsecutiveAction: can't register action %s",argv[3]);
			return;
		}
	}

	bool direclty = false;
	if (argc >= 5)
		direclty = dAtob(argv[4]);
	if (direclty)
		object->setActionDirectly(action, parentAction);
	else
		object->setActionByPrototype(action, parentAction);
}

ConsoleMethod( EnhancedPlayer, terminateAction, void, 3, 3, "(actionName)")
{
	object->terminateAction(argv[2]);
}

ConsoleMethod( EnhancedPlayer, resetEffectors, void, 3, 3, "(effectorsName)")
{
	GuideBot::EffectorIDVector effectors;
	if (!argv[2] || !GuideBot::Effector::convertFromString(argv[2],effectors))
	{
		Con::errorf("EnhancedPlayer::resetEffector: can't reset effectors %s",argv[2] ? argv[2] : "");
	}
	object->resetEffectors(effectors);
}
ConsoleMethod( EnhancedPlayer, playVisual, void, 3, 4, "(visualName,int effectorId)")
{
	std::string visualName(argv[2]);
	ScriptAction* parentAction = 0; 
	if (argc==4)
	{
		parentAction = dynamic_cast<ScriptAction*>(Sim::findObject(argv[3]));
	}
	object->playVisual(visualName,parentAction);
}
ConsoleMethod( EnhancedPlayer, changeState, void, 3, 4, "(stateName, [parentAction])")
{
	std::string stateName = argv[2];

	ScriptAction* parentAction = NULL;
	if (argc == 4)
	{
		parentAction = dynamic_cast<ScriptAction*>(Sim::findObject(argv[3]));
		if (!parentAction)
		{
			Con::errorf("ScriptAction::changeState: find action %s",argv[3]);
			return;
		}
	}
	object->changeState(stateName, parentAction);
}

ConsoleMethod( EnhancedPlayer, setScriptAction, const char*, 3, 3, "()")
{
	ScriptAction* prototype = dynamic_cast<ScriptAction*>(Sim::findObject(argv[2]));
	ScriptAction* action  = static_cast<ScriptAction*>(object->setActionByPrototype(prototype));
	return action ? action->getIdString() : NULL;
}

ConsoleMethod( EnhancedPlayer, setLookDirMode, void, 3, 6, "()")
{
	GuideBot::ActionLookAt::LookDirMode mode = GuideBot::ActionLookAt::L_FORWARD;
	GuideBot::Vector3 point;
	GuideBot::WorldObject* worldObject = NULL;
	int priority = 1;
	if (!dStrcmp(argv[2],"forward"))
	{
		mode = GuideBot::ActionLookAt::L_FORWARD;
	}
	else if (!dStrcmp(argv[2],"point"))
	{
		mode = GuideBot::ActionLookAt::L_POINT;
		dSscanf( argv[3], "%g %g %g", &point.x, &point.y, &point.z);
	}
	else if (!dStrcmp(argv[2],"object"))
	{
		mode = GuideBot::ActionLookAt::L_OBJECT;
		SceneObject* object = (SceneObject*) Sim::findObject(argv[3]);
		
		if (object->getTypeMask()/* & DefaultObjectType*/)
		{
			SceneObject* player = static_cast<SceneObject*>(object);
			worldObject = player->getWorldObject();
		}

		if (!worldObject)
		{
			Con::errorf("EnhancedPlayer::setLookDirMode: object doesn't have needed representation: %s",argv[3]);
			return;
		}
	}
	else
	{
		Con::errorf("EnhancedPlayer::setLookDirMode: unknown look dir mode: %s",argv[2]);
		return;
	}

	if ((argc==4 && mode==GuideBot::ActionLookAt::L_FORWARD)|| argc==5)
	{
		priority = mode==GuideBot::ActionLookAt::L_FORWARD ? dAtoi(argv[3]) : dAtoi(argv[4]);
	}

	ScriptAction* scriptParent = NULL;
	if ((argc==5 && mode==GuideBot::ActionLookAt::L_FORWARD)|| argc==6)
	{
		scriptParent = static_cast<ScriptAction*>(Sim::findObject(argv[5]));
	}

	object->setLookDirMode(mode, &point, worldObject, priority, scriptParent);
}

ConsoleMethod( EnhancedPlayer, moveToPoint, void, 3, 3, "(position)")
{
	GuideBot::Vector3 v( 0.0f, 0.0f, 0.0f );
	dSscanf( argv[2], "%g %g %g", &v.x, &v.y, &v.z );
	object->moveToPoint(v);
}
ConsoleMethod( EnhancedPlayer, moveToObject, void, 3, 4, "(object [,parentAction])")
{
	SimObject* simObj = Sim::findObject(argv[2]);
	if (!simObj)
	{
		Con::errorf("EnhancedPlayer::moveToObject: can't find object: %s",argv[2]);
		return;
	}

	GuideBot::WorldObject* worldObject = NULL;
	if (object->getTypeMask()/* & DefaultObjectType*/)
	{
		SceneObject* sceneObject = static_cast<SceneObject*>(simObj);
		worldObject = sceneObject->getWorldObject();
	}

	if (!worldObject)
	{
		Con::errorf("EnhancedPlayer::moveToObject: object doesn't have needed representation: %s",argv[2]);
		return;
	}

	ScriptAction* parentAction = NULL;
	if (argc == 4)
	{
		parentAction = static_cast<ScriptAction*>(Sim::findObject(argv[3]));
		if (!parentAction)
		{
			Con::errorf("ScriptAction::addConsecutiveAction: can't register action %s",argv[3]);
			return;
		}
	}	

	static const float defaultPrecision = 1.5f;

	object->moveToObject(worldObject, defaultPrecision, parentAction);
}

ConsoleMethod( EnhancedPlayer, enablePerception, void, 3, 3, "()")
{
	object->enablePerception(dAtob(argv[2]));
}

ConsoleMethod( EnhancedPlayer, getEnemy, S32, 2, 2, "()")
{
	GuideBot::WorldObject* enemy = object->getEnemy();
	SceneObject*	sceneObject = enemy ? static_cast<SceneObject*>(enemy->getUserData()) : NULL;
	return sceneObject ? sceneObject->getId() : 0;
}

ConsoleMethod( EnhancedPlayer, getActiveAttackInfo, S32, 2, 2, "()")
{
	S32 id = 0;
	GuideBot::AttackInfo* attackInfo = object->getActiveAttackInfo();
	if (attackInfo)
	{
		AttackInfoDataBlock*	attackInfoDataBlock = static_cast<AttackInfoDataBlock*>(attackInfo->getUserData());
		GB_ASSERT(attackInfoDataBlock,"Attack info must have data block pointer as user data");
		id = attackInfoDataBlock ? attackInfoDataBlock->getId() : 0;
	}
	return id;
}

ConsoleMethod( EnhancedPlayer, getActorState, S32, 2, 2, "()")
{
	S32 id = 0;
	GuideBot::State* state = object->getActorState();
	if (state)
	{
		StateDataBlock* stateDataBlock = static_cast<StateDataBlock*>(state->getUserData());
		GB_ASSERT(stateDataBlock,"State must have data block pointer as user data");
		id = stateDataBlock ? stateDataBlock->getId() : 0;
	}
	return id;
}

ConsoleMethod( EnhancedPlayer, hasVisual, bool, 3, 3, "(visualName)")
{
	std::string visualName = argv[2];
	bool res = object->hasVisual(visualName);
	return res;
}

ConsoleMethod( EnhancedPlayer, setDead, void, 2, 3, "(bool withoutAction)")
{
	bool withoutAction = argc==3 ? dAtob(argv[2]) : false;
	object->setDead(withoutAction);
}

ConsoleMethod( EnhancedPlayer, setPitch, void, 3, 3, "(float pitch)")
{
	float pitch = dAtof(argv[2]);
	object->setPitch(pitch);
}

bool createWaypointsFromPath(SimPath::Path* path, GuideBot::Waypoints& waypoints)
{
	for (SimPath::Path::iterator itr = path->begin(); itr != path->end(); itr++)
	{
		Marker* pMarker = static_cast<Marker*>(*itr);
		if (pMarker != NULL)
		{
			static const char* waypointIdField = StringTable->insert( "waypointId" );
			const char* waypointIdStr = pMarker->getDataField( waypointIdField, NULL );
			if (waypointIdStr)
			{
				GuideBot::WorldObject::ObjectId waypointId = dAtoi(waypointIdStr);
				GuideBot::Waypoint* waypoint = dynamic_cast<GuideBot::Waypoint*>(
													GuideBot::WorldObject::findById(waypointId));
				if (waypoint)
				{
					waypoints.push_back(waypoint);
				}
				else
				{
					Con::errorf("createWaypointsFromPath: can't find waypoint by id: %d", waypointId);
				}
			}
		}
	}

	if (waypoints.size()==0)
	{
		Con::errorf("EnhancedPlayer::moveByWaypoints: can't create any waypoints from path: %p", path);
		return false;
	}
	return true;
}

ConsoleMethod( EnhancedPlayer, moveByWaypoints, void, 4, 6, "(path, mode [, precision])")
{
	SimPath::Path* path = dynamic_cast<SimPath::Path*>(Sim::findObject(argv[2]));
	GuideBot::Waypoints waypoints;
	if (!path || !createWaypointsFromPath(path , waypoints))
	{
		Con::errorf("EnhancedPlayer::moveByWaypoints: can't create waypoints from path: %s",argv[2]);
		return;
	}

	GuideBot::ActionMoveByWaypoints::FollowMode mode = GuideBot::ActionMoveByWaypoints::FM_DIRECT;
	String modeStr = argv[3];
	if (modeStr.equal("direct"))
		mode = GuideBot::ActionMoveByWaypoints::FM_DIRECT;
	else if (modeStr.equal("circular"))
		mode = GuideBot::ActionMoveByWaypoints::FM_CIRCULAR;
	else if (modeStr.equal("random"))
		mode = GuideBot::ActionMoveByWaypoints::FM_RANDOM;
	else
	{
		Con::errorf("EnhancedPlayer::moveByWaypoints: unknown follow mode: %s",argv[3]);
		return;
	}

	float precision = 0.5f;
	if (argc==5)
	{
		precision = dAtof(argv[4]);
	}

	ScriptAction* parentAction = NULL;
	if (argc == 6)
	{
		parentAction = static_cast<ScriptAction*>(Sim::findObject(argv[5]));
		if (!parentAction)
		{
			Con::errorf("ScriptAction::addConsecutiveAction: can't register action %s",argv[5]);
			return;
		}
	}	

	object->moveByWaypoints(waypoints, mode, precision, parentAction);
}

ConsoleMethod( EnhancedPlayer, isControlledByPlayer, bool, 2, 2, "")
{
	return object->isControlledByPlayer();
}

ConsoleMethod( EnhancedPlayer, pA, void, 2, 3, "()")
{
	std::ostringstream buff;
	object->printActions(buff,argc==3 ? dAtob(argv[2]) : false);
	Con::printf("%s",buff.str().c_str());
}

ConsoleMethod( EnhancedPlayer, setmove, void, 2, 2, "()")
{
	object->setActionByName("ActionMove");
}
ConsoleMethod( EnhancedPlayer, resetmtp, void, 2, 2, "()")
{
	object->terminateAction("ActionMoveToPoint");
}
ConsoleFunction( useTransition, void, 2, 2, "(bool)")
{
	usingTransition  = dAtob(argv[1]);
}

ConsoleMethod( EnhancedPlayer, getAlertness, F32, 2, 2, "()")
{
	return object->getAlertness();
}


ConsoleFunction( addPerceptionEvent, int, 4,  10, "(pos, eventType, sensorType, alertness, [initiator, radiusMin, radiusMax, volume, lifeTime])")
{
	GuideBot::Vector3 pos( 0.0f, 0.0f, 0.0f );
	dSscanf( argv[1], "%g %g %g", &pos.x, &pos.y, &pos.z );

	int eventType = dAtoi(argv[2]);
	int sensorType = dAtoi(argv[3]);
	float alertness = dAtof(argv[4]);

	float radiusMin = 10.f;
	float radiusMax = 10.f;
	float volume = 1.f;
	float lifeTime = -1.f;
	GuideBot::WorldObject* initiator = NULL;
	if (argc >= 6 && dStrcmp(argv[5],""))
	{
		SceneObject* object = static_cast<SceneObject*>(Sim::findObject(argv[5]));
		if (object)
			initiator = object->getWorldObject();		
	}
	if (argc >= 7)
		radiusMin = radiusMax = dAtof(argv[6]);
	if (argc >= 8)
		radiusMax  = dAtof(argv[7]);
	if (argc >= 9)
		volume = dAtof(argv[8]);
	if (argc == 10)
		lifeTime = dAtof(argv[9]);
		
	GuideBot::PerceptionEvent* event = new GuideBot::PerceptionEvent();
	event->m_sensorType = (GuideBot::SensorType)sensorType;
	event->m_eventType = (GuideBot::PerceptionEvent::PerceptionEventType)eventType;
	event->m_lifeTime = lifeTime;
	event->m_alertness = alertness;
	event->m_radiusInner = radiusMin;
	event->m_radiusOuter = radiusMax;
	event->m_volume = volume;
	event->m_initiatorID = initiator ? initiator->getId() : GuideBot::BaseObject::INVALID_OBJECT_ID;
	event->setPos(pos);
	GuideBot::PerceptionEventId id = GuideBot::PerceptionEvent::addEvent(event);
	return id;
}

ConsoleFunction( removePerceptionEvent, bool, 2,  2, "(eventId)")
{
	GuideBot::PerceptionEventId eventId = dAtoi(argv[1]);
	return GuideBot::PerceptionEvent::removeEvent(eventId);
}

ConsoleMethod( EnhancedPlayer, setViewDistance, void, 3, 3, "(viewDistance)")
{
	object->setViewDistance(dAtof(argv[2]));
}

ConsoleMethod( EnhancedPlayer, setFov, void, 3, 3, "(fov)")
{
	object->setFov(dAtof(argv[2]));
}

ConsoleMethod( EnhancedPlayer, setPauseAction, void, 3, 5, "(time [, parent, effectors])")
{
	GuideBot::Time time = dAtoi(argv[2]);
	GuideBot::ActionPause* action = CREATE_ACTION(ActionPause);
	action->setDuration(time);

	ScriptAction* parentAction = 0; 
	if (argc>3)
	{
		parentAction = dynamic_cast<ScriptAction*>(Sim::findObject(argv[3]));
	}

	if (argc>4)
	{
		GuideBot::EffectorIDVector effectors;
		GuideBot::Effector::convertFromString(argv[4], effectors);
		action->setRequiredEffectors(effectors,true);
	}
	
	object->setActionByPrototype(action, parentAction);
}
