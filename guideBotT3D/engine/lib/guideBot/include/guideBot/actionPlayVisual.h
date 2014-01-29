//-----------------------------------------------------------------------------
// Guide Bot
// Copyright (C) LogicKing.com, Inc.
//-----------------------------------------------------------------------------

#ifndef _ACTIONPLAYVISUAL_H
#define _ACTIONPLAYVISUAL_H

#include "guideBot/action.h"

namespace GuideBot
{

////////////////////////////////////////////////////
//////*********ActionPlayVisual*******//////////////
////////////////////////////////////////////////////
typedef int VisualIdx;
const int INVALID_VISUAL_IDX = -1;

class ActionPlayVisual: public Action
{
	typedef Action Parent;
public:
	DEFINE_REGISTER_ROUTINE(ActionPlayVisual);
	ActionPlayVisual();
	virtual void setParams(Visual* visual,bool allowInactive = false);

	virtual void prepareClone(Action* actionClone);
	virtual bool check(Actor* owner, Action* parentAction);
	virtual void init();
	virtual bool update();
	virtual void terminate(TerminationStatus status = FINISHED);
	virtual void onEffectorsLost(const EffectorIDVector& lostEffectors);
	virtual void onEffectorsAcquired(const EffectorIDVector& newEffectors);

	Visual* getVisual(){return m_visual;};

	//end callback
	virtual void onVisualEnd(float pos);
	typedef Delegate<void(float)> VisualEndCallback;

	void setHoldAtEnd(bool flg) {m_holdAtEnd = flg;};
	void setTimeScale(float scale);
	void setPosition(float pos);
	void setClientControlled();

	VisualIdx getVisualIdx() {return m_visualIdx;};

	//debug
	virtual void print(std::ostringstream& buff);

protected:
	void play();
	void stop();
	
	Visual*					m_visual;
	bool					m_allowInactive;
	
	bool					m_isPlaying;

	Time					m_playingTime;
	Time					m_startTime;
	VisualEndCallback		m_visualEndCallback;
	bool					m_holdAtEnd;

	VisualIdx				m_visualIdx;
};

} //namespace GuideBot

#endif