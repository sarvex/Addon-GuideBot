//-----------------------------------------------------------------------------
// Guide Bot
// Copyright (C) LogicKing.com, Inc.
//-----------------------------------------------------------------------------
// Test and debug utility functions
//-----------------------------------------------------------------------------

//test ai
function pauseGame(%val)
{
	if ((%val) || (%val $= ""))
	{
		if ($timeScale != 0)
		{
			$timeScale = 0;
		}
		else
			$timeScale = 1;
	}
}
GlobalActionMap.bind(keyboard, pause, pauseGame);



function tact()
{
	new AIPlayer(act) 
	{
      canSaveDynamicFields = "1";
      Enabled = "1";
      position = "535.095 661.057 258.09";
      //position = "535.095 659.057 258.09";
      rotation = "1 0 0 0";
      scale = "1 1 1";
      dataBlock = "BoomBotData";
      collision = "0";
   };
}

datablock EnhancedPlayerData(EnhancedBoomBotData: BaseEnhancedPlayerData)
{
   initState = "StateNormal";//"StateAlert";
   
   attackInfo[0] = "BlasterAttack";
   attackInfo[1] = "RocketAttack";
};

function act()
{
	new EnhancedPlayer(act) 
	{
      canSaveDynamicFields = "1";
      Enabled = "1";
      position = "535.095 661.057 258.09";
      rotation = "1 0 0 0";
      scale = "1 1 1";
      dataBlock = "EnhancedBoomBotData";
      collision = "0";
   };
   act.setInventory(BlasterGun,1);
   act.setInventory(BlasterAmmo,100);
   
   act.setInventory(RocketLauncher,1);
   act.setInventory(RocketLauncherAmmo,100);
}

function act2()
{
	new EnhancedPlayer(act2) 
	{
      canSaveDynamicFields = "1";
      Enabled = "1";
      position = "535.095 665.057 258.09";
      rotation = "1 0 0 0";
      scale = "1 1 1";
      dataBlock = "EnhancedBoomBotData";
      collision = "0";
   };
}


function mtp()
{
   act.moveToPoint("515.095 661.057 258.09");
   //act.setMoveDestination("515.095 661.057 258.09");
   //tact.setMoveDestination("515.095 659.057 258.09");
}

function mtp2()
{
   act.moveToPoint("535.095 661.057 258.09");
   //act.setMoveDestination("535.095 661.057 258.09");
   //tact.setMoveDestination("535.095 659.057 258.09");
}

function setLDM()
{
   act.setLookDirMode("point","0 0 0",1);
}

function setLDMa()
{
   act.setLookDirMode("object",$playerForAi);
}

function evaluator(%owner,%enemy)
{
   return 0;
}

datablock AnchorDataBlock(AnchorWithMediumObstacle)
{
	radius = 2;
    hasObstacle = true;
    type = "medium";
};

function anch()
{
   new AnchorObject(anch) 
	{
	  dataBlock = "AnchorWithMediumObstacle";
      canSaveDynamicFields = "1";
      position = "515 661 256.5";
   };
   
   new AnchorObject(anch2) 
	{
	  dataBlock = "AnchorWithMediumObstacle";
      Enabled = "1";
      position = "510 676 256.5";
   };
}

function boxAnch()
{
   new RigidBody(boxAnch) {
	  //dataBlock = "PhysBox";
      dataBlock = "PhysBoxWithAnchor";
      position = "512  674 256.9";
      rotation = "0.000835297 0.0117746 -0.99993 0.369035";
      scale = "0.5 0.5 0.5";
      canSaveDynamicFields = "1";
      Enabled = "1";
   };
/*   new RigidBody(boxAnch2) {
      dataBlock = "PhysBoxWithAnchor";
      position = "512  677 256.9";
      rotation = "0.000835297 0.0117746 -0.99993 0.369035";
      scale = "0.5 0.5 0.5";
      canSaveDynamicFields = "1";
      Enabled = "1";
   };*/
}

function createPath()
{
  new Path(Path01) {
      isLooping = "1";
      canSaveDynamicFields = "1";
      Enabled = "1";
	  
	   new Marker(m1) {
		  seqNum = "13";
		  type = "Normal";
		  msToNext = "1000";
		  smoothingType = "Spline";
		  position = "534.009 661.715 256.295";
		  rotation = "1 0 0 0";
		  scale = "1 1 1";
		  isRenderEnabled = "true";
		  canSaveDynamicFields = "1";
		  Enabled = "1";
	   };
	   new Marker(m4) {
		  seqNum = "14";
		  type = "Normal";
		  msToNext = "1000";
		  smoothingType = "Spline";
		  position = "534.009 679.814 256.295";
		  rotation = "1 0 0 0";
		  scale = "1 1 1";
		  isRenderEnabled = "true";
		  canSaveDynamicFields = "1";
		  Enabled = "1";
	   };
   };
}

function dPath()
{
	Path01.delete();
}

function smwp()
{
	sold.moveByWaypoints(Path01,"circular");
}

function neutral(%toOffender)
{
	if (%toOffender)
		addIntoTeam($playerForAi,"offender");
	else
		addIntoTeam($playerForAi,"guard");
}

function patrol(%enable)
{
	if (%enable)
		sold.setActionByPrototype(setActionTakeWeapon("MachineGunImage","SoldierPatrol"));//sold.changeState();
	else
		sold.changeState("SoldierNormal");
}


// ActionGuard

/*
createAlternativeAction("ActionGuardScriptTest", "brain"
						, "%owner.setAction(\"ActionNothing\",%this);", "return 1;"
						, "%owner.setAction(\"ActionNothing\",%this);", "return 2;"
						, "%owner.setAction(\"ActionNothing\",%this);", "return 3;"
						, "%owner.setAction(\"ActionNothing\",%this);", "return 4;"
						);
*/
function identi()
{
	echo("identi");
	return 1;
}

