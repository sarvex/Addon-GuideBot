//-----------------------------------------------------------------------------
// Guide Bot
// Copyright (C) LogicKing.com, Inc.
//-----------------------------------------------------------------------------

#ifndef _ACTION_MANAGER_H
#define _ACTION_MANAGER_H

#include "guideBot/action.h"

namespace GuideBot
{

////////////////////////////////////////////////////
//////************ControlAction********/////////////
////////////////////////////////////////////////////

/// Control Action is a base for actions that control execution of other actions.

class ControlAction: public Action
{
	typedef Action Parent;
public:
	DEFINE_REGISTER_ROUTINE(ControlAction);
	ControlAction();
	virtual bool update();
	virtual void terminate(TerminationStatus status = FINISHED);
	
	Action*		getActionOnlyActive(const char* actionName);
	// update dynamic params if this action is active, otherwise return false
	bool		validateAction(Action* action); 

protected:
	virtual void onChildTerminated(Action* terminatedChild);

	Action* getActiveAction();

	Action*				m_activePrototype;
};

////////////////////////////////////////////////////
//////*******AlternativeControlAction*****//////////
////////////////////////////////////////////////////

class AlternativeControlAction: public ControlAction
{
	typedef ControlAction Parent;
public:
	DEFINE_REGISTER_ROUTINE(AlternativeControlAction);

	virtual bool update();
	virtual void terminate(TerminationStatus status = FINISHED);

	typedef Delegate<int (Actor*,Action*)>	ActionScoreEvaluator;
	typedef Delegate<void (Actor*,Action*)>	ActionOnTermination;
	struct ActionCallbacks
	{
		ActionScoreEvaluator evaluator;
		ActionOnTermination	onTermination;
	};

	
	void	addActionDirectly(Action* action, const ActionScoreEvaluator& evaluator = ActionScoreEvaluator());
	Action* addActionByPrototype(Action* prototype, const ActionScoreEvaluator& evaluator = ActionScoreEvaluator());
	Action* addActionByName(const std::string&/*char**/ prototypeName, const ActionScoreEvaluator& evaluator = ActionScoreEvaluator());
	

	void	removeAction(const std::string& actionName);
	
	Action* getAction(const std::string& actionName);
	
	ActionCallbacks* getActionCallbacks(const std::string& actionName);
	ActionCallbacks* getActionCallbacks(Action* action);
	
protected:
	virtual void onChildTerminated(Action* terminatedChild);

	typedef std::map<Action*, ActionCallbacks>	 ActionEvaluatorMap;
	ActionEvaluatorMap	m_prototypes;

};

////////////////////////////////////////////////////
//////*******ConsecutiveControlAction*****//////////
////////////////////////////////////////////////////

class ConsecutiveControlAction: public ControlAction
{
	typedef ControlAction Parent;
public:
	DEFINE_REGISTER_ROUTINE(ConsecutiveControlAction);
	
	ConsecutiveControlAction();

	virtual bool update();
	virtual void terminate(TerminationStatus status = FINISHED);

	enum CheckResult {	CR_STOP = -1,
						CR_SKIP,
						CR_EXECUTE};

	typedef Delegate<CheckResult (Actor*,Action*)>	ActionChecker;
	void	addActionDirectly(Action* action,ActionChecker checker = ActionChecker());
	Action* addActionByPrototype(Action* actionPrototype,ActionChecker checker = ActionChecker());
	Action* addActionByName(const std::string&/*char**/  prototypeName,ActionChecker checker = ActionChecker());
	
	void	removeAction(const std::string& actionName);
	Action* getAction(const std::string& actionName);
	Action* getAction(size_t idx);

	void restart();
	void stop();

	
	typedef Delegate<void ()>	FinishedCallback;
	void setFinishedCallback(FinishedCallback& finishCallback);
	

protected:
	virtual void onChildTerminated(Action* terminatedChild);
	void proceedToNextAction();

	struct PrototypeInfo
	{
		PrototypeInfo():action(NULL) {};
		PrototypeInfo(Action* _action,ActionChecker& _checker):action(_action),checker(_checker) {};
		Action* action;
		ActionChecker checker;
	};
	typedef std::vector<PrototypeInfo> PrototypeInfos;
	PrototypeInfos		m_prototypes;
	bool				m_inExecuted;
	int					m_activeIdx;
	FinishedCallback	m_finishedCallback;
};

#define GET_ACTION(controlAction,actionName) (static_cast<actionName*>(controlAction->getAction(#actionName)))
#define GET_ACTIVE_ACTION(controlAction,actionName) (static_cast<actionName*>(controlAction->getActionOnlyActive(#actionName)))

} //namespace GuideBot

#endif