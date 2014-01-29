//-----------------------------------------------------------------------------
// Guide Bot
// Copyright (C) LogicKing.com, Inc.
//-----------------------------------------------------------------------------

#include "guideBot/worldObject.h"
#include "guideBot/platform.h"

#include <algorithm>

using namespace GuideBot;

////////////////////////////////////////////////////
//////***********BaseObject**********///////////////
////////////////////////////////////////////////////
BaseObject::ObjectId BaseObject::m_totalObjectsCounter = 0;
BaseObject::ObjectsMap BaseObject::m_allObjects;
const BaseObject::ObjectId BaseObject::INVALID_OBJECT_ID = -1;

BaseObject::BaseObject(): m_userData(NULL) 
{
	m_id = INVALID_OBJECT_ID;
};

BaseObject::~BaseObject()
{
	unregisterObject();
}

BaseObject::ObjectId BaseObject::registerObject()
{
	GB_ASSERT(m_id == INVALID_OBJECT_ID, "Object already registered!");
	m_id = m_totalObjectsCounter;
	m_allObjects.insert(std::make_pair(m_id, this));
	m_totalObjectsCounter++;
	return m_id;
}

void BaseObject::unregisterObject()
{
	m_allObjects.erase(m_id);
	m_id = INVALID_OBJECT_ID;
}

bool BaseObject::isValid(ObjectId id)
{
	if(id == INVALID_OBJECT_ID) 
		return false;
	else
		return m_allObjects.find(id) != m_allObjects.end();
}

BaseObject* BaseObject::findById(ObjectId id)
{
	ObjectsMap::iterator it = m_allObjects.find(id);
	if(it != m_allObjects.end())
		return it->second;
	return NULL;
}

////////////////////////////////////////////////////
//////***********WorldObject**********//////////////
////////////////////////////////////////////////////

static const EnumDescription::Record worldObjectTypeEnums[] =
{
	{ WorldObject::WO_OBJECT, "object" },
	{ WorldObject::WO_WAYPOINT, "waypoint" },
	{ WorldObject::WO_OBSTACLE, "obstacle" },
	{ WorldObject::WO_ANCHOR, "anchor" },
	{ WorldObject::WO_GRENADE, "grenade" },
	{ WorldObject::WO_ACTOR, "actor" },
};

EnumDescription WorldObject::enumDescription(WorldObject::WO_TYPES_NUM, worldObjectTypeEnums);

WorldObject::WorldObject(WorldObjectType type) :	m_type(type)
													, m_tm(true)
													, m_isDestroyed(false) 
													, m_parent(NULL)
{
	//GB_INFO("World object: constructor %d %p ",(int)type,this);
}

WorldObject::~WorldObject()
{
	//GB_INFO("World object: destroying %p time: %d",this,Platform::getTime());
}

const Matrix& WorldObject::getTm()
{
	if (m_parent)
	{
		return m_parent->getTm();
	}
	else
	{
		return getObjectTm();
	}
}

Vector3 WorldObject::getLookDir()
{
	return getTm().getColumn(1);
};

void	WorldObject::setLookDir(const Vector3& lookDir)
{
	static const Vector3 worldUp(0.f,0.f,1.f);
	Vector3 forward = lookDir;
	if (!forward.isUnit())
		forward.normalize();
	Vector3 right = crossProduct(forward, worldUp);
	Vector3 up = crossProduct(right,forward);

	Matrix tm(true);
	tm.setColumn(0,right);
	tm.setColumn(1,forward);
	tm.setColumn(2,up);
	tm.setPosition(getPos());

	setTm(tm);
}

Vector3	WorldObject::getPos()
{
	return getTm().getPosition();
}

void WorldObject::setPos(const Vector3& pos)
{
	Matrix tm = getTm();
	tm.setPosition(pos);
	setTm(tm);
}

void WorldObject::getAABB(Vector3& _min, Vector3& _max)
{
	_min = _max = getPos();
}


void WorldObject::setDestroyed(bool isDestroyed) 
{
	m_isDestroyed = isDestroyed;
	//GB_INFO("World object: set destroyed %p time: %d",this,Platform::getTime());
}

////////////////////////////////////////////////////
//////*************Waypoint***********//////////////
////////////////////////////////////////////////////
Waypoints Waypoint::m_waypoints;

Waypoint::Waypoint(const Matrix& tm, float radius):	WorldObject(WO_WAYPOINT)
														, m_radius(radius)
{
	m_tm = tm;
	m_waypoints.push_back(this);
}

Waypoint::Waypoint(WorldObjectType type, const Matrix& tm, float radius):	WorldObject(type)
																			, m_radius(radius)
{
	m_tm = tm;
	m_waypoints.push_back(this);
}

Waypoint::~Waypoint()
{
	Waypoints::iterator it = std::find(m_waypoints.begin(),m_waypoints.end(),this);
	GB_ASSERT(it!=m_waypoints.end(), "Waypoint::~Waypoint: can't unregister waypoint");
	m_waypoints.erase(it);
}

void Waypoint::getAABB(Vector3& _min, Vector3& _max)
{
	_min = _max = getPos();
}

bool Waypoint::isInside(const Vector3& pos,float tolerance)
{
	float effectiveRadius = m_radius+tolerance;
	bool res = getDistSq(pos) < effectiveRadius *effectiveRadius;
	return res;
}

float	Waypoint::getDistSq(const Vector3& pos)
{
	float distSq = (getPos() - pos).lengthSq();
	return distSq;
}



////////////////////////////////////////////////////
//////*************Obstacle***********//////////////
////////////////////////////////////////////////////
static const EnumDescription::Record obstacleTypeEnums[] =
{
	{ Obstacle::OT_LOW, "low" },
	{ Obstacle::OT_MEDIUM, "medium" },
	{ Obstacle::OT_HIGH, "high" },
};

EnumDescription Obstacle::enumDescription(Obstacle::OT_TYPES_NUM, obstacleTypeEnums);

Obstacles Obstacle::m_obsacles;

Obstacle::Obstacle(ObstacleType type, const Matrix& tm, float radius):	Waypoint(WO_OBSTACLE, tm, radius)
																		, m_obstacleType(type)
{
	m_obsacles.push_back(this);
};

Obstacle::~Obstacle()
{
	Obstacles::iterator it = std::find(m_obsacles.begin(),m_obsacles.end(),this);
	GB_ASSERT(it!=m_obsacles.end(), "Obstacle::~Obstacle: can't unregister obstacle");
	m_obsacles.erase(it);
}
