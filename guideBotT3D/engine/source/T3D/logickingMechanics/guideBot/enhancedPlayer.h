//-----------------------------------------------------------------------------
// Guide Bot
// Copyright (C) LogicKing.com, Inc.
//-----------------------------------------------------------------------------

#ifndef _ENHANCED_PLAYER_H
#define	_ENHANCED_PLAYER_H

#include "T3D/aiPlayer.h"
#include "gfx/sim/debugDraw.h"

#include "guideBot/actor.h"
#include "guideBot/anchor.h"
#include "guideBot/stringUtils.h"
#include "T3D/logickingMechanics/guideBot/torqueGuideBotPlatform.h"

////////////////////////////////////////////////////
//////***********VisualDataBlock************/////////
////////////////////////////////////////////////////

class VisualDataBlock: public SimDataBlock, public GuideBot::Visual
{
	typedef SimDataBlock Parent;
public:
	VisualDataBlock();

	DECLARE_CONOBJECT( VisualDataBlock);
	static void initPersistFields();

	virtual bool onAdd();
	virtual void onRemove();

	virtual void packData(BitStream* stream);
	virtual void unpackData(BitStream* stream);
	bool preload(bool server, String &errorStr);

	class CallbackCaller
	{
	public:
		CallbackCaller(const char* name);
		void execute(GuideBot::Actor* owner);
	protected:
		std::string	m_funcName;
	};

	const char* getAnimation(){ return m_visualAnimStr;};
	SFXTrack* getSound(){ return m_visualSoundTrack;};

protected:
	enum Constants { MaxCallbacks = 10};
	const char*			m_visualNameStr;
	const char*			m_visualAnimStr;
	SFXTrack*			m_visualSoundTrack;
	const char*			m_visualEffectorsStr;
	
	F32					m_callbackTime[MaxCallbacks];
	const char*			m_callbackFunc[MaxCallbacks];

	typedef std::vector<CallbackCaller*> CallbackCallers;
	CallbackCallers		m_callbackCallers;

};


////////////////////////////////////////////////////
//////***********StateDataBlock************/////////
////////////////////////////////////////////////////

class StateDataBlock: public SimDataBlock
{
	typedef SimDataBlock Parent;
public:
	StateDataBlock();

	DECLARE_CONOBJECT( StateDataBlock );
	static void initPersistFields();

	virtual bool onAdd();
	virtual void onRemove();

	virtual void packData(BitStream* stream);
	virtual void unpackData(BitStream* stream);

	GuideBot::State*	getState(){return m_state;};
	const char*			getPose() {return m_pose;};
protected:
	bool getVisualEffectors(size_t idx, GuideBot::EffectorIDVector& effectors);

	enum Constants { MaxMoveDirs   = 16, MaxVisuals = 32, MaxChanges = 16 };

	//movement data
	VectorF				m_movementDir[MaxMoveDirs];
	const char*			m_movementVisual[MaxMoveDirs];
	F32					m_movementVel[MaxMoveDirs];

	//alias data
	const char*			m_alias[MaxVisuals];
	const char*			m_aliasVisual[MaxVisuals];

	//change data
	const char*			m_changeState[MaxChanges];
	const char*			m_changeVisual[MaxChanges];

	const char*			m_enableCallback;
	const char*			m_disableCallback;

	GuideBot::State*	m_state;

	//torque specific state data
	const char*			m_pose;
	

};

////////////////////////////////////////////////////
//////********AttackInfoDataBlock**********/////////
////////////////////////////////////////////////////
class AttackInfoDataBlock: public SimDataBlock
{
	typedef SimDataBlock Parent;
public:
	AttackInfoDataBlock();

	DECLARE_CONOBJECT(AttackInfoDataBlock);
	static void initPersistFields();
	virtual bool onAdd();
	GuideBot::AttackInfo*	getAttackInfo(){return &m_attackInfo;};
	float evaluatorCaller(GuideBot::Actor* actor,GuideBot::WorldObject* enemyObj, float distSq, float fov,
							GuideBot::AttackInfo* info, unsigned int lastExecutionTime, bool isActive);
protected:
	float		m_minDist;
	float		m_maxDist;
	S32			m_fovAngle;
	S32			m_periodicity;
	float		m_manoeuvreMinDist;
	float		m_manoeuvreMaxDist;
	bool		m_strafe;
	S32			m_pause;
	bool		m_blockMovements;
	const char* m_evaluator;
	const char* m_action;
	const char* m_weapon;
	const char* m_state;
	const char* m_coverState;
	const char* m_attackState;
	bool		m_lookAtEnemy;

