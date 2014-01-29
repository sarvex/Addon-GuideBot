//-----------------------------------------------------------------------------
// Guide Bot
// Copyright (C) LogicKing.com, Inc.
//-----------------------------------------------------------------------------
// Descrition GiBot's visual states 
// In this file all animation and states from which giBot is made 
// of are described
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Visuals
// Visual is a combination of animation + sound + script callbacks
// Visual is intended to be played on certain effectors, so
// some visual are can be played concurently (if they use different effectors).
//-----------------------------------------------------------------------------


//UP RUNNING STATE
datablock VisualDataBlock(idleVisual)     	{	visualAnim = "root";	visualEffectors = "legs"; };

datablock VisualDataBlock(forwardVisual)  	{ 
											visualAnim = "run";  //name of Animation
											visualSound = runSound; // reference to SfxProfile
  										    
											visualEffectors = "legs";   // name of effector or effectors visual will be played on
											
											callbackTime[0] = 0.327;  	//relative time from 0 to 1 (depending on animation length)
											callbackFunction[0] = stepCallback;   //name of script function that will be called on callBackTime
											
											callbackTime[1] = 0.8;   	
											callbackFunction[1] = stepCallback; };
											
											
//  adding callback to every step bot takes, this will play step sound and add perception event, so other bot will be able to hear steps 
function stepCallback(%owner)
{
	addPerceptionEvent(%owner.getPosition(), // position of an event, 
						$GuideBot::PE_NEUTRAL, // type of event $GuideBot::PE_NEUTRAL or $GuideBot::PE_DANGER
						$GuideBot::ST_HEARING, // type of sensor $GuideBot::ST_HEARING , $GuideBot::ST_VISUAL, $GuideBot::ST_KNOWLEDGE
						0.3, //alertness from 0 - no treat, to 1 - enemy is here
						%owner, // intiator object, if needed
						5, // radius min 
						20, // radius max 
						1, // volume of an event (from 0 to 1)
						300 // time in ms event will be active
						);
					
    if(isObject(%owner.stepSound))
    {
        %pos = %owner.getPosition();
        %owner.stepSound.setVolume(0.1);
        %owner.stepSound.setTransform(%pos);
        %owner.stepSound.play();
    }    
}

datablock VisualDataBlock(backVisual)		{ visualAnim = "back";		visualEffectors = "legs"; 
                                                    callbackTime[0] = 0.327;  	callbackFunction[0] = stepCallback;   
											        callbackTime[1] = 0.8;   	callbackFunction[1] = stepCallback; };
datablock VisualDataBlock(leftVisual)     	{ visualAnim = "left";    	visualEffectors = "legs"; 
                                                    callbackTime[0] = 0.327;  	callbackFunction[0] = stepCallback;   
											        callbackTime[1] = 0.8;   	callbackFunction[1] = stepCallback; };
datablock VisualDataBlock(rightVisual) 	  	{ visualAnim = "right";     visualEffectors = "legs"; 
                                                    callbackTime[0] = 0.327;  	callbackFunction[0] = stepCallback;   
											        callbackTime[1] = 0.8;   	callbackFunction[1] = stepCallback; };

datablock VisualDataBlock(forwardRightVisual)	{ visualAnim = "up_run_forward_right";	visualEffectors = "legs"; 
                                                        callbackTime[0] = 0.327;  	callbackFunction[0] = stepCallback;   
											            callbackTime[1] = 0.8;   	callbackFunction[1] = stepCallback; };
datablock VisualDataBlock(forwardLeftVisual)	{ visualAnim = "up_run_forward_left";	visualEffectors = "legs"; 
                                                        callbackTime[0] = 0.327;  	callbackFunction[0] = stepCallback;   
											            callbackTime[1] = 0.8;   	callbackFunction[1] = stepCallback; };
