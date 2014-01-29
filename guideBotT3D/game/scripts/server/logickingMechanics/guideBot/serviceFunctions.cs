////////////////////////////////////////////////////
//////*********ActionExecuteChunk*****//////////////
////////////////////////////////////////////////////
function createActionExecuteChunk()
{
   %action = new ScriptAction() { class = "ActionExecuteChunk";  effectors = "hands";};
   return %action;
}
registerAction(createActionExecuteChunk());

function ActionExecuteChunk::setParams(%this,%chunk)
{
   %this.chunk = %chunk;
}

function ActionExecuteChunk::prepareClone(%this,%clone)
{
   %clone.chunk = %this.chunk;
}

function ActionExecuteChunk::init(%this)
{
	//echo("ActionExecuteChunk::init ",%this.chunk);
	%owner = %this.getOwner();
	evaluateChunk(%this.chunk,%this,%owner); //
	if (!%this.hasChilds())
		%this.terminate();
}
function ActionExecuteChunk::onChildTerminated(%this, %terminatedChild, %immediate)
{
	%this.terminate();
}

function EnhancedPlayer::executeChunk(%this,%param1)
{
   %action = getActionPrototype("ActionExecuteChunk");
   %action.setParams(%param1);
   %this.setScriptAction(%action);
}

////////////////////////////////////////////////////
//////*********ActionController*******//////////////
////////////////////////////////////////////////////
function createActionController()
{
   %action = new ScriptAction() { class = "ActionController";};
   return %action;
}
registerAction(createActionController());
function ActionController::setParams(%this, %type, %actions, %evaluators)
{
	//echo("ActionController::setParams ", %actions.count()," ", %evaluators.count());
	%this.type = %type;
	%this.actions = %actions;
	%this.evaluators = %evaluators;
}

function ActionController::prepareClone(%this,%clone)
{
	%clone.type = %this.type;
	%clone.actions = %this.actions;
	%clone.evaluators = %this.evaluators;
	//echo("ActionController::prepareClone ", %clone.actions," ", %clone.evaluators);
}

function ActionController::init(%this)
{
	//echo("ActionController::init ",%this.actions.count()," ",%this.evaluators.count());
	%action = getActionPrototype("ActionExecuteChunk");
	for(%count = 0; %count < %this.actions.count(); %count++)
	{
		%action.setParams(%this.actions.getValue(%count));
		if (%this.type $= "Consecutive")
			%this.addConsecutiveAction(%action, %this.evaluators.getValue(%count));
		else
			%this.addAlternativeAction(%action, %this.evaluators.getValue(%count));
	}
	
	if (%this.type $= "Consecutive")
	{
		%this.setFinishCallback("ActionController::onConsecutionFinished");
	}
}

function ActionController::onConsecutionFinished(%this)
{
	%this.terminate();
}


function createController(%type, %actionName, %effectors,
								  %actionChunk_0,%actionEval_0,
								  %actionChunk_1,%actionEval_1,
								  %actionChunk_2,%actionEval_2,
								  %actionChunk_3,%actionEval_3,
								  %actionChunk_4,%actionEval_4,
								  %actionChunk_5,%actionEval_5,
								  %actionChunk_6,%actionEval_6)
{
	%actions =  new ArrayObject();
	%evaluators =  new ArrayObject();
	%i = 0;
	while(%action["Chunk",%i]!$="" ||  %action["Eval",%i]!$="")
	{
		//echo("CreateController ", %type, " ", %action["Chunk",%i], " ", %action["Eval",%i]);
		%actions.add(%i,%action["Chunk",%i]);
		%evaluators.add(%i,%action["Eval",%i]);
		%i++;
	}
	
	 %action = getActionPrototype("ActionController", true);
	 %action.setRequiredEffectors(%effectors);
	 %action.setParams(%type,%actions,%evaluators);
	 
	 registerAction(%action,%actionName);
}


