//-----------------------------------------------------------------------------
// Guide Bot
// Copyright (C) LogicKing.com, Inc.
//-----------------------------------------------------------------------------
#ifndef _ANCHOR_H 
#define	_ANCHOR_H

#include "guideBot/worldObject.h"
#include "guideBot/FastDelegate.h"

namespace GuideBot
{

class Actor;

////////////////////////////////////////////////////
//////*************Anchor*************//////////////
////////////////////////////////////////////////////
class Anchor;
typedef std::vector<Anchor*>	Anchors;


class Anchor: public Waypoint
{
public:
	Anchor(const Matrix& tm, float radius);
	~Anchor();
	typedef Delegate<void (Anchor*, Actor*)> AnchorCallback;

	void setGuest(Actor* guest) { m_guest = guest;};
	Actor* getGuest() { return m_guest;};

	bool isVacant(Actor* guest);

	Vector3 getCoverPosition(WorldObject* enemy);
	bool isInCover(Vector3 pos, WorldObject* enemy);

	
	static Anchors&  getAnchors(){return m_anchors;};
protected:
	AnchorCallback	m_enterCallback;
	AnchorCallback	m_leaveCallback;

	float			m_coverAngle;
	float			m_coverDist;

	Actor*			m_guest;
	static Anchors	m_anchors;
};

} //namespace GuideBot

#endif