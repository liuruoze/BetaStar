#include "MicroHighTemplar.h"
#include "UnitUtil.h"
#include "CombatCommander.h"

using namespace UAlbertaBot;

MicroHighTemplar::MicroHighTemplar()
{
}

void MicroHighTemplar::update()
{
	// if one unit and other unit has low health and low mana
	if (getUnits().size() >= 2)
	{
		judgeWhetherMerge();
	}
}


void MicroHighTemplar::judgeWhetherMerge()
{
	BWAPI::Unitset		needMergeGroup;
	for (const auto unit : getUnits())
	{
		/*
		if (double(unit->getHitPoints()) / double(unit->getType().maxHitPoints()) < 0.50 &&
			double(unit->getShields()) / double(unit->getType().maxShields()) < 0.25)
		{
			needMergeGroup.insert(unit);
		}
		*/

		if (double(unit->getEnergy()) / double(unit->getType().maxEnergy()) < 0.25)
		{
			needMergeGroup.insert(unit);
		}
	}

	UAB_ASSERT(needMergeGroup.size() < 2, "needMergeGroup.size() >= 2");

	// if one unit and other unit has low health and low mana
	if (needMergeGroup.size() < 2)
	{
		return;
	}

	// Takes 2 high templar to merge one archon.
	BWAPI::Unitset mergeGroup;
	for (const auto templar : needMergeGroup)
	{
		int framesSinceCommand = BWAPI::Broodwar->getFrameCount() - templar->getLastCommandFrame();

		if (templar->getLastCommand().getType() == BWAPI::UnitCommandTypes::Use_Tech_Unit && framesSinceCommand < 10)
		{
			// Wait. There's latency before the command takes effect.
		}
		else if (templar->getOrder() == BWAPI::Orders::ArchonWarp && framesSinceCommand > 5 * 24)
		{
			// The merge has been going on too long. It may be stuck. Stop and try again.
			// Micro::Move(templar, gatherPoint);
		}
		else if (templar->getOrder() == BWAPI::Orders::PlayerGuard)
		{
			if (templar->getLastCommand().getType() == BWAPI::UnitCommandTypes::Use_Tech_Unit && framesSinceCommand > 10)
			{
				// Tried and failed to merge. Try moving first.
				// Micro::Move(templar, gatherPoint);
			}
			else
			{
				mergeGroup.insert(templar);
			}
		}
	}

	// We will merge 1 pair per call, the pair closest together.
	int closestDist = 9999;
	BWAPI::Unit closest1 = nullptr;
	BWAPI::Unit closest2 = nullptr;

	for (const auto ht1 : mergeGroup)
	{
		for (const auto ht2 : mergeGroup)
		{
			if (ht2 == ht1)    // loop through all ht2 until we reach ht1
			{
				break;
			}
			int dist = ht1->getDistance(ht2);
			if (dist < closestDist)
			{
				closestDist = dist;
				closest1 = ht1;
				closest2 = ht2;
			}
		}
	}

	if (closest1)
	{
		(void)Micro::MergeArchon(closest1, closest2);
	}
}

void MicroHighTemplar::attackMineingWorkers(BWAPI::Unit caster)
{
	//int radius = BWAPI::WeaponTypes::Psionic_Storm.maxRange();
	//Log().Get() << "BWAPI::WeaponTypes::Psionic_Storm.maxRange: " << radius;

	//BWAPI::Unitset workersNear = caster->getUnitsInWeaponRange(BWAPI::WeaponTypes::Psionic_Storm.maxRange(), BWAPI::Filter::IsWorker && BWAPI::Filter::IsCarryingMinerals && BWAPI::Filter::IsCompleted && BWAPI::Filter::IsEnemy);

	BWAPI::Unitset workersNear = caster->getUnitsInRadius(9 * 32, BWAPI::Filter::IsWorker && BWAPI::Filter::IsOwned);

	if (!workersNear.empty())
	{
		for (auto worker : workersNear)
		{
			if (Config::Debug::DrawUnitTargetInfo)
			{
				BWAPI::Broodwar->drawCircleMap(worker->getPosition(), 3, BWAPI::Colors::Blue, false);
			}
		}

		BWAPI::Position targetPosition = workersNear.getPosition();
		if (targetPosition && targetPosition.isValid())
		{
			Log().Get() << "attackMineingWorkers::Find worker, begin to attack!";
			//Micro::CastSpellonPosition(caster, targetPosition, BWAPI::TechTypes::Psionic_Storm);
		}
		else
		{
			Log().Get() << "attackMineingWorkers::TargetPosition is not valid!";
		}
	}
	else
	{
		Log().Get() << "attackMineingWorkers::No workers found!";
	}
}


