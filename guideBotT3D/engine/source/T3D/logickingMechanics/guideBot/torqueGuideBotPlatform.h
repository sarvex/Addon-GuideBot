//-----------------------------------------------------------------------------
// Guide Bot
// Copyright (C) LogicKing.com, Inc.
//-----------------------------------------------------------------------------


#ifndef _TORQUE_GUIDEBOT_PLATFORM_H
#define _TORQUE_GUIDEBOT_PLATFORM_H

#include "math/mPoint3.h"
#include "math/mMatrix.h"
#include "guideBot/mathUtils.h"

namespace GuideBot
{
	//types conversion functions
	inline Vector3 toGbVector(const VectorF& vec)
	{
		return Vector3(vec.x,vec.y,vec.z);
	}

	inline VectorF fromGbVector(const Vector3& vec)
	{
		return VectorF(vec.x,vec.y,vec.z);
	}

	Matrix	toGbMatrix(const MatrixF& matrix);
	MatrixF	fromGbMatrix(const Matrix& vec);

	//initial activation of GuideBot library
	bool init();
}

#endif //_TORQUE_GUIDEBOT_PLATFORM_H