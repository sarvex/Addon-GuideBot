//-----------------------------------------------------------------------------
// Guide Bot
// Copyright (C) LogicKing.com, Inc.
//-----------------------------------------------------------------------------

#include "guideBot/actionMove.h"
#include "guideBot/actor.h"

#include <algorithm>
#include <float.h>

using namespace GuideBot;
namespace
{
	const float MOVING_THRESHOLD = 0.001f;
}

////////////////////////////////////////////////////
//////*************ActionIdle*********//////////////
////////////////////////////////////////////////////

IMPLEMENT_REGISTER_ROUTINE(ActionIdle);

ActionIdle::ActionIdle() :	m_enableSlopes(true)
							, m_checkEnemy(false)
							, m_idleState(NULL)
{
	addRequiredEffector(E_BRAIN,false);
	addRequiredEffector(E_LEGS,false,0);
}

void ActionIdle::setParams(bool enableSlopes)
{
	m_enableSlopes = enableSlopes;
}

void ActionIdle::prepareClone(Action* actionClone)
{
	Parent::prepareClone(actionClone);
	ActionIdle* actionIdleClone = static_cast<ActionIdle*>(actionClone);
	actionIdleClone->m_enableSlopes = m_enableSlopes;
}

void ActionIdle::init()
{
	if (m_enableSlopes)
		m_owner->setActionByName("ActionSlope",this);
	setMoveAction();
	m_idleState = m_owner->getActorState();
}

bool ActionIdle::update()
{
	if (!Parent::update())
		return false;

	if (hasEffector(E_BRAIN))
	{
		if (m_owner->getAlertness() == 1)
			m_owner->setActionByName("ActionFight");
		else if (m_idleState != m_owner->getActorState() && 
					(!m_owner->isControlledByPlayer() && m_owner->isEnabledPerception()))
		{
			//return to idle state
			m_owner->changeState(m_idleState->getName(),this);
		}
	}
	
	return true;
}
void ActionIdle::onEffectorsLost(const EffectorIDVector& lostEffectors)
{
	Parent::onEffectorsLost(lostEffectors);
	if (!hasEffector(E_LEGS))
		m_owner->setMovingActive(false);
}

void ActionIdle::onEffectorsAcquired(const EffectorIDVector& newEffectors)
{
	Parent::onEffectorsAcquired(newEffectors);
	setMoveAction();
}

void ActionIdle::setMoveAction()
{
	if (hasEffector(E_LEGS))
	{
		m_owner->setActionByName("ActionMove",this);
		m_owner->setMovingActive(true);
	}
}

////////////////////////////////////////////////////
//////*************ActionMove*********//////////////
////////////////////////////////////////////////////


IMPLEMENT_REGISTER_ROUTINE(ActionMove);

ActionMove::ActionMove():m_curState(NULL)
{
	addRequiredEffector(E_LEGS);
	addRequiredEffector(E_EYES,false,0);
}

void ActionMove::init()
{
	Parent::init();
	//set looking forward action
	m_owner->setLookDirMode(ActionLookAt::L_FORWARD,NULL,NULL,0,this);
	update();
}


bool ActionMove::update()
{
	if (!Parent::update())
		return false;

	Vector3 moveDir = m_owner->getLastMoveDir(); //m_owner->getMoveDir();

	//find appropriate movement desc of current state
	State* state = m_owner->getActorState();
	Vector3 localDir = moveDir;
	Matrix worldToObj = m_owner->getTm();
	worldToObj.inverse();
	worldToObj.mulV(localDir);
	
	const MovementDesc& desc = state->selectMovementDesc(localDir);

	//calc move vector
	Vector3 moveVector = desc.m_velocity*moveDir;
	//GB_INFO("ActionMove:: move %f %f %f", moveVector.x, moveVector.y, moveVector.z);
	m_owner->move(moveVector);
	

	Vector3 localActualVel = m_owner->m_actualVelocity;
	localActualVel.z = 0.f;
	worldToObj.mulV(localActualVel);
	if (localActualVel.lengthSq() < MOVING_THRESHOLD)
		localActualVel = Vector3::ZERO;
	const MovementDesc& actualDesc = state->selectMovementDesc(localActualVel);

	//play visual if it has been changed
	if (m_currentVisual != actualDesc.m_visualName || m_curState != m_owner->getActorState())
	{
		m_currentVisual = actualDesc.m_visualName;
		m_curState = m_owner->getActorState();
		ActionPlayVisual* action = static_cast<ActionPlayVisual*>(m_owner->playVisual(actualDesc.m_visualName, this));
		//set client controlled flag
		if (action)
			action->setClientControlled();
	}

	ActionPlayVisual* playVisual = GET_CHILD(ActionPlayVisual);	
	if (playVisual)
	{
		float dot = dotProtuct(localActualVel,actualDesc.m_dir)/actualDesc.m_velocity;
		playVisual->setTimeScale(actualDesc.m_velocity > 0.f ? 
			maxValue(dot,0.1f) : 1.f);
	}
	

	return true;
}

