//-----------------------------------------------------------------------------
// Guide Bot
// Copyright (C) LogicKing.com, Inc.
//-----------------------------------------------------------------------------
// GI-Soldier attack infos
//-----------------------------------------------------------------------------


datablock AttackInfoDataBlock(SimpleAttack)
{
	minDist  	= 0;						// min distance to enemy for attack execution
	maxDist     = 30;						// max distance to enemy for attack execution
	fov         = 20;    					// max fov of enemy for attack execution (max angle to enemy)
	periodicity = 0;						// periodicity of this attack (choose attack one time in periodicity)
	evaluator   = "BaseWeaponEvaluator";	// attached function that returns numeric estimation - greater value means 
				
	manoeuvreMinDist = 15;					// manoeuvre description: minimum distance (default: 2)
	manoeuvreMaxDist = 30;					// manoeuvre description: minimum distance (default: 2)
	strafe		= true;						// manoeuvre description: enable strafe movements(default:true)
	pause	   	= 1500;						// pause between attack action (i.e."shoots")
	blockMovements = false;					// block movements during attack action(acquiring E_BRAIN_MOVING)
	action      = "ActionShot";				
	state       = "StateNormal";			// background state
	weapon      = "MachineGunImage";
	ammo        = "BlasterAmmo";
	coverState	= "";
	attackState	= "";						// attack action state (i.e. special state for "shoots")
	lookAtEnemy = true;						// look at the enemy during attack (default:true)
};

datablock AttackInfoDataBlock(RocketAttack)
{
	minDist     = 5;
	maxDist     = 50;
	fov         = 20;
	periodicity = 0;
	evaluator   = "BaseWeaponEvaluator";
	
	manoeuvreMinDist = 15;
	manoeuvreMaxDist = 40;
	pause	   	= 2000;
	action      = "ActionShot";
	state       = "StateNormal";
	weapon      = "RocketLauncherImage";
	ammo        = "RocketLauncherAmmo";
	coverState	= "";
};

datablock AttackInfoDataBlock(AttackFromCover)
{
	minDist     = 0;
	maxDist     = 60;
	fov         = 20;
	periodicity = 0;
	evaluator   = "BaseWeaponEvaluator";
	
	manoeuvreMinDist = 15;
	manoeuvreMaxDist = 30;
	pause	   	= 1500;
	//strafe		= false;
	action      = "ActionShot";
	state       = "SoldierNormal";
	weapon      = "MachineGunImage";
	ammo        = "MachineGunAmmo";
	coverState	= "SoldierStealth";
};

datablock AttackInfoDataBlock(RangeAttack)
{
	minDist     = 0;
	maxDist     = 30;
	fov         = 20;
	periodicity = 0;
	evaluator   = "BaseWeaponEvaluator";
	
	manoeuvreMinDist = 15;
	manoeuvreMaxDist = 30;
	pause	   	= 1500;
	blockMovements = false;//true
	action      = "AimedShotAction";//"ActionShot";
	state       = "SoldierNormal";
	weapon      = "MachineGunImage";
	ammo        = "MachineGunAmmo";
	attackState	= "SoldierKnee"; //"SoldierKneeIdle"; //
};

datablock AttackInfoDataBlock(GrenadeAttack)
{
	minDist     = 0;						
	maxDist     = 30;
	fov         = 20;
	periodicity = 15000;
	evaluator   = "GrenadeWeaponEvaluator";
	
	manoeuvreMinDist = 15;
	manoeuvreMaxDist = 30;
	strafe		= false;
	pause	   	= 0;
	action      = "ActionThrowGrenade";
	state       = "SoldierNormal";
	weapon      = "MachineGunImage";
	ammo        = "GrenadeAmmo";
	//script action specific info
	grenade     = "GrenadeImage";   
};

datablock AttackInfoDataBlock(PanicFleeAttack)
{
	minDist     = 0;
	maxDist     = 40;
	fov         = 45;
	periodicity = 0;
	evaluator   = "PanicFleeEvaluator";
	
	manoeuvreMinDist = 25;
	manoeuvreMaxDist = 40;
	lookAtEnemy = false;
	enableStrafe = false;
};

//-----------------------------------------------------------------------------
// Evaluator functions returns estimation of attack based on distance
// and other params.  Bot will execute attack with the highest value
// of c
function BaseWeaponEvaluator(%owner, %enemy, %enemyDistSq, %enemyFov, %attackInfo, %lastExecutionTime, %isActive)
{
	if (%enemy.getWorldObjectType() $= "grenade")
		return 0;

	%curTime = GuideBot_getTime();
	%enemyDist = mSqrt(%enemyDistSq);
	//echo("BaseWeaponEvaluator ", %attackInfo.getName(), " dist: ", %enemyDist," ", %attackInfo.minDist, " - ", %attackInfo.maxDist );
	if (%enemyDist > %attackInfo.minDist && %enemyDist < %attackInfo.maxDist && %enemyFov < %attackInfo.fov 
			&& %owner.getInventory(%attackInfo.ammo)>0 && %curTime - %lastExecutionTime>%attackInfo.periodicity)
	{
		//echo("BaseWeaponEvaluator 1");
		if (%enemyDist > %attackInfo.manoeuvreMinDist && %enemyDist < %attackInfo.manoeuvreMaxDist)
			return 2;
		else
			return 1;
	}	  
	return 0;
}

function GrenadeWeaponEvaluator(%owner, %enemy, %enemyDistSq, %enemyFov, %attackInfo, %lastExecutionTime, %isActive)
{
	//echo("GrenadeWeaponEvaluator ", %attackInfo.getName());
	if (BaseWeaponEvaluator(%owner, %enemy, %enemyDistSq, %enemyFov, %attackInfo, %lastExecutionTime) > 0)
	{
		//echo("Grenade attack was choosed");
		return 5;
	}
	
	return 0;
}
function PanicFleeEvaluator(%owner, %enemy, %enemyDistSq, %enemyFov, %attackInfo, %lastExecutionTime, %isActive)
{
	//echo("PanicFleeEvaluator ",%enemy.getName()," type " ,%enemy.getWorldObjectType());
	if (%enemy.getWorldObjectType() $= "grenade")
	{
		//echo("PanicFleeEvaluator choosed");
		return 6;
	}
	return 0;
}