datablock VisualDataBlock(backRightVisual)     	{ visualAnim = "up_run_back_right";    visualEffectors = "legs"; 
                                                        callbackTime[0] = 0.327;  	callbackFunction[0] = stepCallback;   
											            callbackTime[1] = 0.8;   	callbackFunction[1] = stepCallback;};                                                        
datablock VisualDataBlock(backLeftVisual) 	  	{ visualAnim = "up_run_back_left";     visualEffectors = "legs"; 
                                                        callbackTime[0] = 0.327;  	callbackFunction[0] = stepCallback;   
											            callbackTime[1] = 0.8;   	callbackFunction[1] = stepCallback; };
datablock VisualDataBlock(updownVisual)		{ visualAnim = "up_0-180";  visualEffectors = "spine";};
datablock VisualDataBlock(shootVisual) 		{ visualAnim = "shoot";    	visualEffectors = "hands";
											callbackTime[0] = 0.5;   	callbackFunction[0] = shootCallback;};
											
datablock VisualDataBlock(grenadeVisual) 	{ visualAnim = "up_grenade";	visualEffectors = "hands"; //legs
											callbackTime[0] = 0.41;   	callbackFunction[0] = showGrenadeCallback;
											callbackTime[1] = 0.787;   	callbackFunction[1] = throwGrenadeCallback;};
datablock VisualDataBlock(deathVisual) 		{ visualAnim = "up_dead";   visualEffectors = "legs";};
datablock VisualDataBlock(normalToKneeVisual) { time = 300; visualEffectors = "brain_moving";};

											
											
//BOW STATE
datablock VisualDataBlock(bowIdleVisual) 	{ visualAnim = "bow_idle";  	visualEffectors = "legs";};
datablock VisualDataBlock(bowForwardVisual) { visualAnim = "bow_forward"; 	visualEffectors = "legs";};
datablock VisualDataBlock(bowBackVisual)	{ visualAnim = "bow_back";  	visualEffectors = "legs";};
datablock VisualDataBlock(bowLeftVisual) 	{ visualAnim = "bow_left";    	visualEffectors = "legs";};
datablock VisualDataBlock(bowRightVisual)	{ visualAnim = "bow_right";   	visualEffectors = "legs";};

datablock VisualDataBlock(bowUpdownVisual) 	{ visualAnim = "bow_0-180";   	visualEffectors = "spine";};
datablock VisualDataBlock(bowShootVisual) 	{ visualAnim = "bow_shoot";   	visualEffectors = "hands legs";
											callbackTime[0] = 0.9;    	callbackFunction[0] = shootCallback;};
datablock VisualDataBlock(bowDeathVisual) 	{ visualAnim = "bow_dead";   	visualEffectors = "legs";};

//BEND KNEE STATE
datablock VisualDataBlock(kneeIdleVisual)	{ visualAnim = "bend_knee_idle"; 	visualEffectors = "legs";};
datablock VisualDataBlock(kneeUpdownVisual) { visualAnim = "bend_knee_0-180";   visualEffectors = "spine";};

datablock VisualDataBlock(kneeShootVisual) 	{ visualAnim = "bend_knee_shoot";   visualEffectors = "hands legs";
											callbackTime[0] = 0.5;   	callbackFunction[0] = shootCallback;};
											
											
datablock VisualDataBlock(kneeGrenadeVisual){ visualAnim = "bend_knee_grenade"; visualEffectors = "legs";   
											callbackTime[0] = 0.46;   	callbackFunction[0] = showGrenadeCallback;
											callbackTime[1] = 0.74;   	callbackFunction[1] = throwGrenadeCallback;};
datablock VisualDataBlock(kneeDeathVisual) 	{ visualAnim = "bend_knee_dead";   	visualEffectors = "legs";};
datablock VisualDataBlock(bendToCrawlVisual){ visualAnim = "from_bend_to_crawl"; visualEffectors = "hands legs";};
datablock VisualDataBlock(kneeToNormalVisual) { time = 100;  visualEffectors = "brain_moving";}; 
											