void ActionMove::terminate(TerminationStatus status /*= FINISHED*/)
{
	Parent::terminate(status);
}

////////////////////////////////////////////////////
//////********ActionMoveToPoint*******//////////////
////////////////////////////////////////////////////
IMPLEMENT_REGISTER_ROUTINE(ActionMoveToPoint);

ActionMoveToPoint::ActionMoveToPoint()
{
	addRequiredEffector(E_BRAIN_MOVING);
}

void ActionMoveToPoint::setParams(const Vector3& destination, float precision)
{
	m_destination = destination;
	m_precisionSq = precision*precision;
}

void ActionMoveToPoint::updateDynamicParams(Action* actionClone)
{
	Parent::updateDynamicParams(actionClone);
	
	ActionMoveToPoint*  actionMoveToPoint = static_cast<ActionMoveToPoint*>(actionClone);
	actionMoveToPoint->m_destination = m_destination;
	actionMoveToPoint->m_precisionSq = m_precisionSq;
}

void ActionMoveToPoint::init()
{	
	update();
}

bool ActionMoveToPoint::update()
{
	if (!Parent::update())
		return false;

	Vector3 toDest = m_destination - m_owner->getPos();
	toDest.z = 0.f;
	if (toDest.lengthSq()<m_precisionSq)
		terminate();
	else
	{
		toDest.normalize();
		//GB_INFO("ActionMoveToPoint::update() move dir %f %f ",toDest.x, toDest.y);
		m_owner->setMoveDir(toDest);
	}

	return true;
}

void ActionMoveToPoint::terminate(TerminationStatus status /*= FINISHED*/)
{
	m_owner->setMoveDir(Vector3::ZERO);
	Parent::terminate(status);
}

////////////////////////////////////////////////////
//////********ActionMoveToObject*******//////////////
////////////////////////////////////////////////////
IMPLEMENT_REGISTER_ROUTINE(ActionMoveToObject);

ActionMoveToObject::ActionMoveToObject():	m_targetObject(NULL)
											, m_targetObjectId(BaseObject::INVALID_OBJECT_ID)
{

}

void ActionMoveToObject::setParams(WorldObject* target, float precision)
{
	GB_ASSERT(target && BaseObject::isValid(target->getId()), "Target object must be defined");
	m_precisionSq = precision*precision;
	m_targetObject = target;
	m_targetObjectId = m_targetObject->getId();
}

void ActionMoveToObject::updateDynamicParams(Action* actionClone)
{
	Parent::updateDynamicParams(actionClone);
	ActionMoveToObject*  actionMoveToObject = static_cast<ActionMoveToObject*>(actionClone);
	actionMoveToObject->m_targetObject = m_targetObject;
	actionMoveToObject->m_targetObjectId = m_targetObjectId;
	actionMoveToObject->m_precisionSq = m_precisionSq;
}

bool ActionMoveToObject::update()
{
	if (!WorldObject::isValid(m_targetObjectId) ||  m_targetObject->getDestroyed())
	{
		m_targetObject = NULL;
		m_targetObjectId = -1;
		terminate();
		return false;
	}
	m_destination = m_targetObject->getPos();
	if (!Parent::update())
		return false;
	return true;
}

////////////////////////////////////////////////////
//////**********ActionMoveByWaypoints*****//////////
////////////////////////////////////////////////////
IMPLEMENT_REGISTER_ROUTINE(ActionMoveByWaypoints);

ActionMoveByWaypoints::ActionMoveByWaypoints():	m_mode(FM_DIRECT)
												, m_curWaypointIdx(-1)
												, m_precision(0.f)
{
	addRequiredEffector(E_BRAIN_MOVING,false);
}

