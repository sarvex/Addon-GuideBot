//-----------------------------------------------------------------------------
// Guide Bot
// Copyright (C) LogicKing.com, Inc.
//-----------------------------------------------------------------------------

#include "T3D/logickingMechanics/guideBot/scriptAction.h"
#include "T3D/logickingMechanics/guideBot/enhancedPlayer.h"

IMPLEMENT_CONOBJECT(ScriptAction);


ScriptAction::ScriptAction():	m_effectorsStr(NULL)
								, m_consecutiveAction(NULL)								
								, m_alternativeAction(NULL)								
{

}

ScriptAction::~ScriptAction()
{
	//carefully, ScriptAction cannot be deleted from SimSet destructor, only from Actor or ActionLibrary
	//for SimObject correct deleting
	unregisterObject();	

	for (size_t i=0; i<m_evaluators.size(); i++)
	{
		ScriptActionEvaluator* evaluator = m_evaluators[i];
		GB_SAFE_DELETE(evaluator);
	}
}

void ScriptAction::initPersistFields()
{
	Parent::initPersistFields();
	addField("effectors", TypeCaseString, Offset(m_effectorsStr, ScriptAction));
	addField("priority", TypeS8, Offset(m_priority, ScriptAction));
}

bool ScriptAction::onAdd()
{
	if (!Parent::onAdd())
		return false;
	if (m_effectorsStr)
	{
		GuideBot::EffectorIDVector effectors;
		GuideBot::Effector::convertFromString(m_effectorsStr,effectors);
		setRequiredEffectors(effectors);
	}
	
	return true;
}
GuideBot::Action* ScriptAction::_createAction()
{
	std::string actionName = getClassNamespace();//ParentAction::getName();
	std::string createFunc = "create";
	createFunc.append(actionName);
	const char* argv[1];
	argv[0] = createFunc.c_str();
	const char* buf = Con::execute(1, argv);
	ScriptAction * action = dynamic_cast<ScriptAction *>(Sim::findObject(buf));
	if (!action)
		Con::errorf("ScriptAction::_createAction: can't create action %s",actionName.c_str());

	action->setName(ParentAction::getName().c_str());//actionName.c_str());
	return action;
}
void ScriptAction::prepareClone(Action* actionClone)
{
	ParentAction::prepareClone(actionClone);
	
	ScriptAction* scriptActionClone = static_cast<ScriptAction*>(actionClone);
	Con::executef(this, "prepareClone", scriptActionClone->getIdString());
}

bool ScriptAction::check(GuideBot::Actor* owner, GuideBot::Action* parentAction)
{
	if (!ParentAction::check(owner,parentAction))
		return false;
	bool res = true;
	
	if (isMethod("check"))
	{
		EnhancedPlayer* enhancedPlayer = dynamic_cast<EnhancedPlayer*>(owner);
		ScriptAction* parentScriptAction = parentAction ? dynamic_cast<ScriptAction*>(parentAction) : NULL;
		res = dAtob(Con::executef(this, "check", enhancedPlayer->getIdString(), 
						parentScriptAction ? parentScriptAction->getIdString():""));
	}
	
	return res;
}

void ScriptAction::init()
{
	ParentAction::init();
	//initialize control actions by demand
	
	//execute script inin function
	Con::executef(this, "init");
}

bool ScriptAction::update()
{
	if (!ParentAction::update())
		return false;
	Con::executef(this, "update");
	return true;
}

void ScriptAction::terminate(TerminationStatus status /*= FINISHED*/)
{
	Con::executef(this, "onTerminate",Con::getIntArg(status==INTERUPTED ? 1 : 0));
	ParentAction::terminate(status);
}

void ScriptAction::onEffectorsLost(const GuideBot::EffectorIDVector& lostedEffectors)
{
	size_t effectorsNum = lostedEffectors.size();
	std::string effectorsStr;
	GuideBot::Effector::convertToString(lostedEffectors,effectorsStr);
	Con::executef(this, "onEffectorsLost",Con::getIntArg((S32)effectorsNum),effectorsStr.c_str());
}

