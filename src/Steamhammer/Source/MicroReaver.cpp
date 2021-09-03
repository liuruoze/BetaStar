#include "MicroReaver.h"

// by Yuzhou Wu, 2019-07-15

using namespace UAlbertaBot;

MicroReavers::MicroReavers()
{
}

void MicroReavers::executeMicro(const BWAPI::Unitset & targets)
{
	const BWAPI::Unitset & reavers = getUnits();
	//BWAPI::Broodwar->printf("success");
	// The set of potential targets.
	BWAPI::Unitset reaverTargets;
	std::copy_if(targets.begin(), targets.end(), std::inserter(reaverTargets, reaverTargets.end()),
		[](BWAPI::Unit u) {
		return
			u->isVisible() &&
			u->isDetected() &&
			u->getType() != BWAPI::UnitTypes::Zerg_Larva &&
			u->getType() != BWAPI::UnitTypes::Zerg_Egg &&
			!u->isStasised();
	});

	BWAPI::Unitset dragoonUnits;
	for (const auto unit : BWAPI::Broodwar->enemy()->getUnits())
	{
		if ((unit->isVisible() && unit->exists()) &&
			unit->getType() == BWAPI::UnitTypes::Protoss_Dragoon)
		{
			dragoonUnits.insert(unit);
		}
	}

	std::vector<BWAPI::Unit> AttackingDragoons;
	for (const auto unit : BWAPI::Broodwar->self()->getUnits())
	{
		if (unit->exists() && unit->isCompleted() && unit->isAttackFrame() && 
			unit->getType() == BWAPI::UnitTypes::Protoss_Dragoon)
		{
			AttackingDragoons.push_back(unit);
		}
	}

	int g = -1;
	for (const auto reaver : reavers)
	{
		g++;
		if (buildScarabOrInterceptor(reaver))
		{
			// If we started one, no further action this frame.
			continue;
		}


		if (stayHomeUntilReady(reaver))
		{
			BWAPI::Position fleeTo(InformationManager::Instance().getMyMainBaseLocation()->getPosition());
			//Micro::AttackMove(reaver, fleeTo);
			Micro::Move(reaver, fleeTo);
			continue;
		}

		/*
		if (notSafeHealthAndShield(reaver))
		{
			BWAPI::Position fleeTo(InformationManager::Instance().getMyMainBaseLocation()->getPosition());
			Micro::Move(reaver, fleeTo);
			continue;
		}*/

		// call our army to protect us
		if (reaver->isAttacking() || reaver->isAttackFrame() || reaver->isUnderAttack() || reaver->isStartingAttack())
		{
			// call dragoon to protect
			BWAPI::Unitset dragoonsNear = reaver->getUnitsInRadius(13 * 32, !BWAPI::Filter::IsBuilding && BWAPI::Filter::IsOwned);
			BWAPI::Unitset dragoonsEnemy = reaver->getUnitsInRadius(13 * 32, BWAPI::Filter::IsEnemy);

			std::vector<BWAPI::Unit> dragoons;
			for (const auto unit : dragoonsEnemy)
			{
				if ((unit->isCompleted() && unit->exists()) &&
					unit->isVisible() && unit->isAttackFrame() &&
					(unit->getType() == BWAPI::UnitTypes::Protoss_Dragoon))
				{
					dragoons.push_back(unit);
				}
			}

			if (dragoons.size() > 0)
			{
				int n = -1;
				for (const auto unit : dragoonsNear)
				{
					n++;
					if ((unit->isCompleted() && unit->exists()) &&
						!unit->isUnderAttack() && !unit->isAttackFrame() &&
						(unit->getType() == BWAPI::UnitTypes::Protoss_Dragoon))
					{
						Micro::AttackUnit(unit, dragoons[n % dragoons.size()]);
					}
				}
			}
		}


		if (order.isCombatOrder())
		{
			// If the reaver has recently picked a target that is still valid, don't do anything
			// If we change targets too quickly, our interceptors don't have time to react and we don't attack anything
			if (reaver->getLastCommand().getType() == BWAPI::UnitCommandTypes::Attack_Unit &&
				(BWAPI::Broodwar->getFrameCount() - reaver->getLastCommandFrame()) < 48)
			{
				BWAPI::Unit currentTarget = reaver->getLastCommand().getTarget();
				if (currentTarget && currentTarget->exists() &&
					currentTarget->isVisible() && currentTarget->getHitPoints() > 0 &&
					reaver->getDistance(currentTarget) <= (8 * 32))
				{
					continue;
				}
			}

			// If a target is found,
			BWAPI::Unit target = nullptr;
				target = getTarget(reaver, reaverTargets);

			if (target)
			{
				if (order.isPressingOrder())
				{
					if (target->getDistance(order.getPosition()) > order.getRadius())
					{
						if (BWAPI::Broodwar->getFrameCount() > Config::Macro::CounterTerranPushTime * 60 * 24)
						{
							Micro::AttackMove(reaver, order.getPosition());
							continue;
						}
						else
						{
							InformationManager::Instance().getLocutusUnit(reaver).moveTo(order.getPosition(), order.getType() == SquadOrderTypes::Pressing);
							continue;
						}
					}
				}


				if (Config::Debug::DrawUnitTargetInfo)
				{
					BWAPI::Broodwar->drawLineMap(reaver->getPosition(), reaver->getTargetPosition(), BWAPI::Colors::Purple);
				}

				// attack it.
				Micro::AttackUnit(reaver, target);
			}
			else
			{
				if (AttackingDragoons.size() >= 2 && g >= 0)
				{
					//BWAPI::Broodwar->printf("Full and dragoons");
					Micro::RightClick(reaver, AttackingDragoons[g % AttackingDragoons.size()]);
				}
				else
				{
					//BWAPI::Broodwar->printf("Move to third base");
					Micro::Move(reaver, order.getPosition());
				}
			}
		}
	}
}

// Should the unit stay (or return) home until ready to move out?
bool MicroReavers::stayHomeUntilReady(const BWAPI::Unit u) const
{
	return
		u->getType() == BWAPI::UnitTypes::Protoss_Reaver && u->getScarabCount() < 3;
}


// Should the unit return home if it has not enough health and shield
bool MicroReavers::notSafeHealthAndShield(const BWAPI::Unit unit) const
{
	return
		double(unit->getHitPoints()) / double(unit->getType().maxHitPoints()) < 0.75 &&
		double(unit->getShields()) / double(unit->getType().maxShields()) < 0.5;
}
