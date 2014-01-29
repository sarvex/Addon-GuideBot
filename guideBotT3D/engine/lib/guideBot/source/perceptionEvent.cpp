//-----------------------------------------------------------------------------
// Guide Bot
// Copyright (C) LogicKing.com, Inc.
//-----------------------------------------------------------------------------


#include "guideBot/perceptionEvent.h"

using namespace GuideBot;

PerceptionEvents PerceptionEvent::m_events;
PerceptionEventId PerceptionEvent::m_eventsCount = 0;


PerceptionEvent::PerceptionEvent() : WorldObject(WO_PERCEPTION_EVENT)
{
	m_activationTime = 0;
	m_lifeTime = 0;
	m_alertness = NO_ALERTNESS;
	m_radiusInner = 1.f;
	m_radiusOuter = 1.f;
	m_volume = 1.f;
	m_initiatorID = INVALID_OBJECT_ID;
	m_eventType = PE_NEUTRAL;
}


PerceptionEvent::~PerceptionEvent()
{
}

PerceptionEventId PerceptionEvent::addEvent(PerceptionEvent* event)
{
	GB_ASSERT(event, "Wrong event pointer");

	event->registerObject();
	PerceptionEventId curEventId = m_eventsCount;
	event->m_activationTime = Platform::getTime();
	m_events.insert(std::make_pair(curEventId, event));
	m_eventsCount++; 
	return curEventId;
}

bool PerceptionEvent::removeEvent(const PerceptionEventId eventId)
{
	PerceptionEvents::iterator it = m_events.find(eventId);
	if(it == m_events.end())
		return false;
	delete it->second;
	m_events.erase(it);
	return true;
}

void PerceptionEvent::updateEvents()
{
	Time curTime = Platform::getTime();

	PerceptionEvents::iterator it = m_events.begin();
	while(it != m_events.end())
	{
		PerceptionEvent& event = *it->second;
		if(event.m_lifeTime != -1 && event.m_activationTime + event.m_lifeTime < curTime)
			it = m_events.erase(it);
		else
			it++;
	}
}

float PerceptionEvent::getVolumeInPos(const Vector3& listenerPos)
{
	float volume = 0.f;
	Vector3 dir = listenerPos - getPos();
	float dist = dir.length() - m_radiusInner;
	float radiusDelta = m_radiusOuter -  m_radiusInner;
	float factor;
	if (radiusDelta>0.f)
	{
		dist = clamp(dist, 0.f, radiusDelta);	
		factor = 1.f - dist / radiusDelta;
	}
	else
	{
		factor = dist>0.f ? 0.f : 1.f;
	}
	volume = lerp( 0.f, m_volume, factor);
	return volume;
}