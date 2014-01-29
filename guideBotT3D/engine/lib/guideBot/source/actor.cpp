//-----------------------------------------------------------------------------
// Guide Bot
// Copyright (C) LogicKing.com, Inc.
//-----------------------------------------------------------------------------

#include "guideBot/actor.h"
#include "guideBot/actorState.h"
#include "guideBot/platform.h"
#include "guideBot/stringUtils.h"

#include <algorithm>
#include "float.h"

using namespace GuideBot;

#define DEFAULT_FOV	((float)M_PI*2.f/3.f)
#define DEFAULT_VIEW_DISTANCE 25.f
#define DEFAULT_FORGET_TIME 25000


#define DEFAULT_HEARING_THRESHOLD 0.1f


#define ACTOR_EFFECTORS_NUM E_EFFECTORS_NUM
static EffectorID actorEffectors[ACTOR_EFFECTORS_NUM] = {E_BODY, E_SPINE, E_HEAD, E_BRAIN, 
														E_BRAIN_MOVING, E_EYES, E_LEGS,E_HANDS};

Actor::EnemyEvaluator Actor::m_enemyEvaluator;

Actor::Actor(): WorldObject(WO_ACTOR)
				, m_state(NULL)
				, m_moveDir(Vector3::ZERO)
				, m_velocity(0.f)
				, m_velocityVector(Vector3::ZERO)
				, m_pitch(0.f)
				, m_enablePerception(false)
				, m_isDead(false)
				, m_fov(DEFAULT_FOV)
				, m_viewDistance(DEFAULT_VIEW_DISTANCE)
				, m_hearingThreshold(DEFAULT_HEARING_THRESHOLD)
				, m_forgetTime(DEFAULT_FORGET_TIME)
				, m_enemy(NULL)
				, m_activeAttackInfo(NULL)
				, m_collisionWall(Vector3::ZERO)
				, m_wallAvoidance(Vector3::ZERO)
				, m_lastMoveDir(Vector3::ZERO)
				, m_actualVelocity(Vector3::ZERO)
				, m_wallCollision(false)
				, m_isControlledByPlayer(false)
				, m_alertness(NO_ALERTNESS)
{
	//create root action and effectors
	m_rootAction = new Action();
	m_rootAction->setName("RootAction");
	for(size_t i = 0;i<ACTOR_EFFECTORS_NUM;i++)
	{
		m_effectorIDs.push_back(actorEffectors[i]);
		m_effectors.insert(std::make_pair(actorEffectors[i],new Effector(this,actorEffectors[i],m_rootAction)));
	}
	m_rootAction->setRequiredEffectors(m_effectorIDs);
	m_rootAction->acquireEffectors(m_effectorIDs);
}

Actor::~Actor()
{
	//destroyAllActions(); - called from enhanced player onRemove
	GB_ASSERT(!m_rootAction, "Actor::Destructor: Root action must be already destroyed");

	//destroy effectors
	EffectorMap::iterator it = m_effectors.begin();
	EffectorMap::iterator end = m_effectors.end();
	while(it!=end)
	{
		Effector* effector = it->second;
		GB_SAFE_DELETE(effector);
		it++;
	}
	m_effectors.clear();
}
void Actor::setLookDir(const Vector3& lookDir)
{
	Vector3 lookDir2d = lookDir;
	lookDir2d.z = 0;
	GB_ASSERT(!lookDir2d.isZero(),"Actor::setLookDir: look dir must be non-zero");

	Parent::setLookDir(lookDir2d);
}

Vector3 Actor::getEyesLookDir()
{
	return getLookDir();
}

void Actor::destroyAllActions()
{
	releaseAction(m_rootAction);
	m_rootAction = NULL;
	
	//delete actions
	ActionSet::iterator ita = m_actionsToDelete.begin();
	ActionSet::iterator enda = m_actionsToDelete.end();
	while(ita!=enda)
	{
		Action* action = *ita;
		GB_SAFE_DELETE(action);
		ita++;
	}
	m_actionsToDelete.clear();
}

void Actor::setActorState(State* state)
{
	//GB_INFO("Actor::setActorState: %s",state->getName().c_str());
	if (m_state != state)
	{
		if (m_state && !m_state->getDisableCallback().empty())
			call(m_state->getDisableCallback());

		m_state = state;

		if (m_state && !m_state->getEnableCallback().empty())
			call(m_state->getEnableCallback());
	}
	
}

Action* Actor::changeState(const std::string& stateName,Action* parentAction)
{
	State* newState = getState(stateName);
	if (!newState)
	{
		GB_ERROR("Actor::changeState: can't find state to change to");
		return false;
	}

	ActionChangeState* prototype = static_cast<ActionChangeState*>(ActionLibrary::instance().getActionPrototype(("ActionChangeState")));
	prototype->setParams(stateName);
	
	return setActionByPrototype(prototype,parentAction);	
}

Action* Actor::setActionByName(const char* actionName,Action* parentAction /*= NULL*/)
{
	Action* prototype = ActionLibrary::instance().getActionPrototype(actionName);
	Action* action = setActionByPrototype(prototype,parentAction); 
	
	return action;
}

