//-----------------------------------------------------------------------------
// Guide Bot
// Copyright (C) LogicKing.com, Inc.
//-----------------------------------------------------------------------------

#include "guideBot/actionPlayVisual.h"
#include "guideBot/actor.h"

#include <algorithm>

using namespace GuideBot;

////////////////////////////////////////////////////
//////*********ActionPlayVisual*******//////////////
////////////////////////////////////////////////////
IMPLEMENT_REGISTER_ROUTINE(ActionPlayVisual);
ActionPlayVisual::ActionPlayVisual():	m_visual(NULL)
										, m_allowInactive(false)
										, m_isPlaying(false)
										, m_holdAtEnd(false)
										, m_visualIdx(INVALID_VISUAL_IDX)
{

}

void ActionPlayVisual::setParams(Visual* visual,bool allowInactive)
{
	GB_ASSERT(visual,"ActionPlayVisual::setParams: visual must be defined");
	m_allowInactive = allowInactive;
	m_visual = visual;
	setRequiredEffectors(m_visual->getEffectors());
}

void ActionPlayVisual::prepareClone(Action* actionClone)
{
	Parent::prepareClone(actionClone);
	ActionPlayVisual*  actionVisualClone = static_cast<ActionPlayVisual*>(actionClone);

	actionVisualClone->m_visual = m_visual;
	actionVisualClone->m_allowInactive = m_allowInactive;
}

bool ActionPlayVisual::check(Actor* owner, Action* parentAction)
{
	if (!Parent::check(owner,parentAction) && !m_allowInactive)
		return false;

	//check whether this visual already played on effectors
	bool alreadyPlayed = true;
	for (size_t i = 0; i<m_acquiredEffectors.size(); i++)
	{
		const EffectorIDVector& effectorIdVector = parentAction->getRequiredEffectors();
		if (std::find(effectorIdVector.begin(),effectorIdVector.end(),m_acquiredEffectors[i])!=effectorIdVector.end())
		{
			Action* settedAction =  owner->getEffector(m_acquiredEffectors[i])->getActiveChildAction(parentAction);
			ActionPlayVisual* settedActionVisual = settedAction ? dynamic_cast<ActionPlayVisual*>(settedAction): NULL; 
			if (settedActionVisual && settedActionVisual->getVisual()==m_visual)
				continue;
		}
		alreadyPlayed = false;
		break;			
	}

	return !alreadyPlayed;
}

void ActionPlayVisual::init()
{
	if (!m_allowInactive || m_acquiredEffectors.size() == m_requiredEffectors.size())
	{
		play();
	}
}

bool ActionPlayVisual::update()
{
	if (!Parent::update())
		return false;

	Time curTime = Platform::getTime();
	if (m_playingTime!=INVALID_TIME && curTime - m_startTime > m_playingTime)
	{
		terminate();
		return false;
	}
	return true;
}

void ActionPlayVisual::terminate(TerminationStatus status /*= FINISHED*/)
{
	if (m_isPlaying)
		stop();

	Parent::terminate(status);
}

void ActionPlayVisual::onEffectorsLost(const EffectorIDVector& lostEffectors)
{
	if (!m_allowInactive)
		Parent::onEffectorsLost(lostEffectors);	//check principal effectors
	else
		stop();
}

void ActionPlayVisual::onEffectorsAcquired(const EffectorIDVector& newEffectors)
{
	Parent::onEffectorsAcquired(newEffectors);
	if (m_acquiredEffectors.size() == m_requiredEffectors.size())
	{
		//we have all effectors now
		play();
	}
}

void ActionPlayVisual::play()
{
	m_isPlaying = true;

	bool playing = false;
	m_visualIdx = m_owner->startVisual(m_visual);
	if (m_visualIdx != INVALID_VISUAL_IDX)
	{
		playing = true;
		
		//set end callback for non-cyclic visual
		if (!m_owner->isCyclicVisual(m_visual))
		{
			m_visualEndCallback.bind(this,&ActionPlayVisual::onVisualEnd);
			m_owner->setVisualEndCallback(m_visualIdx,&m_visualEndCallback);
		}

		m_owner->setAnimCallbacks(m_visualIdx,m_visual->getCallbackMap());
	}

	m_playingTime = m_visual->getPlayingTime();
	if (!playing && m_playingTime==INVALID_TIME)
		terminate();
	else
		m_startTime = Platform::getTime();
}

void ActionPlayVisual::stop()
{
	m_isPlaying = false;
	if (m_visualIdx != INVALID_VISUAL_IDX)
	{
		m_owner->stopVisual(m_visualIdx);
		m_visualIdx = INVALID_VISUAL_IDX;
	}	
}

void ActionPlayVisual::onVisualEnd(float pos)
{
	if (!m_holdAtEnd)
		terminate();
	return;
}

void ActionPlayVisual::setTimeScale(float scale)
{
	if (!m_isPlaying || m_visualIdx == INVALID_VISUAL_IDX)
		return;
	m_owner->setAnimationTimeScale(m_visualIdx, scale);
}

void ActionPlayVisual::setPosition(float pos)
{
	if (!m_isPlaying || m_visualIdx == INVALID_VISUAL_IDX)
		return;
	m_owner->setAnimationPosition(m_visualIdx, pos);
}

void ActionPlayVisual::setClientControlled()
{
	if (!m_isPlaying || m_visualIdx == INVALID_VISUAL_IDX)
		return;
	m_owner->setClientControlled(m_visualIdx);
}

void ActionPlayVisual::print(std::ostringstream& buff)
{
	Parent::print(buff);
	if (m_visual)
	{
		buff<<"    visual: "<<m_visual->getName()<<std::endl;
	}
}
