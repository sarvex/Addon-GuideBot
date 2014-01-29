////////////////////////////////////////////////////
//////************Script action*******//////////////
////////////////////////////////////////////////////
//script action interface

//function ScriptAction::prepareClone(%this,%clone)
//function ScriptAction::check(%this,%owner,%parentScriptAction)
//function ScriptAction::init(%this)
//function ScriptAction::update(%this)
//function ScriptAction::onTerminate(%this,%immediate)
//function ScriptAction::onEffectorsLost(%this, %lostEffectorsNum, %lostEffectorsName)
//function ScriptAction::onChildTerminated(%this, %terminatedChild, %immediate)

//default implementations
function ScriptAction::onEffectorsLost(%this, %lostEffectorsNum, %lostEffectorsName)
{
   //echo("ScriptAction::onEffectorsLost ",%this.getAcquiredEffectors());
   if (%this.getAcquiredEffectors() $= "")
   {
      %this.terminate(true);
   }
}
////////////////////////////////////////////////////
//////************Simple action*******//////////////
////////////////////////////////////////////////////

function createSimpleAction()
{
   %action = new ScriptAction()
      {  
         class = "SimpleAction";
      };
   return %action;   
}
registerAction(createSimpleAction());

function SimpleAction::setParams(%this,%params1)
{
   //echo("SimpleAction::setParams ",%params1);
   %this.params1 = %params1;
   %this.setRequiredEffectors("legs");
}

function EnhancedPlayer::setSimpleAction(%this,%params1)
{
   //echo("EnhancedPlayer::setSimpleAction ",%params1);
   %action = getActionPrototype("SimpleAction");
   %action.setParams(%params1);
   %this.setScriptAction(%action);
}

////////////////////////////////////////////////////
//////**********ActionTakeWeapon******//////////////
////////////////////////////////////////////////////

function createActionTakeWeapon()
{
   %action = new ScriptAction()
      {  
         class = "ActionTakeWeapon";
         effectors = "hands";// legs
         priority = 2;
      };
   return %action;   
}
registerAction(createActionTakeWeapon());

function ActionTakeWeapon::init(%this)
{
   //echo("ActionTakeWeapon::init: weapon: ", %this.newWeapon, " state: ", %this.newWeapon);
   %owner = %this.getOwner();
   if (%owner.getMountedImage(0) && %owner.getMountedImage(0).getName() !$= %this.newWeapon)
   {
		//echo("ActionTakeWeapon::init: unmount previous weapon ", %owner.getMountedImage(0),%this.newWeapon);
		%owner.unmountImage(0);
   }
   %currentStateName = %owner.getActorState().getName();
   if (%this.newState !$= %currentStateName)
   {
		%owner.changeState(%this.newState, %this);
   }
   else
		%owner.playVisual("drawWeaponVisual", %this);
}
function ActionTakeWeapon::onChildTerminated(%this, %terminatedChild, %immediate)
{
   //echo("ActionTakeWeapon::onChildTerminated: ",%this.newWeapon);
   %owner = %this.getOwner();
   %owner.mountImage(%this.newWeapon, 0);
   %this.terminate();
}

function ActionTakeWeapon::setParams(%this,%weapon,%state)
{
   %this.newWeapon = %weapon;
   %this.newState  = %state;
}

function ActionTakeWeapon::prepareClone(%this,%clone)
{
   %clone.newWeapon = %this.newWeapon;
   %clone.newState  = %this.newState;
}

function setActionTakeWeapon(%weapon,%state)
{
   %action = getActionPrototype("ActionTakeWeapon");
   %action.setParams(%weapon,%state);
   return %action;
}

////////////////////////////////////////////////////
//////************Draw weapon*********//////////////
////////////////////////////////////////////////////
function createActionDrawWeapon()
{
   %action = new ScriptAction()
      {  
         class = "ActionDrawWeapon";
         effectors = "hands";
         priority = 2;
      };
   return %action;   
}
registerAction(createActionDrawWeapon());

function ActionDrawWeapon::init(%this)
{
   %owner = %this.getOwner();
   %owner.playVisual("drawWeaponVisual", %this);
}

function ActionDrawWeapon::onChildTerminated(%this, %terminatedChild, %immediate)
{
   %owner = %this.getOwner();
    
   //%owner.mountImage(BlasterGunImage, 0);
   //echo("ActionDrawWeapon::onChildTerminated");
   %owner.mountImage(RocketLauncherImage, 0);
   %this.terminate();
}