Action* Actor::setActionDirectly(Action* newAction,Action* parentAction /*= NULL*/)
{
	GB_ASSERT(newAction,"Actor::setActionDirectly: newAction is NULL");
	parentAction = parentAction ? parentAction : m_rootAction;

	//check if we can do action, determine acquired effectors
	if (!newAction->check(this,parentAction))
	{
		//GB_INFO("setActionByPrototype: wasn't set: %s",prototype->getDescString().c_str());
		
		//delete action
		m_actionsToDelete.insert(newAction);	//can be deleted here directly
		return NULL;
	}

	return setAction(newAction,parentAction);
}

Action* Actor::setActionByPrototype(Action* prototype,Action* parentAction /*= NULL*/)
{
	GB_ASSERT(prototype,"Actor::setAction: prototype is NULL");
	parentAction = parentAction ? parentAction : m_rootAction;

	//check if we can do action, determine acquired effectors
	if (!prototype->check(this,parentAction))
	{
		//GB_INFO("setActionByPrototype: wasn't set: %s",prototype->getDescString().c_str());
		return NULL;
	}

	//create new action from prototype
	Action* newAction = prototype->clone();	
	
	return setAction(newAction,parentAction);
}

Action* Actor::setAction(Action* newAction, Action* parentAction)
{
	static std::string tabs;
	//GB_INFO("%ssetAction: %s (%p)",tabs.c_str(),newAction->getDescString().c_str(),newAction);
	tabs.append("    ");

	//check for parent action validity
	GB_ASSERT(parentAction->getTerminationStatus()==Action::NONE, "Actor::setAction: setting action with terminated parent");

	//set owner and parent action
	newAction->assignsParents(this,parentAction);

	const EffectorIDVector &acquiredEffectors = newAction->getAcquiredEffectors();
	loseEffectors(acquiredEffectors,parentAction);

	setActionToEffectors(newAction,parentAction);

	//add to list for initialization on actor update
	//m_actionsToInit.insert(newAction);
	newAction->init();
	//check if it was self terminated on init
	if (m_actionsToDelete.find(newAction) != m_actionsToDelete.end())
	{
		newAction = NULL;
	}
	
	tabs.erase(tabs.size()-4,4);
	return newAction;
}

void Actor::setActionToEffectors(Action* action, Action* parent)
{
	GB_ASSERT(action,"Actor::setActionToEffectors: action must be defined");
	
	EffectorIDVector actualNonAcquiredEffectors = action->getAcquiredEffectors();
	const EffectorIDVector &requiredEffectors = action->getRequiredEffectors();
	//set new action to effectors
	for (size_t i = 0; i<requiredEffectors.size(); i++)
	{
		Effector* effector = getEffector(requiredEffectors[i]);
		GB_ASSERT(effector, format("Actor::getEffector: actor doesn't have effector %s", \
											Effector::convertToString(requiredEffectors[i])));
		Actions parentChain;
		getResponsibleParent(requiredEffectors[i],parent,&parentChain);
		//insert parents
		int customPriority = action->getEffectorPriority(requiredEffectors[i]);

		//case of multiple responsible children
		if (parentChain.size()==1 && (parent->hasEffectorResponsibleChild(requiredEffectors[i])))
		{
			parent->addResponsibleChildForEffector(requiredEffectors[i], action);
		}

		for (size_t j = parentChain.size()-1; j>0; j--)
		{
			//action notification about new added effectors can be placed here
			Action* actionToInsert = parentChain[j-1];
			Action* parentAction = parentChain[j];
			Action* responsibleChild = j>1 ?  parentChain[j-2] : action;
			actionToInsert->addRequiredEffector(requiredEffectors[i],false,customPriority,responsibleChild);
			if (effector->insertAction(actionToInsert,parentAction))
			{
				actionToInsert->addAcquiredEffector(requiredEffectors[i]);
			}
		}

		//insert action
		if (effector->insertAction(action,parent))
		{
			//maybe this is only for debug
			EffectorIDVector::iterator it = std::find(actualNonAcquiredEffectors.begin(),actualNonAcquiredEffectors.end(),
									requiredEffectors[i]);
			GB_ASSERT(it!=actualNonAcquiredEffectors.end(),"Actor::setActionToEffectors: can't set action properly.");
			actualNonAcquiredEffectors.erase(it);
		}
	}

	GB_ASSERT(actualNonAcquiredEffectors.empty(),"Actor::setActionToEffectors: can't set action properly.");
}

bool Actor::compareActions(Action* action1, Action* action2,EffectorID effectorID, 
							int priority1, int priority2)
{
	//return true if action1 <= action2
	//bool res = action1->getEffectorPriority(effectorID) <= action2->getEffectorPriority(effectorID) 
	bool res = priority1 <= priority2 
				&& !action1->getHighPriorityFlag();
	return res;
}

