//-----------------------------------------------------------------------------
// Guide Bot
// Copyright (C) LogicKing.com, Inc.
//-----------------------------------------------------------------------------

#include "guideBot/anchor.h"
#include "T3D/logickingMechanics/guideBot/torqueGuideBotPlatform.h"
#include "scene/sceneObject.h"
#include "console/console.h"

const float DEFAULT_RADIUS = 0.2f;

ConsoleMethod( SceneObject, getWorldObjectType, const char*, 2, 2, "()")
{
	GuideBot::WorldObject* worldObject = object->getWorldObject();
	const char* res = NULL;
	if (worldObject)
	{
		GuideBot::WorldObject::WorldObjectType type = worldObject->getType();
		res = GuideBot::WorldObject::enumDescription.getNameByValue(type);
		if (!res)
		{
			Con::errorf("SceneObject::getWorldObjectType: can't find string for enum value %d", type);
		}
	}
	return res;
}

ConsoleFunction(destroyWorldObject, void,2,2, "worldObject id")
{
	GuideBot::WorldObject::ObjectId id = dAtoi(argv[1]);
	GuideBot::WorldObject* waypoint = dynamic_cast<GuideBot::WorldObject*>(GuideBot::WorldObject::findById(id));
	if (!waypoint)
	{
		Con::errorf("destroyWorldObject: can't find waypoint by id %d",id);
		return;
	}

	waypoint->setDestroyed(true);
	GB_SAFE_DELETE(waypoint);
}

namespace
{
	void readTransform(const char* str, MatrixF& tr )
	{
		Point3F pos;
		AngAxisF aa;
		dSscanf(str,"%g %g %g %g %g %g %g",
			&pos.x,&pos.y,&pos.z,&aa.axis.x,&aa.axis.y,&aa.axis.z,&aa.angle);
		aa.setMatrix(&tr);
		tr.setColumn(3,pos);
	}
}

ConsoleFunction(createWaypoint,S32,3,4, "object, transform(pos axis angle), radius")
{
	//parent object
	SceneObject* object = static_cast<SceneObject*>(Sim::findObject(argv[1]));
	GuideBot::WorldObject* worldObject = object ? object->getWorldObject() : NULL;
	if (!worldObject)
	{
		Con::errorf("createWaypoint: can't find object or its representation %s", argv[1]);
		return (S32) GuideBot::WorldObject::INVALID_OBJECT_ID;
	}
	//transform
	MatrixF tmMat;
	readTransform(argv[2], tmMat);
	
	//radius
	float radius = argc == 4 ? dAtof(argv[3]) : DEFAULT_RADIUS;
	
	GuideBot::Waypoint* waypoint = new GuideBot::Waypoint(GuideBot::toGbMatrix(tmMat),radius);
	waypoint->setParent(worldObject);
	waypoint->registerObject();

	return waypoint->getId();
}

ConsoleFunction(createAnchor,S32,3,4, "object, transform(pos axis angle), radius")
{
	//parent object
	GuideBot::WorldObject* worldObject  = NULL;
	if (argv[1] && argv[1][0])
	{
		SceneObject* object = static_cast<SceneObject*>(Sim::findObject(argv[1]));
		worldObject = object ? object->getWorldObject() : NULL;
		if (!worldObject)
		{
			Con::errorf("createAnchor: can't find object or its representation %s", argv[1]);
			return (S32) GuideBot::WorldObject::INVALID_OBJECT_ID;
		}
	}
	

	//transform
	MatrixF tmMat;
	readTransform(argv[2], tmMat);

	//radius
	float radius = argc == 4 ? dAtof(argv[3]) : DEFAULT_RADIUS;

	GuideBot::Anchor* anchor = new GuideBot::Anchor(GuideBot::toGbMatrix(tmMat),radius);
	anchor->setParent(worldObject);
	anchor->registerObject();

	return anchor->getId();
}

ConsoleFunction(createObstacle,S32,4,5, "object, transform(pos axis angle), type, radius")
{
	S32 resID = GuideBot::WorldObject::INVALID_OBJECT_ID;

	//parent object
	GuideBot::WorldObject* worldObject  = NULL;
	if (argv[1] && argv[1][0])
	{
		SceneObject* object = static_cast<SceneObject*>(Sim::findObject(argv[1]));
		worldObject = object ? object->getWorldObject() : NULL;
		if (!worldObject)
		{
			Con::errorf("createObstacle: can't find object or its representation %s", argv[1]);
			return resID;
		}
	}

	//transform
	MatrixF tmMat;
	readTransform(argv[2], tmMat);

	//obstacle type
	GuideBot::Obstacle::ObstacleType type = GuideBot::Obstacle::OT_MEDIUM;
	if (!GuideBot::Obstacle::enumDescription.getValueByName(argv[3],type))
	{
		Con::errorf("createObstacle: can't parse obstacle type %s", argv[3]);
		return resID;
	}

	//radius
	float radius = argc == 5 ? dAtof(argv[4]) : DEFAULT_RADIUS;

	GuideBot::Obstacle* obstacle = new GuideBot::Obstacle(type, GuideBot::toGbMatrix(tmMat),radius);
	obstacle->setParent(worldObject);
	obstacle->registerObject();
	resID = obstacle->getId();
	
	return resID;
}



