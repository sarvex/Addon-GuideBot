//-----------------------------------------------------------------------------
// Guide Bot
// Copyright (C) LogicKing.com, Inc.
//-----------------------------------------------------------------------------

#include "T3D/logickingMechanics/guideBot/sceneWorldObject.h"
#include "console/console.h"

SceneWorldObject::SceneWorldObject(SceneObject* object /*= NULL*/, 
			WorldObjectType objectType /*= WO_OBJECT*/):	WorldObject(objectType)
															, m_sceneObject(object)
{
	setUserData(object);
};

const GuideBot::Matrix& SceneWorldObject::getObjectTm()
{
	if (m_sceneObject)
	{
		m_tm = GuideBot::toGbMatrix(m_sceneObject->getTransform());
	}
	
	return m_tm;
}

void SceneWorldObject::getAABB(GuideBot::Vector3& _min, GuideBot::Vector3& _max)
{
	if (m_sceneObject)
	{
		const Box3F& worldBox = m_sceneObject->getWorldBox();
		_min = GuideBot::toGbVector(worldBox.minExtents);
		_max = GuideBot::toGbVector(worldBox.maxExtents);
	}
}