//PATROL STATE
datablock VisualDataBlock(patrolIdleVisual) { visualAnim = "patrol_idle";   visualEffectors = "legs";};
datablock VisualDataBlock(patrolForwardVisual){visualAnim = "patrol";   	visualEffectors = "legs";
                                                        callbackTime[0] = 0.327;  	callbackFunction[0] = stepCallback;   
											            callbackTime[1] = 0.8;   	callbackFunction[1] = stepCallback; };

new SFXProfile(inspectSound)
{
   filename = "art/sound/inspect";
   description = AudioDefault3D;
   preload = true;
};

datablock VisualDataBlock(inspectVisual) 	{ visualAnim = "up_view";   visualSound = "inspectSound";	visualEffectors = "hands legs";};
datablock VisualDataBlock(patrolToUpVisual) { visualAnim = "from_patrol_to_up"; visualEffectors = "hands";};

//CRAWL STATE
datablock VisualDataBlock(crawlIdleVisual)	{ visualAnim = "crawl_idle";   	visualEffectors = "legs";};
datablock VisualDataBlock(crawlForwardVisual){visualAnim = "crawl_forward"; visualEffectors = "legs";};
datablock VisualDataBlock(crawlShootVisual)	{ visualAnim = "crawl_shoot";   visualEffectors = "legs";
											callbackTime[0] = 0.5;   	callbackFunction[0] = shootCallback;};
datablock VisualDataBlock(crawlDeathVisual) { visualAnim = "crawl_dead";   	visualEffectors = "legs";};
datablock VisualDataBlock(crawlToBendVisual){ visualAnim = "from_craw_to_bend";	visualEffectors = "hands legs";};


//other visuals
datablock VisualDataBlock(drawWeaponVisual) { /*visualAnim = "";*/	   	visualEffectors = "hands";};
datablock VisualDataBlock(dummyChangeVisual){ 							visualEffectors = "hands"; };



//-----------------------------------------------------------------------------
// States 
//
//-----------------------------------------------------------------------------


// Normal state - soldier 
datablock StateDataBlock(SoldierNormal)
{
	//aliases for state visuals
	alias[0] = "idle";    		aliasVisual[0] = "idleVisual";

	alias[1] = "forward";   	aliasVisual[1] = "forwardVisual";
	alias[2] = "back";			aliasVisual[2] = "backVisual";
	alias[3] = "left";			aliasVisual[3] = "leftVisual";
	alias[4] = "right";			aliasVisual[4] = "rightVisual";

	alias[5] = "forward_right"; aliasVisual[5] = "forwardRightVisual";
	alias[6] = "forward_left";  aliasVisual[6] = "forwardLeftVisual";
	alias[7] = "back_right";   	aliasVisual[7] = "backRightVisual";
	alias[8] = "back_left";   	aliasVisual[8] = "backLeftVisual";

	alias[9] = "look";			aliasVisual[9] = "updownVisual";
	alias[10] = "shoot";   		aliasVisual[10] = "shootVisual";
	alias[11] = "throwGrenade"; aliasVisual[11] = "grenadeVisual";

	alias[12] = "inspect";		aliasVisual[12] = "inspectVisual";
	alias[13] = "death";   		aliasVisual[13] = "deathVisual";
	
	//movement data
	movementDir[0] = "0.0 0.0 0.0";		movementVisual[0] = "idle";
	movementDir[1] = "0.0 1.0 0.0";    	movementVisual[1] = "forward";			movementVel[1] = 4;
	movementDir[2] = "0.0 -1.0 0.0";   	movementVisual[2] = "back";				movementVel[2] = 3;
	movementDir[3] = "-1.0 0.0 0.0";	movementVisual[3] = "left";				movementVel[3] = 2;   
	movementDir[4] = "1.0 0.0 0.0";  	movementVisual[4] = "right";  			movementVel[4] = 2;
	movementDir[5] = "1.0 1.0 0.0"; 	movementVisual[5] = "forward_right";   	movementVel[5] = 4;
	movementDir[6] = "-1.0 1.0 0.0";   	movementVisual[6] = "forward_left";    	movementVel[6] = 4;
	movementDir[7] = "-1.0 -1.0 0.0";   movementVisual[7] = "back_left";   		movementVel[7] = 3;   
	movementDir[8] = "1.0 -1.0 0.0";   	movementVisual[8] = "back_right";   	movementVel[8] = 3;

	//change states description
	changeState[0] = "SoldierStealth";	changeVisual[0] = "dummyChangeVisual";
	changeState[1] = "SoldierPatrol";	changeVisual[1] = "dummyChangeVisual";
	changeState[2] = "SoldierKnee";   	changeVisual[2] = "normalToKneeVisual";

	crouchState = "SoldierKnee"; //"SoldierStealth";//
};

