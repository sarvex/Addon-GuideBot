//-----------------------------------------------------------------------------
// Torque Game Engine
// Copyright (C) GarageGames.com, Inc.
//-----------------------------------------------------------------------------
// Machine Gun and ammo
//-----------------------------------------------------------------------------

datablock SFXProfile(MachineGunReloadSound)
{
filename = "art/sound/machineGun/machineGun_reload";
description = AudioClose3d;
preload = true;
};

datablock SFXProfile(MachineGunFireSound)
{
	filename = "art/sound/machineGun/machineGun_fire";
	description = AudioDefault3d;
	preload = true;
};

datablock SFXProfile(MachineGunEmptySound)
{
	filename = "art/sound/crossbow_firing_empty";
	description = AudioClose3d;
	preload = true;
};

datablock SFXProfile(MachineGunHitSound)
{
	filename = "art/sound/machineGun/machineGun_hit";
	description = AudioClose3d;
	preload = true;
};
// ----------------------------------------
// ----------------------------------------

datablock ParticleData(MachineGunHitParticle)
{
   textureName = "art/shapes/particles/spark_wet";
   dragCoefficient = 0.99218;
   windCoefficient = 1;
   gravityCoefficient = 0.998779;
   inheritedVelFactor = 0.199609;
   lifetimeMS = 481;
   lifetimeVarianceMS = 350;
   spinSpeed = 0;
   spinRandomMin = 0;
   spinRandomMax = 0;
   useInvAlpha = 1;
   
   colors[0] = "1 0.976471 0 1";
   colors[1] = "1 0.835294 0 1";
   colors[2] = "1 0.0944882 0 0";
   colors[3] = "1 1 1 1";
   
   sizes[0] = 0.149545;
   sizes[1] = 0.0976622;
   sizes[2] = 0.0976622;
   sizes[3] = 1;
   
   times[0] = 0;
   times[1] = 0.498039;
   times[2] = 1;
   times[3] = 1;
   animTexName = "art/shapes/particles/spark_wet";
};

datablock ParticleEmitterData(MachineGunHitEmitter)
{
  
   ejectionPeriodMS = 3;
   periodVarianceMS = 0;
   
   ejectionVelocity = 5;
   velocityVariance = 1;
   ejectionOffset = 0;  
  
   thetaMin = 0;
   thetaMax = 130;
   
   lifetimeMS = 70;
   lifetimeVarianceMS = 0;
   
   blendStyle = "ADDITIVE";
   srcBlendFactor = "SRC_ALPHA";
   dstBlendFactor = "ONE";
   
   particles = "MachineGunHitParticle";
   orientParticles = "1";
};
// ----------------------------------------
datablock ParticleData(MachineGunFireParticle)
{
   textureName = "art/shapes/particles/shot01.png";
   dragCoefficient = 0.684262;
   windCoefficient = 1;
   gravityCoefficient = 0;
   inheritedVelFactor = 0;
   lifetimeMS = 10;
   lifetimeVarianceMS = 0;
   spinSpeed = 1;
   spinRandomMin = -666;
   spinRandomMax = 1000;
   useInvAlpha = 1;
   
   colors[0] = "1 0.393701 0 1";
   colors[1] = "1 0.173228 0 1";
   colors[2] = "1 0.0944882 0 0";
   colors[3] = "1 1 1 1";
   
   sizes[0] = 1.03766;
   sizes[1] = 0.497467;
   sizes[2] = 0.399805;
   sizes[3] = 0.0946103;
   
   times[0] = 0;
   times[1] = 0.0156863;
   times[2] = 0.027451;
   times[3] = 0.0901961;
   constantAcceleration = "0.833333";
   animTexName = "art/shapes/particles/shot01.png";
};

datablock ParticleEmitterData(MachineGunFireEmitter)
{
  
   ejectionPeriodMS = -1;
   periodVarianceMS = 0;
   
   ejectionVelocity = 0;
   velocityVariance = 0;
   ejectionOffset = 0;  
  
   thetaMin = 0;
   thetaMax = 180;
   
   lifetimeMS = 41;
   lifetimeVarianceMS = 0;
   
   blendStyle = "ADDITIVE";
   srcBlendFactor = "SRC_ALPHA";
   dstBlendFactor = "ONE";
   
   particles = "MachineGunFireParticle";
   phiVariance = "0";
   softnessDistance = "0";
   softParticles = "0";
};

datablock ParticleEmitterNodeData(MachineGunFireNode)
{
   timeMultiple = 1;
};



//-----------------------------------------------------------------------------
// Projectile Object


