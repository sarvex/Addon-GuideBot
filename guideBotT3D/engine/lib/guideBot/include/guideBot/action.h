//-----------------------------------------------------------------------------
// Guide Bot
// Copyright (C) LogicKing.com, Inc.
//-----------------------------------------------------------------------------

#ifndef _ACTION_H
#define _ACTION_H

#include "guideBot/effector.h"
#include "guideBot/anchor.h"

#include "guideBot/FastDelegate.h"

namespace GuideBot
{

class WorldObject;
class Actor;
class Action;
class Visual;
class AnimVisual;
class State;
class ConsecutiveControlAction;
class AlternativeControlAction;

////////////////////////////////////////////////////
//////*************ActionLibrary***********/////////
////////////////////////////////////////////////////

class ActionLibrary
{
public:
	~ActionLibrary();
	static ActionLibrary& instance();
	Action* getActionPrototype(const std::string& actionName);
	Action* createAction(const std::string& actionName);
	bool	checkActionPrototype(const char* actionName);
	void	registerAction(const std::string& actionName,Action* prototype,bool dynamic = false);
	void	clear(bool onlyDynamic = false);
	
private:
	ActionLibrary():m_currentID(0) {};
	struct ActionInfo
	{
		ActionInfo(Action* _action,bool _dynamic): action(_action),dynamic(_dynamic) {};
		Action* action;
		bool	dynamic;
	};

	typedef std::map<std::string,ActionInfo> ActionsMap;
	ActionsMap m_actionsMap;

public:
	//for debug
	typedef std::vector<int>  ActionIDVector;
	ActionIDVector m_createdActions;
	int			   m_currentID;
	
};

#define GET_ACTION_PROTOTYPE(action) (static_cast<GuideBot::action*>(GuideBot::ActionLibrary::instance().getActionPrototype(#action)));
#define CREATE_ACTION(action) (static_cast<GuideBot::action*>(GuideBot::ActionLibrary::instance().createAction(#action)));

struct ActionRegistrator
{
	ActionRegistrator(const char* actionName, Action* prototype);
};

#define DEFINE_REGISTER_ROUTINE(className) \
	virtual Action* _createAction()
#define IMPLEMENT_REGISTER_ROUTINE(className)  \
	Action* className::_createAction() { \
	Action*	action = new className();  \
	action->setName(#className); \
	return action; } \
	static ActionRegistrator className##Registrator(#className, new className)


////////////////////////////////////////////////////
//////*************Action*************//////////////
////////////////////////////////////////////////////
class Action;
typedef std::set<Action*> ActionSet;
typedef std::set<int> AnimIdxSet;

class Action
{
public:
	DEFINE_REGISTER_ROUTINE(Action);//{return NULL;};	//Action is abstract so nothing to do in _createAction
	
	enum TerminationStatus 
	{	
		NONE,		//action is not terminated yet
		FINISHED,	//normal termination, called by itself
		CANCELED,	//action was canceled by parent
		INTERUPTED, //termination because of effector loss
	};

	struct EffectorInfo
	{
		EffectorInfo(bool _principal = true,Action* _responsibleChild = NULL,int _priority = -1)
			: principal(_principal), priority(_priority)
		{
			if (_responsibleChild)
				responsibleChilds.insert(_responsibleChild);
		};
		bool principal;
		int  priority;
		ActionSet	responsibleChilds;
	};
	typedef std::map<EffectorID,EffectorInfo> EffectorInfoMap;

	Action();
	virtual ~Action();
	
	//////*****public action virtual interface***//////
	void assignsParents(Actor* owner,Action* parentAction);
	Action* clone();

	//copy data to clone
	virtual void prepareClone(Action* actionClone);
	//copy parameters that can be changed dynamically
	//(i.e. it's not required to reset action)
	virtual void updateDynamicParams(Action* actionClone);

	//Checks whether current action can be set to an actor-owner,
	//mentioning state actor and its effectors.
	virtual bool check(Actor* owner, Action* parentAction);
	// Initialize action. Acquires needed effectors.
	virtual void init();

	virtual bool update();
		
	//Terminate action's execution.
	virtual void terminate(TerminationStatus status = FINISHED);
	

	//////*************routine***********///////
	void					setRequiredEffectors(const EffectorIDVector& effectors, bool principal = true, int priority = -1);
	const EffectorInfoMap&	getRequiredEffectorInfos(){return m_requiredEffectorsInfo;};
	const EffectorIDVector&	getRequiredEffectors(){return m_requiredEffectors;};
	const EffectorIDVector&	getAcquiredEffectors(){return m_acquiredEffectors;};

	void					addRequiredEffector(EffectorID effector,bool principal = true, 
									int priority = -1,Action* responsibleAction = NULL);
	void					addAcquiredEffector(EffectorID effector);
	void					setEffectorPriority(EffectorID effector,int priority);
	int						getEffectorPriority(EffectorID effector);
	bool					hasEffector(EffectorID effector);
	bool					checkPrincipalEffectors();

	void					loseEffectors(const EffectorIDVector& lostEffectors, bool notify = true);
	void					acquireEffectors(const EffectorIDVector& newEffectors);


	bool					hasEffectorResponsibleChild(EffectorID effectorId);
	void					addResponsibleChildForEffector(EffectorID effectorId, Action* child);


	void				setName(const char* name){m_name = name;};
	const std::string&	getName(){return m_name;};
	virtual std::string getDescString(){return m_name;};

	Actor*				getOwner(){return m_owner;};
	Action*				getParentAction(){return m_parentAction;};
	TerminationStatus	getTerminationStatus() {return m_terminationStatus;};
	
	bool				hasChilds(){return !m_childs.empty();};
	void 				getChilds(Actions& childList, bool recursively = false);	
	Action*				getChild(const std::string& actionName);
	bool 				terminateChild(const std::string& actionName);
	
	

	int		getPriority(){return m_priority;};
	void	setPriority(int priority){ m_priority = priority;};

	bool getHighPriorityFlag() { return m_highPriorityFlag;};
	void setHighPriorityFlag(bool flg) { m_highPriorityFlag = flg;};

	ConsecutiveControlAction* addConsecutiveControl();
	AlternativeControlAction* addAlternativeControl();

	//for debug
	virtual void print(std::ostringstream& buff);

	//.todo:
	//bool operator==(const Action& _test) const - compare actions via unique ID

protected:
	
	virtual void onChildTerminated(Action* terminatedChild);
	//Redistribution of effectors happens. Action should decide
	//whether it can proceed.
	virtual void onEffectorsLost(const EffectorIDVector& lostEffectors);
	virtual void onEffectorsAcquired(const EffectorIDVector& newEffectors);

	void	addChild(Action* child);
	void	removeChild(Action* child);

	void releaseChildResponsibleEffectors(Action* child, const EffectorIDVector& childEffectors);
	
	std::string				m_name;
	TerminationStatus		m_terminationStatus;
	Actor*					m_owner;
	int						m_priority;
	Action*					m_parentAction;
	
	//EffectorInfoMap			m_effectorsInfo;
	EffectorInfoMap			m_requiredEffectorsInfo;
	EffectorIDVector		m_requiredEffectors;
	EffectorIDVector		m_acquiredEffectors;

	bool					m_highPriorityFlag;

	typedef std::map<std::string, Actions> ChildMap;
	ChildMap	m_childs;

	//for debug memory leaks
	int m_id;
};



#define GET_CHILD(actionName) static_cast<actionName*>(getChild(#actionName));

} //namespace GuideBot

#endif
