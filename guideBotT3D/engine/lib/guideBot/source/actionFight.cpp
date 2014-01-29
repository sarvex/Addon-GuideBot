//-----------------------------------------------------------------------------
// Guide Bot
// Copyright (C) LogicKing.com, Inc.
//-----------------------------------------------------------------------------

#include "guideBot/actionFight.h"
#include "guideBot/actor.h"

#include <algorithm>

using namespace GuideBot;


namespace
{
	const unsigned int attackSelectingPeriod = 1000;
	const unsigned int anchorSelectingPeriod = 1000;

	const float MAX_DIST_TO_ANCHOR = 40.f;
	const float MIN_DIST_TO_ENEMY = 7.f;
}

////////////////////////////////////////////////////
//////********ActionAvoidDanger******//////////////
////////////////////////////////////////////////////
IMPLEMENT_REGISTER_ROUTINE(ActionAvoidDanger);

ActionAvoidDanger::ActionAvoidDanger():	m_actionFlee(NULL)
										, m_fleePrototype(NULL)
										, m_fleeCloseDist(10.f)
										, m_fleeFarDist(15.f)

{
	addRequiredEffector(E_BRAIN,false);
}

void ActionAvoidDanger::init()
{
	Parent::init();
	ActionFlee* prototype = GET_ACTION_PROTOTYPE(ActionFlee);
	m_fleePrototype = static_cast<ActionFlee*>(prototype->clone());
	m_fleePrototype->setEffectorPriority(E_BRAIN_MOVING,2); //set higher priority
	m_fleePrototype->setFleeDistance(m_fleeCloseDist, m_fleeFarDist-m_fleeCloseDist);
}

void ActionAvoidDanger::terminate(TerminationStatus status /* = FINISHED */)
{
	GB_SAFE_DELETE(m_fleePrototype);
	Parent::terminate(status);
}

bool ActionAvoidDanger::update()
{
	if (!Parent::update())
		return false;
	Actor::MemoryFrame* frame = m_owner->findNearestPerceptionEvent(PerceptionEvent::PE_DANGER);
	bool isFleeing = false;
	if (frame && frame->object)
	{
		if (!m_actionFlee)
		{
			float distSqToObj = (frame->object->getPos() - m_owner->getPos()).lengthSq();
			
			if (distSqToObj<m_fleeCloseDist*m_fleeCloseDist)
			{
				isFleeing = true;
				//set flee state
				if (!m_fleeState.empty() && m_fleeState!=m_owner->getActorState()->getName())
				{
					m_owner->changeState(m_fleeState, this);
				}
				m_actionFlee = static_cast<ActionFlee*>(m_owner->setActionByPrototype(m_fleePrototype, this));
			}			
		}
		
		if (m_actionFlee)
		{
			if (m_actionFlee->getTacticalMovement() != ActionManoeuvre::Holding)
			{
				isFleeing = true;
				m_actionFlee->setTarget(frame->object);
			}
		}
	}
	
	if (!isFleeing && m_actionFlee)
		m_actionFlee->terminate(CANCELED);

	return true;
}

void ActionAvoidDanger::onChildTerminated(Action* terminatedChild)
{
	Parent::onChildTerminated(terminatedChild);
	if (m_actionFlee == terminatedChild)
		m_actionFlee = NULL;
}

void ActionAvoidDanger::setFleeState(const std::string& state)
{
	m_fleeState = state;
}


////////////////////////////////////////////////////
//////************ActionFight*********//////////////
////////////////////////////////////////////////////
IMPLEMENT_REGISTER_ROUTINE(ActionFight);

ActionFight::ActionFight() :	m_enemy(NULL)
								, m_curAttackInfo(NULL)
								, m_anchor(NULL)
								, m_lastAttackSelectedTime(0)
								, m_movingToPoint(NULL)
								, m_lookAtAction(NULL)
{
	addRequiredEffector(E_BRAIN);
	addRequiredEffector(E_BRAIN_MOVING);
	//addRequiredEffector(E_LEGS);
	addRequiredEffector(E_HANDS);
}