void ScriptAction::onChildTerminated(Action* terminatedChild)
{
	ScriptAction* terminatedScriptChild = terminatedChild ? dynamic_cast<ScriptAction*>(terminatedChild) : NULL;
	//.todo: status in scripts
	Con::executef(this,"onChildTerminated",terminatedScriptChild ? terminatedScriptChild->getIdString() : "",
		Con::getIntArg(terminatedChild->getTerminationStatus()==INTERUPTED ? 1 : 0)); 
}

void ScriptAction::addConsecutiveAction(Action* action, const std::string& checkChunk)
{
	if (!m_consecutiveAction)
	{
		m_consecutiveAction = addConsecutiveControl();
		m_consecutiveAction->restart();
	}

	GuideBot::ConsecutiveControlAction::ActionChecker checker;
	if (!checkChunk.empty())
	{
		ScriptActionEvaluator* caller = new ScriptActionEvaluator(checkChunk);
		m_evaluators.push_back(caller);
		checker.bind(caller,&ScriptActionEvaluator::callScriptChecker);
	}

	m_consecutiveAction->addActionByPrototype(action,checker);
	
}

void ScriptAction::addAlternativeAction(GuideBot::Action* action, const std::string& scoreEvaluatorStr)
{
	if (!m_alternativeAction)
		m_alternativeAction = addAlternativeControl();

	GuideBot::AlternativeControlAction::ActionScoreEvaluator scoreEvaluator;
	if (!scoreEvaluatorStr.empty())
	{
		ScriptActionEvaluator* caller = new ScriptActionEvaluator(scoreEvaluatorStr);
		m_evaluators.push_back(caller);
		scoreEvaluator.bind(caller,&ScriptActionEvaluator::callScriptEvaluator);
	}
	m_alternativeAction->addActionByPrototype(action,scoreEvaluator);	
}

void ScriptAction::setFinishCallback(const std::string& finishCallbackFunctionName)
{
	if (!m_consecutiveAction)
		return;
	m_finishCallbackFunction = finishCallbackFunctionName;
	GuideBot::ConsecutiveControlAction::FinishedCallback callback; 
	callback.bind(this,&ScriptAction::finishCallbackCaller);
	m_consecutiveAction->setFinishedCallback(callback);
}

void ScriptAction::finishCallbackCaller()
{
	if (m_finishCallbackFunction.empty())
		return;
	Con::evaluatef("%s(%d);",m_finishCallbackFunction.c_str(),getId());
}

const char* evaluateChunk(const char* chunkStr, const char* action, const char* owner)
{
	static char caller[1024];
	//dSprintf(caller, 1024, " function chunkExecuteFunction(%%this, %%owner) { %s } chunkExecuteFunction(%s, %s); ",chunkStr, action, owner );
	dSprintf(caller, 1024, "%%this=%s; %%owner=%s; %s",action, owner,chunkStr);
	const char* res = Con::evaluate(caller);
	return res;
}

ScriptActionEvaluator::ScriptActionEvaluator(const std::string& chunkStr): m_chunkStr(chunkStr)
{
	
}


const char* ScriptActionEvaluator::callScriptChunk(GuideBot::Actor* actor, GuideBot::Action* action)
{
	EnhancedPlayer* enhancedPlayer = dynamic_cast<EnhancedPlayer*>(actor);
	ScriptAction* scriptAction = dynamic_cast<ScriptAction*>(action);
	const char* returnBuf = evaluateChunk(m_chunkStr.c_str(), scriptAction ? scriptAction->getIdString():"",
		enhancedPlayer->getIdString());
	return returnBuf;
}

int ScriptActionEvaluator::callScriptEvaluator(GuideBot::Actor* actor, GuideBot::Action* action)
{
	const char* returnBuf  = callScriptChunk(actor,action);
	int res = dAtoi(returnBuf);
	return res;
}


GuideBot::ConsecutiveControlAction::CheckResult ScriptActionEvaluator::callScriptChecker(GuideBot::Actor* actor, 
																						 GuideBot::Action* action)
{
	const char* returnBuf  = callScriptChunk(actor,action);
	GuideBot::ConsecutiveControlAction::CheckResult res = 
		(GuideBot::ConsecutiveControlAction::CheckResult)dAtoi(returnBuf);
	return res;
}


