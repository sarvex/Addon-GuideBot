//-----------------------------------------------------------------------------
// Guide Bot
// Copyright (C) LogicKing.com, Inc.
//-----------------------------------------------------------------------------

#ifndef _ACTIONS_H
#define _ACTIONS_H


#include "guideBot/action.h"

namespace GuideBot
{

////////////////////////////////////////////////////
//////*********ActionChangeState******//////////////
////////////////////////////////////////////////////

class ActionChangeState: public Action
{
	typedef Action Parent;
public:
	DEFINE_REGISTER_ROUTINE(ActionChangeState);
	ActionChangeState();
	virtual void setParams(const std::string& stateName);

	virtual void prepareClone(Action* actionClone);
	virtual bool check(Actor* owner, Action* parentAction);
	virtual void init();
	virtual void onChildTerminated(Action* terminatedChild);
	virtual void terminate(TerminationStatus status = FINISHED);

	void print(std::ostringstream& buff);
	virtual std::string getDescString();
protected:
	State*		m_state;
	Visual*		m_changeVisual;
	std::string m_stateName;
	bool		m_changeOnStart;
};

////////////////////////////////////////////////////
//////***********ActionLookAt*********//////////////
////////////////////////////////////////////////////

class ActionLookAt : public Action
{ 
	typedef Action Parent;
public:
	DEFINE_REGISTER_ROUTINE(ActionLookAt);
	ActionLookAt();

	enum LookDirMode {	L_FORWARD, L_POINT, L_DIR, L_OBJECT };
	void setParams(LookDirMode mode,const Vector3* point = NULL, WorldObject* object = NULL);
	virtual void setDirection(const Vector3& vec);
	virtual void updateDynamicParams(Action* actionClone);
	virtual void init();
	virtual bool update();
	virtual void terminate(TerminationStatus status = FINISHED);
protected:
	LookDirMode		m_mode;
	Vector3			m_targetPoint;
	WorldObject*	m_targetObject;
	Vector3			m_direction;
};

////////////////////////////////////////////////////
//////************ActionDeath*********//////////////
////////////////////////////////////////////////////
class ActionDeath: public Action
{
	typedef Action Parent;
public:
	DEFINE_REGISTER_ROUTINE(ActionDeath);
	ActionDeath();
	virtual void init();
	virtual void onChildTerminated(Action* terminatedChild);
protected:

};

////////////////////////////////////////////////////
//////************ActionSlope*********//////////////
////////////////////////////////////////////////////
class ActionPlayVisual;
class ActionSlope: public Action
{
	typedef Action Parent;
public:
	DEFINE_REGISTER_ROUTINE(ActionSlope);
	ActionSlope();

	virtual void init();
	virtual bool update();
	
	ActionPlayVisual* getSlopeVisual();
protected:
	virtual void onEffectorsAcquired(const EffectorIDVector& newEffectors);

	void playSlopeVisual();
	void updateSlopeValue();

	State*	m_state;
	float 	m_pitch;
};

////////////////////////////////////////////////////
//////************ActionPause*********//////////////
////////////////////////////////////////////////////
class ActionPause: public Action
{
	typedef Action Parent;
public:
	DEFINE_REGISTER_ROUTINE(ActionPause);
	ActionPause();
	void setDuration(GuideBot::Time duration);
	
	virtual void updateDynamicParams(Action* actionClone);

	virtual void init();
	virtual bool update();
protected:
	Time m_duration;
	Time m_initTime;
};

} //namespace GuideBot

#endif