
singleton TSShapeConstructor(MachineGunDts)
{
   baseShape = "./machineGun.dts";
};

function MachineGunDts::onLoad(%this)
{
   %this.setDetailLevelSize("-1", "1");
}
