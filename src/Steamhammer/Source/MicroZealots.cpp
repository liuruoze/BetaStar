#include "MicroZealots.h"
#include "UnitUtil.h"
#include "MathUtil.h"
#include "BuildingPlacer.h"
#include "StrategyManager.h"
#include "CombatCommander.h"

namespace { auto & bwemMap = BWEM::Map::Instance(); }

using namespace UAlbertaBot;

// Note: Melee units are ground units only. Scourge is a "ranged" unit.

MicroZealots::MicroZealots() 
{ 
}

void MicroZealots::getTargets(BWAPI::Unitset & targets) const
{
	if (order.getType() != SquadOrderTypes::HoldWall)
	{
		MicroManager::getTargets(targets);
		return;
	}

	LocutusWall& wall = BuildingPlacer::Instance().getWall();

	for (const auto unit : BWAPI::Broodwar->enemy()->getUnits())
	{
		if (unit->exists() &&
			(unit->isCompleted() || unit->getType().isBuilding()) &&
			unit->getHitPoints() > 0 &&
			unit->getType() != BWAPI::UnitTypes::Unknown
			&& (wall.tilesInsideWall.find(unit->getTilePosition()) != wall.tilesInsideWall.end() ||
				wall.tilesOutsideButCloseToWall.find(unit->getTilePosition()) != wall.tilesOutsideButCloseToWall.end()))
		{
			targets.insert(unit);
		}
	}
}

void MicroZealots::executeMicro(const BWAPI::Unitset & targets) 
{
	assignTargets(targets);
}