/////////////////////////////////////////////////////////////////
ConsoleFunction( registerAction, void, 2, 3, "(ScriptAction* action, [actionName])")
{
	ScriptAction * action = dynamic_cast<ScriptAction*>(Sim::findObject(argv[1]));
	if (!action)
	{
		Con::errorf("ActionLibrary::RegisterAction: can't register action %s",argv[1]);
		return;
	}
	std::string	actionName; 
	if (argc == 3)
	{
		actionName = argv[2];
	}
	else
		actionName = action->getClassNamespace();
	action->setName(actionName.c_str());
	GuideBot::ActionLibrary::instance().registerAction(actionName,action,true);
}

ConsoleFunction( getActionPrototype,const char*,2,3,"actionName, bool createNew")
{
	ScriptAction* action = NULL;
	if (argc==3 && dAtob(argv[2]))
	{
		action = dynamic_cast<ScriptAction*>(GuideBot::ActionLibrary::instance().createAction(argv[1]));
	}
	else
		action = dynamic_cast<ScriptAction*>(GuideBot::ActionLibrary::instance().getActionPrototype(argv[1]));

	if (!action)
	{	
		Con::errorf("ActionLibrary::getActionPrototype: can't create script action prototype %s",argv[1]);
		return NULL;
	}
	return action->getIdString();
}

ConsoleFunction( clearActionManager,void ,1,2,"bool onlyScriptActions")
{
	GuideBot::ActionLibrary::instance().clear(argc==2 ? dAtoi(argv[1]) : false );
}

ConsoleMethod(ScriptAction,getActionName,const char*,2,2,"")
{
	return object->GuideBot::Action::getName().c_str();
}

ConsoleMethod(ScriptAction,getOwner,const char*,2,2,"")
{
	GuideBot::Actor* actor = object->getOwner();
	EnhancedPlayer* player = static_cast<EnhancedPlayer*>(actor);
	return player->getIdString();
}

ConsoleMethod(ScriptAction,terminate,void,2,3,"bool immediate")
{
	object->terminate(argc == 3 && argv[2] ? GuideBot::Action::CANCELED : GuideBot::Action::FINISHED);
}

ConsoleMethod(ScriptAction,setRequiredEffectors,void,3,3,"effectorsName")
{
	GuideBot::EffectorIDVector effectors;
	if (!argv[2] || !GuideBot::Effector::convertFromString(argv[2],effectors))
	{
		Con::errorf("ScriptAction::setRequiredEffectors: can't set required effectors %s",argv[2] ? argv[2] : "");
		return;
	}
	
	object->setRequiredEffectors(effectors);
}

ConsoleMethod(ScriptAction,getAcquiredEffectors,const char*,2,2,"")
{
	std::string effectorsStr;
	GuideBot::Effector::convertToString(object->getAcquiredEffectors(),effectorsStr);

	return effectorsStr.c_str();
}

ConsoleMethod(ScriptAction,hasChilds,bool,2,2,"return count of children")
{
	bool res = object->hasChilds();
	return res;
}

ConsoleFunction(evaluateChunk,void,4,4,"chunk, action, owner")
{
	evaluateChunk(argv[1], argv[2], argv[3]);
}

ConsoleMethod(ScriptAction,addConsecutiveAction,void,4,4,"action, checker")
{
	ScriptAction * action = dynamic_cast<ScriptAction*>(Sim::findObject(argv[2]));
	if (!action)
	{
		Con::errorf("ScriptAction::addConsecutiveAction: can't register action %s",argv[2]);
		return;
	}
	object->addConsecutiveAction(action,argv[3]);
}

ConsoleMethod(ScriptAction,addAlternativeAction,void,4,4,"action, evaluator")
{
	ScriptAction * action = dynamic_cast<ScriptAction*>(Sim::findObject(argv[2]));
	if (!action)
	{
		Con::errorf("ScriptAction::addAlternativeAction: can't register action %s",argv[2]);
		return;
	}
	object->addAlternativeAction(action,argv[3]);
}

ConsoleMethod(ScriptAction,setFinishCallback,void,3,3,"callbackFunctionName")
{
	object->setFinishCallback(argv[2]);
}