	GuideBot::AttackInfo	m_attackInfo;
};

////////////////////////////////////////////////////
//////***Waypoint data block and object*******//////
////////////////////////////////////////////////////
class WaypointDataBlock: public GameBaseData
{
	typedef GameBaseData Parent;
public:
	WaypointDataBlock();
	DECLARE_CONOBJECT(WaypointDataBlock);
	static void initPersistFields();

	float					m_radius;
};

class WaypointObject: public GameBase
{
	typedef GameBase Parent;
public:
	WaypointObject();
	DECLARE_CONOBJECT(WaypointObject);
	virtual bool onNewDataBlock(GameBaseData* dptr, bool reload);
	virtual bool onAdd();
	virtual void onRemove();

	GuideBot::Waypoint* getWaypoint() { return m_waypoint;};
protected:
	GuideBot::Waypoint* m_waypoint;
private:
	WaypointDataBlock*		mDataBlock;
};

/// ObstacleObject class.
/// Obstacle has radius, Actor will try to 
/// avoid using steering behaviors.
class ObstacleObject: public SceneObject
{
	public:
	enum ObstacleType
	{
		OT_LOW	= 0,
		OT_MEDIUM	= 1,
		OT_HIGH = 2
	};

	typedef SceneObject Parent;

public:
	ObstacleObject();

	DECLARE_CONOBJECT(ObstacleObject);
	static void initPersistFields();
	virtual bool onAdd();
	virtual void onRemove();

protected:
	GuideBot::Obstacle::ObstacleType	m_type;
	float					m_radius;
	GuideBot::Obstacle*		m_obstacle;
};

/// Anchor class.
/// Anchor is object in the world that contains
/// some useful information for the actor. Anchor can
/// be viewed as waypoint with additional capabilities.

class AnchorDataBlock: public GameBaseData
{
	typedef GameBaseData Parent;
public:
	AnchorDataBlock();
	DECLARE_CONOBJECT(AnchorDataBlock);
	static void initPersistFields();

	float					m_radius;
	GuideBot::Obstacle::ObstacleType	m_type;
	bool					m_hasObstacle;
};
//DECLARE_CONSOLETYPE(AnchorDataBlock)

class AnchorObject: public GameBase
{
	typedef GameBase Parent;
public:
	AnchorObject();

	DECLARE_CONOBJECT(AnchorObject);
	virtual bool onNewDataBlock(GameBaseData* dptr, bool reload);
	virtual bool onAdd();
	virtual void onRemove();

protected:
	GuideBot::Obstacle*		m_obstacle;
	GuideBot::Anchor*		m_anchor;
private:
	AnchorDataBlock*		mDataBlock;
};


////////////////////////////////////////////////////
//////***********EnhancedPlayer************/////////
////////////////////////////////////////////////////
class EnhancedPlayerData : public PlayerData
{
	typedef PlayerData Parent;
public:
	EnhancedPlayerData();
	DECLARE_CONOBJECT(EnhancedPlayerData);
	static void initPersistFields();
	void packData(BitStream*);
	void unpackData(BitStream*);

	enum { MaxAttackInfosNum  = 16};
	const char*		m_attackInfoNames[MaxAttackInfosNum];
	bool			m_enableSlopes;

	float			m_viewDistance;
	float			m_fov;
	int	m_forgetTime;
};


/// EnhancedPlayer, enhanced and improved version of standard AIPlayer and Player classes.
///
/// EnhancedPlayer performs these additional tasks:
///   - Implements GuideBot's Actor class functionality with Torque classes world.
///   - Implements GuideBot's animation function.
///   - Performs animations client-server synchronization.
///   - Performs interaction with Torque Script.

class EnhancedPlayer : public AIPlayer, public GuideBot::Actor
{
	typedef AIPlayer Parent;
public:
	DECLARE_CONOBJECT( EnhancedPlayer );

	EnhancedPlayer();
	~EnhancedPlayer();

	static void initPersistFields();

	virtual bool onNewDataBlock(GameBaseData* dptr, bool reload);
	virtual bool onAdd();
	virtual void onRemove();
	virtual void processTick(const Move *move);
	void advanceTime(F32 dt);


