//-----------------------------------------------------------------------------
// Guide Bot
// Copyright (C) LogicKing.com, Inc.
//-----------------------------------------------------------------------------


#ifndef _EFFECTOR_H
#define _EFFECTOR_H

#include <vector>
#include <map>
#include <set>
#include <deque>
#include <string>

#include "guideBot/stringUtils.h"

namespace GuideBot
{

class Action;
class Actor;

////////////////////////////////////////////////////
//////****************Effector*************/////////
////////////////////////////////////////////////////

enum EffectorID
{
	E_BODY	= 0,
	E_SPINE,
	E_HEAD,
	E_BRAIN,
	E_BRAIN_MOVING,
	E_EYES,
	E_LEGS,
	E_HANDS,

	E_EFFECTORS_NUM,
};
typedef std::vector<EffectorID> EffectorIDVector;
typedef std::vector<Action*> Actions;

class Effector
{
public:
	Effector(Actor* owner,EffectorID id, Action* rootAction);
	virtual ~Effector();
	EffectorID getId()	{return m_id;};

	//util functions
	static EnumDescription enumDescription;
	static const char* convertToString(const EffectorID effectorID);
	static void convertToString(const EffectorIDVector& effectors, std::string& res);
	static bool convertFromString(const char* effectorsStr, EffectorIDVector& effectors);

	//public interface
	Action*	getActiveChildAction(Action* parentAction,bool onlyActive = true);
	void	getActiveChildActions(Action* parent, Actions& activeChildren);
	void	getDirectChildren(Action* parentAction, Actions& children);
	void	removeActions(Actions& actions);

	bool	insertAction(Action* action,Action* parent);
	void	removeAction(Action* action,Actions& becameActive);

	bool	isActive(Action* action);
	Action*	getActiveAction(const std::string& actionName);
protected:
	struct Node;
	typedef std::deque<Node*> Nodes;

	Node* findNode(Node* node,Action* action);
	Node* findNode(Action* action);
	Node* findActiveNode(Action* action,bool withAssert = true);

	void	getActiveChilds(Actions& activeChildren,Node* node);
	void	removeChildActions(Actions& actions,Node* node);
	bool	isActive(Node* node);
	Actor*			m_owner;
	EffectorID		m_id;

	
	
	struct Node
	{
		Node(Action* _action = NULL, Node* _parent = NULL) : action(_action), parent(_parent) {};
		Action* action;

		Node* parent;
		Nodes childs;
	};

	Node*	m_rootNode;
};

} //namespace GuideBot

#endif