//-----------------------------------------------------------------------------
// Guide Bot
// Copyright (C) LogicKing.com, Inc.
//-----------------------------------------------------------------------------
// Functions to attach Guide Bot to specific platform/engine
//-----------------------------------------------------------------------------

#ifndef _GUIDEBOT_PLATFORM
#define _GUIDEBOT_PLATFORM

#define M_PI         3.14159265358979323846
#define GB_EPSILON 1e-4

namespace GuideBot
{
typedef unsigned int Time;
const Time INVALID_TIME = -1;

namespace Platform
{
	void processAssert(bool expression, const char* message);
	void processWarning(bool expression, const char* message);
	void printError(const char* fmt,...);
	void printInfo(const char* fmt,...);
	Time getTime();

	extern float TickTime;
};

} //namespace GuideBot

#define GB_ASSERT(expression,message) GuideBot::Platform::processAssert((expression)!=0,(message));
#define GB_WARN(expression,message)   GuideBot::Platform::processWarning((expression)!=0,(message));
#define GB_SAFE_DELETE(x)	{delete (x); (x) = NULL; }
#define	GB_ERROR	GuideBot::Platform::printError
#define	GB_INFO		GuideBot::Platform::printInfo



#endif
