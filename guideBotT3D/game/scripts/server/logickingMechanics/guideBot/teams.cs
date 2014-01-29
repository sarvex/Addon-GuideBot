//-----------------------------------------------------------------------------
// Guide Bot
// Copyright (C) LogicKing.com, Inc.
//-----------------------------------------------------------------------------
// Teams relations, and enemy danger evaluation
//-----------------------------------------------------------------------------

//team relationship factor map
//factor < 0 - friendship
//factor == 0 - neutrality 
//factor > 0 - enmity
$teamsRelationship[guard,offenders] = 1;
$teamsRelationship[guard,neutral] = 0;
$teamsRelationship[offenders,guard] = 1;
$teamsRelationship[offenders,neutral] = 0;
$teamsRelationship[neutral,guard] = 0;
$teamsRelationship[neutral,offenders] = 0;

function addIntoTeam(%object,%team)
{
	%object.team = %team;
}

function getTeam(%object)
{
	%team  = %object.team;
	if (%team$="" && isObject(%object.getDataBlock()))
		%team = %object.getDataBlock().team;
	return %team;
}

function getTeamRelationship(%this, %object)
{
	%thisTeam = getTeam(%this);
	%objectTeam = getTeam(%object);

	%relationship = 0; //neutrality by default
	if (%thisTeam!$="" && %objectTeam!$="" && $teamsRelationship[%thisTeam,%objectTeam]!$="")
	{
		%relationship = $teamsRelationship[%thisTeam,%objectTeam];
	}
	else if (%thisTeam$=%objectTeam)
	{
		%relationship = -1; 
	}

	return %relationship;
}



////////////////////////////////////////////////////
//////***********Enemy evaluator***********/////////
////////////////////////////////////////////////////

// Calculating potential threat of each enemy
$GuideBot::enemyGrenadeFactor = 0.001;
function enemyEvaluator(%actor,%enemyObject, %enemyDistSq, %enemyFov)
{
	//echo("enemyEvaluator ", %actor, " ", %enemyObject," ", %enemyDistSq, " ", %enemyFov);
	%isGrenade = %enemyObject.getWorldObjectType()$="grenade";
	if (%isGrenade)
	{
		if (%enemyObject.owner == %actor)
			return 0;
		%danger = 1;
	}
	else
		%danger = getTeamRelationship(%actor,%enemyObject);
	if (%danger>0)
	{
		%danger = %danger/mSqrt(%enemyDistSq);
		if (%isGrenade)
		{
			%danger = %danger*$GuideBot::enemyGrenadeFactor;
		}
	}
	return %danger;
}
setEnemyEvaluator(enemyEvaluator);