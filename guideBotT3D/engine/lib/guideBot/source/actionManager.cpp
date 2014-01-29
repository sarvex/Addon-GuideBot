//-----------------------------------------------------------------------------
// Guide Bot
// Copyright (C) LogicKing.com, Inc.
//-----------------------------------------------------------------------------

#include "guideBot/actionManager.h"
#include "guideBot/actor.h"

using namespace GuideBot;

////////////////////////////////////////////////////
//////*******ControlAction**********////////////////
////////////////////////////////////////////////////

IMPLEMENT_REGISTER_ROUTINE(ControlAction);

ControlAction::ControlAction():	m_activePrototype(NULL)
{

}

bool ControlAction::update()
{
	return true;
}

void ControlAction::terminate(TerminationStatus status /*= FINISHED*/)
{
	Parent::terminate(status);
	m_activePrototype = NULL;
}

void ControlAction::onChildTerminated(Action* terminatedChild)
{
	m_activePrototype = NULL;
}

Action* ControlAction::getActionOnlyActive(const char* actionName)
{
	if (m_activePrototype && m_activePrototype->getName() == actionName)
		return getActiveAction();
	return NULL;
}

Action* ControlAction::getActiveAction()
{
	Action* action = m_childs.empty() ? NULL : m_childs.begin()->second.front();
	return action;
}

bool ControlAction::validateAction(Action* action)
{
	if (m_activePrototype ==  action)
	{
		Action* currentAction = getChild(m_activePrototype->getName());
		GB_ASSERT(currentAction,"ControlAction::validateAction:  action for active prototype must be executed");
		m_activePrototype->updateDynamicParams(currentAction);
		return true;
	}
	return false;
}

////////////////////////////////////////////////////
//////*******AlternativeControlAction*****//////////
////////////////////////////////////////////////////

IMPLEMENT_REGISTER_ROUTINE(AlternativeControlAction);

bool AlternativeControlAction::update()
{
	if (!Parent::update())
		return false;

	Action* newPrototype = NULL;
	int bestScore = 0;
	for (ActionEvaluatorMap::iterator it = m_prototypes.begin(), end = m_prototypes.end();
		it!=end; it++)
	{
		int score = it->second.evaluator.empty() ? 1 : it->second.evaluator(m_owner,it->first);
		if (score >= bestScore)
		{
			bestScore = score;
			newPrototype = it->first;
		}
	}

	if (newPrototype != m_activePrototype)
	{
		Action*	currentAction = getActiveAction();
		if (m_activePrototype && currentAction)
		{
			currentAction->terminate(CANCELED);
		}

		m_activePrototype = newPrototype;

		if (m_activePrototype)
		{
			m_owner->setActionByPrototype(m_activePrototype,this);
		}		
	}

	return true;
}

void AlternativeControlAction::terminate(TerminationStatus status /*= FINISHED*/)
{
	Parent::terminate(status);
	//delete all prototypes
	for (ActionEvaluatorMap::iterator it = m_prototypes.begin(), end = m_prototypes.end();
		it!=end; it++)
	{
		Action* prototype = it->first;
		GB_SAFE_DELETE(prototype);
	}
	m_prototypes.clear();
}

Action* AlternativeControlAction::addActionByName(const std::string&/*char**/  prototypeName, 
											const ActionScoreEvaluator& evaluator /*= ActionScoreEvaluator()*/)
{
	Action* prototype = ActionLibrary::instance().getActionPrototype(prototypeName);
	Action* ownActionPrototype = addActionByPrototype(prototype,evaluator);;
	return ownActionPrototype;
}

Action* AlternativeControlAction::addActionByPrototype(Action* actionPrototype,
											const ActionScoreEvaluator& evaluator /*= ActionScoreEvaluator()*/)
{
	//make copy of prototype - don't use "shared" prototype from action manager
	//(otherwise custom parameters will be lost)
	Action* ownActionPrototype = actionPrototype->clone();
	addActionDirectly(ownActionPrototype,evaluator);
	return ownActionPrototype;
}

void AlternativeControlAction::addActionDirectly(Action* action, 
										const ActionScoreEvaluator& evaluator /*= ActionScoreEvaluator()*/)
{
	ActionCallbacks& callbacks = m_prototypes[action];
	callbacks.evaluator = evaluator;
}

void AlternativeControlAction::removeAction(const std::string& actionName)
{
	for (ActionEvaluatorMap::iterator it = m_prototypes.begin(), end = m_prototypes.end();
		it!=end; it++)
	{
		if (it->first->getName()==actionName)
		{
			if (m_activePrototype == it->first)
			{
				Action*	currentAction = getActiveAction();
				if (currentAction)
				{
					currentAction->terminate(CANCELED);
				}
			}
			m_prototypes.erase(it);
			break;
		}
	}
}

Action* AlternativeControlAction::getAction(const std::string& actionName)
{
	for (ActionEvaluatorMap::iterator it = m_prototypes.begin(), end = m_prototypes.end();
		it!=end; it++)
	{
		if (it->first->getName()==actionName)
		{
			return it->first;
		}
	}
	return NULL;
}