datablock ExplosionData(MachineGunExplosion)
{
   //soundProfile = MachineGunHitSound;
   lifeTimeMS = 10;

	// Point emission
   emitter[0] = MachineGunHitEmitter;
   

   // Dynamic light
   lightStartRadius = 4;
   lightEndRadius = 0;
   lightStartColor = "1 1 0.3";
   lightEndColor = "1 1 0.3";
};


datablock ProjectileData(MachineGunProjectile)
{
	projectileShapeName = "art/shapes/weapons/machineGun/bullet_trace.dts";

	directDamage        = 18;
	areaImpulse         = 2000;

	explosion           = MachineGunExplosion;
	//particleEmitter     = MachineGunFireEmitter;

	muzzleVelocity      = 170;
	velInheritFactor    = 0.1;

	armingDelay         = 0;
	lifetime            = 8000;
	fadeDelay           = 1500;
	bounceElasticity    = 0;
	bounceFriction      = 0;
	isBallistic         = false;
	gravityMod = 0.10;

	hasLight    = true;
	lightRadius = 6.0;
	lightColor  = "1.0 1.0 0.3";
};

function MachineGunProjectile::onCollision(%this,%obj,%col,%fade,%pos,%normal)
{
   if (%col.getType() & $TypeMasks::GameBaseObjectType)
   {
	  if (%col.getType() & $TypeMasks::ShapeBaseObjectType)
		%col.damage(%obj, %pos, %this.directDamage, "Bullet");
	  %force = VectorScale(%normal, 400);
	  %force = VectorSub("0 0 0", %force);
	  %col.applyImpulse(%pos, %force);
   }
}
//-----------------------------------------------------------------------------
// Ammo Item
datablock ItemData(MachineGunAmmo)
{
   // Mission editor category
   category = "Ammo";

   // Add the Ammo namespace as a parent.  The ammo namespace provides
   // common ammo related functions and hooks into the inventory system.
   className = "Ammo";

   // Basic Item properties
   shapeFile = "art/shapes/weapons/machineGun/machineGun.dts";
   mass = 1;
   elasticity = 0.2;
   friction = 0.6;

   // Dynamic properties defined by the scripts
   pickUpName = "blaster ammo";
   maxInventory = 50;
};
/*
datablock ItemData(MachineGunAmmo : CrossbowAmmo)
{
	pickUpName = "machinegun ammo";
};*/

datablock ItemData(MachineGun)
{
	category = "Weapon";
	className = "Weapon";

	// Basic Item properties
	shapeFile = "art/shapes/weapons/machineGun/machineGun.dts";
	mass = 1;
	elasticity = 0.2;
	friction = 0.6;
	emap = true;

	// Dynamic properties defined by the scripts
	pickUpName = "a machine gun";
	image = MachineGunImage;
};


//--------------------------------------------------------------------------
// Rocket Launcher image which does all the work.  Images do not normally exist in
// the world, they can only be mounted on ShapeBase objects.



