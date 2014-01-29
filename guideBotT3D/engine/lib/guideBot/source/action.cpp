//-----------------------------------------------------------------------------
// Guide Bot
// Copyright (C) LogicKing.com, Inc.
//-----------------------------------------------------------------------------

#include "guideBot/action.h"
#include "guideBot/actorState.h"
#include "guideBot/actor.h"
#include "guideBot/stringUtils.h"


#include <algorithm>
#include "float.h"

using namespace GuideBot;

#define	POSITION_DELTA 0.01f

////////////////////////////////////////////////////
//////*************ActionLibrary***********/////////
////////////////////////////////////////////////////
ActionLibrary& ActionLibrary::instance()
{
	static ActionLibrary self;
	return self;
}
ActionLibrary::~ActionLibrary() 
{
	//for debug
	if (!m_createdActions.empty())
	{
		std::stringstream debugStr;
		for(ActionIDVector::iterator it = m_createdActions.begin(), end = m_createdActions.end(); it!=end; it++)
		{
			debugStr<<*it<<" ";
		}
		GB_ERROR("Not deleted actions: %s",debugStr.str().c_str());
	}
	GB_WARN(m_actionsMap.empty() && m_createdActions.empty(),"ActionLibrary: some actions were not deleted. Call ActionLibrary::clear() before exit");
}

bool ActionLibrary::checkActionPrototype(const char* actionName)
{
	std::string name = actionName;
	bool res = m_actionsMap.find(name)!=m_actionsMap.end();
	return res;
}

Action* ActionLibrary::getActionPrototype(const std::string& actionName)
{
	std::string name = actionName;
	ActionsMap::iterator it = m_actionsMap.find(name);
	GB_WARN(it != m_actionsMap.end(),
					format("ActionLibrary doesn't have action %s",actionName.c_str()));
	return (it != m_actionsMap.end()) ? it->second.action : NULL;
}

Action* ActionLibrary::createAction(const std::string& actionName)
{
	Action* prototype = getActionPrototype(actionName);
	Action* newAction = prototype->clone();
	return newAction;
	
}

void ActionLibrary::registerAction(const std::string& actionName,Action* prototype,bool dynamic)
{
	ActionsMap::iterator it = m_actionsMap.find(actionName);
	if (it != m_actionsMap.end())
	{
		GB_ASSERT(dynamic, format("ActionLibrary already have create func for action %s",actionName.c_str()));
		Action* oldAction = it->second.action;
		GB_SAFE_DELETE(oldAction);
		m_actionsMap.erase(it);
	}

	m_actionsMap.insert(std::make_pair(actionName,ActionInfo(prototype,dynamic)));
}

void ActionLibrary::clear(bool onlyDynamic)
{
	std::set<std::string> dynamicActions;

	for (ActionsMap::iterator it = m_actionsMap.begin(),end = m_actionsMap.end();it!=end;it++)	
	{
		if (onlyDynamic)
		{
			if (!it->second.dynamic)
				continue;
			dynamicActions.insert(it->first);
		}

		Action* action = it->second.action;
		GB_SAFE_DELETE(action);
	}

	if (onlyDynamic)
	{
		for(std::set<std::string>::iterator it = dynamicActions.begin(),end = dynamicActions.end(); it!=end; it++)
		{
			ActionsMap::iterator actionIt = m_actionsMap.find(*it);
			m_actionsMap.erase(actionIt);
		}
	}
	else
	{
		m_actionsMap.clear();
	}
}

ActionRegistrator::ActionRegistrator(const char* actionName, Action* prototype)
{
	prototype->setName(actionName);
	ActionLibrary::instance().registerAction(std::string(actionName),prototype);
}

////////////////////////////////////////////////////
//////*************Action*************//////////////
////////////////////////////////////////////////////
IMPLEMENT_REGISTER_ROUTINE(Action);

void Action::print(std::ostringstream& buff)
{
	buff<<getName()<<std::endl;
}


Action::Action():	m_owner(NULL) 
					, m_parentAction(NULL)
					, m_priority(1) 
					, m_highPriorityFlag(false)
					, m_terminationStatus(NONE)
{
	//for debug
	m_id = ActionLibrary::instance().m_currentID;
	ActionLibrary::instance().m_createdActions.push_back(m_id);
}