void ActionFight::setParams()
{
}

void ActionFight::prepareClone(Action* actionClone)
{
	Parent::prepareClone(actionClone);

	//ActionFight* actionFightClone = static_cast<ActionFight*>(actionClone);
}

void ActionFight::terminate(TerminationStatus status /*= FINISHED*/)
{
	Parent::terminate(status);
	//m_owner->setActionByName("ActionHideWeapon");
}

void ActionFight::init()
{
	Parent::init();

	//init control actions
	//AlternativeControlAction* moveManager = static_cast<AlternativeControlAction*>(ActionLibrary::instance().createAction("AlternativeControlAction"));
	//m_moveManager = static_cast<AlternativeControlAction*>(m_owner->setActionDirectly(moveManager,this));
	m_moveManager = addAlternativeControl();
	m_moveManager->addActionByName("ActionManoeuvre");
	AlternativeControlAction::ActionScoreEvaluator evaluator;
	evaluator.bind(&ActionCover::scoreEvaluator);
	m_moveManager->addActionByName("ActionCover",evaluator);

	m_owner->setActionByName("ActionAttack",this);

	//init attack info time table with random values
	Time curTime = Platform::getTime();
	const AttackInfos& attacks =  m_owner->getAttackInfos();
	for (size_t i = 0; i<attacks.size(); i++)
	{
		AttackInfo* info = attacks[i];
		const unsigned int defaultPeriodicity = 500;
		unsigned int randomScale = info->periodicity > 0 ? info->periodicity : defaultPeriodicity;
		m_attackInfoTimeTable[info] = curTime -  (Time)((float)randomScale*randF(0.f,1.f));
	}

	m_owner->setActionByName("ActionAvoidDanger",this);
}

bool ActionFight::update()
{
	if (!Parent::update())
		return false;

	WorldObject* enemy = m_owner->getEnemy();
	if (enemy!=m_enemy)
		onEnemyChanged(enemy);

	if (m_owner->getAlertness() < 1.f)
	{
		terminate();
		return false;
	}

	unsigned int curTime = Platform::getTime();
	if (curTime - m_lastAttackSelectedTime > attackSelectingPeriod)
	{
		m_lastAttackSelectedTime = curTime;
		//select attack
		AttackInfo* bestAttack  = selectBestAttack();
		
		if (bestAttack != m_curAttackInfo)	//attack changed
		{	
			//GB_INFO("AttactInfo changed, new action: %s", bestAttack ? bestAttack->name.c_str() : "");

			if (bestAttack)
			{
				bool needToChangeState  = !bestAttack->state.empty() && (!m_curAttackInfo || bestAttack->state!=m_curAttackInfo->state);
				bool needToChangeWeapon = !bestAttack->weapon.empty() && (!m_curAttackInfo ||  bestAttack->weapon!=m_curAttackInfo->weapon);
				if (needToChangeState || needToChangeWeapon)
				{
					//take selected weapon and proceed to proper state
					m_owner->takeWeapon(bestAttack->weapon,bestAttack->state,this);
				}
			}		
			m_curAttackInfo = bestAttack;
			handleLookDirMode();
			
			//update child actions
			ActionManoeuvre* actionManoeuvre = GET_ACTION(m_moveManager,ActionManoeuvre);
			if (m_curAttackInfo)
				actionManoeuvre->setMinMaxDist(m_curAttackInfo->manoeuvreMinDist,m_curAttackInfo->manoeuvreMaxDist,m_curAttackInfo->strafe);
			else
				actionManoeuvre->setDefaultParams();
			m_moveManager->validateAction(actionManoeuvre);
			
			ActionCover* actionCover = GET_ACTION(m_moveManager,ActionCover);
			actionCover->onAttackChanged(m_curAttackInfo);
			m_moveManager->validateAction(actionCover);

			ActionAttack* attack = GET_CHILD(ActionAttack);
			GB_ASSERT(attack,"Here must be attack action");
			attack->onAttackChanged(m_curAttackInfo);

			ActionAvoidDanger* avoidDanger =  GET_CHILD(ActionAvoidDanger);
			avoidDanger->setFleeState(m_curAttackInfo ? m_curAttackInfo->state : "");
			
		}
	}	

	return true;
}

