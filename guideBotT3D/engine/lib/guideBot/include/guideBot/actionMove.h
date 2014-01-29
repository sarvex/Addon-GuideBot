//-----------------------------------------------------------------------------
// Guide Bot
// Copyright (C) LogicKing.com, Inc.
//-----------------------------------------------------------------------------

#include "guideBot/action.h"

#ifndef _ACTIONMOVE_H
#define _ACTIONMOVE_H

namespace GuideBot
{

////////////////////////////////////////////////////
//////*************ActionIdle*********//////////////
////////////////////////////////////////////////////
class ActionMove;
class ActionIdle:public Action
{
	typedef Action Parent;
public:
	DEFINE_REGISTER_ROUTINE(ActionIdle);
	void setParams(bool enableSlope);
	virtual void prepareClone(Action* actionClone);

	ActionIdle();
	virtual void init();
	virtual bool update();
protected:
	virtual void onEffectorsLost(const EffectorIDVector& lostEffectors);
	virtual void onEffectorsAcquired(const EffectorIDVector& newEffectors);

	void setMoveAction();
	
	State*	m_idleState;
	State*	m_curState;
	bool m_enableSlopes;

	bool m_checkEnemy;
};

////////////////////////////////////////////////////
//////*************ActionMove*********//////////////
////////////////////////////////////////////////////

class ActionMove:public Action
{
	typedef Action Parent;
public:
	DEFINE_REGISTER_ROUTINE(ActionMove);

	ActionMove();
	virtual void init();
	virtual bool update();
	virtual void terminate(TerminationStatus status = FINISHED);
protected:
	std::string m_currentVisual;
	State*		m_curState;
};

////////////////////////////////////////////////////
//////********ActionMoveToPoint*******//////////////
////////////////////////////////////////////////////

class ActionMoveToPoint:public Action
{
	typedef Action Parent;
public:
	DEFINE_REGISTER_ROUTINE(ActionMoveToPoint);
	ActionMoveToPoint();

	void setParams(const Vector3& destination, float precision = 0.25f);
	virtual void updateDynamicParams(Action* actionClone);

	virtual void init();
	virtual bool update();
	virtual void terminate(TerminationStatus status = FINISHED);

protected:

	float	m_precisionSq;
	Vector3 m_destination;
};

////////////////////////////////////////////////////
//////*******ActionMoveToObject*******//////////////
////////////////////////////////////////////////////

class ActionMoveToObject:public ActionMoveToPoint
{
	typedef ActionMoveToPoint Parent;
public:
	DEFINE_REGISTER_ROUTINE(ActionMoveToObject);
	ActionMoveToObject();

	void setParams(WorldObject* target, float precision = 1.5f);
	virtual void updateDynamicParams(Action* actionClone);

	virtual bool update();
protected:
	WorldObject*			m_targetObject;
	BaseObject::ObjectId	m_targetObjectId;
};

////////////////////////////////////////////////////
//////**********ActionMoveByWaypoints*****//////////
////////////////////////////////////////////////////

class ActionMoveByWaypoints : public Action
{ 
	typedef Action Parent;
public:
	DEFINE_REGISTER_ROUTINE(ActionMoveByWaypoints);
	enum FollowMode {
		FM_DIRECT, 
		FM_CIRCULAR, 
		FM_RANDOM
	};

	ActionMoveByWaypoints();

	void setParams(const Waypoints& waypoints, FollowMode mode = FM_DIRECT, float precision = 0.5);
	virtual void prepareClone(Action* actionClone);
	
	virtual void init();
	
protected:
	virtual void onChildTerminated(Action* terminatedChild);

	void proceedToNextWaypoint(bool toNearest = false);
	bool pickUpNextWaypointIdx();
	void findNearestWaypoint(bool preferVisible = true);

	Waypoints	m_waypoints;
	FollowMode	m_mode;
	float		m_precision;
	
	int			m_curWaypointIdx;

};

////////////////////////////////////////////////////
//////**********ActionManoeuvre*******//////////////
////////////////////////////////////////////////////

class ActionManoeuvre : public Action
{ 
	typedef Action Parent;
public:
	DEFINE_REGISTER_ROUTINE(ActionManoeuvre);
	ActionManoeuvre();

	struct ManoeuvreDesc
	{
		ManoeuvreDesc():	isChasing(true), chaseFarDist(10.f), chaseCloseDist(4.f),
			isFleeing(true), fleeFarDist(9.f), fleeCloseDist(3.f),
			isStrafing(true), strafeMinDist(2.f) ,strafeMaxDist(5.f), strafeChangeDirTime(3000)
		{};
		// Chase
		bool isChasing;
		// if enemy is further than m_chaseFarDist, begin chasing
		float  chaseFarDist;
		// if enemy is closer than m_chaseCloseDist, stop chasing
		float  chaseCloseDist;

		// Flee
		bool isFleeing;
		// if enemy is closer than m_fleeCloseDist, start fleeing
		float  fleeCloseDist;
		// if enemy is further than m_fleeFarDist, stop fleeing
		float  fleeFarDist;

		// Strafe
		bool isStrafing;
		float  strafeMinDist; // unused
		float  strafeMaxDist; // unused
		unsigned long strafeChangeDirTime;
	};

	void setParams(const ManoeuvreDesc& desc);
	void setDefaultParams();
	void setMinMaxDist(float minDist, float maxDist, bool strafe);
	void setTarget(WorldObject* targetObject);

	virtual void updateDynamicParams(Action* actionClone);
	virtual bool update();

	virtual void terminate(TerminationStatus status = FINISHED);
	enum TacticalMovement {None, Chasing, Holding, Fleeing};
	TacticalMovement getTacticalMovement(){return m_tacticalMovement;};
protected:
	
	TacticalMovement m_tacticalMovement;

	ManoeuvreDesc	m_desc;
	ManoeuvreDesc	m_defaultDesc;

	Vector3			m_strafeDir;
	unsigned int 	m_lastStrafeTime;

	WorldObject*	m_targetObject;
	BaseObject::ObjectId	m_targetObjectId;
};

////////////////////////////////////////////////////
//////*************ActionFlee*********//////////////
////////////////////////////////////////////////////

class ActionFlee : public ActionManoeuvre
{ 
	typedef ActionManoeuvre Parent;
public:
	DEFINE_REGISTER_ROUTINE(ActionFlee);
	ActionFlee();
	void setFleeDistance(float distance, float farCloseDelta = 10.f);
};

} //namespace GuideBot

#endif