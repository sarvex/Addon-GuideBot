//-----------------------------------------------------------------------------
// Guide Bot
// Copyright (C) LogicKing.com, Inc.
//-----------------------------------------------------------------------------
#ifndef _ACTOR_STATE_H
#define _ACTOR_STATE_H

#include "guideBot/action.h"
#include "guideBot/platform.h"

#include <vector>
#include <map>
#include <string>

namespace GuideBot
{

////////////////////////////////////////////////////
//////**************Visual*****************/////////
////////////////////////////////////////////////////

class Visual
{
public:
	Visual() {};
	Visual(const char* name,const EffectorIDVector& effectorIDs): m_effectorIDs(effectorIDs) 
	{
		m_name = name;
	};

	const std::string&		getName() const	{ return m_name;};
	const EffectorIDVector&	getEffectors()	{ return m_effectorIDs;};

	typedef Delegate<void (Actor*)>	AnimCallbackFunction;
	void addCallback(float time, AnimCallbackFunction callback);

	typedef std::map<float, AnimCallbackFunction>	CallbackMap;
	CallbackMap& getCallbackMap() {return m_callbackMap;};

	Time getPlayingTime() {return m_playTime;};

protected:	
	//name of visual
	std::string			m_name;
	//effectors for playing visual
	EffectorIDVector	m_effectorIDs;
	//action which is responsible for playing visual
	std::string			m_actionName;
	//visual play time
	Time		m_playTime;
	//animation callbacks
	CallbackMap	m_callbackMap;
};

typedef std::map<std::string, Visual*> VisualMap;

////////////////////////////////////////////////////
//////****************State****************/////////
////////////////////////////////////////////////////

struct MovementDesc
{
	MovementDesc()	{};
	MovementDesc(Vector3 dir, float vel,std::string visualName):	m_dir(dir) 
																		, m_velocity (vel)
																		, m_visualName(visualName)
																		{};
	Vector3		m_dir;
	float		m_velocity;
	std::string	m_visualName;	
};

class State: public BaseObject
{
public:
	State(const std::string& _name) : m_name(_name) {};

	const std::string& getName() { return m_name;};
	
	const MovementDesc& selectMovementDesc(const Vector3& dir);
	const std::string&  getRawVisualName(const std::string& visual_name);
	bool getChangeVisualName(const std::string& stateName,std::string& visualName);

	void addMovementDesc(const MovementDesc& desc);
	void addVisualAlias(const std::string& alias,const std::string& visualName)
						{ m_aliasMap.insert(std::make_pair(alias,visualName));};

	void addStateChange(const std::string& state,const std::string& visualName)
						{ m_changeMap.insert(std::make_pair(state,visualName));};

	void setCallbacks(const std::string& enableCallback, const std::string disableCallback)
						{ m_enableCallback = enableCallback; m_disableCallback = disableCallback;};
	const std::string& getEnableCallback()	{return m_enableCallback;};
	const std::string& getDisableCallback()	{return m_disableCallback;};
protected:
	std::string	 m_name;

	typedef std::vector<MovementDesc> MovementData;
	MovementData m_movementData;

	typedef std::map<std::string,std::string>	NameMap;
	NameMap	 m_aliasMap;

	NameMap	 m_changeMap;

	std::string m_enableCallback;
	std::string m_disableCallback;
	
};

} //namespace GuideBot

#endif