void MicroZealots::assignTargets(const BWAPI::Unitset & targets)
{
    const BWAPI::Unitset & meleeUnits = getUnits();
    Squad & squad = CombatCommander::Instance().getSquadData().getSquad(this);

	BWAPI::Unitset meleeUnitTargets;

	//	by wei guo, 20180905
	bool bWeHaveZealotOrDragoon = false;
	for (const auto myUnit : BWAPI::Broodwar->self()->getUnits())
	{
		if (myUnit->isCompleted() && (myUnit->getType() == BWAPI::UnitTypes::Protoss_Zealot
			|| myUnit->getType() == BWAPI::UnitTypes::Protoss_Dragoon))
		{
			bWeHaveZealotOrDragoon = true;
		}
	}

	bool firstWorker = true;
	for (const auto target : targets) 
	{
		if (target->isVisible() &&
			target->isDetected() &&
			!target->isFlying() &&
			target->getPosition().isValid() &&
			target->getType() != BWAPI::UnitTypes::Zerg_Larva && 
			target->getType() != BWAPI::UnitTypes::Zerg_Egg &&
			!target->isStasised() &&
			!target->isUnderDisruptionWeb())             // melee unit can't attack under dweb
		{
			//	by wei guo, 20180905
			if (StrategyManager::Instance().getOpeningGroup() == "cse"
				&& target->getType().isWorker() && firstWorker && !bWeHaveZealotOrDragoon)
			{
				firstWorker = false;
				continue;
			}
			meleeUnitTargets.insert(target);
		}
	}

	
	int siegTankCnt = 0;
	std::vector<BWAPI::Unit> sigedTanks;
	for (const auto unit : BWAPI::Broodwar->enemy()->getUnits())
	{
		if (unit->isVisible() && unit->exists() &&
			(unit->getOrder() == BWAPI::Orders::Sieging ||
			unit->getOrder() == BWAPI::Orders::Unsieging ||
			unit->getType() == BWAPI::UnitTypes::Terran_Siege_Tank_Siege_Mode ||
			unit->getType() == BWAPI::UnitTypes::Terran_Siege_Tank_Tank_Mode))
		{
			sigedTanks.push_back(unit);
			siegTankCnt++;
		}
	}
	

	
	int i = -1;
	for (const auto meleeUnit : meleeUnits)
	{
		i++;
		if (meleeUnit->isBurrowed())
		{
			// For now, it would burrow only if irradiated. Leave it.
			continue;
		}


		// Special case for irradiated zerg units.
		if (meleeUnit->isIrradiated() && meleeUnit->getType().getRace() == BWAPI::Races::Zerg)
		{
			if (meleeUnit->canBurrow())
			{
				meleeUnit->burrow();
				continue;
			}
			// Otherwise ignore it. Ultralisks should probably just keep going.
		}
	
		if (order.isCombatOrder())
        {
			if (unstickStuckUnit(meleeUnit))
				continue;

			// run away if we meet the retreat criterion. meleeUnitShouldRetreat(meleeUnit, targets)
			// never run away
			if (false)
            {		
            }
			else
			{
				
				if (siegTankCnt >= 2)
				{
					if (1)
					{
						BWAPI::Unitset nearUnits = meleeUnit->getUnitsInRadius(1 * 32, BWAPI::Filter::IsEnemy);
						if (nearUnits.size() > 0)
						{
							BWAPI::Unit target = nullptr;
							for (auto unit : nearUnits)
							{
								if ((unit->isVisible() && unit->exists()) &&
									unit->getOrder() == BWAPI::Orders::Sieging ||
									unit->getOrder() == BWAPI::Orders::Unsieging ||
									unit->getType() == BWAPI::UnitTypes::Terran_Siege_Tank_Siege_Mode ||
									unit->getType() == BWAPI::UnitTypes::Terran_Siege_Tank_Tank_Mode ||
									unit->getType() == BWAPI::UnitTypes::Terran_Goliath)
								{
									target = unit;
								}
							}
							if (target)
							{
								Micro::AttackUnit(meleeUnit, target);
								continue;
							}
						}
					}
					

					BWAPI::Unit target = sigedTanks[i % sigedTanks.size()];
					if (target->getDistance(meleeUnit) < 16 * 32)
					{
						if (Config::Debug::DrawUnitTargetInfo)
						{
							BWAPI::Broodwar->drawLineMap(meleeUnit->getPosition(), meleeUnit->getTargetPosition(), BWAPI::Colors::Green);
						}
						Micro::AttackUnit(meleeUnit, target);
						continue;
					}
					else
					{
						BWTA::BaseLocation* natural = InformationManager::Instance().getMyNaturalLocation();
						if (natural && target->getDistance(natural->getPosition()) < 16 * 32)
						{
							InformationManager::Instance().getLocutusUnit(meleeUnit).moveTo(target->getPosition());
							continue;
						}
					}
				}
				

				BWAPI::Unit target = getTarget(meleeUnit, meleeUnitTargets);
				if (target)
				{
					if (order.isPressingOrder())
					{
						if (target->getDistance(order.getPosition()) > order.getRadius())
						{
							if (BWAPI::Broodwar->getFrameCount() > Config::Macro::CounterTerranPushTime * 60 * 24)
							{
								Micro::AttackMove(meleeUnit, order.getPosition());
								continue;
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
									InformationManager::Instance().getLocutusUnit(meleeUnit).moveTo(order.getPosition(), order.getType() == SquadOrderTypes::Pressing);
									continue;
								}
							}
						}
					}


                    // Bunkers are handled by a special micro manager
                    if (target->getType() == BWAPI::UnitTypes::Terran_Bunker &&
                        target->isCompleted())
                    {
                        squad.addUnitToBunkerAttackSquad(target->getPosition(), meleeUnit);
                    }
                    else
                        Micro::AttackUnit(meleeUnit, target);
				}
                // There are no targets. Move to the order position if not already close.
                else if (meleeUnit->getDistance(order.getPosition()) > 96)
				{
                    // If this unit is doing a bunker run-by, get the position it should move towards
                    auto bunkerRunBySquad = squad.getBunkerRunBySquad(meleeUnit);
                    if (bunkerRunBySquad)
                    {
                        InformationManager::Instance().getLocutusUnit(meleeUnit)
                            .moveTo(bunkerRunBySquad->getRunByPosition(meleeUnit, order.getPosition()));
                    }

                    // Otherwise, maybe add it to a bunker attack squad
                    else if (!StrategyManager::Instance().isRushing() ||
                        order.getType() == SquadOrderTypes::KamikazeAttack ||
                        !squad.addUnitToBunkerAttackSquadIfClose(meleeUnit))
                    {
                        // Neither are appropriate, move towards the order position
                        InformationManager::Instance().getLocutusUnit(meleeUnit).moveTo(order.getPosition(), order.getType() == SquadOrderTypes::Attack);
                    }
				}
			}
		}

		if (Config::Debug::DrawUnitTargetInfo)
		{
			BWAPI::Broodwar->drawLineMap(meleeUnit->getPosition(), meleeUnit->getTargetPosition(),
				Config::Debug::ColorLineTarget);
		}
	}
}

