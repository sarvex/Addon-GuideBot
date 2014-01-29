//-----------------------------------------------------------------------------
// Guide Bot
// Copyright (C) LogicKing.com, Inc.
//-----------------------------------------------------------------------------

#include "guideBot/actions.h"
#include "guideBot/actor.h"

using namespace GuideBot;

////////////////////////////////////////////////////
//////*********ActionChangeState******//////////////
////////////////////////////////////////////////////
IMPLEMENT_REGISTER_ROUTINE(ActionChangeState);
ActionChangeState::ActionChangeState():	m_state(NULL)
										, m_changeVisual(NULL)
										, m_changeOnStart(false)
{
	setHighPriorityFlag(true);
}

void ActionChangeState::setParams(const std::string& stateName)
{
	GB_ASSERT(!stateName.empty(),"ActionChangeState::setParams: state name must be defined");
	m_stateName = stateName;
}

void ActionChangeState::prepareClone(Action* actionClone)
{
	Parent::prepareClone(actionClone);

	ActionChangeState* actionChangeStateClone = static_cast<ActionChangeState*>(actionClone);
	actionChangeStateClone->m_state = m_state;
	actionChangeStateClone->m_stateName = m_stateName;
	actionChangeStateClone->m_changeVisual = m_changeVisual;
}

bool ActionChangeState::check(Actor* owner, Action* parentAction)
{
	//find out change visual and required effectors;
	if (!m_stateName.empty())
		m_state = owner->getState(m_stateName);
	GB_ASSERT(m_state, "ActionChangeState::check: state must be defined");
	State* curState = owner->getActorState();
	if (m_state==curState)
		return false;
	std::string changeVisualName;
	if (!curState->getChangeVisualName(m_state->getName(),changeVisualName)) 
	{
		GB_ERROR("ActionChangeState::check: change visual is not define. Aborted: %s to %s", curState->getName().c_str(), m_stateName.c_str());
		return false;
	}

	m_changeVisual = owner->getVisual(changeVisualName);
	if (!m_changeVisual)
	{
		GB_ERROR("ActionChangeState::check: cann't find change visual %s. State change is aborted", changeVisualName.c_str());
		return false;
	}

	setRequiredEffectors(m_changeVisual->getEffectors(),true,2);// high priority for change state action

	return Parent::check(owner,parentAction);
}

void ActionChangeState::init()
{
	if (m_changeVisual->getPlayingTime()!=INVALID_TIME)
	{
		m_owner->setActorState(m_state);
		m_changeOnStart = true;
	}
	m_owner->playVisual(m_changeVisual,this);
}

void ActionChangeState::onChildTerminated(Action* terminatedChild)
{
	Parent::onChildTerminated(terminatedChild);
	if (!m_changeOnStart && m_owner->getActorState()!=m_state)
		m_owner->setActorState(m_state);
	terminate();
}

void ActionChangeState::terminate(TerminationStatus status /*= FINISHED*/)
{
	if (status == INTERUPTED && checkPrincipalEffectors())
	{
		m_owner->setActorState(m_state);
		return;
	}
	Parent::terminate(status);
}

void ActionChangeState::print(std::ostringstream& buff)
{
	Parent::print(buff);
	if (m_state)
	{
		buff<<" current state: "<<m_owner->getActorState()<<" target state: "<<m_state->getName()<<std::endl;
	}
}

std::string ActionChangeState::getDescString()
{
	std::string str = Parent::getDescString();
	if (m_state)
	{
		str.append(" ");
		str.append(m_state->getName());
	}
	return str;
}

////////////////////////////////////////////////////
//////***********ActionLookAt*********//////////////
////////////////////////////////////////////////////

IMPLEMENT_REGISTER_ROUTINE(ActionLookAt);

ActionLookAt::ActionLookAt():	m_mode(L_FORWARD)
								, m_targetPoint(Vector3::ZERO)
								, m_targetObject(NULL)
								, m_direction(Vector3::ZERO)
{
	addRequiredEffector(E_EYES,false);
}

void ActionLookAt::setParams(LookDirMode mode,const Vector3* point,WorldObject* object)
{
	m_mode = mode;
	if (m_mode == L_POINT && point)
	{
		m_targetPoint = *point;
	}
	else if (m_mode == L_DIR && point)
	{
		m_direction = *point;
	}
	else if (m_mode == L_OBJECT && object)
	{
		m_targetObject = object;
	}	
}

void ActionLookAt::setDirection(const Vector3& dir)
{
	m_direction = dir;
	if (!m_direction .isUnit())
		m_direction.normalizeSafe();
}

void ActionLookAt::updateDynamicParams(Action* actionClone)
{
	Parent::updateDynamicParams(actionClone);

	ActionLookAt* actionLookingClone = static_cast<ActionLookAt*>(actionClone);
	actionLookingClone->m_mode = m_mode;
	actionLookingClone->m_targetPoint = m_targetPoint;
	actionLookingClone->m_targetObject = m_targetObject;
	actionLookingClone->m_direction = m_direction;
	
}

