//-----------------------------------------------------------------------------
// Guide Bot
// Copyright (C) LogicKing.com, Inc.
//-----------------------------------------------------------------------------

#ifndef _EVENTS_H
#define _EVENTS_H

namespace GuideBot
{

class Event
{
	Event();
	virtual ~Event();

	static void updateEvents();

	enum EventType
	{
		EV_SOUND	= 0,
		EV_VISUAL,
		EV_ALL_EFFECTORS,
	};
public:

	unsigned int m_activationTime;
	/// Time for the event to exists and -1 if it exist infinitely
	unsigned int m_lifeTime;

};

} //namespace GuideBot

#endif