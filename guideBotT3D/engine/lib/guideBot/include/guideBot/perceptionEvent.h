//-----------------------------------------------------------------------------
// Guide Bot
// Copyright (C) LogicKing.com, Inc.
//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------

#ifndef _PERCEPTION_EVENT_H
#define _PERCEPTION_EVENT_H

#include <map>

#include "guideBot/platform.h"
#include "guideBot/worldObject.h"

namespace GuideBot
{

class PerceptionEvent;

typedef int PerceptionEventId;
typedef std::map<PerceptionEventId, PerceptionEvent*> PerceptionEvents;


class PerceptionEvent : public WorldObject
{
public:
	PerceptionEvent();
	virtual ~PerceptionEvent();

	enum PerceptionEventType
	{
		PE_NEUTRAL,
		PE_DANGER,
		PE_TYPES_AMOUNT
	};

	float getVolumeInPos(const Vector3& listenerPos);

	PerceptionEventType getEventType() {return m_eventType; }
	SensorType getPerceptionType() {return m_sensorType; }
	float getAlertness() {return m_alertness; }	
protected:
	Time m_activationTime;
public:
	/// Time for the event to exists and -1 if it exist infinitely
	Time m_lifeTime;

	float m_radiusInner;
	float m_radiusOuter;
	float m_volume;
	
	float m_alertness;
	SensorType	m_sensorType;
	PerceptionEventType m_eventType;
	ObjectId m_initiatorID;

public:
	static void updateEvents();
	static PerceptionEventId addEvent(PerceptionEvent* event);
	static bool removeEvent(const PerceptionEventId eventId);

	static PerceptionEvents m_events;
private:
	static PerceptionEventId m_eventsCount;
};

} //namespace GuideBot

#endif