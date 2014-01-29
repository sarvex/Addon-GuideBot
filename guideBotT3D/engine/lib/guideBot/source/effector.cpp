//-----------------------------------------------------------------------------
// Guide Bot
// Copyright (C) LogicKing.com, Inc.
//-----------------------------------------------------------------------------

#include "guideBot/effector.h"
#include "guideBot/actor.h"
#include "guideBot/platform.h"

#include <algorithm>

using namespace GuideBot;

////////////////////////////////////////////////////
//////****************Effector*************/////////
////////////////////////////////////////////////////
static const EnumDescription::Record effectorIDEnums[] =
{
	{ E_BODY, "body" },
	{ E_SPINE, "spine" },
	{ E_HEAD, "head" },
	{ E_BRAIN, "brain" },
	{ E_BRAIN_MOVING, "brain_moving" },
	{ E_EYES, "eyes" },
	{ E_LEGS, "legs" },
	{ E_HANDS, "hands" },
};

EnumDescription Effector::enumDescription(E_EFFECTORS_NUM, effectorIDEnums);

const char* Effector::convertToString(const EffectorID effectorID)
{
	return enumDescription.getNameByValue(effectorID);
}

void Effector::convertToString(const EffectorIDVector& effectors, std::string& res)
{
	for (size_t i = 0; i<effectors.size();i++)
	{
		res.append(enumDescription.getNameByValue(effectors[i]));
		res.append(" ");
	}
}

bool Effector::convertFromString(const char* effectorsStr, EffectorIDVector& effectors)
{
	size_t i = 0;
	const char* r  = NULL;
	while ( strlen(r = getWord(effectorsStr, i, " \t\n"))>0)
	{
		EffectorID id;
		if (!enumDescription.getValueByName(r,id))
		{
			GB_ERROR( "Effector::convertFromString: unknown effector %s",r);
			return false;
		}
		else
			effectors.push_back(id);
		i++;
	}

	if (effectors.empty())
		return false;

	return true;
}

Effector::Effector(Actor* owner,EffectorID id, Action* rootAction) :	m_owner(owner)
																		, m_id(id)
																		, m_rootNode( new Node(rootAction))
{
	
}

Effector::~Effector()
{
	GB_ASSERT(!m_rootNode,"Effector::Destructor: root node must be destroyed");
}

Effector::Node* Effector::findNode(Node* node,Action* action)
{
	Node* res = NULL;
	if (node->action==action)
		res = node;
	else
	{
		//search among children
		for(Nodes::reverse_iterator rit = node->childs.rbegin(), rend = node->childs.rend();
			rit!=rend; rit++)
		{
			if ((res = findNode(*rit,action))!=NULL)
				return res;
		}
	}

	return res;
}

Effector::Node* Effector::findNode(Action* action)
{
	Node* node = findNode(m_rootNode,action);
	GB_ASSERT(node,"Effector::findNode: can't find action on effector");

	return node;	
}

Effector::Node* Effector::findActiveNode(Action* action,bool withAssert)
{
	Node* node = m_rootNode;
	while (node->action!=action)
	{
		if (node->childs.empty())
		{
			node = NULL;
			break;
		}
		node = node->childs.back();
	}

	GB_ASSERT(!withAssert || node,"Effector::findActiveNode: can't find active action on effector");

	return node;	
}

bool Effector::isActive(Action* action)
{
	bool res = findActiveNode(action,false) != NULL;
	return res;
}

Action* Effector::getActiveAction(const std::string& actionName)
{
	Node* node = m_rootNode;
	while (!node->action || node->action->getName()!=actionName)
	{
		if (node->childs.empty())
			return NULL;
		node = node->childs.back();
	}
	
	return node->action;
}

Action*	Effector::getActiveChildAction(Action* parent /*=NULL*/, bool onlyActive /*= true*/)
{
	Node* node = onlyActive ? findActiveNode(parent) : findNode(parent); 

	Action* child = node->childs.empty() ? NULL : node->childs.back()->action;

	GB_ASSERT(!child || child->getParentAction() == parent, \
		"Effector::getChildAction: logical stack corruption at the effector");

	return child;
}

void Effector::getActiveChilds(Actions& activeChildren,Node* node)
{
	while(!node->childs.empty())
	{
		node = node->childs.back();
		activeChildren.push_back(node->action);
	}
}

void Effector::getActiveChildActions(Action* parent, Actions& activeChildren)
{
	Node* node = findActiveNode(parent);
	getActiveChilds(activeChildren,node);
}

void Effector::getDirectChildren(Action* parent, Actions& children)
{
	Node* node = findNode(parent);

	for (Nodes::iterator it = node->childs.begin(),end = node->childs.end(); it!=end; it++)
	{
		children.push_back((*it)->action);
	}
}

bool Effector::insertAction(Action* action,Action* parent)
{
	Node* node = findNode(parent);	//findActiveNode

	//create new node for action
	Node* newNode = new Node(action,node);

	//insert to proper place 
	Nodes::iterator toInsert = node->childs.end();
	Nodes::reverse_iterator rit = node->childs.rbegin(),rend = node->childs.rend();
	
	while (rit!=rend && !m_owner->compareActions((*rit)->action,newNode->action,m_id,
										(*rit)->action->getEffectorPriority(m_id),
										newNode->action->getEffectorPriority(m_id)))
	{
		toInsert--;
		rit++;
	}
	node->childs.insert(toInsert,newNode);

	//check if it is active - last in the deque
	bool active = isActive(newNode);
	return active;
}

bool Effector::isActive(Node* node)
{
	while(node->parent)
	{
		if (node->parent->childs.back() != node)
			return false;
		node = node->parent;
			
	}
	return true;
}


void Effector::removeAction(Action* action,Actions& becameActive)
{
	Node* node = findNode(action);

	bool wasActive = isActive(node);
	
	Node* parent = node->parent;
	if (parent)
	{
		Nodes::iterator it = std::find(parent->childs.begin(),parent->childs.end(),node);
		GB_ASSERT(it!=parent->childs.end(),"Effector::removeAction: logical stack corruption");
		it = parent->childs.erase(it);
		//activating actions (down to hierarchy)
		if (wasActive)
		{
			getActiveChilds(becameActive,parent);
		}
	}

	GB_ASSERT(node->childs.empty(), "Effector::removeAction: removing action has childs");

	if (node == m_rootNode)
		m_rootNode = NULL;
	GB_SAFE_DELETE(node);
}

void	Effector::removeActions(Actions& actions)
{
	removeChildActions(actions, m_rootNode);
}

void	Effector::removeChildActions(Actions& actions,Node* node)
{
	for (Nodes::iterator it = node->childs.begin(),end = node->childs.end(); it!=end; it++)
	{
		actions.push_back((*it)->action);
		removeChildActions(actions,*it);
		GB_ASSERT((*it)->childs.empty(),"Effector::removeChildActions: action has children");
		GB_SAFE_DELETE((*it));
	}
	node->childs.clear();
}