Action::~Action() 
{
	//for debug
	ActionLibrary::ActionIDVector& createdActions = ActionLibrary::instance().m_createdActions;
	ActionLibrary::ActionIDVector::iterator it = std::find(createdActions.begin(),createdActions.end(),m_id);
	GB_ASSERT(it != createdActions.end(),"Action::~Action: unknown action");
	//Con::printf("Deleting action: %p name - %s",this,m_name.c_str());
	createdActions.erase(it);
};

void Action::assignsParents(Actor* owner,Action* parentAction)
{
	m_owner = owner;
	m_parentAction = parentAction;

	//add child to the parent
	if (parentAction)
	{
		parentAction->addChild(this);
	}
}

Action* Action::clone()
{
	Action* action = _createAction();
	GB_ASSERT(action,"Action::clone: clone action wasn't created");

	prepareClone(action);
	
	return action;
}

void Action::prepareClone(Action* actionClone)
{
	actionClone->m_priority = m_priority;
	actionClone->m_requiredEffectorsInfo = m_requiredEffectorsInfo;
	actionClone->m_requiredEffectors = m_requiredEffectors;
	actionClone->m_acquiredEffectors = m_acquiredEffectors;

	updateDynamicParams(actionClone);
}

void Action::updateDynamicParams(Action* actionClone)
{

}

bool Action::check(Actor* owner, Action* parentAction)
{
	bool res = false;

	//acquire effectors
	bool principalAcquired = owner->checkEffectors(this, parentAction,m_acquiredEffectors);
	
	//make decision current action can be set on acquiredEffectors
	res = principalAcquired;

	return res;
}

void Action::init()
{

}

bool Action::update()
{
	if  (getAcquiredEffectors().empty())
		return false;
	return true;
}

void Action::terminate( TerminationStatus status /*= FINISHED*/ )
{
	//GB_ASSERT(m_terminationStatus==NONE,"Action is already deleted");
	if (m_terminationStatus!=NONE)
		return;
	
	m_terminationStatus = status;

	//terminate children, release effectors, add for deleting
	m_owner->releaseAction(this);
	m_acquiredEffectors.clear();

	//notify and remove from parent (do this after releaseAction, because parent can terminate itself on child termination)
	if (m_parentAction)
	{
		m_parentAction->removeChild(this);
	}

}

void Action::onChildTerminated(Action* terminatedChild)
{

}

void Action::onEffectorsLost(const EffectorIDVector &lostEffectors)
{
	if (!checkPrincipalEffectors())
		terminate(INTERUPTED);
}

void Action::onEffectorsAcquired(const EffectorIDVector& newEffectors)
{
	
}

void Action::loseEffectors(const EffectorIDVector& lostEffectors, bool notify /*= true*/)
{
	//acquired effectors can be changed,so lostEffectors can already contain lost effectors
	EffectorIDVector actualLostEffectors;
	
	//actual lost effectors = acquired effectors \ lost effectors
	if (!m_acquiredEffectors.empty())
	{
		for(size_t i = 0; i<lostEffectors.size();i++)
		{
			EffectorIDVector::iterator it = std::find(m_acquiredEffectors.begin(),
				m_acquiredEffectors.end(),lostEffectors[i]);
			if (it!=m_acquiredEffectors.end())
			{
				m_acquiredEffectors.erase(it);
				actualLostEffectors.push_back(lostEffectors[i]);
			}
		}
	}

	if (notify && !actualLostEffectors.empty())
	{
		//notify only about actual lost effectors
		onEffectorsLost(actualLostEffectors);
	}

}

void Action::acquireEffectors(const EffectorIDVector& newEffectors)
{
	m_acquiredEffectors.insert(m_acquiredEffectors.end(),newEffectors.begin(),newEffectors.end());
	onEffectorsAcquired(newEffectors);
}