Action* Actor::getResponsibleParent(EffectorID effectorID, Action* parentAction,Actions* parentChain)
{
	if (parentChain)
		parentChain->push_back(parentAction);
	const EffectorIDVector &parentEffectors = parentAction->getRequiredEffectors();
	//does parent have such effector
	if (std::find(parentEffectors.begin(),parentEffectors.end(),effectorID)!=parentEffectors.end())
		return parentAction;
	else
	{
		GB_ASSERT(parentAction, format("Actor::isAquired: actor doesn't have effector %s", Effector::convertToString(effectorID)));
		parentAction = parentAction->getParentAction();
		return getResponsibleParent(effectorID,parentAction,parentChain);
	}
}

bool Actor::checkEffectors(Action* action, Action* parentAction,EffectorIDVector &acquiredEffectors)
{
	acquiredEffectors.clear();

	bool principalAcquired = true;
	const Action::EffectorInfoMap &requiredEffectors = action->getRequiredEffectorInfos();
	for (Action::EffectorInfoMap::const_iterator it = requiredEffectors.begin(),end = requiredEffectors.end();
			it!=end; it++)
	{
		EffectorID effectorID = it->first;
		
		Effector* effector = getEffector(effectorID);
		GB_ASSERT(effector, format("Actor::getEffector: actor doesn't have effector %s", Effector::convertToString(effectorID)));
		
		bool principal = it->second.principal;

		
		
		//check if effector will be acquired by action when action is setting to parent
		bool isAcquired = false;

		Actions parentChain;
		Action* responsibleParent = getResponsibleParent(effectorID,parentAction,&parentChain);
		if (effector->isActive(responsibleParent))
		{
			isAcquired = true;
			int customActionPriority = action->getEffectorPriority(effectorID);
			Action* currentAction = parentChain.size()>1 ? parentChain[parentChain.size()-2] : action;
			Action* settedAction =  effector->getActiveChildAction(responsibleParent);
			//in our case, this can be done simpler, because comparing is based on action's priority 
			//and doesn't depend on action(so action can be used instead of currentAction)
			if (!settedAction || compareActions(settedAction,currentAction,effectorID,
										settedAction->getEffectorPriority(effectorID),customActionPriority))
			{
				isAcquired = true;
				acquiredEffectors.push_back(effectorID);
			}	
		}

		if (principal && !isAcquired)
		{
			principalAcquired = false;
		}
	}

	return principalAcquired;
}

bool Actor::isActionSetted(const char* actionName)
{
	std::string name = actionName;
	bool res = m_rootAction->getChild(name) != NULL;
	return res;
}

Action*	Actor::getActiveAction(const std::string actionName, EffectorID effectorID)
{
	EffectorIDVector effectors;
	effectors.push_back(effectorID);
	return getActiveAction(actionName,effectors);
}

Action* Actor::getActiveAction(const std::string actionName, const EffectorIDVector& effectors )
{
	const EffectorIDVector& checkedEffectors = effectors.empty() ? m_effectorIDs : effectors;
	EffectorIDVector::const_iterator it = checkedEffectors.begin();
	EffectorIDVector::const_iterator end = checkedEffectors.end();
	Action* action = NULL;
	while (it!=end && !action)
	{
		Effector* effector = getEffector(*it);
		action = effector->getActiveAction(actionName);
		it++;
	}

	return action;
}

bool Actor::terminateAction(const char* actionName)
{
	std::string name = actionName;
	bool res = m_rootAction->terminateChild(name) != NULL;
	return res;
}

void Actor::terminateAllActions()
{
	Actions childs;
	m_rootAction->getChilds(childs);
	for (Actions::iterator it= childs.begin(),end= childs.end();
			it!=end; it++)
	{
		Action* action = *it;
		action->terminate(Action::CANCELED);
	}
}

struct ActionEffectorsInfo
{
	struct Info
	{
		Info (Action* _action) : action(_action) {};
		Action*				action;
		EffectorIDVector	effectorIDVector;
	};
	typedef std::deque<Info> ActionEffectorsDeque;

	void addInfo(Effector* effector,Actions actions)
	{	
		for(size_t i = 0; i < actions.size(); i++)
		{
			_addInfo(effector,actions[i]);
		}
	}
	ActionEffectorsDeque m_deque;
protected:
	void _addInfo(Effector* effector,Action* action)
	{
		ActionEffectorsDeque::iterator it=m_deque.begin(),end = m_deque.end();
		for (; it!=end; it++)
		{
			if (it->action==action)
				break;
		}
		if (it==end)
		{
			m_deque.push_back(Info(action));
			it = --m_deque.end();
		}
		it->effectorIDVector.push_back(effector->getId());
	}
};	

