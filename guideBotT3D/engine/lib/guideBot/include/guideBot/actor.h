//-----------------------------------------------------------------------------
// Guide Bot
// Copyright (C) LogicKing.com, Inc.
//-----------------------------------------------------------------------------
#ifndef _ACTOR_H
#define _ACTOR_H

#include <vector>
#include <map>
#include <sstream>
#include "math.h"

#include "guideBot/actorState.h"
#include "guideBot/worldObject.h"
#include "guideBot/action.h"
#include "guideBot/actions.h"
#include "guideBot/actionPlayVisual.h"
#include "guideBot/actionMove.h"
#include "guideBot/actionFight.h"
#include "guideBot/perceptionEvent.h"

namespace GuideBot 
{

class Visual;
class AnimVisual;
class State;

typedef std::map<EffectorID,Effector*>	EffectorMap;

class Actor : public WorldObject
{
	typedef WorldObject Parent;
public:
	Actor();
	~Actor();

	//world object interface
	virtual void setLookDir(const Vector3& lookDir);

	virtual Vector3 getEyesLookDir();
	

	
	virtual void move(const Vector3& velocity);

	//animation interface
	virtual VisualIdx	startVisual(Visual* visual) = NULL;
	virtual void		stopVisual(int visualIdx) = NULL;
	virtual void		updateVisuals(float dt) = NULL;
	virtual bool	isCyclicVisual(Visual* visual) = NULL;
	virtual void	setAnimationTimeScale(VisualIdx visualIdx, float scale) = NULL;
	virtual void	setAnimationPosition(VisualIdx visualIdx, float pos) = NULL;
	virtual void	setClientControlled(VisualIdx visualIdx) = NULL;

	//sound interface



	//animation callbacks
	typedef GuideBot::ActionPlayVisual::VisualEndCallback VisualEndCallback;
	virtual void setVisualEndCallback(int threadIdx, VisualEndCallback* callback) = NULL;
	typedef GuideBot::Visual::AnimCallbackFunction AnimationCallback;
	typedef GuideBot::Visual::CallbackMap	CallbackMap;
	virtual void setAnimCallbacks(int threadIdx, CallbackMap& callbacks) = NULL;

	virtual Visual* getVisual(const std::string& visualName) = NULL;
	virtual State*  getState (const std::string& stateName)  = NULL;
	
	virtual void	call(const std::string& callbackName)  = NULL;


	void update();

	//state routine	
	virtual void	setActorState(State* state);
	State*	getActorState() {return m_state;};
	Action*	changeState(const std::string& stateName,Action* parentAction = NULL);

	//action routine
	Action* setActionByName(const char*  actionName,Action* parentAction = NULL);
	Action* setActionByPrototype(Action* action,Action* parentAction = NULL);
	Action* setActionDirectly(Action* action,Action* parentAction = NULL);
	
	bool	terminateAction(const char*  actionName);
	void	terminateAllActions();
	void	releaseAction(Action* action);
	bool	isActionSetted(const char* actionName);
	Action*	getActiveAction(const std::string actionName, EffectorID effectorID);
	Action*	getActiveAction(const std::string actionName, const EffectorIDVector& effectors = EffectorIDVector());

	//effector routine
	void		releaseActionEffectors(Action* action,const EffectorIDVector& effectorIDs);
	void		loseEffectors(const EffectorIDVector&effectors, Action* parentAction);
	Effector*	getEffector(EffectorID id);
	bool		compareActions(Action* action1, Action* action2,EffectorID effectorID,int priority1, int priority2);
	
	//for "manual" effectors blocking/releasing 
	void		blockEffectors(const EffectorIDVector& effectors);
	void		resetEffectors(const EffectorIDVector& effectors);	
	

	bool	checkEffectors(Action* action, Action* parentAction,EffectorIDVector &acquiredEffectors);
	void	notifyAcquiringEffectors(const EffectorIDVector& effectors, Action *parent);

	//actor functions
	void	setMoveDir(const Vector3& moveDir);
	Vector3 getMoveDir() {return m_moveDir;};
	void	resetMoveDir() {m_lastMoveDir = m_moveDir; m_moveDir=Vector3::ZERO;};
	Vector3 getLastMoveDir() {return m_lastMoveDir;};

	//common actions
	bool hasVisual(const std::string& visualName);
	Action* playVisual(const std::string& visualName, Action* parentAction = NULL,bool allowInactive = false);
	Action* playVisual(Visual* visual, Action* parentAction = NULL,bool allowInactive = false);
	Action*	moveToPoint(const Vector3& point,float precision = 0.2f,Action* parent = NULL,bool onlyPrototype = false);
	Action*	moveToObject(WorldObject* target,float precision = 1.5f,Action* parent = NULL,bool onlyPrototype = false);
	Action*	moveByWaypoints(const Waypoints& waypoints,ActionMoveByWaypoints::FollowMode mode = ActionMoveByWaypoints::FM_DIRECT,
		float precision = 0.5f,Action* parent = NULL);

	Action*	setLookDirMode(ActionLookAt::LookDirMode mode,const Vector3* point = NULL, WorldObject* object = NULL,int priority = 1,Action* parent = NULL);
	virtual Action* takeWeapon(const std::string& weapon, const std::string& state,Action* parent = NULL,bool onlyPrototype = false) = NULL;

	Action* assignToGuard(const Waypoints& waypoints, const std::string& actionPatrol = "",
						const std::string& actionInspect = "", const std::string& actionAlert = "",
						const std::string& actionFight = "", Action* parent = NULL);