void Action::addRequiredEffector(EffectorID effector,bool principal, int priority, Action* responsibleChild)
{
//	EffectorInfoMap::iterator it = m_requiredEffectorsInfo.find(effector);/* || responsibleChild*/
	GB_ASSERT(m_requiredEffectorsInfo.find(effector) == m_requiredEffectorsInfo.end(), \
			format("Action::addRequiredEffector: action have already contained  this effector in required effector list%s",Effector::convertToString(effector)));
/*	if (it!=m_requiredEffectorsInfo.end() && responsibleChild)
	{
		GB_ASSERT(it->second.responsibleChilds.find(responsibleChild)==it->second.responsibleChilds.end(), \
			"Action::addRequiredEffector: such responsible child already exists");
		it->second.responsibleChilds.insert(responsibleChild);
		return;
	}
*/	m_requiredEffectorsInfo[effector] = EffectorInfo(principal,responsibleChild,priority);
	m_requiredEffectors.push_back(effector);
}

void Action::addAcquiredEffector(EffectorID effector)
{
	GB_ASSERT(m_requiredEffectorsInfo.find(effector) != m_requiredEffectorsInfo.end(), \
			format("Action::addAcquiredEffector: acquired effector must be in required effector list %s",Effector::convertToString(effector)));
	GB_ASSERT(std::find(m_acquiredEffectors.begin(),m_acquiredEffectors.end(),effector) == 	m_acquiredEffectors.end(), \
			format("Action::addAcquiredEffector: can't find effector %s",Effector::convertToString(effector)));
	m_acquiredEffectors.push_back(effector);
	
	//for debug
	// EffectorIDVector effectors; effectors.push_back(effector);
	// onEffectorsAcquired(effectors);
}

void Action::setEffectorPriority(EffectorID effector,int priority)
{
	EffectorInfoMap::iterator it = m_requiredEffectorsInfo.find(effector);
	GB_ASSERT(it != m_requiredEffectorsInfo.end(),format("Action::getEffectorPriority: can't find effector %s",Effector::convertToString(effector)));
	it->second.priority = priority;
}

int Action::getEffectorPriority(EffectorID effector)
{
	EffectorInfoMap::iterator it = m_requiredEffectorsInfo.find(effector);
	GB_ASSERT(it != m_requiredEffectorsInfo.end(),format("Action::getEffectorPriority: can't find effector %s",Effector::convertToString(effector)));
	int priority = it->second.priority;
	if (priority == -1)
		 return m_priority;
	else
		return priority;
}

void Action::setRequiredEffectors(const EffectorIDVector& effectors,bool principal, int priority)
{ 
	m_requiredEffectors.clear();
	m_requiredEffectorsInfo.clear();
	for (size_t i = 0; i<effectors.size(); i++)
	{
		addRequiredEffector(effectors[i], principal, priority);
	}
};

bool Action::hasEffector(EffectorID effector)
{
	bool res = std::find(m_acquiredEffectors.begin(),m_acquiredEffectors.end(),effector)!=
		m_acquiredEffectors.end();
	return res;

}

bool Action::checkPrincipalEffectors()
{
	for(EffectorInfoMap::iterator it = m_requiredEffectorsInfo.begin(), end = m_requiredEffectorsInfo.end();
			it!=end; it++)
	{
		if (it->second.principal && !hasEffector(it->first))
		{
			return false;
		}
	}
	return true;
}

void Action::addChild(Action* child)
{
	GB_ASSERT(!child->getName().empty(), "Action must have a name");
	m_childs[child->getName()].push_back(child);
}