function createConsecutiveAction( %actionName, %effectors,
								  %actionChunk_0,%actionEval_0,
								  %actionChunk_1,%actionEval_1,
								  %actionChunk_2,%actionEval_2,
								  %actionChunk_3,%actionEval_3,
								  %actionChunk_4,%actionEval_4,
								  %actionChunk_5,%actionEval_5,
								  %actionChunk_6,%actionEval_6)
{
	createController("Consecutive", %actionName, %effectors,
								  %actionChunk_0,%actionEval_0,
								  %actionChunk_1,%actionEval_1,
								  %actionChunk_2,%actionEval_2,
								  %actionChunk_3,%actionEval_3,
								  %actionChunk_4,%actionEval_4,
								  %actionChunk_5,%actionEval_5,
								  %actionChunk_6,%actionEval_6);
}

function createAlternativeAction( %actionName, %effectors,
								  %actionChunk_0,%actionEval_0,
								  %actionChunk_1,%actionEval_1,
								  %actionChunk_2,%actionEval_2,
								  %actionChunk_3,%actionEval_3,
								  %actionChunk_4,%actionEval_4,
								  %actionChunk_5,%actionEval_5,
								  %actionChunk_6,%actionEval_6)
{
	createController("Alternative", %actionName, %effectors,
								  %actionChunk_0,%actionEval_0,
								  %actionChunk_1,%actionEval_1,
								  %actionChunk_2,%actionEval_2,
								  %actionChunk_3,%actionEval_3,
								  %actionChunk_4,%actionEval_4,
								  %actionChunk_5,%actionEval_5,
								  %actionChunk_6,%actionEval_6);
}

$CheckResult::STOP = -1;
$CheckResult::SKIP = 0;
$CheckResult::EXECUTE = 1;

createConsecutiveAction("testAction1","hands"
						,"%owner.moveToObject(wp1,%this);", "return $CheckResult::EXECUTE;"
						,"%owner.moveToObject(wp3,%this);", "return $CheckResult::EXECUTE;"
						);
						
createConsecutiveAction("testAction","hands"
						,"%owner.setAction(testAction1,%this);", "return $CheckResult::EXECUTE;"
						,"%owner.moveToObject(wp2,%this);", "return $CheckResult::EXECUTE;"
						);

createAlternativeAction("testAlternative","hands"
						,"%owner.moveToObject(wp1,%this);", "return $testVal;"
						,"%owner.moveToObject(wp3,%this);", "return 1;"
						);

function createActionNothing()
{
   %action = new ScriptAction() { class = "ActionNothing";effectors = "hands";};
   return %action;
}
registerAction(createActionNothing());
/*
function ActionNothing::update(%this)
{
	return true;
}*/
						

function ssss()
{
	//sold.setActionAlias("testAction1","testAction2");
	sold.setAction("testAction");
}

function ssss2()
{
	sold.setAction("testAlternative");
}

function createActionFromChunk(%actionName, %chunkStr, %effectors)
{
	 %action = getActionPrototype("ActionExecuteChunk", true);
	 %action.setRequiredEffectors(%effectors);
	 %action.setParams(%chunkStr);
	 registerAction(%action,%actionName);
	 return %actionName;
}

/*
createActionFromChunk("ActionCustomPatrol","echo(customPatrol);","hands");
function guard()
{
	sold.assignToGuard("wp");
	//sold.assignToGuard("wp","ActionCustomPatrol");
}
*/


/*
createConsecutiveAction("turnOnAlarm", // name
                        addMember("%actor:setAction("gotoSiren",%this)", " return true"))
						"%actor:setAction("pause", 1000)", " return true",
						"%actor:playAnim(""), " return true" - evaluator,
						"%actor:popActionAlias("alert", "turnOnAlarm")", "");

createConsecutiveAction("gotoSiren", // name
                        "%actor:moveToAnchor(%actor.alarmAnchor1)", " return true" - evaluator, 
						"%actor:moveToAnchor(%actor.alarmAnchor2), "return true" - evaluator,
						"%actor:moveToAnchor(%actor.alarmAnchor3)", "");
%actor
onSpawn = %this.setGuardAction(); %this.pushActionAlias("alert", "turnOnAlarm");
*/