	//memory system
	void setViewDistance(float viewDistance) { m_viewDistance = viewDistance; }
	void setFov(float fov) { m_fov = fov; }

	struct MemoryFrame
	{
		MemoryFrame() : time(0), distSq(0.f), fov(0.f), alertness(0.f), forgetTime(INVALID_TIME),
						los(false), object(NULL), objectId(INVALID_OBJECT_ID) {};
		// describes sensors information have been received from
		unsigned int type;
		// alertness level
		float alertness;
		// last contact time
		unsigned int time;
		// distance to object
		float		 distSq;
		// object fov
		float		 fov;
		// is object on line of sight
		bool 		 los;
		//custom forget time
		Time		 forgetTime;
		//object
		ObjectId		objectId;
		//redundant to objectId
		WorldObject*	object;
	};
	typedef std::map<ObjectId, MemoryFrame> MemoryFrames;
	void setForgetTime(unsigned int forgetTime) { m_forgetTime = forgetTime; }

	void  setEnemy(WorldObject* newEnemy);
	WorldObject* getEnemy();
	const MemoryFrame* getEnemyFrame();
	const MemoryFrame* getMemoryFrame(WorldObject* object);

	void addAttackInfo(AttackInfo* attackInfo);
	const AttackInfos& getAttackInfos(){return m_attackInfos;};
	
	void setActiveAttackInfo(AttackInfo* activeAttackInfo) { m_activeAttackInfo = activeAttackInfo;};
	AttackInfo* getActiveAttackInfo() {return m_activeAttackInfo;};
	
	
	typedef Delegate<float(Actor*,WorldObject*)> EnemyEvaluator;
	static void setEnemyEvaluator(const EnemyEvaluator& evaluator = EnemyEvaluator());

	float	getVelocity()	{return m_velocity;};
	Vector3	getVelocityVector()	{return m_velocityVector;};
	
	void	setPitch(float pitch);
	float	getPitch() {return m_pitch;};

	//perception
	void	enablePerception(bool flg){ m_enablePerception = flg;};
	bool	isEnabledPerception() {return m_enablePerception;}

	//death 
	void	setDead(bool withoutAction = false);
	bool	isDead() {return m_isDead;};

	//team relationship factor for enemy evaluation (by default, everybody is enemy)
	virtual float getTeamRelationship(WorldObject* object)	{return 1.f;};
	
	virtual bool checkObjectVisibility(WorldObject* object, float* pDistSq = NULL, float* pFov = NULL);
	virtual bool checkObjectLos(WorldObject* object) = NULL;

	MemoryFrame* findNearestObjectInMemory(WorldObjectType type, float dist = -1.f, float fov = M_PI);
	MemoryFrame* findNearestPerceptionEvent(PerceptionEvent::PerceptionEventType type, float dist = -1.f, float fov = M_PI);
	//debug
	void	printActions(std::ostringstream& buff, bool full = false);

	float	getAlertness() {return m_alertness;}

	void setControlledByPlayer(bool flg);
	bool isControlledByPlayer() {return m_isControlledByPlayer;};

	void setMovingActive(bool flg) {m_isMovingActive = flg;};
	bool isMovingActive(){return m_isMovingActive;};
	Vector3				m_actualVelocity;
protected:	
	virtual void updateEffectors();
	virtual void updateSensors();
	virtual void updateMemory();
	virtual void checkPerceptionEvents();
	
	virtual void updateVision() = NULL;
	
	Action* setAction(Action* newAction, Action* parentAction);
	void	setActionToEffectors(Action* action, Action* parent);
	
	//create and add memory frame if object is visible
	void	addVisualFrameToMemory(WorldObject* object, bool force = false);
	//sets frame type and time, adds to frame map
	void	addMemoryFrame(ObjectId object, MemoryFrame& frame, bool updateOnly = false);
	
	//enemy selection
	virtual void selectEnemy();
	static float defaultEnemyDangerEvaluator(Actor* actor,WorldObject* enemy);
	
	void		destroyAllActions();

	void		stopAnimations(AnimIdxSet& animToStop);

	Action* 	getResponsibleParent(EffectorID effectorID, Action* parentAction,Actions* parentChain = NULL);


	//debug
	void print(Effector* effector,Action* action,std::ostringstream& buff,std::string& tabs,bool full);

	//current actor state
	State*				m_state;

	//actor effectors
	Action*				m_rootAction;
	EffectorIDVector	m_effectorIDs;
	EffectorMap			m_effectors;

	float				m_pitch;
	float				m_velocity;
	Vector3				m_velocityVector;
	Vector3				m_moveDir;

	ActionSet			m_actionsToInit;
	ActionSet			m_actionsToDelete;
	typedef std::map<Action*,EffectorIDVector> ActivatedEffectorsMap;
	ActivatedEffectorsMap	m_activatedEffectors;

	//memory
	MemoryFrames		m_memoryFrames;
	unsigned int		m_forgetTime;

	//perception
	bool				m_enablePerception;
	//eyesight
	float				m_fov;
	float				m_viewDistance;
	//hearing
	float				m_hearingThreshold;
	
	//current alertness level
	float				m_alertness;

	WorldObject*		m_enemy;
	AttackInfos			m_attackInfos;
	AttackInfo*			m_activeAttackInfo;
	
	
	bool				m_isDead;

	static EnemyEvaluator m_enemyEvaluator;

	Vector3				m_collisionWall;
	Vector3				m_wallAvoidance;
	bool				m_wallCollision;
	Vector3				m_lastMoveDir;

	bool m_isControlledByPlayer;
	bool m_isMovingActive;
};

} //namespace GuideBot

#endif