void Actor::loseEffectors(const EffectorIDVector& effectors, Action* parentAction)
{
	//determine lost effectors for actions on  effectors
	ActionEffectorsInfo lostEffectorsInfo;
	for (size_t i = 0; i<effectors.size(); i++)
	{
		Effector* effector = getEffector(effectors[i]);
		GB_ASSERT(effector, format("Actor::getEffector: actor doesn't have effector %s", Effector::convertToString(effectors[i])));
		Action* responsibleParent = getResponsibleParent(effectors[i],parentAction);
		Actions activeActions;
		effector->getActiveChildActions(responsibleParent, activeActions);
		lostEffectorsInfo.addInfo(effector,activeActions);
	}

	//notify actions about lost effectors
	ActionEffectorsInfo::ActionEffectorsDeque::reverse_iterator rit = lostEffectorsInfo.m_deque.rbegin(),
		rend = lostEffectorsInfo.m_deque.rend();
	while(rit!=rend)
	{
		Action* action  = rit->action;
		EffectorIDVector& effectors = rit->effectorIDVector;
		action->loseEffectors(effectors);
		rit++;
	}
}

void Actor::blockEffectors(const EffectorIDVector& effectors)
{
	Action* dummy = ActionLibrary::instance().createAction("Action");
	dummy->setPriority(10);
	dummy->setRequiredEffectors(effectors);
	setActionDirectly(dummy);
}

void Actor::resetEffectors(const EffectorIDVector& effectors)
{
	loseEffectors(effectors,m_rootAction);
	/*
	for (size_t i = 0; i<effectors.size(); i++)
	{
		Effector* effector = getEffector(effectors[i]);
		Action* action = effector->getActiveChildAction(m_rootAction);
		if (action)
			action->terminate(Action::CANCELED);
	}*/
}

Effector* Actor::getEffector(EffectorID id)
{
	Effector* effector = NULL;
	EffectorMap::iterator it = m_effectors.find(id);
	if (it!=m_effectors.end())
		effector = it->second;
	GB_WARN(effector, format("Actor::getEffector: actor doesn't have effector %s", Effector::convertToString(id)));
	return effector;
}

void Actor::update()
{
	//GB_INFO("Actor::update()");
	//reset parameters
	resetMoveDir();
	m_velocityVector = Vector3::ZERO;
	m_velocity = 0.f;
	
	if (m_enablePerception)
	{
		updateSensors();
	}
	updateMemory();
	updateEffectors();
}

void Actor::updateEffectors()
{
	GB_ASSERT(m_state,"Actor::updateEffectors: actor doesn't have state");
	
	//delete actions
	ActionSet::iterator ita = m_actionsToDelete.begin();
	ActionSet::iterator enda = m_actionsToDelete.end();
	while(ita!=enda)
	{
		Action* action = *ita;
		GB_SAFE_DELETE(action);
		ita++;
	}
	m_actionsToDelete.clear();

	//update actions
	Actions actionsToUpdate;
	m_rootAction->getChilds(actionsToUpdate, true);
/*
	for (Actions::reverse_iterator rit=actionsToUpdate.rbegin(), rend = actionsToUpdate.rend();
			rit!=rend; rit++)
*/
	for (Actions::iterator it=actionsToUpdate.begin(), end = actionsToUpdate.end();
		it!=end; it++)
	{
		Action* action = *it; //*rit;
		//check whether action wasn't terminated yet
		if (action->getTerminationStatus()==Action::NONE)
		{
			action->update();
		}
	}
	
	//notify actions about acquiring(activating) effectors
	for (ActivatedEffectorsMap::iterator it = m_activatedEffectors.begin(),end = m_activatedEffectors.end();
		it!=end; it++)
	{
		Action* action = it->first;
		EffectorIDVector& effectors = it->second;

		//check action activity on effectors
		EffectorIDVector::iterator jt = effectors.begin();
		while(jt!=effectors.end())
		{
			Effector* effector = getEffector(*jt);
			if (effector->isActive(action))
				jt++;
			else
				jt = effectors.erase(jt);
		}

		if (!effectors.empty())
		{
			//GB_INFO("Actor::update: reacquiring %s (%p)",action->getDescString().c_str(), action);
			action->acquireEffectors(effectors);
		}
	}
	m_activatedEffectors.clear();
}

void Actor::releaseActionEffectors(Action* action,const EffectorIDVector& effectorIDs)
{
	for (size_t i=0;i<effectorIDs.size();i++)
	{
		Effector* effector = getEffector(effectorIDs[i]);
		GB_ASSERT(effector, format("Actor::getEffector: actor doesn't have effector %s", Effector::convertToString(effectorIDs[i])));
		Actions children;
		effector->getDirectChildren(action, children);
		GB_ASSERT(children.empty(), "Actor::releaseActionEffectors: action isn't allowed to have children on effector at this moment");

		Actions becameActive;
		effector->removeAction(action,becameActive);
		for(size_t j=0; j<becameActive.size(); j++)
		{
			m_activatedEffectors[becameActive[j]].push_back(effectorIDs[i]);
		}
	}
}

