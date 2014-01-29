//-----------------------------------------------------------------------------
// Guide Bot
// Copyright (C) LogicKing.com, Inc.
//-----------------------------------------------------------------------------
// GI Soldier Bot Enhanced Player datablock declaration
//-----------------------------------------------------------------------------

function SoldierBotData::create(%data)
{  	
	%bot = new EnhancedPlayer() {
		dataBlock = %data;
	};
	return %bot;
}

// ALERT SOUND
new SFXProfile(alertSound)
{
   filename = "art/sound/alertSound";
   description = AudioDefault3D;
   preload = true;
};

function SoldierBotData::onAdd(%this, %obj)
{
   %obj.setInventory(MachineGunAmmo,10000);
   %obj.setInventory(GrenadeAmmo,50);
   %obj.mountImage(MachineGunImage, 0);
   %obj.signal("onSpawn");
   %obj.stepSound = sfxCreateSource(step_01, 0, 0, 0);
   %obj.throwGrenadeSound = sfxCreateSource(throwGrenadeSound, 0, 0, 0);
   %obj.alertSound = sfxCreateSource(alertSound, 0, 0, 0);
}

$GuideBot::alertNotificationLag = 500;
function SoldierBotData::onAlert(%this, %obj)
{
	if(isObject(%obj.alertSound))
    {
        %pos = %obj.getPosition();
        %obj.alertSound.setVolume(1);
        %obj.alertSound.setTransform(%pos);
        %obj.alertSound.play();
    }    
	%enemy = %obj.getEnemy();
	
	// Shoot to our team mates about spoted danger
	schedule($GuideBot::alertNotificationLag, 0, "addPerceptionEvent", %obj.getPosition(), 0, 3, 0.3, %enemy, 10, 10, 1, 500);
}


datablock EnhancedPlayerData(SoldierBotData:BaseEnhancedPlayerData)
{
   shapeFile = "art/shapes/players/soldier/soldier.dts";
   ragdoll = "SoldierRagDoll";
   
   enablePerception = true;
   enableSlopes = false;
   
   attackInfo[0] = "AttackFromCover";
   attackInfo[1] = "RangeAttack";
   attackInfo[2] = "GrenadeAttack";
   //attackInfo[3] = "PanicFleeAttack";   
   
   //ai vision params
   fov = 120;
   viewDistance = 100;
   // forget enemy time in ms
   forgetTime = 25000;
      
   team = "guard";
};

//datablock for soldier controlled by player
datablock EnhancedPlayerData(PlayerSoldierData:SoldierBotData)
{
   enableSlopes = true;
   team = "offenders";
   isInvincible = false;
   maxDamage = 100;
};


//-----------------------------------------------------------------------------
// for Game Mechanics Editor
//-----------------------------------------------------------------------------
activatePackage(TemplateFunctions);
registerTemplate("SoldierBot", "GuideBot", "SoldierBotData::create(SoldierBotData);");

setTemplateField("SoldierBot", "enablePerception", "false", "bool", "AI", "Perception activity flag.");
setTemplateField("SoldierBot", "initState", "SoldierNormal", "", "AI", "Bot's team name.");
setTemplateField("SoldierBot", "team", "guard", "", "AI", "Bot's team name.");

setTemplateEvent("SoldierBot", "onSpawn", "", " %this - our object. ");

deactivatePackage(TemplateFunctions);
