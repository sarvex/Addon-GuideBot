
singleton TSShapeConstructor(GrenadeDts)
{
   baseShape = "./grenade.dts";
};

function GrenadeDts::onLoad(%this)
{
   %this.setDetailLevelSize("-1", "1");
}