datablock StateDataBlock(SoldierKnee: SoldierNormal)
{
	//Aliases for state visuals. Aliases allow to map concrete visuals to the names that used in a state
	alias[0] = "idle";    		aliasVisual[0] = "kneeIdleVisual";

	alias[1] = "forward";   	aliasVisual[1] = "bowForwardVisual";
	alias[2] = "back";			aliasVisual[2] = "bowBackVisual";
	alias[3] = "left";			aliasVisual[3] = "bowLeftVisual";
	alias[4] = "right";			aliasVisual[4] = "bowRightVisual";

	alias[5] = "forward_right"; aliasVisual[5] = "";
	alias[6] = "forward_left";  aliasVisual[6] = "";
	alias[7] = "back_left";   	aliasVisual[7] = "";
	alias[8] = "back_right";   	aliasVisual[8] = "";

	alias[9] = "look";		aliasVisual[9] = "kneeUpdownVisual"; //"updownVisual"; //"bowUpdownVisual";//
	//alias[10] = "shoot";   	aliasVisual[10] = "";
	alias[11] = "throwGrenade"; aliasVisual[11] = "kneeGrenadeVisual"; //bowGrenadeVisual
	alias[13] = "death";   		aliasVisual[13] = "kneeDeathVisual";
	
	movementVel[1]       = 2;
	movementVel[2]       = 2;
	movementVel[3]       = 2;
	movementVel[4]       = 2;
	movementVisual[5] = "";
	movementVisual[6] = "";
	movementVisual[7] = "";
	movementVisual[8] = "";

	//change states description
	changeState[0] = "SoldierNormal";	changeVisual[0] = "kneeToNormalVisual";
	changeState[1] = "SoldierCrawl";	changeVisual[1] = "bendToCrawlVisual";

	
	// will determine size of a bounding box can be (stand, crouch, prone, swim)
	pose = "crouch";
	
	standUpState = "SoldierNormal";
	crouchState = "SoldierCrawl";
};

datablock StateDataBlock(SoldierStealth: SoldierKnee)
{
	alias[10] = "shoot";   		aliasVisual[10] = "bowShootVisual";
	//alias[11] = "throwGrenade"; aliasVisual[11] = "kneeGrenadeVisual";
	
	changeState[2] = "SoldierPatrol";	changeVisual[2] = "dummyChangeVisual";
};