////////////////////////////////////////////////////
//////************Hide weapon*******////////////////
////////////////////////////////////////////////////
function createActionHideWeapon()
{
   %action = new ScriptAction()
      {  
         class = "ActionHideWeapon";
         effectors = "hands";
         priority = 2;
      };
   return %action;   
}
registerAction(createActionHideWeapon());

function ActionHideWeapon::init(%this)
{
   %owner = %this.getOwner();
   %owner.unmountImage(0);
   %this.terminate();
}



////////////////////////////////////////////////////
//////************Shot action*********//////////////
////////////////////////////////////////////////////
function createActionShot()
{
   %action = new ScriptAction()
      {  
         class = "ActionShot";
         effectors = "hands";
      };
   return %action;   
}
registerAction(createActionShot());

$GuideBot::defaultBurstLength = 800;
$GuideBot::defaultBurstLengthDispersion = 200;

function ActionShot::init(%this)
{
	//echo("ActionShot::init");
	%owner = %this.getOwner();
	if (%owner.hasVisual("shoot"))
	{
		//shooting on callback
		%owner.playVisual("shoot", %this);
	}
	else
	{
		//%owner.setImageTrigger(0,true);
		shootCallback(%owner);
	}

	if (!%owner.isControlledByPlayer())
	{
		%this.burstLength = $GuideBot::defaultBurstLength + $GuideBot::defaultBurstLengthDispersion*(2*getRandom()-1);
		%this.shotBeginTime = GuideBot_getTime();
	}

}

function ActionShot::update(%this)
{
	if (%this.burstLength>0)
	{
		%curTime = GuideBot_getTime();
		if (%curTime  - %this.shotBeginTime > %this.burstLength)
		{
			%this.burstLength = 0;
			%this.terminate();
		}
	}	
}
/*
function ActionShot::onTerminate(%this,%immediate)
{
	%owner = %this.getOwner();
	%owner.setImageTrigger(0,false);
}
*/
function ActionShot::onChildTerminated(%this, %terminatedChild, %immediate)
{
	%this.terminate();
}

function shootCallback(%owner)
{
	%owner.setImageTrigger(0,true);
	%owner.setImageTrigger(0,false);
}


////////////////////////////////////////////////////
//////************Aimed shot action*********////////
////////////////////////////////////////////////////
createConsecutiveAction("AimedShotAction", "brain_moving hands"
						, "%owner.setPauseAction(1000, %this, \"brain_moving\");", ""
						, "%owner.setAction(\"ActionShot\", %this);", ""); 


////////////////////////////////////////////////////
//////************Use action**********//////////////
////////////////////////////////////////////////////

function createActionUse()
{
   %action = new ScriptAction()
      {  
         class = "ActionUse";
         effectors = "hands";
      };
   return %action;
}
registerAction(createActionUse());

function ActionUse::init(%this)
{
   //echo("ActionUse::init ",%this.objToUse);
   %owner = %this.getOwner();
   %useObjPos = %this.objToUse.getPosition();
   %owner.setLookDirMode("point",%useObjPos,1,%this);
   
   %dir = VectorNormalize(VectorSub(%useObjPos,%owner.getPosition()));
   %target = %owner.checkObjectsForUse(%dir);
   //echo("ActionUse::init checkObjectsForUse ",%target);
   if(%target == %this.objToUse.getId())
   {
       //echo("Successful use");
       %this.objToUse.use(%owner);
   }
   %this.terminate();
}

function ActionUse::setParams(%this,%objToUse)
{
   %this.objToUse = %objToUse;
}

function ActionUse::prepareClone(%this,%clone)
{
   %clone.objToUse = %this.objToUse;
}

function EnhancedPlayer::setActionUse(%this,%objToUse)
{
   if (!isObject(%objToUse))
      return;
   //echo("EnhancedPlayer::setActionUse ",%objToUse);
   %action = getActionPrototype("ActionUse");
   %action.setParams(%objToUse);
   %this.setScriptAction(%action);
}

////////////////////////////////////////////////////
//////**********Crouch action*********//////////////
////////////////////////////////////////////////////
function createActionCrouch()
{
   %action = new ScriptAction()  { class = "ActionCrouch"; effectors = "body";};
   return %action;
}
registerAction(createActionCrouch());
function ActionCrouch::check(%this,%owner,%parentScriptAction)
{
   %state = %owner.getActorState();
   %res = %state.crouchState!$="";
   return %res;
}
function ActionCrouch::init(%this)
{
   %owner = %this.getOwner();
   %state = %owner.getActorState();
   if (%state.crouchState$="")
   {
		%this.terminate();
		return;
   }
   %owner.changeState(%state.crouchState, %this);
}