void MicroHighTemplar::attackNearestEnemy(BWAPI::Unit caster, int id)
{
	const int HT_SEARCH_RADIUS = 12;
	BWAPI::Unitset enemysNear = caster->getUnitsInRadius(HT_SEARCH_RADIUS * 32, !BWAPI::Filter::IsBuilding && BWAPI::Filter::IsEnemy);

	if (!enemysNear.empty())
	{
		BWAPI::Unit target = nullptr;
		int maxScore = 0;
		for (auto unit : enemysNear)
		{
			if (Config::Debug::DrawUnitTargetInfo)
			{
				BWAPI::Broodwar->drawCircleMap(unit->getPosition(), 5, BWAPI::Colors::Blue, false);
			}
			if (unit->isVisible())
			{
				int score = calculateScore(unit);

				if (score > maxScore)
				{
					maxScore = score;
					target = unit;
				}
			}
		}

		if (target && target->getPosition().isValid())
		{
			if (Config::Debug::DrawUnitTargetInfo)
			{
				BWAPI::Broodwar->drawBoxMap(BWAPI::Position(-48, -48) + target->getPosition(),
					BWAPI::Position(48, 48) + target->getPosition(), BWAPI::Colors::Red, false);
			}
			Log().Get() << "attackNearestEnemy::Find enemy, begin to attack!";
			Micro::CastSpellonPosition(caster, target->getPosition(), BWAPI::TechTypes::Psionic_Storm);
		}
		else
		{
			Log().Get() << "attackNearestEnemy::No target or targetPosition is not valid!";
		}
	}
	else
	{
		std::vector<BWAPI::Unit> AttackingDragoons;
		Squad & pressingSquad = CombatCommander::Instance().getSquadData().getSquad("Pressing");
		if (!pressingSquad.isEmpty())
		{
			for (const auto unit : pressingSquad.getUnits())
			{
				if (unit->exists() &&
					unit->getType() == BWAPI::UnitTypes::Protoss_Dragoon &&
					unit->getOrder() == BWAPI::Orders::AttackUnit)
				{
					AttackingDragoons.push_back(unit);
				}
			}
		}
		if (AttackingDragoons.size() >= 2)
		{
			//BWAPI::Broodwar->printf("Full and dragoons");
			Micro::RightClick(caster, AttackingDragoons[id % AttackingDragoons.size()]);
		}
		else
		{
			//BWAPI::Broodwar->printf("Move to third base");
			Micro::Move(caster, order.getPosition());
		}

	}
}

int MicroHighTemplar::calculateScore(const BWAPI::Unit & target)
{
	BWAPI::Unitset unitsNear = target->getUnitsInRadius(40, !BWAPI::Filter::IsBuilding);
	float score = 0;
	for (auto unit : unitsNear)
	{
		if (unit->getPlayer() == BWAPI::Broodwar->enemy())
		{
			if (!unit->isUnderStorm())
			{
				if (unit->isVisible() && unit->exists() &&
					unit->getOrder() == BWAPI::Orders::Sieging ||
					unit->getOrder() == BWAPI::Orders::Unsieging ||
					unit->getType() == BWAPI::UnitTypes::Terran_Siege_Tank_Siege_Mode)
				{
					if (unit->getHitPoints() <= 105)
					{
						score += 10;
					}
					else
					{
						score += 5;
					}
				}
				else if (unit->isVisible() && unit->exists() &&
					unit->getType() == BWAPI::UnitTypes::Terran_Siege_Tank_Tank_Mode)
				{
					if (unit->getHitPoints() <= 105)
					{
						score += 7;
					}
					else
					{
						score += 3;
					}
				}
				else if (unit->isVisible() && unit->exists() &&
					unit->getType() == BWAPI::UnitTypes::Terran_Goliath)
				{
					score += 1;
				}
				else if (unit->isVisible() && unit->exists() &&
					unit->getType() == BWAPI::UnitTypes::Terran_SCV)
				{
					score += 0.33;
				}
				else if (unit->isVisible() && unit->exists() &&
					unit->getType() == BWAPI::UnitTypes::Terran_Vulture)
				{
					score += 0.25;
				}
				else if (unit->isVisible() && unit->exists() &&
					unit->getType() == BWAPI::UnitTypes::Terran_Medic)
				{
					score += 0.4;
				}
				else if (unit->isVisible() && unit->exists() &&
					unit->getType() == BWAPI::UnitTypes::Terran_Marine)
				{
					score += 0.2;
				}
				else if (unit->isVisible() && unit->exists())
				{
					score += 0.35;
				}
			}
		}
		if (unit->getPlayer() == BWAPI::Broodwar->self())
		{
			if (unit->getType() == BWAPI::UnitTypes::Protoss_Shuttle)
			{
				score -= 10;
			}
			else
			{
				score -= 2;
			}
		}
	}
	return int(score);
}




