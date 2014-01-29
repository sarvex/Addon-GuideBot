//-----------------------------------------------------------------------------
// Guide Bot
// Copyright (C) LogicKing.com, Inc.
//-----------------------------------------------------------------------------
// Base enhanced player data declaration
//-----------------------------------------------------------------------------

////////////////////////////////////////////////////
//////********EnhancedPlayerData******//////////////
////////////////////////////////////////////////////

datablock EnhancedPlayerData(BaseEnhancedPlayerData)
{
   renderFirstPerson = true;

   className = Armor;
   shapeFile = "art/shapes/actors/Gideon/gideon.dts";
   cameraMaxDist = 3;
   computeCRC = true;

   canObserve = true;
   cmdCategory = "Clients";

   cameraDefaultFov = 100.0;
   cameraMinFov = 5.0;
   cameraMaxFov = 120.0;
   
   debrisShapeName = "art/shapes/actors/common/debris_player.dts";
   debris = playerDebris;

   aiAvoidThis = true;

   minLookAngle = -1.4;
   maxLookAngle = 1.4;
   maxFreelookAngle = 3.0;

   mass = 100;
   drag = 1.3;
   maxdrag = 0.4;
   density = 1.1;
   maxDamage = 300;
   maxEnergy =  60;
   repairRate = 0.33;
   energyPerDamagePoint = 75.0;

   rechargeRate = 0.256;

   runForce = 48 * 90;
   runEnergyDrain = 0;
   minRunEnergy = 0;
   maxForwardSpeed = 8;
   maxBackwardSpeed = 6;
   maxSideSpeed = 6;

   crouchForce = 45.0 * 9.0;
   maxCrouchForwardSpeed = 4.0;
   maxCrouchBackwardSpeed = 2.0;
   maxCrouchSideSpeed = 2.0;

   maxUnderwaterForwardSpeed = 8.4;
   maxUnderwaterBackwardSpeed = 7.8;
   maxUnderwaterSideSpeed = 7.8;

   jumpForce = 8.3 * 90;
   jumpEnergyDrain = 0;
   minJumpEnergy = 0;
   jumpDelay = 15;
   airControl = 0.3;

   recoverDelay = 9;
   recoverRunForceScale = 1.2;

   minImpactSpeed = 45;
   speedDamageScale = 0.4;

   boundingBox = "1 1 2";
   crouchBoundingBox = "1 1 1.4";
   swimBoundingBox = "1 2 2";
   pickupRadius = 0.75;

   // Damage location details
   boxNormalHeadPercentage       = 0.83;
   boxNormalTorsoPercentage      = 0.49;
   boxHeadLeftPercentage         = 0;
   boxHeadRightPercentage        = 1;
   boxHeadBackPercentage         = 0;
   boxHeadFrontPercentage        = 1;

   // Foot Prints
   //decalData   = PlayerFootprint;
   //decalOffset = 0.25;

   footPuffEmitter = LightPuffEmitter;
   footPuffNumParts = 10;
   footPuffRadius = 0.25;

   dustEmitter = LiftoffDustEmitter;

   splash = PlayerSplash;
   splashVelocity = 4.0;
   splashAngle = 67.0;
   splashFreqMod = 300.0;
   splashVelEpsilon = 0.60;
   bubbleEmitTime = 0.4;
   splashEmitter[0] = PlayerWakeEmitter;
   splashEmitter[1] = PlayerFoamEmitter;
   splashEmitter[2] = PlayerBubbleEmitter;
   mediumSplashSoundVelocity = 10.0;
   hardSplashSoundVelocity = 20.0;
   exitSplashSoundVelocity = 5.0;

   // Controls over slope of runnable/jumpable surfaces
   runSurfaceAngle  = 70;
   jumpSurfaceAngle = 80;
   maxStepHeight = 1;  //two meters
   minJumpSpeed = 20;
   maxJumpSpeed = 30;

   horizMaxSpeed = 68;
   horizResistSpeed = 33;
   horizResistFactor = 0.35;

   upMaxSpeed = 80;
   upResistSpeed = 25;
   upResistFactor = 0.3;

   footstepSplashHeight = 0.35;

   //NOTE:  some sounds commented out until wav's are available

   // Footstep Sounds
   FootSoftSound        = FootLightSoftSound;
   FootHardSound        = FootLightHardSound;
   FootMetalSound       = FootLightMetalSound;
   FootSnowSound        = FootLightSnowSound;
   FootShallowSound     = FootLightShallowSplashSound;
   FootWadingSound      = FootLightWadingSound;
   FootUnderwaterSound  = FootLightUnderwaterSound;

   groundImpactMinSpeed    = 10.0;
   groundImpactShakeFreq   = "4.0 4.0 4.0";
   groundImpactShakeAmp    = "1.0 1.0 1.0";
   groundImpactShakeDuration = 0.8;
   groundImpactShakeFalloff = 10.0;

   //exitingWater         = ExitingWaterLightSound;

   observeParameters = "0.5 4.5 4.5";

   // Allowable Inventory Items

   maxInv[RocketLauncher] = 1;
   maxInv[RocketLauncherAmmo] = 20;
   maxInv[GrenadeLauncher] = 1;
   maxInv[GrenadeLauncherAmmo] = 20;

   physicsPlayerType  = "Capsule";
   
   //.logicking guideBot
   maxInv[MachineGun] = 1;
   maxInv[MachineGunAmmo] = 10000;
   
   maxInv[GrenadeItem] = 1;
   maxInv[GrenadeAmmo] = 100;

   //ai vision params
   fov = 120;
   viewDistance = 60;
   // forget enemy time in ms
   forgetTime = 25000;
};

// STEP SOUNDS
new SFXProfile(step_01)
{
   filename = "art/sound/human_step_01";
   description = AudioDefault3D;
   preload = true;
};											
new SFXProfile(step_02)
{
   filename = "art/sound/human_step_02";
   description = AudioDefault3D;
   preload = true;
};

// THROW GRENADE
new SFXProfile(throwGrenadeSound)
{
   filename = "art/sound/throwSound";
   description = AudioDefault3D;
   preload = true;
};

function EnhancedPlayerData::onAdd(%this,%obj)
{
    %obj.signal("onSpawn");
    %obj.stepSound = sfxCreateSource(step_01, 0, 0, 0);
    %obj.throwGrenadeSound = sfxCreateSource(throwGrenadeSound, 0, 0, 0);
}

function EnhancedPlayerData::onDisabled(%this,%obj,%state)
{
   // Schedule corpse removal.  Just keeping the place clean.
   %obj.schedule($CorpseTimeoutValue - 1000, "startFade", 1000, 0, true);
   %obj.schedule($CorpseTimeoutValue, "delete");

    if(%obj.playerControlled)
        commandToServer('playOrbitCamera', %obj.getTransform());
	%hasRagdoll = isObject(%this.ragdoll);
	%obj.setDead(%hasRagdoll);
	if(%hasRagdoll)
	{
		%obj.disableCollision();
		createRagDoll(%this.ragdoll, %obj);
		%obj.schedule(0, "delete");
	}
}

function EnhancedPlayerData::damage(%this, %obj, %sourceObject, %position, %damage, %damageType)
{
   if (%obj.getState() $= "Dead")
      return;
      
   %obj.updateHealth();
   %obj.damagePos = %position;
   %obj.applyDamage(%damage);
}