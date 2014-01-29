//-----------------------------------------------------------------------------
// Guide Bot
// Copyright (C) LogicKing.com, Inc.
//-----------------------------------------------------------------------------
// ActionGuard and ActionPatrol are examples of high level 
// specified solely from scripts.
//-----------------------------------------------------------------------------

// ActionPatrol - bot will follow waypoint's path specified in "patrolWaypoints" dynamic field, until 
createConsecutiveAction("ActionPatrol", // name of the action
						"brain", // effectors list, separated by space "effector1 effector2"

						// Child-action N1  - take the weapon into the hands
						// action body:
						"%owner.setActionByPrototype(setActionTakeWeapon(\"MachineGunImage\", \"SoldierPatrol\"), %this);",
						// checker function N1, if we're in patrol state, we already have weapon in hands
						"if (%owner.getActorState().getName() $= \"SoldierPatrol\") return $CheckResult::SKIP; else return $CheckResult::EXECUTE;",
						
						// Child-action N2  - move by waypoints
						"%owner.moveByWaypoints(%owner.patrolWaypoints,\"circular\", 0.5, %this);", 
						// Executing the action, we don't need any checks here, because 
						"return $CheckResult::EXECUTE;"
						);

						
// ActionGuard - depending on evaluator function choose one of the child actions
createAlternativeAction("ActionGuardScript", 
						"brain", // effectors list, separated by space "effector1 effector2"

						// Child-action N1  - partrol waypoints
						"echo(ActionPatrol); %owner.setAction(\"ActionPatrol\", %this);",
						// Evaluator function N1 - always return 1
						"return 1;", 
						
						// Child-action N2  - inspect. Play look-around animation, to if something sounds suspicious.
						"echo(ActionInspect); %owner.playVisual(\"inspect\", %this);",
						// Evaluator function N2 - check alertness
						"if (%owner.getAlertness() > $GuideBot::AWARE_ALERTNESS) return 2; else return 0;",
						
						// Child-action N3  - Not yet implemented. If bot hear something like gun shot, or  explosions he has to go and check it
						"echo(ActionAlert); ", 
						// Evaluator function N3 - check alertness
						"if (%owner.getAlertness() > $GuideBot::ALARM_ALERTNESS) return 3; else return 0;",
						
						// Child-action N4  - If bot saw the enemy it will start fighting immediately
						"echo(ActionFight); %owner.getDataBlock().onAlert(%owner); %owner.setAction(\"ActionFight\",%this);", 
						// Evaluator function N4 - check alertness
						"if (%owner.getAlertness() >= $GuideBot::ENEMY_ALERTNESS) return 4; else return 0;"
						);

						
// assigining waypoints path and stating ActionGuardScripts
function EnhancedPlayer::setGuard(%this, %patrolWaypoints)
{
   %this.patrolWaypoints = %patrolWaypoints;
   %this.setAction("ActionGuardScript");
}				