void Actor::releaseAction(Action* action)
{
	//GB_INFO("ReleaseAction begin: %s (%p)",action->getDescString().c_str(), action);
	static std::string tabs;
	//GB_INFO("%sReleaseAction: %s (%p)",tabs.c_str(), action->getDescString().c_str(), action);
	tabs.append("    ");

	GB_ASSERT(m_actionsToDelete.find(action) == m_actionsToDelete.end(),
						"Action has already been deleted. Action logic is corrupted!");
	m_actionsToDelete.insert(action);

	//terminate childs
	Actions childs;
	action->getChilds(childs);
	for (Actions::iterator it=childs.begin(), end=childs.end();
			it!=end; it++)
	{
		Action* child = *it;
		child->terminate(Action::CANCELED);
	}
	
	//remove from effectors
	const EffectorIDVector& effectorIDs = action->getRequiredEffectors();
	for (size_t i=0;i<effectorIDs.size();i++)
	{
		Effector* effector = getEffector(effectorIDs[i]);
		GB_ASSERT(effector, format("Actor::getEffector: actor doesn't have effector %s", Effector::convertToString(effectorIDs[i])));

		Actions becameActive;
		effector->removeAction(action,becameActive);
		for(size_t j=0; j<becameActive.size(); j++)
		{
			m_activatedEffectors[becameActive[j]].push_back(effectorIDs[i]);
		}		
	}
	tabs.erase(tabs.size()-4,4);
	//GB_INFO("ReleaseAction end: %s",action->getDescString().c_str());
}

void Actor::setMoveDir(const Vector3& moveDir)
{
	Vector3 newMoveDir = moveDir;
	newMoveDir.z = 0.f;
	newMoveDir.normalizeSafe();
	if (!newMoveDir.isZero() && !m_isControlledByPlayer)
	{
		//obstacle avoidance
		Vector3 ourPos = getPos();
		Vector3 ourPos2D = ourPos;
		ourPos2D.z = 0.f;
		Vector3 futurePos2D = ourPos2D + getVelocity()*newMoveDir*Platform::TickTime;

		Obstacles& obstacles = Obstacle::getObstacles();
		
		Obstacle* nearestObstacle = NULL;
		float minDist = FLT_MAX;		
		for(size_t i=0;i<obstacles.size(); i++)
		{
			Obstacle* obstacle = obstacles[i];
			Vector3 obstaclePos = obstacle->getPos();
			Vector3 obstaclePos2D = obstaclePos;
			obstaclePos2D.z = 0.f;
			float obstacleRad = obstacle->getRadius() + 0.5f;

			if (obstaclePos.z - ourPos.z > 1.f)
				continue;
			Vector3 obstToFuture = obstaclePos2D - futurePos2D;
			float obstToFutureLength = obstToFuture.length();
			if (obstToFutureLength<obstacleRad && obstToFutureLength*obstToFutureLength<minDist)
			{
				minDist = obstToFutureLength;
				nearestObstacle = obstacle;
			}
		}

		if (nearestObstacle)
		{
			Vector3 obstaclePos = nearestObstacle->getPos();
			Vector3 obstaclePos2D = obstaclePos;
			obstaclePos2D.z = 0.f;
			float obstacleRad = nearestObstacle->getRadius() + 0.5f;
			Vector3 obstToFuture = obstaclePos2D - futurePos2D;
			float obstToFutureLength = obstToFuture.length();
			Vector3 vec = obstaclePos2D - ourPos2D;
			vec.normalizeSafe();
			//calculate pos on obstacle bound  in perpendicular to move direction
			float cosAngle = dotProtuct(vec,moveDir);
			if (cosAngle>GB_EPSILON)
			{
				Vector3 vec2 = obstToFutureLength*cosAngle*newMoveDir+ourPos2D;//futurePos;
				vec2 = vec2 - obstaclePos2D;
				vec2.normalizeSafe(); 
				vec2 = vec2*obstacleRad+obstaclePos2D;

				newMoveDir = vec2 - ourPos2D;
				newMoveDir.normalizeSafe();
			}
		}

		/*
		for(size_t i=0;i<obstacles.size(); i++)
		{
			Obstacle* obstacle = obstacles[i];
			Vector3 obstaclePos = obstacle->getPos();
			Vector3 obstaclePos2D = obstaclePos;
			obstaclePos2D.z = 0.f;
			float obstacleRad = obstacle->getRadius() + 0.5f;
			
			if (obstaclePos.z - ourPos.z > 1.f)
				continue;
			Vector3 obstToFuture = obstaclePos2D - futurePos2D;
			float obstToFutureLength = obstToFuture.len();
			if (obstToFutureLength<obstacleRad)		
			{
				Vector3 vec = obstaclePos2D - ourPos2D;
				vec.normalize();
				//calculate pos on obstacle bound  in perpendicular to move direction
				float cosAngle = mDot(vec,moveDir);
				if (cosAngle<POINT_EPSILON)
					continue;
				Vector3 vec2 = obstToFutureLength*cosAngle*newMoveDir+ourPos2D;//futurePos;
				vec2 = vec2 - obstaclePos2D;
				vec2.normalize(); 
				vec2 = vec2*obstacleRad+obstaclePos2D;

				//static Vector3 sizes(0.2f,0.2f,0.2f);
				//DebugDrawer::get()->drawBox(obstaclePos-sizes,obstaclePos+sizes,ColorF::BLUE);
				//DebugDrawer::get()->drawBox(vec2-sizes,vec2+sizes);

				newMoveDir = vec2 - ourPos2D;
				newMoveDir.normalize();

				break;
			}
		}*/
	}
	m_moveDir = newMoveDir;
}

