//-----------------------------------------------------------------------------
// Guide Bot
// Copyright (C) LogicKing.com, Inc.
//-----------------------------------------------------------------------------
// Specific compatibility functions to link GuideBot and Torque3d
//-----------------------------------------------------------------------------


#include "T3D/logickingMechanics/guideBot/torqueGuideBotPlatform.h"

#include "guideBot/platform.h"
#include "guideBot/worldObject.h"
#include "guideBot/perceptionEvent.h"

#include "console/console.h"
#include "core/strings/stringFunctions.h"
#include "T3D/gameBase/gameProcess.h"

namespace GuideBot
{

namespace Platform
{
	void processAssert(bool expression, const char* message)
	{
		AssertFatal(expression,message);
	}

	Time getTime()
	{
		return ServerProcessList::get()->getTotalTicks()*TickMs;
	}

	void processWarning(bool expression, const char* message)
	{
		AssertWarn(expression,message);
	}

	void printError(const char* fmt,...)
	{
		static char buffer[4096];
		va_list args;
		va_start(args, fmt);
		dVsprintf(buffer, sizeof(buffer), fmt, args);
		va_end(args);
		Con::errorf(buffer);
	}

	void printInfo(const char* fmt,...)
	{
		static char buffer[4096];
		va_list args;
		va_start(args, fmt);
		dVsprintf(buffer, sizeof(buffer), fmt, args);
		va_end(args);
		Con::printf(buffer);
	}

	float TickTime = TickSec;
}; //namespace Platform

Matrix	toGbMatrix(const MatrixF& matrix)
{
	Matrix res(true);
	res.setColumn(0,toGbVector(matrix.getColumn3F(0)));
	res.setColumn(1,toGbVector(matrix.getColumn3F(1)));
	res.setColumn(2,toGbVector(matrix.getColumn3F(2)));
	res.setColumn(3,toGbVector(matrix.getColumn3F(3)));
	return res;
}

MatrixF	fromGbMatrix(const Matrix& matrix)
{
	MatrixF res(true);
	res.setColumn(0,fromGbVector(matrix.getColumn(0)));
	res.setColumn(1,fromGbVector(matrix.getColumn(1)));
	res.setColumn(2,fromGbVector(matrix.getColumn(2)));
	res.setColumn(3,fromGbVector(matrix.getColumn(3)));
	return res;
}

bool init()
{
	// export constants to script namespace

	//alertness
	Con::setFloatVariable("$GuideBot::NO_ALERTNESS", NO_ALERTNESS);
	Con::setFloatVariable("$GuideBot::NORMAL_ALERTNESS", NORMAL_ALERTNESS);
	Con::setFloatVariable("$GuideBot::AWARE_ALERTNESS", AWARE_ALERTNESS);
	Con::setFloatVariable("$GuideBot::ALARM_ALERTNESS", ALARM_ALERTNESS);
	Con::setFloatVariable("$GuideBot::AWARE_ALERTNESS", AWARE_ALERTNESS);
	Con::setFloatVariable("$GuideBot::ENEMY_ALERTNESS", ENEMY_ALERTNESS);

	//sensors
	Con::setIntVariable("$GuideBot::ST_VISUAL", ST_VISUAL);
	Con::setIntVariable("$GuideBot::ST_HEARING", ST_HEARING);
	Con::setIntVariable("$GuideBot::ST_KNOWLEDGE", ST_KNOWLEDGE);

	//perception events type
	Con::setIntVariable("$GuideBot::PE_NEUTRAL", PerceptionEvent::PE_NEUTRAL);
	Con::setIntVariable("$GuideBot::PE_DANGER", PerceptionEvent::PE_DANGER);

	return true;
}

} //namespace GuideBot

///////////////////////////////////////////////////////////////////
ConsoleFunction( GuideBot_getTime, S32, 1, 1, "()")
{
	return GuideBot::Platform::getTime();
}