void ActionMoveByWaypoints::setParams(const Waypoints& waypoints, FollowMode mode /* = FM_DIRECT */, float precision /* = 0.5 */)
{
	m_waypoints = waypoints;
	m_mode = mode;
	m_precision = precision;
	m_curWaypointIdx = -1;
}

void ActionMoveByWaypoints::prepareClone(Action* actionClone)
{
	Parent::prepareClone(actionClone);

	ActionMoveByWaypoints* actionMoveByWpClone = static_cast<ActionMoveByWaypoints*>(actionClone);
	actionMoveByWpClone->m_waypoints = m_waypoints;
	actionMoveByWpClone->m_mode = m_mode;
	actionMoveByWpClone->m_precision = m_precision;
	actionMoveByWpClone->m_curWaypointIdx = m_curWaypointIdx;
}

void ActionMoveByWaypoints::init()
{
	Parent::init();
	const bool proceedToNearest = true;
	proceedToNextWaypoint(proceedToNearest);
}

void ActionMoveByWaypoints::onChildTerminated(Action* terminatedChild)
{
	Parent::onChildTerminated(terminatedChild);
	if (terminatedChild->getTerminationStatus()== FINISHED)
	{
		proceedToNextWaypoint();
	}
}

void ActionMoveByWaypoints::findNearestWaypoint(bool preferVisible /*= true*/)
{
	Vector3 pos = m_owner->getPos();
	m_curWaypointIdx = -1;
	float minDistSq = FLT_MAX;
	bool visibleFound = false;
	for (size_t i = 0; i <m_waypoints.size(); i++)
	{
		Vector3 wayPointPos = m_waypoints[i]->getPos();
		float distSq = (pos-wayPointPos).lengthSq();

		if (preferVisible)
		{
			bool isVisible = m_owner->checkObjectVisibility(m_waypoints[i]);
			if (isVisible)
			{
				visibleFound = true;
				minDistSq = FLT_MAX; //reset min dist, because we're starting to search only visible way points
			}
			else if (visibleFound)
				continue; //we've already found at least one visible way point, so skip invisible one
		}

		if (distSq < minDistSq)
		{
			minDistSq = distSq;
			m_curWaypointIdx = i;
		}
	}
}

void ActionMoveByWaypoints::proceedToNextWaypoint(bool toNearest /*= false*/)
{
	if (toNearest)
	{
		findNearestWaypoint();
	}
	else if (!pickUpNextWaypointIdx())
	{
		terminate();
		return;
	}
	GB_ASSERT(m_curWaypointIdx>=0 && m_curWaypointIdx<m_waypoints.size(),"ActionMoveByWaypoints::update: wrong curWaypointIdx");

	Waypoint* currentWp = m_waypoints[m_curWaypointIdx];
	if (!m_owner->moveToObject(currentWp, m_precision,this))
	{
		terminate();
	}
}

bool ActionMoveByWaypoints::pickUpNextWaypointIdx()
{
	if (m_mode == FM_RANDOM)
	{
		randI(0,m_waypoints.size()-1);
		return true;
	}
	
	m_curWaypointIdx++;
	if (m_curWaypointIdx == m_waypoints.size())
	{
		if (m_mode == FM_DIRECT)
			return false;
		else if (m_mode == FM_CIRCULAR)
			m_curWaypointIdx = 0;
	}
	
	return true;
}

////////////////////////////////////////////////////
//////**********ActionManoeuvre*******//////////////
////////////////////////////////////////////////////

IMPLEMENT_REGISTER_ROUTINE(ActionManoeuvre);

ActionManoeuvre::ActionManoeuvre():	m_strafeDir(Vector3::ZERO)
									, m_lastStrafeTime(0)
									, m_targetObject(NULL)
									, m_targetObjectId(BaseObject::INVALID_OBJECT_ID)
									, m_tacticalMovement(None)
{
	addRequiredEffector(E_BRAIN_MOVING,false);
	m_defaultDesc.isStrafing = m_defaultDesc.isChasing = m_defaultDesc.isFleeing = false;
	setDefaultParams();
}

void ActionManoeuvre::setMinMaxDist(float minDist, float maxDist, bool strafe)
{
	static const float defaultBuffer = 1.f;
	if (maxDist<=minDist || maxDist - minDist < defaultBuffer)
	{
		GB_ERROR("ActionManoeuvre::setMinMaxDist: maxDist must be greater than minDist  ");
	}
	m_desc.isFleeing = m_desc.isChasing = true;
	m_desc.chaseFarDist =  maxDist;
	m_desc.fleeFarDist =  maxDist  - defaultBuffer;
	m_desc.chaseCloseDist = minDist + defaultBuffer;
	m_desc.fleeCloseDist = minDist;

	m_desc.isStrafing = strafe;
}