void Actor::move(const Vector3& velocity)
{
	m_velocityVector = velocity;
	m_velocity = velocity.length();
}

void Actor::setPitch(float pitch)
{
	m_pitch = clamp(pitch, -1.f,1.f);
}

bool Actor::hasVisual(const std::string& visualName)
{
	const std::string& rawVisualName = m_state->getRawVisualName(visualName);
	bool res = getVisual(rawVisualName) != NULL;
	return res;	
}


Action* Actor::playVisual(const std::string& visualName, Action* parentAction, bool allowInactive)
{
	const std::string& rawVisualName = m_state->getRawVisualName(visualName);
	Visual* visual = getVisual(rawVisualName);
	if (!visual)
	{
		GB_ERROR("Actor::playVisual: cant find visual %s", visualName.c_str());
		return NULL;
	}
	
	return playVisual(visual, parentAction, allowInactive);
	
}

Action* Actor::playVisual(Visual* visual, Action* parentAction, bool allowInactive)
{
	GB_ASSERT(visual,"Actor::playVisual: visual must be defined");

	ActionPlayVisual* prototype = GET_ACTION_PROTOTYPE(ActionPlayVisual);
	prototype->setParams(visual, allowInactive);
	return setActionByPrototype(prototype, parentAction);
}



Action* Actor::moveToPoint(const Vector3& point, float precision, Action* parent, bool onlyPrototype)
{
	ActionMoveToPoint* prototype = static_cast<ActionMoveToPoint*>
									(ActionLibrary::instance().getActionPrototype("ActionMoveToPoint"));
	prototype->setParams(point, precision);
	if (onlyPrototype)
		return prototype;
	return setActionByPrototype(prototype, parent);
}

Action*	Actor::moveToObject(WorldObject* target, float precision /*= 1.5f*/, Action* parent /*= NULL*/,
							bool onlyPrototype /*= false*/)
{
	GB_ASSERT(target,"Actor::moveToObject: target object must be defined");
	ActionMoveToObject* prototype = static_cast<ActionMoveToObject*>
									(ActionLibrary::instance().getActionPrototype("ActionMoveToObject"));
	prototype->setParams(target,precision);
	if (onlyPrototype)
		return prototype;
	return setActionByPrototype(prototype,parent);	
}

Action*	Actor::moveByWaypoints(const Waypoints& waypoints, ActionMoveByWaypoints::FollowMode mode /*= ActionMoveByWaypoints::FM_DIRECT*/,
						float precision /*= 0.5f*/, Action* parent /*= NULL*/)
{
	GB_ASSERT(waypoints.size()>0,"Actor::moveByWaypoints: waypoints must be defined");
	ActionMoveByWaypoints* prototype = static_cast<ActionMoveByWaypoints*>
		(ActionLibrary::instance().getActionPrototype("ActionMoveByWaypoints"));
	prototype->setParams(waypoints,mode,precision);

	return setActionByPrototype(prototype,parent);	
}
Action*	Actor::setLookDirMode(ActionLookAt::LookDirMode mode, const Vector3* point, WorldObject* object,int priority,Action* parent)
{
	//check params
	if ((mode == ActionLookAt::L_POINT || mode == ActionLookAt::L_DIR) && !point)
	{
		GB_ERROR("Actor::setLookDirMode:  for mode L_POINT point must be defined");
		return NULL;
	}
	else if (mode == ActionLookAt::L_OBJECT && !object)
	{
		GB_ERROR("Actor::setLookDirMode:  for mode L_OBJECT actor must be defined");
		return NULL;
	}

	ActionLookAt* looking = static_cast<ActionLookAt*>(ActionLibrary::instance().getActionPrototype("ActionLookAt"));
	looking->setParams(mode,point,object);
	if (priority!=1)
		looking->setPriority(priority);
	return setActionByPrototype(looking,parent);
}

void Actor::updateMemory()
{
	m_alertness = NORMAL_ALERTNESS;

	ObjectIds deletedFrames;
	unsigned int curTime = Platform::getTime();
	MemoryFrames::iterator it= m_memoryFrames.begin();
	while (it != m_memoryFrames.end())
	{
		Time forgetTime = it->second.forgetTime == INVALID_TIME ? m_forgetTime : it->second.forgetTime;
		if (it->second.time + forgetTime < curTime)
		{
			deletedFrames.push_back(it->first);
		}
		else
		{
			if((!WorldObject::isValid(it->first) || it->second.object->getDestroyed()) 
				/*  && (it->second.object->getType() == WO_GRENADE || it->second.object->getType() == WO_ACTOR)*/)
			{
				deletedFrames.push_back(it->first);
			}
			// object is still active
			else
			{
				// update alertness level
				if(m_alertness < it->second.alertness)
					m_alertness = it->second.alertness;
			}
		}
		it++;
	}

	for(size_t i = 0; i<deletedFrames.size();i++)
	{
		m_memoryFrames.erase(deletedFrames[i]);
	}

	selectEnemy();
	//GB_INFO("Actor: enemy update %p", m_enemy);
}


