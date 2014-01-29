////////////////////////////////////////////////////
//////************Grenade*************//////////////
////////////////////////////////////////////////////
datablock RigidBodyData(GrenadePhysicsBody : PhysBox )
{	
   //mass = 0;
   dampOnCollisionL = 0.9;
   dampOnCollisionA = 0.9;
};

function GrenadePhysicsBody::onContact(%this, %obj, %pos, %vel, %slideOrCollide)
{
	//echo(" GrenadePhysicsBody::onContact ", %vel SPC %slideOrCollide);
	addPerceptionEvent(%pos, 1, 3, 0.3,"",5,10,1,500);
	parent::onContact(%this, %obj, %pos, %vel, %slideOrCollide);
}

datablock ItemData(GrenadeAmmo : BlasterAmmo)
{
	pickUpName = "grenade ammo";
};

datablock ItemData(GrenadeItem: BlasterGun)
{
	pickUpName = "a grenades";
	image = GrenadeImage;
};

/*datablock ExplosionData(GrenadeExplosion: RocketLauncherExplosion)
{
};*/

datablock ProjectileData(GrenadeData)
{
	//projectileShapeName = "art/shapes/crates/crate1.dts";
	projectileShapeName = "art/shapes/weapons/grenade/grenade.dts";
	//scale = "0.08 0.06 0.06";
	
	physicBodyData = "GrenadePhysicsBody";
	physicScale = "0.2 0.15 0.15";
	lifetime       = 4200;
	explosion = RocketLauncherExplosion;
	
    damageRadius = 4;
    radiusDamage = 60;
	damageType = "Grenade";
    areaImpulse = 1100;
   
	directDamage        = 0;

	muzzleVelocity      = 32;
};

function GrenadeData::onExplode(%data, %proj, %position, %mod)
{
   //echo("GrenadeData::onExplode("@%data.getName()@", "@%proj@", "@%position@", "@%mod@")");
   addPerceptionEvent(%position, 0, 3, 0.3,"", 10, 10, 1, 1000);//10,10,1,-1
   parent::onExplode(%data, %proj, %position, %mod);
}

datablock ShapeBaseImageData(GrenadeImage : BlasterGunImage) 
{
	//shapeFile = "art/shapes/crates/crate1.dts";
	shapeFile = "art/shapes/weapons/grenade/grenade.dts";
	//imageScale = ".08 0.06 0.06";//".2 0.15 0.15";
	offset = "0 0 0";
	mountPoint = 1;
	eyeOffset = "0 0 0";
	item = GrenadeItem;
	ammo = GrenadeAmmo;
	projectile = GrenadeData;
};

////////////////////////////////////////////////////
//////************Grenade gun*************//////////
////////////////////////////////////////////////////

datablock ShapeBaseImageData(GrenadeGunImage : BlasterGunImage)
{
   ammo = GrenadeAmmo;
   projectile = GrenadeData;
};
function GrenadeGunImage::onFire(%this, %obj, %slot)
{
	return BlasterGunImage::onFire(%this, %obj, %slot);
}