// Choose a target from the set. Never return null!
BWAPI::Unit MicroZealots::getTarget(BWAPI::Unit meleeUnit, const BWAPI::Unitset & targets)
{
	int bestScore = -999999;
	BWAPI::Unit bestTarget = nullptr;

    BWAPI::Position myPositionInFiveFrames = InformationManager::Instance().predictUnitPosition(meleeUnit, 5);
    bool inOrderPositionArea = bwemMap.GetArea(meleeUnit->getTilePosition()) == bwemMap.GetArea(BWAPI::TilePosition(order.getPosition()));

	for (const auto target : targets)
	{
		const int priority = getAttackPriority(meleeUnit, target);		// 0..12
		const int range = meleeUnit->getDistance(target);				// 0..map size in pixels
		const int closerToGoal =										// positive if target is closer than us to the goal
			meleeUnit->getDistance(order.getPosition()) - target->getDistance(order.getPosition());

		// Skip targets that are too far away to worry about.
		if (range >= 13 * 32)
		{
			continue;
		}

		// Let's say that 1 priority step is worth 64 pixels (2 tiles).
		// We care about unit-target range and target-order position distance.
		int score = 2 * 32 * priority - range;

        // Kamikaze and rush attacks ignore all tier 2+ combat units
        if ((StrategyManager::Instance().isRushing() || order.getType() == SquadOrderTypes::KamikazeAttack) &&
            UnitUtil::IsCombatUnit(target) && 
            !UnitUtil::IsTierOneCombatUnit(target->getType())
            && !target->getType().isWorker())
        {
            continue;
        }


        // Consider whether to attack enemies that are outside of our weapon range when on the attack
        bool inWeaponRange = meleeUnit->isInWeaponRange(target);
        if (!inWeaponRange && order.getType() != SquadOrderTypes::Defend)
        {
			// Never chase units that can kite us easily
			if (target->getType() == BWAPI::UnitTypes::Protoss_Dragoon ||
				target->getType() == BWAPI::UnitTypes::Terran_Vulture) continue;

            // Check if the target is moving away from us
            BWAPI::Position targetPositionInFiveFrames = InformationManager::Instance().predictUnitPosition(target, 5);
            if (target->isMoving() && 
                range <= MathUtil::EdgeToEdgeDistance(meleeUnit->getType(), myPositionInFiveFrames, target->getType(), targetPositionInFiveFrames))
            {
                // Never chase workers
                if (target->getType().isWorker()) continue;

                // When rushing, don't chase anything when outside the order position area
                if (StrategyManager::Instance().isRushing() && !inOrderPositionArea) continue;
            }

            // Skip targets behind a wall
            if (InformationManager::Instance().isBehindEnemyWall(meleeUnit, target)) continue;
        }

        // When rushing, prioritize workers that are building something
        if (StrategyManager::Instance().isRushing() && target->getType().isWorker() && target->isConstructing())
        {
            score += 4 * 32;
        }

		// Adjust for special features.

		// Prefer targets under dark swarm, on the expectation that then we'll be under it too.
		if (target->isUnderDarkSwarm())
		{
			if (meleeUnit->getType().isWorker())
			{
				// Workers can't hit under dark swarm. Skip this target.
				continue;
			}
			score += 4 * 32;
		}

		// A bonus for attacking enemies that are "in front".
		// It helps reduce distractions from moving toward the goal, the order position.
		if (closerToGoal > 0)
		{
			score += 2 * 32;
		}

		// This could adjust for relative speed and direction, so that we don't chase what we can't catch.
		if (inWeaponRange)
		{
			if (meleeUnit->getType() == BWAPI::UnitTypes::Zerg_Ultralisk)
			{
				score += 12 * 32;   // because they're big and awkward
			}
			else
			{
				score += 4 * 32;
			}
		}
		else if (!target->isMoving())
		{
			if (target->isSieged() ||
				target->getOrder() == BWAPI::Orders::Sieging ||
				target->getOrder() == BWAPI::Orders::Unsieging)
			{
				score += 48; // changed by ruo-ze
			}
			else
			{
				score += 32;
			}
		}
		else if (target->isBraking())
		{
			score += 16;
		}
		else if (target->getType().topSpeed() >= meleeUnit->getType().topSpeed())
		{
			score -= 2 * 32;
		}

		if (target->isUnderStorm())
		{
			score -= 4 * 32;
		}

		// Prefer targets that are already hurt.
		if (target->getType().getRace() == BWAPI::Races::Protoss && target->getShields() <= 5)
		{
			score += 32;
            if (target->getHitPoints() < (target->getType().maxHitPoints() / 3))
            {
                score += 24;
            }
		}
		else if (target->getHitPoints() < target->getType().maxHitPoints())
		{
			score += 24;
            if (target->getHitPoints() < (target->getType().maxHitPoints() / 3))
            {
                score += 24;
            }
        }

        // Avoid defensive matrix
        if (target->isDefenseMatrixed())
        {
            score -= 4 * 32;
        }

		if (score > bestScore)
		{
			bestScore = score;
			bestTarget = target;
		}
	}

	// never attack a spider mine;
	/*if (bestTarget->getType() == BWAPI::UnitTypes::Terran_Vulture_Spider_Mine)
	{
		bestTarget = nullptr;
	}*/

	return shouldIgnoreTarget(meleeUnit, bestTarget) ? nullptr : bestTarget;
}