	void writePacketData(GameConnection *conn, BitStream *stream);
	void readPacketData (GameConnection *conn, BitStream *stream);
	U32  packUpdate  (NetConnection *conn, U32 mask, BitStream *stream);
	void unpackUpdate(NetConnection *conn, BitStream *stream);

	SimObjectId getId() const { return Parent::getId(); }
	//bool registerObject() { return Parent::registerObject(); }
	ObjectId registerObject() { return GuideBot::Actor::registerObject(); }

	//world object interface
	virtual void getAABB(GuideBot::Vector3& _min, GuideBot::Vector3& _max) const;

	virtual void setPosition(const Point3F& pos,const Point3F& viewRot);
	//player interface
	virtual GuideBot::Vector3 getEyesLookDir();

	//animation interface
	typedef GuideBot::VisualIdx VisualIdx;
	virtual VisualIdx	startVisual(GuideBot::Visual* visual);
	virtual void		stopVisual(VisualIdx visualIdx);
	virtual void		updateVisuals(float dt);
	virtual bool	isCyclicVisual(GuideBot::Visual* visual);
	virtual void	setAnimationTimeScale(int animIdx, float scale);
	virtual void	setAnimationPosition(int animIdx, float pos);

	
	virtual GuideBot::Visual* getVisual(const std::string& visualName);
	virtual GuideBot::State*  getState (const std::string& stateName);

	virtual void	setActorState(GuideBot::State* state);

	virtual void	call(const std::string& callbackName);

	//common actions
	virtual GuideBot::Action* takeWeapon(const std::string& weapon, const std::string& state,
							GuideBot::Action* parent = NULL, bool onlyPrototype = false);
	
	virtual void setVisualEndCallback(VisualIdx visualIdx,VisualEndCallback* callback);
	virtual void setAnimCallbacks(VisualIdx visualIdx,CallbackMap& callbacks);
	virtual void setClientControlled(VisualIdx visualIdx);

	

	struct VisualInfo
	{
		VisualInfo():	visual(NULL)
						, visualIdx(GuideBot::INVALID_VISUAL_IDX)
						, animIdx(-1)
						, thread(NULL)
						, callback(NULL)
						, timeScale(1.f)
						, position(0.f)
						, lastUpdateTime(0.f)
						, clientControlled(false)
						, soundSource(NULL)
		{};

		VisualDataBlock* visual;
		VisualIdx	visualIdx;
		//animation
		int			animIdx;
		TSThread*	thread;
		VisualEndCallback*	callback;
		float 		timeScale;
		float 		position;
		CallbackMap	callbackMap;
		float		lastUpdateTime;
		bool		clientControlled;
		
		//sound
		SFXSource*	soundSource;
	};

	typedef std::map<VisualIdx,VisualInfo>	VisualInfoMap;
	typedef std::set<VisualIdx>			VisualInfoIdxSet;

	void prepareVisualInfo(VisualDataBlock* visualData, VisualInfo& info);
	void startVisual(VisualInfo& visualInfo);
	
	virtual void* getUserData() {return this;};

	virtual float getTeamRelationship(WorldObject* object);

	virtual void getMuzzleVector(U32 imageSlot,VectorF* vec);

	static GuideBot::EnumDescription poseEnumDescription;
protected:
	
	/// Bit masks for different types of events
	enum MaskBits {
		ActorStateMask   = Parent::NextFreeMask << 0,
		NextFreeMask = Parent::NextFreeMask << 1
	};

	virtual void updateVision();
	virtual bool checkObjectLos(WorldObject* object);

	void moveUpdate();
	
	const char*		m_initState;
	bool			m_enablePerception;	
	
	VisualInfoMap	m_visualInfoMap;
	VisualInfoIdxSet m_visualToStop;

	virtual void createWorldObject();
	virtual void destroyWorldObject();

	EnhancedPlayerData*	mDataBlock;

	virtual void wallAvoindance(const VectorF& wallNormal);

	///Update head animation
	virtual void updateLookAnimation(F32 dT = 0.f);

	GuideBot::ActionLookAt*	m_controlledLookAt;

	VectorF m_actorVelocity;
	bool	m_updateMoveAction;

	bool	m_prevTrigger[MaxTriggerKeys];

	static const GuideBot::EnumDescription::Record poseEnums[];
};

typedef GuideBot::Obstacle::ObstacleType ObstacleTypes;
DefineEnumType( ObstacleTypes );

#endif