void ActionFight::onActionAttackExecuted(AttackInfo* executedAttackInfo)
{
	GB_ASSERT(executedAttackInfo,"ActionFight::onActionAttackExecuted: executedAttackInfo must be defined");

	const float defaultScale = 0.2f;
	const unsigned int defaultPeriodicity = 500;
	unsigned int randomScale = executedAttackInfo->periodicity > 0 ? executedAttackInfo->periodicity : defaultPeriodicity;
	m_attackInfoTimeTable[executedAttackInfo] = Platform::getTime() - 
							((float)randomScale*defaultScale*randF(0.f,1.f));
}

void ActionFight::onChildTerminated(Action* terminatedChild)
{
	Parent::onChildTerminated(terminatedChild);
	if ( m_lookAtAction == terminatedChild)
		m_lookAtAction = NULL;

}

void ActionFight::onEffectorsLost(const EffectorIDVector& lostEffectors)
{
	Parent::onEffectorsLost(lostEffectors);
}

void ActionFight::onEffectorsAcquired(const EffectorIDVector& newEffectors)
{
	Parent::onEffectorsAcquired(newEffectors);
}

void ActionFight::onEnemyChanged(WorldObject* newEnemy)
{
	m_enemy = newEnemy;
	handleLookDirMode();
	m_lastAttackSelectedTime = 0; //force attack info selecting 

	ActionManoeuvre* actionManoeuvre = GET_ACTION(m_moveManager,ActionManoeuvre);
	actionManoeuvre->setTarget(m_enemy);
	m_moveManager->validateAction(actionManoeuvre);
}

void ActionFight::handleLookDirMode()
{
	//set looking mode
	if (m_enemy && (!m_curAttackInfo || m_curAttackInfo->lookAtEnemy))
	{
		if (!m_lookAtAction)
		{
			Action* prototype = ActionLibrary::instance().getActionPrototype("ActionLookAt");
			prototype->setPriority(2);
			m_lookAtAction = static_cast<ActionLookAt*>(m_owner->setActionByPrototype(prototype,this));
		}
		m_lookAtAction->setParams(ActionLookAt::L_OBJECT,NULL,m_enemy);
	}
	else if (m_lookAtAction)
	{
		m_lookAtAction->terminate(CANCELED);
	}
}


AttackInfo* ActionFight::selectBestAttack()
{
	WorldObject* enemy = m_owner->getEnemy();
	const Actor::MemoryFrame* enemyFrame = m_owner->getEnemyFrame();

	if(!enemy) return NULL;

	float maxScore = 0.f;
	AttackInfo* bestAttack = NULL;
	const AttackInfos& attacks =  m_owner->getAttackInfos();
	for (size_t i = 0; i<attacks.size(); i++)
	{
		AttackInfo* info = attacks[i];
		int lastExecutionTime = m_attackInfoTimeTable.find(info) != m_attackInfoTimeTable.end() ?
			m_attackInfoTimeTable[info] : 0;
		float score;
		bool isActive = info == m_curAttackInfo;
		if (!info->evaluator.empty())
			score = info->evaluator(m_owner, enemy, enemyFrame->distSq, enemyFrame->fov, info, 
			lastExecutionTime, isActive);
		else
		{
			score = defaultAttackEvaluator(enemy, enemyFrame->distSq, enemyFrame->fov, info,
				lastExecutionTime, isActive);
		}

		if (score>maxScore)
		{
			bestAttack = info;
			maxScore = score;
		}
	}

	return bestAttack;
}

float ActionFight::defaultAttackEvaluator(WorldObject* enemy, float distSq, float fov, AttackInfo* attackInfo,
										  unsigned int lastExecutionTime, bool isActive )
{
	unsigned int curTime = Platform::getTime();
	//default evaluator
	if (distSq>attackInfo->minDistSq && distSq < attackInfo->maxDistSq && fov < attackInfo->fov && 
		attackInfo->periodicity <= (curTime-lastExecutionTime))
	{
		return 1.f;
	}

	return 0.f;
}

