
singleton TSShapeConstructor(M4gunDts)
{
   baseShape = "./m4gun.dts";
};

function M4gunDts::onLoad(%this)
{
   %this.setDetailLevelSize("-1", "1");
}
