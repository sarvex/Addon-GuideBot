//-----------------------------------------------------------------------------
// Guide Bot
// Copyright (C) LogicKing.com, Inc.
//-----------------------------------------------------------------------------

package logickingGuideBotFunctions
{


// Show observer after player death
function serverCmdPlayOrbitCamera(%client, %transform)
{ 
    %client.camera.setMode("Corpse", %client.player);
    %z = getWord(%transform, 2);
    %z = %z + 2;
    %transform = setWord(%transform, 2, %z);
    %client.camera.setTransform(%transform);
    %client.setControlObject(%client.camera);
}

function gameOver(%client)
{
	commandToClient(%client, 'playerDied', "GI has got you! Fight again?");
}

function DeathMatchGame::initGameVars(%game)
{
	parent::initGameVars(%game);
	$Game::DefaultPlayerClass        = "EnhancedPlayer";
	$Game::DefaultPlayerDataBlock    = "PlayerSoldierData";   
}
   
function GameCore::spawnPlayer(%game, %this, %spawnPoint)
{
	if(%spawnPoint $= "")
	{
		%spawnPoint = pickPlayerSpawnPoint($Game::DefaultPlayerSpawnGroups);
	}

	parent::spawnPlayer(%game,%this, %spawnPoint);

	addToTeam(%this.player, $PLAYERS_TEAM);
	
	addIntoTeam(%this.player,"offenders");//add to offender team (guidebot)
	
	updateGameScore(0);
	$playerForAi = %this.player;
	%this.player.playerControlled = true;
}

    function GameCore::loadOut(%game, %player)
    {
		%player.setInventory(GrenadeAmmo,%player.maxInventory(GrenadeAmmo));
        %player.setInventory(MachineGunAmmo,%player.maxInventory(MachineGunAmmo));
     	%player.mountImage(MachineGunImage, 0);
     	%player.scheduledUpdate();
        
    }

};

activatePackage(logickingGuideBotFunctions);