AlternativeControlAction::ActionCallbacks* AlternativeControlAction::getActionCallbacks(const std::string& actionName)
{
	Action* action = getAction(actionName);
	return getActionCallbacks(action);
}

AlternativeControlAction::ActionCallbacks* AlternativeControlAction::getActionCallbacks(Action* action)
{
	if (m_prototypes.find(action)==m_prototypes.end())
		return NULL;
	else
		return &m_prototypes[action];
}

void AlternativeControlAction::onChildTerminated(Action* terminatedChild)
{
	ActionCallbacks* callbacks =  getActionCallbacks(m_activePrototype);
	if (callbacks && !callbacks->onTermination.empty())
		callbacks->onTermination(m_owner,terminatedChild);

	Parent::onChildTerminated(terminatedChild);
}
////////////////////////////////////////////////////
//////*******ConsecutiveControlAction****///////////
////////////////////////////////////////////////////
IMPLEMENT_REGISTER_ROUTINE(ConsecutiveControlAction);

ConsecutiveControlAction::ConsecutiveControlAction():	m_activeIdx(-1) 
														, m_inExecuted(false)
{

}

bool ConsecutiveControlAction::update()
{
	if (!Parent::update())
		return false;

	if (m_inExecuted && !getActiveAction())
		proceedToNextAction();

	return true;
}

void ConsecutiveControlAction::terminate(TerminationStatus status /*= FINISHED*/)
{
	Parent::terminate(status);

	//destroy all prototypes
	for (size_t i=0; i<m_prototypes.size();	i++)
	{
		Action* prototype = m_prototypes[i].action;
		GB_SAFE_DELETE(prototype);
	}
	m_prototypes.clear();
}

Action* ConsecutiveControlAction::addActionByName(const std::string&/*char**/ prototypeName, ActionChecker checker)
{
	Action* prototype = ActionLibrary::instance().getActionPrototype(prototypeName);
	Action* ownActionPrototype = addActionByPrototype(prototype,checker);
	return ownActionPrototype;
}

Action* ConsecutiveControlAction::addActionByPrototype(Action* actionPrototype, ActionChecker checker)
{
	//make copy of prototype - don't use "shared" prototype from action library
	//(otherwise custom parameters will be lost)
	Action* ownActionPrototype = actionPrototype->clone();
	addActionDirectly(ownActionPrototype,checker);
	return ownActionPrototype;
}

void ConsecutiveControlAction::addActionDirectly(Action* action, ActionChecker checker)
{	
	m_prototypes.push_back(PrototypeInfo(action,checker));
}

void ConsecutiveControlAction::removeAction(const std::string& actionName)
{
	for (size_t i=0; i < m_prototypes.size(); i++)
	{
		Action* action = m_prototypes[i].action;
		if (action->getName()==actionName)
		{
			m_prototypes.erase(m_prototypes.begin()+i);

			if (m_activePrototype == action)
			{
				Action*	currentAction = getActiveAction();
				if (currentAction)
				{
					currentAction->terminate(CANCELED);
				}
			}

			if (i>=m_activeIdx)
				m_activeIdx--;

			break;
		}
	}
}

Action* ConsecutiveControlAction::getAction(size_t idx)
{
	if (idx<0 || idx>m_prototypes.size())
		return NULL;
	Action* action = m_prototypes[idx].action;
	return action;
}

Action* ConsecutiveControlAction::getAction(const std::string& actionName)
{
	for (size_t i=0; i<m_prototypes.size();	i++)
	{
		Action* action = m_prototypes[i].action;
		if (action->getName()==actionName)
		{
			return action;
		}
	}
	return NULL;
}

void ConsecutiveControlAction::proceedToNextAction()
{
	bool setted = false;
	CheckResult res = CR_EXECUTE;
	while(!setted)
	{
		m_activeIdx++;
		if (m_activeIdx<m_prototypes.size())
		{
			Action* prototype = m_prototypes[m_activeIdx].action;
			
			if (!m_prototypes[m_activeIdx].checker.empty())
			{
				res = m_prototypes[m_activeIdx].checker(m_owner,prototype);
			}
			else
			{
				res = CR_EXECUTE;
			}

			if (res == CR_EXECUTE)
			{
				m_activePrototype = prototype;
				setted = m_owner->setActionByPrototype(m_activePrototype,this) != NULL;
			}
		}
		else
			res = CR_STOP;
		
		if (res == CR_STOP)
		{
			stop();
			return;
		}
	}
}


void ConsecutiveControlAction::restart()
{
	Action*	activeAction = getActiveAction();
	if (activeAction && !activeAction->getHighPriorityFlag())
	{
		activeAction->terminate(Action::CANCELED);
	}
	m_activeIdx = -1;
	m_inExecuted = true;
}

void ConsecutiveControlAction::stop()
{
	m_inExecuted = false;
	if (!m_finishedCallback.empty())
		m_finishedCallback();
	
}

void ConsecutiveControlAction::setFinishedCallback(FinishedCallback& finishCallback)
{
	m_finishedCallback = finishCallback;
}

void ConsecutiveControlAction::onChildTerminated(Action* terminatedChild)
{
	Parent::onChildTerminated(terminatedChild);
	if (terminatedChild->getTerminationStatus()==INTERUPTED)
		stop();
}