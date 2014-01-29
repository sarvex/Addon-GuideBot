//-----------------------------------------------------------------------------
// Guide Bot
// Copyright (C) LogicKing.com, Inc.
//-----------------------------------------------------------------------------


#ifndef _SCRIPT_ACTION_H
#define _SCRIPT_ACTION_H

#include "guideBot/action.h"
#include "guideBot/actionManager.h"
#include "console/scriptObjects.h"

class ScriptActionEvaluator;
class ScriptAction : public ScriptObject, public GuideBot::Action
{
	typedef ScriptObject Parent;
	typedef GuideBot::Action ParentAction;
public:
	DECLARE_CONOBJECT(ScriptAction);

	ScriptAction();
	~ScriptAction();

	//action interface
	virtual GuideBot::Action* _createAction();
	virtual void prepareClone(GuideBot::Action* actionClone);
	virtual bool check(GuideBot::Actor* owner, GuideBot::Action* parentAction);
	virtual void init();
	virtual bool update();
	virtual void terminate(TerminationStatus status = FINISHED);
	virtual void onEffectorsLost(const GuideBot::EffectorIDVector& lostedEffectors);
	virtual void onChildTerminated(Action* terminatedChild);

	static void initPersistFields();
	virtual bool onAdd();

	void addConsecutiveAction(GuideBot::Action* action, const std::string& checkChunk);
	void addAlternativeAction(GuideBot::Action* action, const std::string& scoreEvaluator);

	void setFinishCallback(const std::string& finishCallbackFunctionName);
	void finishCallbackCaller();

protected:
	GuideBot::ConsecutiveControlAction* m_consecutiveAction;
	GuideBot::AlternativeControlAction* m_alternativeAction;
	const char*  m_effectorsStr;

	typedef std::vector<ScriptActionEvaluator*> ScriptActionEvaluators;
	ScriptActionEvaluators	m_evaluators;

	std::string m_finishCallbackFunction;
};

class ScriptActionEvaluator
{
public:
	ScriptActionEvaluator(const std::string& chunkStr);
	GuideBot::ConsecutiveControlAction::CheckResult callScriptChecker(GuideBot::Actor* actor, GuideBot::Action* action);
	int callScriptEvaluator(GuideBot::Actor* actor, GuideBot::Action* action);
protected:
	const char* ScriptActionEvaluator::callScriptChunk(GuideBot::Actor* actor, GuideBot::Action* action);

	std::string m_chunkStr;
};


#endif	//_SCRIPT_ACTION_H