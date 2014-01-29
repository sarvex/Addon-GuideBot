////////////////////////////////////////////////////
//////*********Marker with obstacle***//////////////
////////////////////////////////////////////////////
datablock MissionMarkerData(SimpleObstacleMarker : WayPointMarker)
{
	class = "ObstacleMarker";
};

function ObstacleMarker::create(%datablock)
{
	 %obj = new MissionMarker() {
		dataBlock = %datablock;
	 };
	 return (%obj);
}

function ObstacleMarker::onAdd(%this, %obj)
{
   	//echo("ObstacleMarker::onAdd ", %this.getName(), %obj.radius, %obj.type);
	%obj.obstacleId = createObstacle(%obj, %obj.getTransform(), %obj.type, %obj.radius);
}
 
function ObstacleMarker::onRemove(%this,%obj)
{
	//echo("ObstacleMarker::onRemove");
	if (%obj.obstacleId)
	{
		//echo("Marker::onRemove: delete ", %obj.obstacleId);
		destroyWorldObject(%obj.obstacleId);
	}
}


////////////////////////////////////////////////////
//////*********Marker with waypoint***//////////////
////////////////////////////////////////////////////

$WaypointDefaultRadius = 0.2;

 function Marker::onAdd(%this)
 {
   	//echo("Marker::onAdd", %this.getName());
	%radius = $WaypointDefaultRadius;
	if (%this.radius)
		%radius = %this.radius;
	%this.waypointId = createWaypoint(%this, %this.getTransform(), %radius);
 }
 
 function Marker::onRemove(%this)
 {
	//echo("Marker::onRemove");
	if (%this.waypointId)
	{
		//echo("Marker::onRemove: delete ", %this.waypointId);
		destroyWorldObject(%this.waypointId);
	}
 }
 
////////////////////////////////////////////////////
//////******Rigid body with anchor****//////////////
////////////////////////////////////////////////////
function anchorObjectOnAdd(%obj, %isDynamic)
{
    %radius = $WaypointDefaultRadius;
	if (%obj.radius!$="")
		%radius = %obj.radius;

	if (%isDynamic)
		%parentObject = %obj;
		
	%obj.anchorId = createAnchor(%parentObject, %obj.getTransform(), %radius);
	if (%obj.hasObstacle!$="")
		%obj.obstacleId = createObstacle(%parentObject, %obj.getTransform(), %obj.type, %radius);
}

function anchorObjectOnRemove(%obj)
{
    if (%obj.anchorId!$="")
	{
		destroyWorldObject(%obj.anchorId);
	}
	if (%obj.obstacleId!$="")
	{
		destroyWorldObject(%obj.obstacleId);
	}
}

 function PhysicsWithAnchorData::onAdd(%this, %obj)
 {
    anchorObjectOnAdd(%obj,true);
 }
 
function PhysicsWithAnchorData::onRemove(%this, %obj)
{
    anchorObjectOnRemove(%obj);
}
 
datablock RigidBodyData(PhysBoxWithAnchor: PhysBox )
{
	class = "PhysicsWithAnchorData";
};
 
////////////////////////////////////////////////////
//////******PhysicsShape with anchor****////////////
////////////////////////////////////////////////////
function PhysicsShapeWithAnchor::onAdd(%this, %obj)
{
	anchorObjectOnAdd(%obj,true);
}

function PhysicsShapeWithAnchor::onRemove(%this, %obj)
{
	anchorObjectOnRemove(%obj);
}

datablock PhysicsShapeData(PhysicsCube)
{
	class = "PhysicsShapeWithAnchor";
    shapeName = "art/shapes/crates/crate1.dts";
	mass = 5;
	friction = 1.5;
};
 
 

 
 //-----------------------------------------------------------------------------
// for Game Mechanics Editor
//-----------------------------------------------------------------------------
$OBSTACLE_TYPE_LIST = "low medium high";
activatePackage(TemplateFunctions);
inheritTemplate("PhysBoxWithAnchor", "AbstractRigidBody");
registerTemplate("PhysBoxWithAnchor", "GuideBot", "RigidBodyData::create(PhysBoxWithAnchor);");

setTemplateField("PhysBoxWithAnchor", "radius", "1.5", "float", "AI", "Anchor radius.");
setTemplateField("PhysBoxWithAnchor", "hasObstacle", "true", "bool", "AI", "Obstacle for rigid body.");
setTemplateField("PhysBoxWithAnchor", "type", $OBSTACLE_TYPE_LIST, "list", "AI", "Obstacle type.");

registerTemplate("StaticBoxWithAnchor", "GuideBot", "StaticShapeData::create(BodyWithAnchor);");

setTemplateField("StaticBoxWithAnchor", "radius", "1.5", "", "AI", "Anchor radius.");
setTemplateField("StaticBoxWithAnchor", "hasObstacle", "true", "bool", "AI", "Obstacle for rigid body.");
setTemplateField("StaticBoxWithAnchor", "type", $OBSTACLE_TYPE_LIST, "list", "AI", "Obstacle type.");

registerTemplate("SimpleObstacleMarker", "GuideBot", "ObstacleMarker::create(SimpleObstacleMarker);");
setTemplateField("SimpleObstacleMarker", "radius", "0.5", "float", "AI", "Anchor radius.");
setTemplateField("SimpleObstacleMarker", "type", $OBSTACLE_TYPE_LIST, "list", "AI", "Obstacle type.");


deactivatePackage(TemplateFunctions);