void MicroHighTemplar::executeMicro(const BWAPI::Unitset & targets)
{
	if (!order.isCombatOrder()) return;

	const BWAPI::Unitset & templarUnits = getUnits();
	int id = 0;
	for (const auto templar : templarUnits)
	{
		if (unstickStuckUnit(templar))
			continue;

		// we don't go too far into the enemy base if we are in pressing order
		if (order.isPressingOrder())
		{
			if (templar->getDistance(order.getPosition()) > order.getRadius() &&
				templar->getDistance(InformationManager::Instance().getEnemyMainBaseLocation()->getPosition()) <
				templar->getDistance(InformationManager::Instance().getMyMainBaseLocation()->getPosition())
				)
			{
				InformationManager::Instance().getLocutusUnit(templar).moveTo(order.getPosition(), order.getType() == SquadOrderTypes::Pressing);
				continue;
			}
		}


		if (Config::Debug::DrawUnitTargetInfo)
		{
			BWAPI::Broodwar->drawCircleMap(templar->getPosition(), 9 * 32, BWAPI::Colors::Red, false);
			//BWAPI::Broodwar->drawBoxMouse(BWAPI::Position(-48, -48), BWAPI::Position(48, 48), BWAPI::Colors::Red, false);
		}

		if (templar->exists() && templar->isCompleted() && templar->getEnergy() >= 100)
		{
			attackNearestEnemy(templar, id);
		}
		id++;
	}
}


// Choose a target from the set, or null if we don't want to attack anything
BWAPI::Unit MicroHighTemplar::getTarget(BWAPI::Unit templar, const BWAPI::Unitset & targets, std::vector<std::pair<BWAPI::Position, BWAPI::UnitType>> & enemyDefenses)
{
	int bestScore = -999999;
	BWAPI::Unit bestTarget = nullptr;
	bool willInDefenseRange = false;

	for (const auto target : targets)
	{
		const int priority = getAttackPriority(templar, target);		// 0..12
		const int range = templar->getDistance(target);				// 0..map size in pixels
		int closerToGoal =										// positive if target is closer than us to the goal
			templar->getDistance(order.getPosition()) - target->getDistance(order.getPosition());

		if ((order.getType() != SquadOrderTypes::Sneak)
			|| (!InformationManager::Instance().isEnemyMainBaseEliminated()
			&& !InformationManager::Instance().isSneakTooLate()))
		{
			// Skip targets that are too far away to worry about.
			if (range >= 13 * 32)
			{
				continue;
			}
		}
		else
		{
			closerToGoal = 0;
		}

		if (order.getType() == SquadOrderTypes::Sneak)
		{
			for (auto const & defense : enemyDefenses)
			{
				int threshold = defense.second.isBuilding() ? 7 * 32 : 9 * 32;
				int detectorDistance = MathUtil::DistanceFromPointToLine(templar->getPosition(), target->getPosition(), defense.first);
				if (detectorDistance < threshold)
				{
					willInDefenseRange = true;
					break;
				}
			}
			if (willInDefenseRange) 
				continue;
		}

		// Let's say that 1 priority step is worth 64 pixels (2 tiles).
		// We care about unit-target range and target-order position distance.
		int score = 2 * 32 * priority - range;

		// Adjust for special features.

		// Prefer targets under dark swarm, on the expectation that then we'll be under it too.
		if (target->isUnderDarkSwarm())
		{
			score += 4 * 32;
		}

		// A bonus for attacking enemies that are "in front".
		// It helps reduce distractions from moving toward the goal, the order position.
		if (closerToGoal > 0)
		{
			score += 2 * 32;
		}

		// This could adjust for relative speed and direction, so that we don't chase what we can't catch.
		if (templar->isInWeaponRange(target))
		{
			if (target->getType() == BWAPI::UnitTypes::Zerg_Ultralisk)
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
				score += 48;
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


		if (score > bestScore)
		{
			bestScore = score;
			bestTarget = target;
		}
	}

	return shouldIgnoreTarget(templar, bestTarget) ? nullptr : bestTarget;
}


// get the attack priority of a type
int MicroHighTemplar::getAttackPriority(BWAPI::Unit attacker, BWAPI::Unit target) const
{
	BWAPI::UnitType targetType = target->getType();

	if (targetType == BWAPI::UnitTypes::Terran_Siege_Tank_Siege_Mode)
	{
		return 10;
	}
	if (targetType == BWAPI::UnitTypes::Terran_Siege_Tank_Tank_Mode)
	{
		return 8;
	}

	if (targetType.isWorker())
	{
		return 6;
	}

	return 5;
}