////////////////////////////////////////////////////
//////************ActionAttack********//////////////
////////////////////////////////////////////////////
IMPLEMENT_REGISTER_ROUTINE(ActionAttack);
ActionAttack::ActionAttack():	m_attackInfo(NULL)
								, m_lastAttackTime(0)
								, m_attackAction(NULL)
								, m_activeAttackInfo(NULL)
								, m_manager(NULL)
{
	addRequiredEffector(E_HANDS,false);
}


void ActionAttack::init()
{
	Parent::init();
}


bool ActionAttack::update()
{
	if (!Parent::update())
		return false;

	if (m_activeAttackInfo)
		return false;

	unsigned int curTime = Platform::getTime();
	const Actor::MemoryFrame* enemyFrame = m_owner->getEnemyFrame();
	if (m_attackInfo && !m_attackInfo->action.empty() && 
		curTime - m_lastAttackTime > m_attackInfo->pause && enemyFrame && enemyFrame->los)
	{
		//GB_INFO("ActionAttack start action");
		m_activeAttackInfo = m_attackInfo;
		m_owner->setActiveAttackInfo(m_activeAttackInfo);
		//m_attackAction = m_owner->setActionByName(m_attackInfo->action, this);
		m_manager->restart();
	}	

	return true;
}

void ActionAttack::onAttackEnd()
{
	if (!m_activeAttackInfo)
		return;
	//GB_INFO("ActionAttack end action");

	ActionFight* actionFight = static_cast<ActionFight*>(getParentAction());
	if (actionFight)
		actionFight->onActionAttackExecuted(m_activeAttackInfo);

	m_activeAttackInfo = NULL;
	m_owner->setActiveAttackInfo(NULL);
	unsigned int curTime = Platform::getTime();
	m_lastAttackTime = curTime;	
}

void ActionAttack::onChildTerminated(Action* terminatedChild)
{
	Parent::onChildTerminated(terminatedChild);
	onAttackEnd();
}

void ActionAttack::onAttackChanged(AttackInfo* attackInfo)
{
	m_attackInfo = attackInfo;
	if (m_attackInfo)
	{
		m_manager = addConsecutiveControl();
		
		ActionChangeState* toAttackState = NULL;

		if (!m_attackInfo->attackState.empty())
		{
			//set attack visual
			toAttackState = GET_ACTION_PROTOTYPE(ActionChangeState);
			toAttackState->setParams(m_attackInfo->attackState);
			m_manager->addActionByPrototype(toAttackState);
/*
			//wait some time (wait for end of blending into target state)
			ActionPause* actionPause = CREATE_ACTION(ActionPause);
			if (m_attackInfo->blockMovements)
				actionPause->addRequiredEffector(E_BRAIN_MOVING);
			actionPause->setDuration(1300); //3000);//
			m_manager->addActionDirectly(actionPause);
*/
		}
		

		//attack action
		Action* action = ActionLibrary::instance().createAction(m_attackInfo->action);
		if (m_attackInfo->blockMovements)
			action->addRequiredEffector(E_BRAIN_MOVING);
		m_manager->addActionDirectly(action);
		//m_manager->addActionByName(m_attackInfo->action);

		if (!m_attackInfo->attackState.empty())
		{
/*
			//pause after attack
			ActionPause* actionPause = CREATE_ACTION(ActionPause);
			if (m_attackInfo->blockMovements)
				actionPause->addRequiredEffector(E_BRAIN_MOVING);
			actionPause->setDuration(500); //3000);//
			m_manager->addActionDirectly(actionPause);
*/			
			//return into normal state
			toAttackState->setParams(m_attackInfo->state);
			m_manager->addActionByPrototype(toAttackState);
		}

		//callback for onAttackEnd
		ConsecutiveControlAction::FinishedCallback callback;
		callback.bind(this,&ActionAttack::onAttackEnd);
		m_manager->setFinishedCallback(callback);
	}
}

