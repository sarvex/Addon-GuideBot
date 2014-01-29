//-----------------------------------------------------------------------------
// Guide Bot
// Copyright (C) LogicKing.com, Inc.
//-----------------------------------------------------------------------------

#include "guideBot/anchor.h"
#include "guideBot/actor.h"
#include "guideBot/platform.h"

#include <algorithm>

using namespace GuideBot;

////////////////////////////////////////////////////
//////*************Anchor*************//////////////
////////////////////////////////////////////////////
Anchors Anchor::m_anchors;

Anchor::Anchor(const Matrix& tm, float radius): Waypoint(WO_ANCHOR,tm,radius), m_guest(NULL)
{
	m_anchors.push_back(this);
	m_coverAngle = (float) M_PI/4.f;
	m_coverDist = 3.f;
}

Anchor::~Anchor()
{
	Anchors::iterator it = std::find(m_anchors.begin(),m_anchors.end(),this);
	GB_ASSERT(it!=m_anchors.end(), "Anchor::~Anchor: can't unregister anchor");
	m_anchors.erase(it);
}

bool Anchor::isVacant(Actor* guest)
{
	bool res = !m_guest ||  m_guest == guest;
	return res;
}

Vector3 Anchor::getCoverPosition(WorldObject* enemy)
{
	Vector3 coverPos;
	Vector3 ourPos = getPos();
	Vector3 toEnemy = enemy->getPos() - getPos();
	toEnemy.normalizeSafe();
	coverPos = ourPos - (m_radius+0.5f)*toEnemy;
	return coverPos;
}

bool Anchor::isInCover(Vector3 pos, WorldObject* enemy)
{
	//check by dist
	if (!isInside(pos, m_coverDist))
		return false;

	//check by angle
	Vector3 anchorPos = getPos();
	Vector3 fromEnemy = anchorPos - enemy->getPos();
	Vector3 toUs = pos - anchorPos;
	fromEnemy.z = toUs.z = 0.f;
	fromEnemy.normalizeSafe();
	toUs.normalizeSafe();

	float cosAngle = dotProtuct(fromEnemy,toUs);
	if (cosAngle>cosf(m_coverAngle))
		return true;

	return false;
}