void ActionLookAt::init()
{
	Parent::init();
	update();
}

void ActionLookAt::terminate(TerminationStatus status /* = FINISHED */)
{
	m_owner->setPitch(0.f);
	Parent::terminate(status);
}


bool ActionLookAt::update()
{
	if (!Parent::update())
		return false;

	Vector3 lookDir;

	if (m_mode == L_FORWARD)
	{
		lookDir = m_owner->getLastMoveDir(); //m_owner->getMoveDir();
	}
	else if (m_mode == L_DIR)
	{
		lookDir = m_direction;
	}
	else
	{
		if (m_mode == L_OBJECT)
		{
			if (m_targetObject->getDestroyed())
			{
				m_targetObject = NULL;
				terminate();
				return false;
			}
			m_targetPoint = m_targetObject->getPos();
		}
		Vector3 pos = m_owner->getPos();
		lookDir = m_targetPoint - pos;
	}

	if (!lookDir.isZero())
	{
		//GB_INFO("ActionLookAt::update() %f %f %f",lookDir.x, lookDir.y,lookDir.z);
		lookDir.normalize();
		m_owner->setLookDir(lookDir);
		m_owner->setPitch(lookDir.z);
	}
	else
	{
		m_owner->setPitch(0.f);
	}

	return true;
}

////////////////////////////////////////////////////
//////************ActionDeath*********//////////////
////////////////////////////////////////////////////
IMPLEMENT_REGISTER_ROUTINE(ActionDeath);
ActionDeath::ActionDeath()
{
	addRequiredEffector(E_BODY,true,10);
}
void ActionDeath::init()
{
	ActionPlayVisual* playVisual = static_cast<ActionPlayVisual*>(m_owner->playVisual("death",this));
	if (playVisual)
		playVisual->setHoldAtEnd(true);
}

void ActionDeath::onChildTerminated(Action* terminatedChild)
{
	Parent::onChildTerminated(terminatedChild);
	terminate();
}

////////////////////////////////////////////////////
//////************ActionSlope*********//////////////
////////////////////////////////////////////////////
IMPLEMENT_REGISTER_ROUTINE(ActionSlope);
ActionSlope::ActionSlope():	m_state(NULL)
							, m_pitch(0.f)
{
	addRequiredEffector(E_SPINE,false);
}

void ActionSlope::init()
{
	Parent::init();
	playSlopeVisual();
}

bool ActionSlope::update()
{
	if (!Parent::update())
		return false;

	if (m_state != m_owner->getActorState())
		playSlopeVisual();

	if (m_pitch != m_owner->getPitch())
		updateSlopeValue();



	return true;
}

void ActionSlope::onEffectorsAcquired(const EffectorIDVector& newEffectors)
{
	Parent::onEffectorsAcquired(newEffectors);
	playSlopeVisual();
}

void ActionSlope::playSlopeVisual()
{
	m_state = m_owner->getActorState();
	ActionPlayVisual* slopeVisual = getSlopeVisual();
	if (slopeVisual)
		slopeVisual->terminate(CANCELED);
	if (m_owner->hasVisual("look"))
	{
		slopeVisual = static_cast<ActionPlayVisual*>(m_owner->playVisual("look",this));
		if (slopeVisual)
		{
			slopeVisual->setTimeScale(0.f);
			slopeVisual->setClientControlled();
			updateSlopeValue();
		}
	}
}

void ActionSlope::updateSlopeValue()
{
	m_pitch = m_owner->getPitch();
	ActionPlayVisual* slopeVisual = getSlopeVisual();
	if (slopeVisual)
	{
		float animationPos = 0.5f*(1.f - m_pitch);
		slopeVisual->setPosition(animationPos);
	}
}

ActionPlayVisual* ActionSlope::getSlopeVisual()
{
	ActionPlayVisual* slopeVisual = GET_CHILD(ActionPlayVisual);
	return slopeVisual;
}

////////////////////////////////////////////////////
//////************ActionPause*********//////////////
////////////////////////////////////////////////////
namespace
{
	const Time defaultPause = 1000;
	const EffectorID defaultEffector = E_HANDS;
}

IMPLEMENT_REGISTER_ROUTINE(ActionPause);
ActionPause::ActionPause():	m_duration(defaultPause)
{
}

void ActionPause::setDuration(Time duration)
{
	m_duration = duration;
}

void ActionPause::updateDynamicParams(Action* actionClone)
{
	Parent::updateDynamicParams(actionClone);
	ActionPause* actionPause = static_cast<ActionPause*>(actionClone);
	actionPause->m_duration = m_duration;
}

void ActionPause::init()
{
	Parent::init();
	if (m_requiredEffectors.empty())
		addRequiredEffector(defaultEffector,true);
	m_initTime = Platform::getTime();
}

bool ActionPause::update()
{
	if (!Parent::update())
		return false;

	Time curTime = Platform::getTime();
	if (curTime - m_initTime > m_duration)
	{
		terminate();
		return false;
	}

	return true;
}