function guard()
{
	if (!isObject(sold))
		sold();
	sold.setGuard(Path01);
}

function asg()
{
	if (!isObject(sold))
		sold();
	sold.assignToGuard(Path01);
}

function guardb()
{
	SoldierBot01.setGuard(Path01);
}




/// Human Test

////////////////////////////////////////////////////
//////*******Human test functions*****//////////////
////////////////////////////////////////////////////
function sra()
{
   new RagDoll(srd) {
      canSaveDynamicFields = "1";
      Enabled = "1";
      position = "535.095 661.057 258.09";
      rotation = "1 0 0 0";
      scale = "1 1 1";
      dataBlock = "SoldierRagDoll"; //SpaceOrcRagDoll
      collision = "0";
   };
}

function sold()
{
	new EnhancedPlayer(sold) 
	{
      canSaveDynamicFields = "1";
      Enabled = "1";
      position = "535.095 661.057 258.09"; //"508  675 257.4"; //"549.453 663.443 257.221";
      rotation = "0 0 1 0"; //rotation = "1 0 0 0";
      scale = "1 1 1";
      dataBlock = "SoldierBotData";
      collision = "0";
	  enablePerception = "false";
   };
   
   //sold.setInventory(BlasterGun,1);
   //sold.setInventory(MachineGunAmmo,1000);
   //sold.setInventory(GrenadeAmmo,20);
   //sold.mountImage(MachineGunImage, 0);
   //sold.mountImage(GrenadeImage, 1);
}

function sold2()
{
	new EnhancedPlayer(sold2) 
	{
      canSaveDynamicFields = "1";
      Enabled = "1";
      position = "533.095 661.057 258.09"; //"549.453 663.443 257.221";
      rotation = "0 0 1 0";
      //rotation = "1 0 0 0";
      scale = "1 1 1";
      dataBlock = "SoldierBotData";
      collision = "0";
   };
   
   sold2.setInventory(BlasterGun,1);
   sold2.setInventory(BlasterAmmo,100);
   sold2.setInventory(GrenadeAmmo,20);
}

function asold()
{
	new AIBot(asold) 
	{
      canSaveDynamicFields = "1";
      Enabled = "1";
      position = "535.095 661.057 258.09"; //"549.453 663.443 257.221";
      rotation = "0 0 1 0";
      //rotation = "1 0 0 0";
      scale = "1 1 1";
      dataBlock = "SpaceOrcBotData";
      collision = "0";
   };
   
   asold.setInventory(BlasterGun,1);
   asold.setInventory(BlasterAmmo,100);
   asold.setInventory(GrenadeAmmo,20);
}

function amtp()
{
	asold.setMoveDestination("515.095 661.057 258.09");
}
function amtp2()
{
   asold.setMoveDestination("535.095 661.057 258.09");
}
function amtp3()
{
   asold.setMoveDestination("535.095 640.057 258.09");
}


function smtp()
{
   sold.moveToPoint("515.095 661.057 258.09");
}

function smtp2()
{
   sold.moveToPoint("535.095 661.057 258.09");
}

function smtp3()
{
   sold.moveToPoint("535.095 640.057 258.09");
}

function ssetLDM()
{
   sold.setLookDirMode("point","0 0 0",1);
}

function ssetLDMo()
{
   sold.setLookDirMode("object",$playerForAi);
}

function sshot()
{
   sold.setImageTrigger(0,true);
	sold.setImageTrigger(0,false);
}
function suse()
{
   sold.setActionUse(GarageLever01);
}
function showGrenadeCallbackTest(%owner)
{
	echo("showGrenade");
	%owner.mountImage(GrenadeImage, 1);
	//echo("showGrenade: initPos ", %initialPosition);
	return;
}
function throwGrenadeCallbackTest(%owner)
{
	echo("throwGrenade");
	%enemy = $playerForAi;
	%projectile = GrenadeData;
	%direction = %owner.getEyeVector();
	//calculate throw params
	%targetPos = %enemy.getPosition();
	%correctionValue = 1.5;
	%targetPos = VectorSub(%targetPos,VectorScale(%direction,%correctionValue));
	%dist = VectorLen(VectorSub(%owner.getPosition(),%targetPos));
	%alpha = 3.14/4;
	%velocity = calculateThrowVelocity(%direction, %dist, %alpha, %projectile.muzzleVelocity);
	%initialPosition = //getWords(%owner.getImageTransform(1),0,2);
	//echo("throwGrenade: initPos ", %initialPosition);
	// Create the projectile object
	%p = new (Projectile)() {
		 dataBlock        = %projectile;
		 initialVelocity  = %velocity;
		 initialPosition  = %initialPosition;
		 sourceObject     = %owner;
		 sourceSlot       = 1;
		 client           = %owner.client;
	};
	%p.owner = %owner;
	%owner.unmountImage(1);
	//$timeScale = 0.0;
}

datablock VisualDataBlock(testThrow)
{
   visualAnim        = "up_grenade";
   visualEffectors   = "hands";
   
   callbackTime[0] = 0.41;
   callbackFunction[0] = showGrenadeCallbackTest;
   
   callbackTime[1] = 0.787;
   callbackFunction[1] = throwGrenadeCallbackTest;
};
function thr()
{
	sold.playVisual("testThrow");
}
function thr2()
{
	sold2.useTransition(0);
	sold.playVisual("testThrow");
	sold2.playVisual("testThrow");
}

function sep()
{
   sold.enablePerception(true);
}

function smto()
{
	sold.moveToObject($playerForAi);
}