// get the attack priority of a type
int MicroZealots::getAttackPriority(BWAPI::Unit attacker, BWAPI::Unit target) const
{
	BWAPI::UnitType targetType = target->getType();

	// Short circuit: Enemy unit which is far enough outside its range is lower priority than a worker.
	int enemyRange = UnitUtil::GetAttackRange(target, attacker);
	if (enemyRange &&
		!targetType.isWorker() &&
		attacker->getDistance(target) > 32 + enemyRange)
	{
		return 8;
	}
	// Short circuit: Units before bunkers!
	if (targetType == BWAPI::UnitTypes::Terran_Bunker)
	{
		return 10;
	}
	// Medics and ordinary combat units. Include workers that are doing stuff.
	if (targetType == BWAPI::UnitTypes::Terran_Medic ||
		targetType == BWAPI::UnitTypes::Protoss_High_Templar ||
		targetType == BWAPI::UnitTypes::Protoss_Reaver)
	{
		return 12;
	}
	if (targetType == BWAPI::UnitTypes::Terran_Siege_Tank_Siege_Mode)
	{
		return 13; //changed by ruo-ze
	}
	if (targetType == BWAPI::UnitTypes::Terran_Siege_Tank_Tank_Mode)
	{
		return 11; //changed by ruo-ze
	}
	if (targetType == BWAPI::UnitTypes::Terran_Goliath)
	{
		return 12; //changed by ruo-ze
	}
	if (targetType == BWAPI::UnitTypes::Terran_Vulture_Spider_Mine)
	{
		return 1; //changed by ruo-ze
	}
	if (targetType == BWAPI::UnitTypes::Terran_Vulture)
	{
		return 2; //changed by ruo-ze
	}
	if (targetType.groundWeapon() != BWAPI::WeaponTypes::None && !targetType.isWorker())
	{
		return 11;
	}
	if (targetType.isWorker() && (target->isRepairing() || target->isConstructing() || unitNearNarrowChokepoint(target)))
	{
		return 11;
	}
	// next priority is bored workers and turrets
	if (targetType.isWorker() || targetType == BWAPI::UnitTypes::Terran_Missile_Turret)
	{
		return 9;
	}
    // Buildings come under attack during free time, so they can be split into more levels.
	// Nydus canal is critical.
	if (targetType == BWAPI::UnitTypes::Zerg_Nydus_Canal)
	{
		return 10;
	}
	if (targetType == BWAPI::UnitTypes::Zerg_Spire)
	{
		return 6;
	}
	if (targetType == BWAPI::UnitTypes::Zerg_Spawning_Pool ||
		targetType.isResourceDepot() ||
		targetType == BWAPI::UnitTypes::Protoss_Templar_Archives ||
		targetType.isSpellcaster())
	{
		return 5;
	}
	// Short circuit: Addons other than a completed comsat are worth almost nothing.
	// TODO should also check that it is attached
	if (targetType.isAddon() && !(targetType == BWAPI::UnitTypes::Terran_Comsat_Station && target->isCompleted()))
	{
		return 1;
	}
	// anything with a cost
	if (targetType.gasPrice() > 0 || targetType.mineralPrice() > 0)
	{
		return 3;
	}
	
	// then everything else
	return 1;
}

// Retreat hurt units to allow them to regenerate health (zerg) or shields (protoss).
bool MicroZealots::meleeUnitShouldRetreat(BWAPI::Unit meleeUnit, const BWAPI::Unitset & targets)
{
    // terran don't regen so it doesn't make sense to retreat
    if (meleeUnit->getType().getRace() == BWAPI::Races::Terran)
    {
        return false;
    }

    // Don't retreat while rushing
    if (StrategyManager::Instance().isRushing())
    {
        return false;
    }

    // we don't want to retreat the melee unit if its shields or hit points are above the threshold set in the config file
    // set those values to zero if you never want the unit to retreat from combat individually
    if (meleeUnit->getShields() > Config::Micro::RetreatMeleeUnitShields || meleeUnit->getHitPoints() > Config::Micro::RetreatMeleeUnitHP)
    {
        return false;
    }

    // if there is a ranged enemy unit within attack range of this melee unit then we shouldn't bother retreating since it could fire and kill it anyway
    for (auto & unit : targets)
    {
        int groundWeaponRange = unit->getType().groundWeapon().maxRange();
        if (groundWeaponRange >= 64 && unit->getDistance(meleeUnit) < groundWeaponRange)
        {
            return false;
        }
    }

	// A broodling should not retreat since it is on a timer and regeneration does it no good.
	if (meleeUnit->getType() == BWAPI::UnitTypes::Zerg_Broodling)
	{
		return false;
	}

	return true;
}