void ActionManoeuvre::setDefaultParams()
{
	m_desc = m_defaultDesc;
}

void ActionManoeuvre::setParams(const ManoeuvreDesc& desc)
{
	m_desc = desc;
}

void ActionManoeuvre::setTarget(WorldObject* targetObject)
{
	m_targetObject = targetObject;
	m_targetObjectId = m_targetObject ? m_targetObject->getId() : BaseObject::INVALID_OBJECT_ID;
}

void ActionManoeuvre::updateDynamicParams(Action* actionClone)
{
	Parent::updateDynamicParams(actionClone);

	ActionManoeuvre* actionManoeuvreClone = static_cast<ActionManoeuvre*>(actionClone);
	actionManoeuvreClone->m_desc = m_desc;
	actionManoeuvreClone->m_targetObject = m_targetObject;
	actionManoeuvreClone->m_targetObjectId = m_targetObjectId;

}

void ActionManoeuvre::terminate(TerminationStatus status /*= FINISHED*/)
{
	m_owner->setMoveDir(Vector3::ZERO);
	Parent::terminate(status);
}

bool ActionManoeuvre::update()
{
	if (!Parent::update())
		return false;

	if (!m_targetObject || !WorldObject::isValid(m_targetObjectId) || m_targetObject->getDestroyed())
	{
		m_targetObject  = NULL;
		m_targetObjectId = BaseObject::INVALID_OBJECT_ID;
		m_owner->setMoveDir(Vector3::ZERO);
		return false;
	}

	if(m_desc.isStrafing)
	{
		if(Platform::getTime() - m_lastStrafeTime > m_desc.strafeChangeDirTime)
		{
			m_lastStrafeTime = Platform::getTime();
			m_strafeDir = Vector3::ZERO;
			int mode = randI(0, 2);
			if (mode)
			{
				Vector3 dir;
				m_owner->getTm().getColumn(0, &dir);
				if (mode == 1)
					dir *= -1.f;
				m_strafeDir = dir;
			}
		}
	}
	else
		m_strafeDir = Vector3::ZERO;

	Vector3 targetPos = m_targetObject->getPos();
	Vector3 toTarget = targetPos - m_owner->getPos();
	float distToEnemy = toTarget.length();

	Vector3 moveDir = m_strafeDir;

	if(m_desc.isChasing)
	{
		// approaching the enemy
		if(distToEnemy > m_desc.chaseCloseDist)
		{
			if(distToEnemy > m_desc.chaseFarDist)
			{
				m_tacticalMovement = Chasing;
			}

			if(m_tacticalMovement == Chasing)
			{
				Vector3 chaseDir = toTarget;
				chaseDir.normalize();
				moveDir += chaseDir;
			}
		}
		else if(distToEnemy < m_desc.chaseCloseDist)
		{
			if(m_tacticalMovement == Chasing)
			{
				m_tacticalMovement = Holding;
			}
		}
	}

	if(m_desc.isFleeing)
	{
		if(distToEnemy < m_desc.fleeFarDist)
		{
			if(distToEnemy < m_desc.fleeCloseDist)
			{
				m_tacticalMovement = Fleeing;
			}

			if(m_tacticalMovement == Fleeing)
			{
				Vector3 fleeDir = -toTarget;
				fleeDir.normalize();
				moveDir += fleeDir;
			}
		}
		else if (distToEnemy > m_desc.fleeFarDist)
		{
			if(m_tacticalMovement == Fleeing)
			{
				m_tacticalMovement = Holding;
			}
		}
	}

	m_owner->setMoveDir(moveDir);

	return true;
}

////////////////////////////////////////////////////
//////*************ActionFlee*********//////////////
////////////////////////////////////////////////////
IMPLEMENT_REGISTER_ROUTINE(ActionFlee);

ActionFlee::ActionFlee()
{
	m_desc.isStrafing = m_desc.isChasing = false;
	m_desc.isFleeing = true;
	setFleeDistance(20.f);
}

void ActionFlee::setFleeDistance(float distance, float farCloseDelta /*= 10.f*/)
{
	m_desc.fleeCloseDist = distance;
	m_desc.fleeFarDist = m_desc.fleeCloseDist + farCloseDelta;
}