void Actor::addMemoryFrame(ObjectId object, MemoryFrame& frame, bool updateOnly /*= false*/)
{
	frame.object = dynamic_cast<WorldObject*>(BaseObject::findById(object));
	GB_ASSERT(frame.object, "Actor::addMemoryFrame: object must be defined");
	
	MemoryFrames::iterator it = m_memoryFrames.find(object);
	if(it == m_memoryFrames.end())
	{
		// don't add new memory frame
		if(updateOnly) 
			return;
	}
	else
		frame.type |= it->second.type;

	//for updateOnly time is not updated
	if (updateOnly)
		frame.time = it->second.time;
	else
		frame.time = Platform::getTime();
	
	m_memoryFrames[object] = frame;
}

void Actor::updateSensors()
{
	checkPerceptionEvents();
	updateVision();
}

void Actor::checkPerceptionEvents()
{
	PerceptionEvent::updateEvents();

	// find events in our radius
	PerceptionEvents::iterator it = PerceptionEvent::m_events.begin();
	for(;it != PerceptionEvent::m_events.end(); it++)
	{
		PerceptionEvent& event = *(it->second);
		if(event.m_initiatorID == getId()) continue;
		Vector3 pos = getPos();
		float eventVolume = event.getVolumeInPos(getPos());
		// check whether event has enough energy to be sensed by the actor
		if(eventVolume > m_hearingThreshold)
		{
			if(event.getAlertness() > NORMAL_ALERTNESS)
			{
				MemoryFrame frame;
				frame.type = event.getPerceptionType();
				frame.alertness = event.getAlertness();
				frame.distSq = -1.f;
				frame.fov = -1.f;
				frame.los = false;
				frame.forgetTime = 0;
				addMemoryFrame(event.getId(), frame);
			}

			if (event.m_initiatorID !=INVALID_OBJECT_ID && BaseObject::isValid(event.m_initiatorID))
			{
				WorldObject* obj = static_cast<WorldObject*>(BaseObject::findById(event.m_initiatorID));
				addVisualFrameToMemory(obj, true);
			}
		}
	}
}


void Actor::addVisualFrameToMemory(WorldObject* object, bool force /*= false*/)
{
	MemoryFrame frame;
	frame.type = ST_VISUAL;
	frame.object = object;
	frame.los = checkObjectVisibility(object, &frame.distSq, &frame.fov);

	// Won't add new frame if we don't see object actually (except force case).
	addMemoryFrame(object->getId(), frame, !frame.los && !force);
}

bool Actor::checkObjectVisibility(WorldObject* object, float* pDistSq /*= NULL*/, float* pFov /*= NULL*/)
{
	Vector3 pos = getPos();
	Vector3 lookDir = getEyesLookDir();// getLookDir();//
	Vector3 objectPos = object->getPos();
	Vector3 dirToObj = objectPos - pos;
	float distSq = dirToObj.lengthSq();
	
	dirToObj.normalizeSafe();
	float dotValue = clamp(dotProtuct(dirToObj,lookDir),-1.f,1.f);
	float fov = acosf(dotValue);
	
	if (pDistSq)	
		*pDistSq = distSq;
	if (pFov)	
		*pFov = fov;
	
	bool isVisible = distSq < m_viewDistance * m_viewDistance && fov < (m_fov/2.f); 
	if (isVisible)
		isVisible = checkObjectLos(object);
	return isVisible;
}

void Actor::setEnemyEvaluator(const EnemyEvaluator& evaluator)
{
	if (evaluator.empty())
		m_enemyEvaluator.bind(&Actor::defaultEnemyDangerEvaluator);
	else
		m_enemyEvaluator = evaluator;

}

float Actor::defaultEnemyDangerEvaluator(Actor* actor,WorldObject* enemy)
{
	float teamFactor = actor->getTeamRelationship(enemy);
	float danger = teamFactor;
	if (danger>0)
	{
		float dist = actor->m_memoryFrames[enemy->getId()].distSq;
		danger *= 1.f / dist;
	}	
	return danger;
}

void Actor::selectEnemy()
{
	if (m_enemyEvaluator.empty())
		setEnemyEvaluator();
	float maxDanger = 0.f;
	WorldObject* enemy = NULL;
	for(MemoryFrames::iterator it= m_memoryFrames.begin(), end = m_memoryFrames.end(); it != end; it++)
	{
		WorldObject* object = it->second.object;
		WorldObjectType objectType = object->getType();
		if (objectType != WorldObject::WO_ACTOR && objectType != WorldObject::WO_GRENADE)
			continue;
		float danger = m_enemyEvaluator(this, object);
		if (danger > maxDanger)
		{
			maxDanger = danger;
			enemy = object;
			m_alertness = ENEMY_ALERTNESS;
		}
	}
	
	setEnemy(enemy);
}