datablock StateDataBlock(SoldierKneeIdle: SoldierNormal)
{
	alias[0]       = "idle";
	aliasVisual[0] = "kneeIdleVisual";
	aliasVisual[1] = "kneeIdleVisual";
	aliasVisual[2] = "kneeIdleVisual";
	aliasVisual[3] = "kneeIdleVisual";
	aliasVisual[4] = "kneeIdleVisual";

	alias[5] = "forward_right"; aliasVisual[5] = "";
	alias[6] = "forward_left";  aliasVisual[6] = "";
	alias[7] = "back_left";   	aliasVisual[8] = "";
	alias[8] = "back_right";   	aliasVisual[7] = "";
	
	
	alias[9] = "look";			aliasVisual[9] = "kneeUpdownVisual"; //"updownVisual";
	alias[10] = "shoot";   		aliasVisual[10] = "kneeShootVisual";
	//alias[11] = "throwGrenade"; 	aliasVisual[11] = "kneeGrenadeVisual";

	movementVel[1]       = 0;
	movementVel[2]       = 0;
	movementVel[3]       = 0;
	movementVel[4]       = 0;
	movementVisual[5] = "";
	movementVisual[6] = "";
	movementVisual[7] = "";
	movementVisual[8] = "";

	changeState[0]       = "SoldierNormal";	changeVisual[0] = "dummyChangeVisual";
	pose = "crouch";
};

datablock StateDataBlock(SoldierCrawl: SoldierNormal)
{
	alias[0] = "idle"; 		aliasVisual[0] = "crawlIdleVisual";
	
	alias[1] = "forward";	aliasVisual[1] = "crawlForwardVisual";
	alias[2] = "back";		aliasVisual[2] = "crawlIdleVisual";
	alias[3] = "left";		aliasVisual[3] = "crawlIdleVisual";
	alias[4] = "right";		aliasVisual[4] = "crawlIdleVisual";
	
	alias[5] = "forward_right"; aliasVisual[5] = "";
	alias[6] = "forward_left";  aliasVisual[6] = "";
	alias[7] = "back_left";   	aliasVisual[8] = "";
	alias[8] = "back_right";   	aliasVisual[7] = "";
	
	
	alias[9] = "look";			aliasVisual[9] = "";
	alias[10] = "shoot";   		aliasVisual[10] = "crawlShootVisual";
	alias[11] = "throwGrenade"; aliasVisual[11] = "";
	alias[13] = "death";   		aliasVisual[13] = "crawlDeathVisual";

	movementVel[1] = 0.5;
	movementVel[2] = 0;
	movementVel[3] = 0;
	movementVel[4] = 0;
	movementVisual[5] = "";
	movementVisual[6] = "";
	movementVisual[7] = "";
	movementVisual[8] = "";

	changeState[0] = "SoldierKnee";	changeVisual[0] = "crawlToBendVisual";

	crouchState = "";
	standUpState = "SoldierKnee";
	pose = "prone";
};

datablock StateDataBlock(SoldierPatrol: SoldierNormal)
{
	alias[0] = "idle"; 		aliasVisual[0] = "patrolIdleVisual";
	
	alias[1] = "forward";	aliasVisual[1] = "patrolForwardVisual";
	alias[2] = "back";		aliasVisual[2] = "patrolIdleVisual";
	alias[3] = "left";		aliasVisual[3] = "patrolIdleVisual";
	alias[4] = "right";		aliasVisual[4] = "patrolIdleVisual";

	alias[5] = "forward_right"; aliasVisual[5] = "";
	alias[6] = "forward_left";  aliasVisual[6] = "";
	alias[7] = "back_left";   	aliasVisual[8] = "";
	alias[8] = "back_right";   	aliasVisual[7] = "";
	

	//alias[9] = "look";			aliasVisual[9] = "updownVisual";
	//alias[10] = "shoot";   		aliasVisual[10] = "shootRunVisual";
	//alias[11] = "throwGrenade";  	aliasVisual[11] = "grenadeVisual";
	
	alias[12] = "inspect";	aliasVisual[12] = "inspectVisual";

	movementVel[1]       = 1.1;
	movementVel[2]       = 0;
	movementVel[3]       = 0;
	movementVel[4]       = 0;
	movementVisual[5] = "";
	movementVisual[6] = "";
	movementVisual[7] = "";
	movementVisual[8] = "";

	//change states visuals
	changeState[0] = "SoldierNormal";	changeVisual[0] = "patrolToUpVisual";

	crouchState = "";
};
