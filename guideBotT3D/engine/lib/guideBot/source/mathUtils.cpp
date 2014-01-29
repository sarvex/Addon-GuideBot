//-----------------------------------------------------------------------------
// Guide Bot
// Copyright (C) LogicKing.com, Inc.
//-----------------------------------------------------------------------------

#include "guideBot/mathUtils.h"
#include <stdio.h>
#include <time.h>

using namespace GuideBot;

const Vector3 Vector3::ZERO = Vector3(0.f,0.f,0.f);
const Vector3 Vector3::X = Vector3(1.f,0.f,0.f);
const Vector3 Vector3::Y = Vector3(0.f,1.f,0.f);
const Vector3 Vector3::Z = Vector3(0.f,0.f,1.f);
const Vector3 Vector3::ONE = Vector3(1.f,1.f,1.f);

const Matrix Matrix::IDENTITY = Matrix(true);

Randomizer GuideBot::gRandomizer;
Randomizer::Randomizer()
{
	srand(time(NULL));
}