Actor::MemoryFrame* Actor::findNearestObjectInMemory(WorldObjectType type, float dist /*= -1.f*/, float fov /*= M_PI*/)
{
	MemoryFrame* nearestObjectFrame = NULL;
	float minDistSq = dist>0.f ? dist*dist : FLT_MAX;
	for(MemoryFrames::iterator it= m_memoryFrames.begin(),end=m_memoryFrames.end();it!=end;++it)
	{
		MemoryFrame& frame = it->second;
		if (frame.object && frame.object->getType()==type &&  frame.distSq < minDistSq && frame.fov<=fov)
		{
			nearestObjectFrame = &frame;
			minDistSq = frame.distSq;
		}
	}
	return nearestObjectFrame;
}

Actor::MemoryFrame* Actor::findNearestPerceptionEvent(PerceptionEvent::PerceptionEventType type, float dist /*= -1.f*/, float fov /*= M_PI*/)
{
	MemoryFrame* nearestEventFrame = NULL;
	float minDistSq = dist>0.f ? dist*dist : FLT_MAX;
	Vector3 pos = getPos();	
	for(MemoryFrames::iterator it= m_memoryFrames.begin(),end=m_memoryFrames.end();it!=end;++it)
	{
		MemoryFrame& frame = it->second;
		PerceptionEvent *perceptionEvent = dynamic_cast<PerceptionEvent*>(frame.object);

		if(!perceptionEvent || perceptionEvent->getType() != WO_PERCEPTION_EVENT || perceptionEvent->m_initiatorID == getId()) continue;

		Vector3 dirToObj = perceptionEvent->getPos() - pos;
		float distSq = dirToObj.lengthSq();
		if (perceptionEvent->getEventType()==type &&  distSq < minDistSq && frame.fov<=fov)
		{
			nearestEventFrame = &frame;						
			minDistSq = distSq;
		}
	}
	return nearestEventFrame;
}

void  Actor::setEnemy(WorldObject* newEnemy)
{
	if (m_enemy != newEnemy)
		m_enemy = newEnemy;
}

WorldObject* Actor::getEnemy()
{
	return m_enemy;
}

const Actor::MemoryFrame* Actor::getEnemyFrame()
{
	return m_enemy ? &m_memoryFrames[m_enemy->getId()] : NULL;
}

const Actor::MemoryFrame* Actor::getMemoryFrame(WorldObject* object)
{
	GB_ASSERT(object, "Actor::getMemoryFrame: wrong wolrd object");
	MemoryFrame* frame = NULL;
	ObjectId id = object->getId();
	MemoryFrames::iterator it = m_memoryFrames.find(id);
	if (it != m_memoryFrames.end())
	{
		frame = &it->second;
	}
	return frame;
}

void	Actor::setDead(bool withoutAction /*= false*/)
{
	GB_ASSERT(!m_isDead, "Actor::setDead: actor is already dead");
	m_isDead = true;
	setDestroyed(true);

	enablePerception(false);
	terminateAllActions();
	if (!withoutAction)
		setActionByName("ActionDeath");
}

void Actor::setControlledByPlayer(bool flg)
{
	m_isControlledByPlayer = flg;
}

void Actor::printActions(std::ostringstream& buff, bool full)
{
	buff.clear();
	buff<<"Actor state: "<<m_state->getName()<<std::endl;
	buff<<"Actor actions on effectors: "<<std::endl;
	for(EffectorMap::iterator it = m_effectors.begin(),end = m_effectors.end();
			it!=end;it++)
	{
		Effector* effector = it->second;
		buff<<Effector::convertToString(effector->getId())<<std::endl;
		std::string tabs;

		print(effector,m_rootAction,buff,tabs,full);
	}
}

void Actor::print(Effector* effector,Action* action,std::ostringstream& buff,std::string& tabs,bool full)
{
	static const char* spaces = "----";
	std::ostringstream actionBuff;
	if (action!=m_rootAction)
	{
		tabs += spaces;
		buff<<tabs;
		action->print(actionBuff);
		std::string actionStr = actionBuff.str();
		size_t pos = 0;
		while ((pos = actionStr.find("\n",pos))!=std::string::npos)
		{
			pos += 1;
			if (pos==actionStr.size())
				break;
			actionStr.insert(pos,tabs);
			pos += tabs.size();
		}
		buff<<actionStr;
	}
	

	Actions childs;
	if (full)
	{
		effector->getDirectChildren(action,childs);
		for (size_t i=0; i<childs.size();i++)
		{
			print(effector,childs[i],buff,tabs,full);
		}
	}
	else
	{
		Action* child = effector->getActiveChildAction(action);
		if (child)
			print(effector,child,buff,tabs,full);
	}
	
	if (action!=m_rootAction)
		tabs.erase(tabs.size()-sizeof(spaces),sizeof(spaces));
}