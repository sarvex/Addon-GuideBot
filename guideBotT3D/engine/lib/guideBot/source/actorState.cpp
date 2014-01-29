//-----------------------------------------------------------------------------
// Guide Bot
// Copyright (C) LogicKing.com, Inc.
//-----------------------------------------------------------------------------

#include "guideBot/actorState.h"

using namespace GuideBot;

////////////////////////////////////////////////////
//////****************State****************/////////
////////////////////////////////////////////////////

const MovementDesc& State::selectMovementDesc(const Vector3& dir)
{
	float maxDot = -1.0;
	int idx = -1;
	for(size_t i = 0; i<m_movementData.size(); i++)
	{
		float curDot = dotProtuct(dir,m_movementData[i].m_dir);
		if (curDot>maxDot)
		{
			maxDot = curDot;
			idx = (int) i;
		}
	}
	GB_ASSERT(idx>=0,"Must be at least idle animation");

	return m_movementData[idx];

}
const std::string& State::getRawVisualName(const std::string& visual_name)
{
	//checking for aliases
	NameMap::iterator it = m_aliasMap.find(visual_name);
	if (it!=m_aliasMap.end())
		return it->second;

	return  visual_name;
}

bool State::getChangeVisualName(const std::string& stateName,std::string& visualName)
{
	NameMap::iterator it = m_changeMap.find(stateName);
	if (it==m_changeMap.end())
		return false;

	visualName = getRawVisualName(it->second);

	return  true;
}

void State::addMovementDesc(const MovementDesc& desc)
{ 
	m_movementData.push_back(desc);
	m_movementData.back().m_dir.normalizeSafe();
};

void Visual::addCallback(float time, AnimCallbackFunction callback)
{
	m_callbackMap.insert(std::make_pair(time,callback));
}