void Action::releaseChildResponsibleEffectors(Action* child, const EffectorIDVector& childEffectors)
{
	//release effectors, which were acquired special for this child
	EffectorIDVector effectorToRelease;
	for (EffectorInfoMap::iterator it = m_requiredEffectorsInfo.begin(),end = m_requiredEffectorsInfo.end();
		it!=end; it++)
	{
		if (it->second.responsibleChilds.find(child)!=it->second.responsibleChilds.end() && 
			std::find(childEffectors.begin(),childEffectors.end(),it->first)!=childEffectors.end())
		{
			it->second.responsibleChilds.erase(child);
			if (it->second.responsibleChilds.empty())
				effectorToRelease.push_back(it->first);
		}
	}

	//delete temporary acquired from acquired effectors, without onEffectorLost 
	//(temporary acquired effectors management is "transparent" for action logic)
	loseEffectors(effectorToRelease,false);
	//delete action from such effectors, if it wasn't terminated by onEffectorLost
		//if (m_terminationStatus==NONE) - commented - crash on removing RootAction
		// effectors are deleted from m_requiredEffectors but aren't released
	m_owner->releaseActionEffectors(this,effectorToRelease);
	//delete temporary acquired effector from required effectors
	for (size_t i = 0; i<effectorToRelease.size(); i++)
	{
		m_requiredEffectorsInfo.erase(effectorToRelease[i]);
		EffectorIDVector::iterator it = std::find(m_requiredEffectors.begin(), m_requiredEffectors.end(),
			effectorToRelease[i]);
		GB_ASSERT(it!=m_requiredEffectors.end(),"Action::onChildTerminated: logic corruption, \
												requiredEffectors must be identical to m_requiredEffectorsInfo");
		m_requiredEffectors.erase(it);
	}

	Action* parent = getParentAction();
	if (parent)
	{
		parent->releaseChildResponsibleEffectors(this,effectorToRelease);
	}
}

void Action::removeChild( Action* child )
{
	releaseChildResponsibleEffectors(child,child->getRequiredEffectors());

	//delete from child map
	for (ChildMap::iterator it=m_childs.begin(), end=m_childs.end();
			it!=end; it++)
	{
		Actions& sameNameChilds = it->second;
		Actions::iterator jt = std::find(sameNameChilds.begin(), sameNameChilds.end(), child);
		if (jt!=sameNameChilds.end())
		{
			sameNameChilds.erase(jt);
			if (sameNameChilds.empty())
			{
				m_childs.erase(it);
			}
			break;
		}
	}

	//call event
	onChildTerminated(child);
}



bool Action::hasEffectorResponsibleChild(EffectorID effectorId) 
{
	bool res = !m_requiredEffectorsInfo[effectorId].responsibleChilds.empty();
	return res;
};

void Action::addResponsibleChildForEffector(EffectorID effectorId, Action* child)
{
	ActionSet& responsibleChilds = m_requiredEffectorsInfo[effectorId].responsibleChilds;
	GB_ASSERT(responsibleChilds.find(child) == responsibleChilds.end(), \
		"Action::addResponsibleChildForEffector: Effector already contain this child");
	responsibleChilds.insert(child);
};

void Action::getChilds(Actions& childList, bool recursively /*= false*/)
{
	for (ChildMap::iterator it=m_childs.begin(), end=m_childs.end();
			it!=end; it++)
	{
		for (Actions::iterator jt=it->second.begin(), jend=it->second.end();
				jt!=jend; jt++)
		{
			Action* child = *jt;
			childList.push_back(child);
			if (recursively)
				child->getChilds(childList,recursively);
		}		
	}	
}

Action* Action::getChild(const std::string& actionName)
{
	Action* action = NULL;
	ChildMap::iterator it = m_childs.find(actionName);
	if (it!=m_childs.end())
	{
		GB_ASSERT(!it->second.empty(), "Empty record mustn't be here");
		action = it->second.back();
	}

	return action;
}

bool 	Action::terminateChild(const std::string& actionName)
{
	Action* child = getChild(actionName);
	if (child)
	{
		child->terminate(CANCELED);
		return true;
	}
	return false;
}

ConsecutiveControlAction* Action::addConsecutiveControl()
{
	ConsecutiveControlAction* control = static_cast<ConsecutiveControlAction*>
									(m_owner->setActionByName("ConsecutiveControlAction",this));
	return control;
}

AlternativeControlAction* Action::addAlternativeControl()
{
	//AlternativeControlAction* action = static_cast<AlternativeControlAction*>(ActionLibrary::instance().createAction("AlternativeControlAction"));
	//action = static_cast<AlternativeControlAction*>(m_owner->setActionDirectly(action,this));
	AlternativeControlAction* action = static_cast<AlternativeControlAction*>
									(m_owner->setActionByName("AlternativeControlAction",this));
	return action;
}