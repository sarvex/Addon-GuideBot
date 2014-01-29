//-----------------------------------------------------------------------------
// Guide Bot
// Copyright (C) LogicKing.com, Inc.
//-----------------------------------------------------------------------------

#ifndef _WORLD_OBJECT_H
#define _WORLD_OBJECT_H

#include <vector>
#include <map>

#include "guideBot/mathUtils.h"
#include "guideBot/stringUtils.h"
#include "guideBot/platform.h"

namespace GuideBot
{

////////////////////////////////////////////////////
////// Types
////////////////////////////////////////////////////

enum SensorType { ST_VISUAL = 1, ST_HEARING = 1 << 1, ST_KNOWLEDGE = 1 << 2};

// Alertness is number in a range of [0, 1]. Types of alertness
const float NO_ALERTNESS		= -1.f;
const float NORMAL_ALERTNESS	= 0.f;
const float AWARE_ALERTNESS		= 0.2f;
const float ALARM_ALERTNESS		= 0.8f;
const float ENEMY_ALERTNESS		= 1.0f;

////////////////////////////////////////////////////
//////***********BaseObject***********//////////////
////////////////////////////////////////////////////
class BaseObject
{
public:
	BaseObject();
	virtual ~BaseObject();

	typedef unsigned int ObjectId;
	static const ObjectId INVALID_OBJECT_ID;

	virtual ObjectId	registerObject();
	virtual void		unregisterObject();

	//user data
	virtual void* getUserData()					{return m_userData;};
	virtual void  setUserData(void* userData)	{ m_userData = userData;};

	ObjectId getId() const {return m_id;}

	static bool isValid(ObjectId id);
	static BaseObject* findById(ObjectId id);
	
protected:
	void*	m_userData;

private:
	typedef std::map<ObjectId, BaseObject*>	ObjectsMap;
	ObjectId m_id;
	
	static ObjectId m_totalObjectsCounter;
	static ObjectsMap m_allObjects;
};

typedef std::vector<BaseObject::ObjectId> ObjectIds;

////////////////////////////////////////////////////
//////***********WorldObject**********//////////////
////////////////////////////////////////////////////

class WorldObject: public BaseObject
{
public:
	enum WorldObjectType 
	{ 
		WO_OBJECT, 
		WO_WAYPOINT, 
		WO_OBSTACLE, 
		WO_ANCHOR, 
		WO_GRENADE, 		
		WO_ACTOR,
		WO_PERCEPTION_EVENT, 
		WO_TYPES_NUM
	};

	WorldObject(WorldObjectType type);
	virtual ~WorldObject();

	WorldObjectType	getType() {return m_type;};

	const Matrix& getTm();
	virtual void setTm(const Matrix& tm) { m_tm = tm;};

	Vector3 getLookDir();
	virtual void	setLookDir(const Vector3& lookDir);

	Vector3	getPos();
	void	setPos(const Vector3& pos);

	virtual void getAABB(Vector3& _min, Vector3& _max);

	void setDestroyed(bool isDestroyed);
	bool getDestroyed() {return m_isDestroyed;}

	void setParent(WorldObject* parent) { m_parent = parent;};

	static EnumDescription enumDescription;
protected:
	virtual const Matrix& getObjectTm() {return m_tm;};

	WorldObjectType	m_type;
	Matrix			m_tm;
	bool			m_isDestroyed;
	WorldObject*	m_parent;
};

typedef std::vector<WorldObject*> WorldObjects;


////////////////////////////////////////////////////
//////*************Waypoint***********//////////////
////////////////////////////////////////////////////
class Waypoint;
typedef std::vector<Waypoint*>	Waypoints;

class Waypoint: public WorldObject
{
	typedef WorldObject Parent;
public:
	Waypoint(const Matrix& tm, float radius);
	Waypoint(WorldObjectType type, const Matrix& tm, float radius);
	~Waypoint();

	virtual void getAABB(Vector3& _min, Vector3& _max);

	float getRadius() {return m_radius;};
	
	bool	isInside(const Vector3& pos, float tolerance = 0.f);
	float	getDistSq(const Vector3& pos);

	static Waypoints&  getWaypoints(){return m_waypoints;};
protected:
	float				m_radius;
	static Waypoints	m_waypoints;
};

////////////////////////////////////////////////////
//////*************Obstacle***********//////////////
////////////////////////////////////////////////////
class Obstacle;
typedef std::vector<Obstacle*>	Obstacles;

class Obstacle: public Waypoint
{
	typedef Waypoint Parent;
public:
	enum ObstacleType 
	{	
		OT_LOW, 
		OT_MEDIUM, 
		OT_HIGH,
		OT_TYPES_NUM
	};

	Obstacle(ObstacleType type, const Matrix& tm, float radius);
	~Obstacle();

	static EnumDescription enumDescription;
	static Obstacles&  getObstacles(){return m_obsacles;};
protected:
	ObstacleType	m_obstacleType;

	static Obstacles m_obsacles;
};

} //namespace GuideBot

#endif