datablock ShapeBaseImageData(MachineGunImage)
{
   // Basic Item properties
   shapeFile = "art/shapes/weapons/machineGun/machineGun.dts";
   emap = true;

   // Specify mount point & offset for 3rd person, and eye offset
   // for first person rendering.
   mountPoint = 0;
   firstPerson = false;
   offset = "0 0 0";
   eyeOffset = "0 0 0";
   
   // The model may be backwards
   // rotation = "0.0 0.0 1.0 180.0";
   // eyeRotation = "0.0 0.0 1.0 180.0";

   // When firing from a point offset from the eye, muzzle correction
   // will adjust the muzzle vector to point to the eye LOS point.
   // Since this weapon doesn't actually fire from the muzzle point,
   // we need to turn this off.
   correctMuzzleVector = true;

   // Add the WeaponImage namespace as a parent, WeaponImage namespace
   // provides some hooks into the inventory system.
   className = "WeaponImage";

   // Projectile && Ammo.
   item = MachineGun;
   ammo = MachineGunAmmo;
   projectile = MachineGunProjectile;
   projectileType = Projectile;
   casing = RocketLauncherShell;
   
   shellExitDir        = "1.0 0.3 1.0";
   shellExitOffset     = "0.15 -0.56 -0.1";
   shellExitVariance   = 15.0;
   shellVelocity       = 3.0;

   // Images have a state system which controls how the animations
   // are run, which sounds are played, script callbacks, etc. This
   // state system is downloaded to the client so that clients can
   // predict state changes and animate accordingly.  The following
   // system supports basic ready->fire->reload transitions as
   // well as a no-ammo->dryfire idle state.

   // Initial start up state
   stateName[0]                     = "Preactivate";
   stateTransitionOnLoaded[0]       = "Activate";
   stateTransitionOnNoAmmo[0]       = "NoAmmo";

   // Activating the gun.  Called when the weapon is first
   // mounted and there is ammo.
   stateName[1]                     = "Activate";
   stateTransitionOnTimeout[1]      = "Ready";
   stateTimeoutValue[1]             = 0.5;
   stateSequence[1]                 = "Activate";
   stateScript[1]                   = "onActivate";

   // Ready to fire, just waiting for the trigger
   stateName[2]                     = "Ready";
   stateTransitionOnNoAmmo[2]       = "NoAmmo";
   stateTransitionOnTriggerDown[2]  = "Fire";

   // Fire the weapon. Calls the fire script which does
   // the actual work.
    stateName[3] = "Fire";
	stateTransitionOnTimeout[3] = "Ready";
	stateTimeoutValue[3] = 0.05;//0.096;
	stateFire[3] = true;
	stateRecoil[3] = LightRecoil;
	stateAllowImageChange[3] = false;
	stateSequence[3] = "Fire";
	stateScript[3] = "onFire";
	//stateSound[3] = MachineGunFireSound;
	stateEmitter[3] = MachineGunFireEmitter;
	stateEmitterTime[3] = 1.0;
	stateEmitterNode[3] = "muzzlePoint";
	

   // Play the relead animation, and transition into
   stateName[4]                     = "Reload";
   stateTransitionOnNoAmmo[4]       = "NoAmmo";
   stateTransitionOnTimeout[4]      = "Ready";
   stateTimeoutValue[4]             = 0.4;
   stateAllowImageChange[4]         = false;
   stateSequence[4]                 = "Reload";
   stateEjectShell[4]               = true;
   stateSound[4]                    = MachineGunReloadSound;

   // No ammo in the weapon, just idle until something
   // shows up. Play the dry fire sound if the trigger is
   // pulled.
   stateName[5]                     = "NoAmmo";
   stateTransitionOnAmmo[5]         = "Reload";
   stateSequence[5]                 = "NoAmmo";
   stateTransitionOnTriggerDown[5]  = "DryFire";

   // No ammo dry fire
   stateName[6]                     = "DryFire";
   stateTimeoutValue[6]             = 1.0;
   stateTransitionOnTimeout[6]      = "NoAmmo";
   stateScript[6]                   = "onDryFire";
   
   stateSoundOnFire = MachineGunFireSound;
};


//-----------------------------------------------------------------------------
function MachineGunImage::onDryFire(%this, %obj, %slot)
{
   error("No ammo!!!");
}



function MachineGunImage::onFire(%this, %obj, %slot)
{
   //echo("MachineGunImage::onFire ");//%obj.getForwardVector()
   
   // For Ai, so bot can hear shooting
   addPerceptionEvent(%obj.getPosition(), 0, 2, 0.8, %obj, 5, 100, 1, 500);
  
   
   %projectile = %this.projectile;
   //if (%obj.isControlledByPlayer())
		%muzzleVector = %obj.getMuzzleVector(%slot);
   /*else
		%muzzleVector =  %obj.getForwardVector();*/
   %objectVelocity = %obj.getVelocity();

   // add little inaccuracy to shooting depending on owner pose
   %speed = VectorLen(%objectVelocity);
   if(%obj.getActorState().getName() $= "SoldierCrawl")
   {   
      %k = 0.014;
   } 
   else if(%obj.getActorState().getName() $= "SoldierKnee")
   {
      %k = 0.027;
	  if(%speed > 1){
		%k = %k * %speed / 1;
	  }
   }
   else
   {
      %k = 0.035;
	  if(%speed > 2){
		%k = %k * %speed / 2;
	  }
   }
   
   %offsetX =  %k * (getRandom() * 2 - 1);
   %offsetY =  %k * (getRandom() * 2 - 1);
   %offsetZ =  %k * (getRandom() * 2 - 1);
   %muzzleVector = (getWord(%muzzleVector, 0) + %offsetX) SPC (getWord(%muzzleVector, 1)  + %offsetY) SPC (getWord(%muzzleVector, 2) + %offsetZ);
   
   %muzzleVelocity = VectorAdd(
      VectorScale(%muzzleVector, %projectile.muzzleVelocity),
      VectorScale(%objectVelocity, %projectile.velInheritFactor));
	  
  
   // Create the projectile object
   %p = new (%this.projectileType)() {
      dataBlock        = %projectile;
      initialVelocity  = %muzzleVelocity;
      initialPosition  = %obj.getMuzzlePoint(%slot);
      sourceObject     = %obj;
      sourceSlot       = %slot;
      client           = %obj.client;
   };
	
   MissionCleanup.add(%p);
   
   
   %pos = %obj.getPosition();
  
   if (%this.stateSoundOnFire!$="")
   {
      sfxPlayOnce(%this.stateSoundOnFire, getWord(%pos, 0), getWord(%pos, 1), getWord(%pos, 2));
   }
   return %p;
}
