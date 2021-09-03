#include "MicroCarriers.h"

using namespace UAlbertaBot;

MicroCarriers::MicroCarriers()
	: unitClosestToEnemy(nullptr)
{ 
}

void MicroCarriers::executeMicro(const BWAPI::Unitset & targets)
{
    const BWAPI::Unitset & carriers = getUnits();

	// The set of potential targets.
	BWAPI::Unitset carrierTargets;
    std::copy_if(targets.begin(), targets.end(), std::inserter(carrierTargets, carrierTargets.end()),
		[](BWAPI::Unit u) {
		return
			u->isVisible() &&
			u->isDetected() &&
			u->getType() != BWAPI::UnitTypes::Zerg_Larva &&
			u->getType() != BWAPI::UnitTypes::Zerg_Egg &&
			!u->isStasised();
	});

	BWAPI::Unitset tankUnits;
	BWAPI::Unitset goliathUnits;
	BWAPI::Unitset missileUnits;
	for (const auto unit : BWAPI::Broodwar->enemy()->getUnits())
	{
		if ((unit->isVisible() && unit->exists()) &&
			unit->getOrder() == BWAPI::Orders::Sieging ||
			unit->getOrder() == BWAPI::Orders::Unsieging ||
			unit->getType() == BWAPI::UnitTypes::Terran_Siege_Tank_Siege_Mode ||
			unit->getType() == BWAPI::UnitTypes::Terran_Siege_Tank_Tank_Mode)
		{
			tankUnits.insert(unit);
		}
		if ((unit->isVisible() && unit->exists()) &&
			unit->getType() == BWAPI::UnitTypes::Terran_Goliath)
		{
			goliathUnits.insert(unit);
		}
	}

	BWAPI::Unit closestSigedTank = nullptr;
	double closestDist = 12 * 32; 
	for (const auto unit : tankUnits)
	{
		// if we haven't already assigned an detectorUnit to this cloaked unit
		int dist = unit->getDistance(carriers.getPosition());

		if (dist < closestDist)
		{
			closestSigedTank = unit;
			closestDist = dist;
		}
	}


    for (const auto carrier : carriers)
	{
		if (buildScarabOrInterceptor(carrier))
		{
			// If we started one, no further action this frame.
			continue;
		}

		// Carriers stay at home until they have enough interceptors to be useful,
		// or retreat toward home to rebuild them if they run low.
		// On attack-move so that they're not helpless, but that can cause problems too....
		// Potentially useful for other units.
		// NOTE Regrouping can cause the carriers to move away from home.
		if (stayHomeUntilReady(carrier))
		{
			BWAPI::Position fleeTo(InformationManager::Instance().getMyMainBaseLocation()->getPosition());
			//Micro::AttackMove(carrier, fleeTo);
			Micro::Move(carrier, fleeTo);
			continue;
		}

		/*
		if (notSafeHealthAndShield(carrier))
		{
			BWAPI::Position fleeTo(InformationManager::Instance().getMyMainBaseLocation()->getPosition());
			Micro::Move(carrier, fleeTo);
			continue;
		}*/

		BWAPI::Unit cloestThreat = nullptr;
		if (1)
		{
			// if we have no dragoons near
			BWAPI::Unitset nearUnits = carrier->getUnitsInRadius(12 * 32, BWAPI::Filter::IsOwned);
			BWAPI::Unitset nearDragoons;
			for (auto unit : nearUnits)
			{
				if ((unit->isCompleted() && unit->exists()) &&
					(unit->isStartingAttack() ||
					unit->isAttackFrame() ||
					unit->isAttacking()) &&
					(unit->getType() == BWAPI::UnitTypes::Protoss_Dragoon))
				{
					nearDragoons.insert(unit);
				}
			}
			
			BWAPI::Unitset nearEnemys = carrier->getUnitsInRadius(8 * 32, BWAPI::Filter::IsEnemy);
			// if we have not enough dragoons near
			BWAPI::Unitset nearGoliaths;
			BWAPI::Unitset nearSCV;
			if (nearDragoons.size() <= 2)
			{
				for (auto unit : nearEnemys)
				{
					if ((unit->isVisible() && unit->exists()) &&
						(unit->isStartingAttack() ||
						unit->isAttackFrame() ||
						unit->isAttacking()) && 
						(unit->getType() == BWAPI::UnitTypes::Terran_Goliath ||
						unit->getType() == BWAPI::UnitTypes::Terran_Missile_Turret))
					{
						nearGoliaths.insert(unit);
					}
					/*
					if ((unit->isVisible() && unit->exists()) &&
						((unit->getOrder() == BWAPI::Orders::Repair &&  (unit->getOrderTarget()->getType() == BWAPI::UnitTypes::Terran_Goliath ||
						unit->getOrderTarget()->getType() == BWAPI::UnitTypes::Terran_Siege_Tank_Siege_Mode ||
						unit->getOrderTarget()->getType() == BWAPI::UnitTypes::Terran_Missile_Turret))
						|| 
						(unit->getOrder() == BWAPI::Orders::ConstructingBuilding && unit->getOrderTarget()->getType() == BWAPI::UnitTypes::Terran_Missile_Turret)) &&
						(unit->getType() == BWAPI::UnitTypes::Terran_SCV))
					{
						nearSCV.insert(unit);
					}
					*/
					if ((unit->isVisible() && unit->exists()) &&
						(unit->getOrder() == BWAPI::Orders::Repair ||
						unit->getOrder() == BWAPI::Orders::ConstructingBuilding )&&
						(unit->getType() == BWAPI::UnitTypes::Terran_SCV))
					{
						nearSCV.insert(unit);
					}
				}
			}

			BWAPI::Unit closestGoliath = nullptr;
			{
				int closestDist = 16 * 32;
				for (const auto unit : nearGoliaths)
				{
					// if we haven't already assigned an detectorUnit to this cloaked unit
					int dist = unit->getDistance(carriers.getPosition());
					if (dist < closestDist)
					{
						closestGoliath = unit;
						closestDist = dist;
					}
				}
			}
			BWAPI::Unit closestSCV = nullptr;
			{				
				int closestDist = 16 * 32;
				for (const auto unit : nearSCV)
				{
					// if we haven't already assigned an detectorUnit to this cloaked unit
					int dist = unit->getDistance(carriers.getPosition());
					if (dist < closestDist)
					{
						closestSCV = unit;
						closestDist = dist;
					}
				}
			}
			if (closestSCV)
				cloestThreat = closestSCV;
			else if (closestGoliath)
				cloestThreat = closestGoliath;
		}

		// call our army to protect us
		if (carrier->isAttacking() || carrier->isAttackFrame() || carrier->isUnderAttack() || carrier->isStartingAttack())
		{
			// call dragoon to protect
			BWAPI::Unitset dragoonsNear = carrier->getUnitsInRadius(20 * 32, !BWAPI::Filter::IsBuilding && BWAPI::Filter::IsOwned);
			BWAPI::Unitset goliathNear = carrier->getUnitsInRadius(8 * 32, BWAPI::Filter::IsEnemy);

			std::vector<BWAPI::Unit> goliaths;
			for (const auto unit : goliathNear)
			{
				if ((unit->isCompleted() && unit->exists()) &&
					unit->isVisible() && unit->isAttackFrame() &&
					(unit->getType() == BWAPI::UnitTypes::Terran_Goliath ||
					unit->getType() == BWAPI::UnitTypes::Terran_Missile_Turret))
				{
					goliaths.push_back(unit);
				}
			}

			if (goliaths.size() > 0)
			{
				int n = -1;
				for (const auto unit : dragoonsNear)
				{
					n++;
					if ((unit->isCompleted() && unit->exists()) &&
						!unit->isUnderAttack() && !unit->isAttackFrame() &&
						(unit->getType() == BWAPI::UnitTypes::Protoss_Dragoon ||
						unit->getType() == BWAPI::UnitTypes::Protoss_Zealot))
					{
						Micro::AttackUnit(unit, goliaths[n % goliaths.size()]);
					}
				}
			}
		}


		if (order.isCombatOrder())
        {
            // If the carrier has recently picked a target that is still valid, don't do anything
            // If we change targets too quickly, our interceptors don't have time to react and we don't attack anything
            if (carrier->getLastCommand().getType() == BWAPI::UnitCommandTypes::Attack_Unit &&
                (BWAPI::Broodwar->getFrameCount() - carrier->getLastCommandFrame()) < 48)
            {
                BWAPI::Unit currentTarget = carrier->getLastCommand().getTarget();
                if (currentTarget && currentTarget->exists() &&
                    currentTarget->isVisible() && currentTarget->getHitPoints() > 0 &&
                    carrier->getDistance(currentTarget) <= (8 * 32))
                {
                    continue;
                }
            }

			// If a target is found,
			BWAPI::Unit target = nullptr;
			if (1)
			{
				if (cloestThreat)
				{
					BWAPI::Broodwar->printf(" attack threat!");
					//Log().Get() << "attack threat " << cloestThreat->getType();
				}
					
			}

			if (cloestThreat)
				target = cloestThreat;
			else if (closestSigedTank)
				target = closestSigedTank;
			else
				target = getTarget(carrier, carrierTargets);

			if (target)
			{	
				//Log().Get() << "target is  " << target->getType() << " " << target->getHitPoints();

				if (order.isPressingOrder())
				{
					if (target->getDistance(order.getPosition()) > order.getRadius())
					{
						if (BWAPI::Broodwar->getFrameCount() > Config::Macro::CounterTerranPushTime * 60 * 24)
						{
							bool moveto = true;
							if ("4e24f217d2fe4dbfa6799bc57f74d8dc939d425b" == BWAPI::Broodwar->mapHash())
							{
								BWTA::BaseLocation* third = InformationManager::Instance().getEnemyThirdLocation();
								if (third)
								{
									if (order.getPosition() == third->getPosition())
										moveto = false;
								}
							}
							if (moveto)
							{
								Micro::AttackMove(carrier, order.getPosition());
								continue;
							}
							else
							{
								BWTA::BaseLocation* naturl = InformationManager::Instance().getMyNaturalLocation();
								if (naturl)
									InformationManager::Instance().getLocutusUnit(carrier).moveTo(naturl->getPosition());
							}
						}
						else
						{
							BWTA::BaseLocation* enemy = InformationManager::Instance().getEnemyMainBaseLocation();
							BWTA::BaseLocation* my = InformationManager::Instance().getMyMainBaseLocation();
							if (enemy && my && target->getDistance(enemy->getPosition()) > target->getDistance(my->getPosition()))
							{

							}
							else
							{
								InformationManager::Instance().getLocutusUnit(carrier).moveTo(order.getPosition(), order.getType() == SquadOrderTypes::Pressing);
								continue;
							}
						}
					}
				}
				

				if (Config::Debug::DrawUnitTargetInfo)
					BWAPI::Broodwar->drawLineMap(carrier->getPosition(), carrier->getTargetPosition(), BWAPI::Colors::Purple);

				// attack it.
                Micro::AttackUnit(carrier, target);
			}
			else
			{
				//Log().Get() << "No target" ;

                // No target found. If we're not near the order position, go there.
				if (carrier->getDistance(order.getPosition()) > 32 * 4)
				{
					bool moveto = true;
					if ("4e24f217d2fe4dbfa6799bc57f74d8dc939d425b" == BWAPI::Broodwar->mapHash())
					{
						BWTA::BaseLocation* third = InformationManager::Instance().getEnemyThirdLocation();
						if (third)
						{
							if (order.getPosition() == third->getPosition())
								moveto = false;
						}
					}
					if (moveto)
					{
						//InformationManager::Instance().getLocutusUnit(carrier).moveTo(order.getPosition());
						
						if (unitClosestToEnemy && unitClosestToEnemy->getPosition().isValid())
						{
							Micro::Move(carrier, unitClosestToEnemy->getPosition());
							//Micro::RightClick(carrier, unitClosestToEnemy);
						}
						else
							InformationManager::Instance().getLocutusUnit(carrier).moveTo(order.getPosition());
					}
					else
					{
						BWTA::BaseLocation* naturl = InformationManager::Instance().getMyNaturalLocation();
						if (naturl)
							InformationManager::Instance().getLocutusUnit(carrier).moveTo(naturl->getPosition());
					}
				}                
			}
		}
	}
}

// Should the unit stay (or return) home until ready to move out?
bool MicroCarriers::stayHomeUntilReady(const BWAPI::Unit u) const
{
	return
		u->getType() == BWAPI::UnitTypes::Protoss_Carrier && u->getInterceptorCount() < 4;
}


// Should the unit return home if it has not enough health and shield
bool MicroCarriers::notSafeHealthAndShield(const BWAPI::Unit unit) const
{
	return
		double(unit->getHitPoints()) / double(unit->getType().maxHitPoints()) < 0.75 &&
		double(unit->getShields()) / double(unit->getType().maxShields()) < 0.5;
}