////////////////////////////////////////////////////
//////************ActionCover*********//////////////
////////////////////////////////////////////////////
IMPLEMENT_REGISTER_ROUTINE(ActionCover);

ActionCover::ActionCover():	m_anchor(NULL)
							, m_lastAnchorUpdateTime(0)
							, m_coverPos(Vector3::ZERO)
							, m_attackInfo(NULL)
							, m_manager(NULL)
{
	//addRequiredEffector(E_LEGS);
	addRequiredEffector(E_BRAIN_MOVING);
}

void ActionCover::updateDynamicParams(Action* actionClone)
{
	Parent::updateDynamicParams(actionClone);

	ActionCover* actionCoverClone = static_cast<ActionCover*>(actionClone);
	actionCoverClone->m_attackInfo = m_attackInfo;
}

void ActionCover::init()
{
	Parent::init();

	//create manager
	/*
	ConsecutiveControlAction* manager = static_cast<ConsecutiveControlAction*>
											(ActionLibrary::instance().createAction("ConsecutiveControlAction"));
	m_manager = static_cast<ConsecutiveControlAction*>(m_owner->setActionDirectly(manager,this));*/
	m_manager = addConsecutiveControl();
	m_manager->addActionByName("ActionMoveToPoint");
}

void ActionCover::onAttackChanged(AttackInfo* newAttackInfo)
{
	m_attackInfo = newAttackInfo;
}

void ActionCover::handleCoverStateChange()
{
	//to cover state
	if (m_attackInfo && !m_attackInfo->coverState.empty() && 
		m_owner->getActorState()->getName() != m_attackInfo->coverState &&
		m_anchor && m_anchor->isInCover(m_owner->getPos(),m_owner->getEnemy())) 
					//m_anchor->isInside(m_owner->getPos(),1.f)
	{
		m_normalState = m_attackInfo->state;	//remember base state(to set it on termination as m_attackInfo can be already new there)
		m_owner->changeState(m_attackInfo->coverState,this);
	}
	//from cover
	else if (m_attackInfo && m_owner->getActorState()->getName() == m_attackInfo->coverState &&
		(!m_anchor || !m_anchor->isInCover(m_owner->getPos(),m_owner->getEnemy())))
						//!m_anchor->isInside(m_owner->getPos(),1.5f))
	{
		m_owner->changeState(m_attackInfo->state,this);
	}
}

bool ActionCover::update()
{
	if (!Parent::update())
		return false;

	WorldObject* enemy = m_owner->getEnemy();
	if (!enemy || !m_attackInfo)
		return false;

	unsigned int curTime = Platform::getTime();
	Vector3 ownerPos = m_owner->getPos();

	Anchors& anchors = Anchor::getAnchors();

	//anchor validation
	bool forceUpdate = false;
	if (m_anchor && std::find(anchors.begin(), anchors.end(), m_anchor) == anchors.end())
	{
		m_anchor = NULL;
		forceUpdate = true;
	}

	handleCoverStateChange();

	if (curTime - m_lastAnchorUpdateTime > attackSelectingPeriod || forceUpdate)
	{
		unsigned int delta = curTime - m_lastAnchorUpdateTime; 
		m_lastAnchorUpdateTime = curTime;
		/*
		//update anchor occupancy statistic
		if (m_anchor)
		m_anchorStat[m_anchor] += delta;
		*/
		updateAnchorStat(m_anchor,delta);
		//choose anchor
		Anchor* newAnchor = NULL;
		float bestRank = 0.f;
		for (size_t i=0; i<anchors.size(); i++)
		{
			Anchor* anchor = anchors[i];
			float rank = anchorRankEvaluator(anchor);
			if (rank >= bestRank)
			{
				bestRank = rank;
				newAnchor = anchor;
			}
		}

		Vector3 newCoverPos;
		if (newAnchor)
		{
			//choose cover position
			newCoverPos = newAnchor->getCoverPosition(enemy);
			Vector3 toCoverPos = newCoverPos - ownerPos;
			toCoverPos.z = 0.f;
			if (!m_anchor || toCoverPos.lengthSq() > 0.25f*0.25f)
				//(newCoverPos - m_coverPos).lenSquared() > POSITION_DELTA
			{
				m_coverPos = newCoverPos;
				
				//set new target for moveToPoint action
				ActionMoveToPoint* moveToPoint = GET_ACTION(m_manager,ActionMoveToPoint);
				moveToPoint->setParams(m_coverPos);
				if (!m_manager->validateAction(moveToPoint))
					m_manager->restart();
			}
		}


		if (m_anchor != newAnchor)
		{
			//set anchor guests
			if (m_anchor)
				m_anchor->setGuest(NULL);
			if (newAnchor)
				newAnchor->setGuest(m_owner);

			m_anchor = newAnchor;
		}
	}

	return true;
}

