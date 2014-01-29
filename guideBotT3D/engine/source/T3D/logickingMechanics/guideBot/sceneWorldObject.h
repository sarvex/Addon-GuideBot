//-----------------------------------------------------------------------------
// Guide Bot
// Copyright (C) LogicKing.com, Inc.
//-----------------------------------------------------------------------------

#ifndef _SCENE_WORLD_OBJECT_H
#define _SCENE_WORLD_OBJECT_H

#include "guideBot/worldObject.h"
#include "T3D/logickingMechanics/guideBot/enhancedPlayer.h"
#include "scene/sceneObject.h"

class SceneWorldObject: GuideBot::WorldObject
{
public:
	SceneWorldObject(SceneObject* object = NULL, WorldObjectType objectType = WO_OBJECT);

	virtual void getAABB(GuideBot::Vector3& _min, GuideBot::Vector3& _max);

protected:
	virtual const GuideBot::Matrix& getObjectTm();
	SceneObject* m_sceneObject;
};



#endif