function ActionCrouch::onChildTerminated(%this, %terminatedChild, %immediate)
{
	%this.terminate();
}

////////////////////////////////////////////////////
//////**********Stand up action*********////////////
////////////////////////////////////////////////////
function createActionStandUp()
{
   %action = new ScriptAction()  { class = "ActionStandUp"; superclass = "ActionCrouch"; effectors = "body";};
   return %action;
}
registerAction(createActionStandUp());
function ActionStandUp::check(%this,%owner,%parentScriptAction)
{
   %state = %owner.getActorState();
   %res = %state.standUpState!$="";
   return %res;
}
function ActionStandUp::init(%this)
{
   %owner = %this.getOwner();
   %state = %owner.getActorState();
   if (%state.standUpState$="")
   {
		%this.terminate();
		return;
   }
   %owner.changeState(%state.standUpState, %this);
}


////////////////////////////////////////////////////
//////******Throw grenader action*****//////////////
////////////////////////////////////////////////////
function createActionThrowGrenade()
{
   %action = new ScriptAction()
      {  
         class = "ActionThrowGrenade";
         effectors = "hands";
      };
   return %action;
}
registerAction(createActionThrowGrenade());

function ActionThrowGrenade::check(%this, %owner, %parentScriptAction)
{
	%result = %owner.hasVisual("throwGrenade");
	return %result;
}

function ActionThrowGrenade::init(%this)
{
   //echo("ActionThrowGrenade::init");
   %owner = %this.getOwner();
   %owner.playVisual("throwGrenade",%this);
}

function ActionThrowGrenade::onChildTerminated(%this,%terminatedChild,%immidiate)
{
   %owner = %this.getOwner();
   //echo("ActionThrowGrenade::onChildTerminated ",%owner);
   
   if (!%immidiate)
   {
      %this.terminate();
   }
}

function showGrenadeCallback(%owner)
{
	if (%owner.isControlledByPlayer())
	{
		%grenade = "GrenadeImage";
	}
	else
	{
		%attackInfo = %owner.getActiveAttackInfo();
		%grenade = %attackInfo.grenade;
	}
   %owner.mountImage(%grenade, 1);	
}

function throwGrenadeCallback(%owner)
{
	if (%owner.isControlledByPlayer())
	{
		%grenade = "GrenadeImage";
		%projectile = %grenade.projectile;
		%direction = %owner.getEyeVector();
		%angle = 3.14/4;
		%z = 0.7;
		%direction = setWord(%direction,2,%z);
		%velocity = VectorScale(%direction, 15);
		%initialPosition = %owner.getMuzzlePoint(1); //getWords(%owner.getImageTransform(1),0,2);
		//echo("throwGrenadeCallback: ", %direction, " ", %velocity);
		// Create the projectile object
		%p = new (Projectile)() {
			 dataBlock        = %projectile;
			 initialVelocity  = %velocity;
			 initialPosition  = %initialPosition;
			 sourceObject     = %owner;
			 sourceSlot       = 1;
			 client           = %owner.client;
		};
	}
	else
	{
		%enemy = %owner.getEnemy();
		if (isObject(%enemy))
		{
			%attackInfo = %owner.getActiveAttackInfo();   //GrenadeImage;
			%projectile = %attackInfo.grenade.projectile; //GrenadeData;
			//echo("throwGrenadeCallback ",%attackInfo.getName()," ",%attackInfo.grenade.projectile);
			%direction = %owner.getEyeVector();

			//calculate throw params
			%targetPos = %enemy.getPosition();

			%correctionValue = 1.5;
			%targetPos = VectorSub(%targetPos,VectorScale(%direction,%correctionValue));

			%dist = VectorLen(VectorSub(%owner.getPosition(),%targetPos));
			%alpha = 3.14/4;
			%velocity = calculateThrowVelocity(%direction, %dist, %alpha, %projectile.muzzleVelocity);

			%initialPosition = getWords( %owner.getImageTransform(1),0,2);
			//echo("Throw initial position: ",%initialPosition);
			//drawGrenadeTrajectory(%initialPosition, %velocity, %dist);
			//echo("Throw velocity: ",VectorLen(%velocity) ,"distance: ",%dist, " direction: ",VectorNormalize(%velocity));


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
		}
	}
	%owner.unmountImage(1);
	if(isObject(%owner.throwGrenadeSound))
    {
        %pos = %owner.getPosition();
        //%owner.throwGrenadeSound.setVolume(0.1);
        %owner.throwGrenadeSound.setTransform(%pos);
        %owner.throwGrenadeSound.play();
    } 
}