void ActionCover::terminate(TerminationStatus status /*= FINISHED*/)
{
	Parent::terminate(status);

	if (m_anchor && m_anchor->getGuest()==m_owner)
	{
		m_anchor->setGuest(NULL);
	}
	m_anchor = NULL;
}

void ActionCover::onChildTerminated(Action* terminatedChild)
{
	Parent::onChildTerminated(terminatedChild);
}

float ActionCover::anchorRankEvaluator(Anchor* anchor)
{
	float rank = -1.f;
	if (anchor->isVacant(m_owner))
	{
		float dist = anchor->getDistSq(m_owner->getPos());

		WorldObject* enemy = m_owner->getEnemy();
		if (!enemy)
			return rank;
		float distToEnemySq = (anchor->getPos() - enemy->getPos()).lengthSq();
		if (distToEnemySq < MIN_DIST_TO_ENEMY*MIN_DIST_TO_ENEMY)
			return rank;

		if (dist < MAX_DIST_TO_ANCHOR*MAX_DIST_TO_ANCHOR)
		{
			rank = 1.f/dist;

			AnchorStatMap::iterator it = m_anchorStat.find(anchor);
			unsigned int occupancyTime = it!=m_anchorStat.end() ? it->second : 0;
			float occupancyTimeSec = (float) occupancyTime / 1000.f;
			if (m_anchor == anchor && occupancyTimeSec < 10.f)
			{
				rank += 5.f; //persistence
			}
			else if(m_anchor != anchor && occupancyTimeSec < 5.f)
			{
				rank += 5.f - occupancyTimeSec;
			}
		}
	}

	return rank;
};

void ActionCover::updateAnchorStat(Anchor* anchor,unsigned int delta)
{
	const float emancipationFactor = 0.3f;
	unsigned int emancipationValue = (unsigned int )((float) delta* emancipationFactor);
	for(AnchorStatMap::iterator it = m_anchorStat.begin(), end = m_anchorStat.end(); 
		it!=end; it++)
	{
		if (it->first!=anchor)
		{
			it->second = std::max(it->second - emancipationValue,(unsigned int ) 0);
		}
	}

	if (anchor)
		m_anchorStat[anchor] += delta;
}

int ActionCover::scoreEvaluator(Actor* actor,Action* action)
{
	ActionCover* self = static_cast<ActionCover*>(action);
	Anchor* resAnchor = NULL;

	WorldObject* enemy = actor->getEnemy();
	if (!enemy)
		return 0;

	if (self->m_attackInfo && !self->m_attackInfo->coverState.empty())
	{
		//check if there is anchor hereabout
		Anchors& anchors = Anchor::getAnchors();
		for (size_t i=0; i<anchors.size(); i++)
		{
			Anchor* anchor = anchors[i];
			float distToEnemySq = (anchor->getPos() - enemy->getPos()).lengthSq();
			if (distToEnemySq < MIN_DIST_TO_ENEMY*MIN_DIST_TO_ENEMY)
				continue;

			//.todo: check conditions(i.e. reasonable distance, availability, etc)
			if (anchor->isVacant(actor))
			{
				resAnchor = anchor;
				break;
			}
		}
	}

	if (resAnchor)
		return 5;
	else
		return 0;
}
