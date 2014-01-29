//-----------------------------------------------------------------------------
// Guide Bot
// Copyright (C) LogicKing.com, Inc.
//-----------------------------------------------------------------------------

#ifndef _ACTIONFIGHT_H
#define _ACTIONFIGHT_H

#include "guideBot/actionMove.h"
#include "guideBot/actionManager.h"

namespace GuideBot
{

////////////////////////////////////////////////////
//////********ActionAvoidDanger******//////////////
////////////////////////////////////////////////////

class ActionAvoidDanger: public Action
{
	typedef Action Parent;
public:
	DEFINE_REGISTER_ROUTINE(ActionAvoidDanger);
	ActionAvoidDanger();
	virtual void init();
	virtual bool update();

	virtual void terminate(TerminationStatus status  = FINISHED);
	void setFleeState(const std::string& state);
protected:
	virtual void onChildTerminated(Action* terminatedChild);
	float m_fleeCloseDist;
	float m_fleeFarDist;
	ActionFlee* m_fleePrototype;
	ActionFlee* m_actionFlee;
	std::string	m_fleeState;
};	


////////////////////////////////////////////////////
//////************ActionFight*********//////////////
////////////////////////////////////////////////////

class AttackInfo : public BaseObject
{
public:
	AttackInfo(): minDistSq(0.f), maxDistSq(0.f), fov(0.f), periodicity(0), 
		pause(0), blockMovements(false),manoeuvreMinDist(2.f), manoeuvreMaxDist(10.f), strafe(true) ,lookAtEnemy(true) {};
	//name (used for debug)
	std::string name;

	//min distance to enemy for attack execution
	float minDistSq;
	//max distance to enemy for attack execution
	float maxDistSq;
	//max fov of enemy for attack execution
	float fov;
	//periodicity of attack (choose attack one time in period)
	unsigned int periodicity;
	//manoeuvre param: minimum distance
	float manoeuvreMinDist;
	//manoeuvre param: maximum distance
	float manoeuvreMaxDist;
	//manoeuvre param: enable strafe movement
	bool  strafe;
	//pause between attack action (i.e."shoots")
	unsigned int pause;
	//block movements during attack action
	bool blockMovements;
	//attack evaluator
	typedef Delegate<float (Actor*, WorldObject*, float, float, AttackInfo*, unsigned int, bool)> AttackEvaluator;
	AttackEvaluator evaluator;
	//attack action to be called
	std::string action;
	//attack weapon
	std::string weapon;
	//background attack state
	std::string state;
	//attack cover state
	std::string coverState;
	//attack action state
	std::string attackState;
	//look at the enemy during attack(define lookAt mode)
	bool lookAtEnemy;
};
typedef std::vector<AttackInfo*> AttackInfos;

class ActionAttack;
class ActionLookAt;
class ActionFight: public Action
{
	typedef Action Parent;
public:
	DEFINE_REGISTER_ROUTINE(ActionFight);
	ActionFight();

	void setParams();

	virtual void prepareClone(Action* actionClone);
	virtual void init();
	virtual bool update();
	virtual void terminate(TerminationStatus status = FINISHED);
	
	AttackInfo* getAttackInfo(){ return m_curAttackInfo;};
	void onActionAttackExecuted(AttackInfo* executedAttackInfo);
protected:
	virtual void onEffectorsLost(const EffectorIDVector& lostEffectors);
	virtual void onEffectorsAcquired(const EffectorIDVector& newEffectors);

	virtual void onChildTerminated(Action* terminatedChild);

	void onEnemyChanged(WorldObject* newEnemy);
	void handleLookDirMode();
	AttackInfo* selectBestAttack();
	float defaultAttackEvaluator(WorldObject* enemy, float distSq, float fov, AttackInfo* attackInfo,
								unsigned int lastExecutionTime, bool isActive);

	WorldObject*	m_enemy;
	AttackInfo*		m_curAttackInfo;
	unsigned int	m_lastAttackSelectedTime;

	typedef std::map<AttackInfo*, unsigned int> AttackInfoTimeTable;
	AttackInfoTimeTable	m_attackInfoTimeTable;

	Anchor*			m_anchor;
	Vector3			m_coverPos; 
	ActionMoveToPoint*			m_movingToPoint;
	
	ActionLookAt*	m_lookAtAction;

	AlternativeControlAction*	m_moveManager;
};

const ActionFight fight;

////////////////////////////////////////////////////
//////************ActionAttack********//////////////
////////////////////////////////////////////////////
class ActionAttack: public Action
{
	typedef Action Parent;
public:
	DEFINE_REGISTER_ROUTINE(ActionAttack);
	ActionAttack();
	void init();
	virtual bool update();
	void onAttackChanged(AttackInfo* attackInfo);
	void onAttackEnd();
protected:
	virtual void onChildTerminated(Action* terminatedChild);

	AttackInfo*		m_attackInfo;
	AttackInfo*		m_activeAttackInfo;
	Action*			m_attackAction;
	unsigned int	m_lastAttackTime;
	ConsecutiveControlAction*	m_manager;
};

////////////////////////////////////////////////////
//////************ActionCover*********//////////////
////////////////////////////////////////////////////
class ActionCover: public Action
{
	typedef Action Parent;
public:
	DEFINE_REGISTER_ROUTINE(ActionCover);
	ActionCover();
	virtual void updateDynamicParams(Action* actionClone);
	virtual void init();
	virtual bool update();
	virtual void terminate(TerminationStatus status = FINISHED);

	static int scoreEvaluator(Actor* actor,Action* actionCover);
	void onCovered();
	void onAttackChanged(AttackInfo* attackInfo);
	void handleCoverStateChange();

	/*	
	bool fromCoverChecker(Actor* owner, Action* action);
	bool toCoverChecker(Actor* owner, Action* action);
	*/
protected:
	virtual void onChildTerminated(Action* terminatedChild);

	float anchorRankEvaluator(Anchor* anchor);
	void updateAnchorStat(Anchor* anchor,unsigned int delta);
	Anchor*			m_anchor;
	unsigned int	m_lastAnchorUpdateTime;
	Vector3			m_coverPos; 
	AttackInfo*		m_attackInfo;
	ConsecutiveControlAction*	m_manager;

	typedef std::map<Anchor*, unsigned int>	 AnchorStatMap;
	AnchorStatMap	m_anchorStat;
	std::string		m_normalState;
};

} //namespace GuideBot

#endif