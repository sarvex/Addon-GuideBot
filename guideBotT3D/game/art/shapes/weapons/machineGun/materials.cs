//*****************************************************************************
// Bullet trace Materials
//*****************************************************************************
new Material(BulletTraceMat)
{
   baseTex[0] = "bullettr_texture";
   emissive[0] = true;
   glow[0] = true;
};

singleton Material(DefaultMaterial0)
{
   mapTo = "m_249";
   diffuseMap[0] = "art/shapes/weapons/machineGun/m_249";
   normalMap[0] = "art/shapes/weapons/machineGun/m_249_bump.tga";
};
