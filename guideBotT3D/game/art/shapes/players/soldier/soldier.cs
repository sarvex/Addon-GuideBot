//-----------------------------------------------------------------------------
// Torque Game Engine Advanced
// Copyright (C) GarageGames.com, Inc.
//-----------------------------------------------------------------------------

singleton TSShapeConstructor(soldier)
{   
   baseShape = "./soldier.dts";
   
   //UP RUNNING STATE
   sequence0 = "art/shapes/players/soldier/animations/up_idle.dsq root";
   //running
   sequence1 = "art/shapes/players/soldier/animations/up_run_forward.dsq run";
   sequence2 = "art/shapes/players/soldier/animations/up_run_back.dsq back";
   sequence3 = "art/shapes/players/soldier/animations/up_run_left.dsq left";
   sequence4 = "art/shapes/players/soldier/animations/up_run_right.dsq right";
   //bias running
   sequence5 = "art/shapes/players/soldier/animations/up_run_forward_left_02.dsq up_run_forward_left";
   sequence6 = "art/shapes/players/soldier/animations/up_run_forward_right_01.dsq up_run_forward_right";
   sequence7 = "art/shapes/players/soldier/animations/up_run_back_left_01.dsq up_run_back_left";
   sequence8 = "art/shapes/players/soldier/animations/up_run_back_right_02.dsq up_run_back_right";
   //shooting
   sequence9 = "art/shapes/players/soldier/animations/up_shoot.dsq	shoot";
   sequence10 = "art/shapes/players/soldier/animations/up_shoot_01.dsq	shoot01";
   sequence11 = "art/shapes/players/soldier/animations/up_shoot_02.dsq	shoot02";
   //pitch
   sequence12 = "art/shapes/players/soldier/animations/up_0-180.dsq	up_0-180";
   //grenade
   sequence13 = "art/shapes/players/soldier/animations/up_grenade.dsq up_grenade";
   //transition to patrol state
   sequence14 = "art/shapes/players/soldier/animations/from_up_to_patrol.dsq from_up_to_patrol";
   
   //WALKING
   sequence15 = "art/shapes/players/soldier/animations/up_go_forward.dsq go_forward";
   sequence16 = "art/shapes/players/soldier/animations/up_go_back.dsq go_back";
   sequence17 = "art/shapes/players/soldier/animations/up_go_left.dsq go_left";
   sequence18 = "art/shapes/players/soldier/animations/up_go_right.dsq go_right";
   
   //BOW STATE
   sequence19 = "art/shapes/players/soldier/animations/bow_idle.dsq bow_idle";
   //bow walking
   sequence20 = "art/shapes/players/soldier/animations/bow_go_forward.dsq bow_forward";
   sequence21 = "art/shapes/players/soldier/animations/bow_go_back.dsq bow_back";
   sequence22 = "art/shapes/players/soldier/animations/bow_go_left.dsq bow_left";
   sequence23 = "art/shapes/players/soldier/animations/bow_go_right.dsq bow_right";
   //bow shooting
   sequence24 = "art/shapes/players/soldier/animations/bow_shoot.dsq bow_shoot";
   //bow pitch
   sequence25 = "art/shapes/players/soldier/animations/bow_0-180.dsq bow_0-180";   
   //bow grenade
   sequence26 = "art/shapes/players/soldier/animations/bow_grenade.dsq bow_grenade";
   
   //BEND KNEE STATE
   sequence27 = "art/shapes/players/soldier/animations/bend_knee_idle.dsq bend_knee_idle";
   //bend shooting
   sequence28 = "art/shapes/players/soldier/animations/bend_knee_shoot.dsq bend_knee_shoot";
   //bend pitch
   sequence29 = "art/shapes/players/soldier/animations/bend_knee_0-180.dsq bend_knee_0-180";
   //bend grenade
   sequence30 = "art/shapes/players/soldier/animations/bend_knee_grenade.dsq bend_knee_grenade";
   //transition to crawl state
   sequence31 = "art/shapes/players/soldier/animations/from_bend_to_crawl.dsq from_bend_to_crawl";
   
   //CRAWL STATE
   sequence32 = "art/shapes/players/soldier/animations/crawl_idle.dsq crawl_idle";
   sequence33 = "art/shapes/players/soldier/animations/crawl_forward.dsq  crawl_forward";
   //crawl shooting
   sequence34 = "art/shapes/players/soldier/animations/crawl_shoot.dsq crawl_shoot";
   //transition to bend
   sequence35 = "art/shapes/players/soldier/animations/from_craw_to_bend.dsq from_craw_to_bend";
   
   //PATROL STATE
   sequence36 = "art/shapes/players/soldier/animations/patrol_idle.dsq patrol_idle";
   //patrol walking
   sequence37 = "art/shapes/players/soldier/animations/patrol.dsq patrol";
   //patrol viewing 
   sequence38 = "art/shapes/players/soldier/animations/up_view.dsq up_view";
   //transition to up state
   sequence39 = "art/shapes/players/soldier/animations/from_patrol_to_up.dsq from_patrol_to_up";
   
   //death
   sequence40 = "art/shapes/players/soldier/animations/up_dead.dsq up_dead";
   sequence41 = "art/shapes/players/soldier/animations/bow_dead.dsq bow_dead";
   sequence42 = "art/shapes/players/soldier/animations/crawl_dead.dsq crawl_dead";
   sequence43 = "art/shapes/players/soldier/animations/bend_knee_dead.dsq bend_knee_dead";
   
};
//--- OBJECT WRITE BEGIN ---
function soldier::onLoad(%this)
{
   %this.addSequence("art/shapes/players/soldier/animations/up_idle.dsq", "root");
   %this.addSequence("art/shapes/players/soldier/animations/up_run_forward.dsq", "run");
   %this.addSequence("art/shapes/players/soldier/animations/up_run_back.dsq", "back");
   %this.addSequence("art/shapes/players/soldier/animations/up_run_left.dsq", "left");
   %this.addSequence("art/shapes/players/soldier/animations/up_run_right.dsq", "right");
   %this.addSequence("art/shapes/players/soldier/animations/up_run_forward_left_02.dsq", "up_run_forward_left");
   %this.addSequence("art/shapes/players/soldier/animations/up_run_forward_right_01.dsq", "up_run_forward_right");
   %this.addSequence("art/shapes/players/soldier/animations/up_run_back_left_01.dsq", "up_run_back_left");
   %this.addSequence("art/shapes/players/soldier/animations/up_run_back_right_02.dsq", "up_run_back_right");
   %this.addSequence("art/shapes/players/soldier/animations/up_shoot.dsq", "shoot");
   %this.addSequence("art/shapes/players/soldier/animations/up_shoot_01.dsq", "shoot01");
   %this.addSequence("art/shapes/players/soldier/animations/up_shoot_02.dsq", "shoot02");
   %this.addSequence("art/shapes/players/soldier/animations/up_0-180.dsq", "up_0-180");
   %this.addSequence("art/shapes/players/soldier/animations/up_grenade.dsq", "up_grenade");
   %this.addSequence("art/shapes/players/soldier/animations/from_up_to_patrol.dsq", "from_up_to_patrol");
   %this.addSequence("art/shapes/players/soldier/animations/up_go_forward.dsq", "go_forward");
   %this.addSequence("art/shapes/players/soldier/animations/up_go_back.dsq", "go_back");
   %this.addSequence("art/shapes/players/soldier/animations/up_go_left.dsq", "go_left");
   %this.addSequence("art/shapes/players/soldier/animations/up_go_right.dsq", "go_right");
   %this.addSequence("art/shapes/players/soldier/animations/bow_idle.dsq", "bow_idle");
   %this.addSequence("art/shapes/players/soldier/animations/bow_go_forward.dsq", "bow_forward");
   %this.addSequence("art/shapes/players/soldier/animations/bow_go_back.dsq", "bow_back");
   %this.addSequence("art/shapes/players/soldier/animations/bow_go_left.dsq", "bow_left");
   %this.addSequence("art/shapes/players/soldier/animations/bow_go_right.dsq", "bow_right");
   %this.addSequence("art/shapes/players/soldier/animations/bow_shoot.dsq", "bow_shoot");
   %this.addSequence("art/shapes/players/soldier/animations/bow_0-180.dsq", "bow_0-180");
   %this.addSequence("art/shapes/players/soldier/animations/bow_grenade.dsq", "bow_grenade");
   %this.addSequence("art/shapes/players/soldier/animations/bend_knee_idle.dsq", "bend_knee_idle");
   %this.addSequence("art/shapes/players/soldier/animations/bend_knee_shoot.dsq", "bend_knee_shoot");
   %this.addSequence("art/shapes/players/soldier/animations/bend_knee_0-180.dsq", "bend_knee_0-180");
   %this.addSequence("art/shapes/players/soldier/animations/bend_knee_grenade.dsq", "bend_knee_grenade");
   %this.addSequence("art/shapes/players/soldier/animations/from_bend_to_crawl.dsq", "from_bend_to_crawl");
   %this.addSequence("art/shapes/players/soldier/animations/crawl_idle.dsq", "crawl_idle");
   %this.addSequence("art/shapes/players/soldier/animations/crawl_forward.dsq", "crawl_forward");
   %this.addSequence("art/shapes/players/soldier/animations/crawl_shoot.dsq", "crawl_shoot");
   %this.addSequence("art/shapes/players/soldier/animations/from_craw_to_bend.dsq", "from_craw_to_bend");
   %this.addSequence("art/shapes/players/soldier/animations/patrol_idle.dsq", "patrol_idle");
   %this.addSequence("art/shapes/players/soldier/animations/patrol.dsq", "patrol");
   %this.addSequence("art/shapes/players/soldier/animations/up_view.dsq", "up_view");
   %this.addSequence("art/shapes/players/soldier/animations/from_patrol_to_up.dsq", "from_patrol_to_up");
   %this.addSequence("art/shapes/players/soldier/animations/up_dead.dsq", "up_dead");
   %this.addSequence("art/shapes/players/soldier/animations/bow_dead.dsq", "bow_dead");
   %this.addSequence("art/shapes/players/soldier/animations/crawl_dead.dsq", "crawl_dead");
   %this.addSequence("art/shapes/players/soldier/animations/bend_knee_dead.dsq", "bend_knee_dead");
   %this.setSequenceCyclic("up_0-180", "0");
   %this.setSequenceCyclic("up_grenade", "0");
   %this.setSequenceCyclic("bend_knee_grenade", "0");
   %this.setSequenceCyclic("bow_grenade", "0");
   %this.setSequenceCyclic("shoot", "0");
   %this.setSequenceCyclic("bow_shoot", "0");
   %this.setSequenceBlend("up_0-180", "1", "root", "0");
   %this.setSequenceBlend("shoot", "1", "root", "0");
   %this.setSequenceCyclic("up_view", "0");
   %this.setNodeTransform("eye", "0.0160379 0.110966 1.70148 0.997993 0.00835681 -0.0627767 0.0211377", "1");
   %this.setSequenceCyclic("from_patrol_to_up", "0");
   %this.setSequenceBlend("from_patrol_to_up", "1", "patrol_idle", "0");
   %this.setSequenceCyclic("from_bend_to_crawl", "0");
   %this.setSequenceCyclic("from_craw_to_bend", "0");
   %this.setSequenceCyclic("crawl_shoot", "0");
   %this.setSequenceCyclic("bow_0-180", "0");
   %this.setSequenceBlend("bow_0-180", "1", "bow_idle", "0");
   %this.setSequenceCyclic("bend_knee_shoot", "0");
   %this.setSequenceCyclic("from_up_to_patrol", "0");
   %this.setSequenceBlend("bow_shoot", "1", "bow_idle", "0");
   %this.setSequenceBlend("crawl_shoot", "1", "crawl_idle", "0");
   %this.setSequenceCyclic("bend_knee_0-180", "0");
   %this.setSequenceBlend("bend_knee_0-180", "0", "", "0");
   %this.setSequenceBlend("bend_knee_0-180", "1", "bend_knee_idle", "0");
   %this.setSequenceCyclic("shoot", "1");
   %this.setSequenceCyclic("bow_shoot", "1");
   %this.setSequenceCyclic("bend_knee_shoot", "1");
   %this.setSequenceCyclic("crawl_shoot", "1");
   %this.setSequenceBlend("crawl_shoot", "0", "crawl_idle", "0");
   %this.setSequenceBlend("bow_shoot", "0", "bow_idle", "0");
   %this.setSequenceBlend("up_grenade", "1", "root", "0");
   %this.setSequenceCyclic("up_dead", "0");
   %this.setSequenceCyclic("crawl_dead", "0");
   %this.setSequenceCyclic("bow_dead", "0");
   %this.setSequenceCyclic("bend_knee_dead", "0");
}

singleton TSShapeConstructor(soldier)
{
   baseShape = "art/shapes/players/soldier/soldier.dts";
   upAxis = "DEFAULT";
   unit = "-1";
   lodType = "DetectDTS";
   singleDetailSize = "2";
   ignoreNodeScale = "0";
   adjustCenter = "0";
   adjustFloor = "0";
   forceUpdateMaterials = "0";
   canSaveDynamicFields = "1";
   Enabled = "1";
};
//--- OBJECT WRITE END ---
