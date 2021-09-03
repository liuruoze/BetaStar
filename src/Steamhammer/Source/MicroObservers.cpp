#include "MicroObservers.h"
#include "StrategyManager.h"
#include <random>

using namespace UAlbertaBot;

MicroObservers::MicroObservers()
	: unitClosestToEnemy(nullptr)
{
}

void MicroObservers::executeMicro(const BWAPI::Unitset & targets)
{
	const BWAPI::Unitset & detectorUnits = getUnits();

	if (detectorUnits.empty())
	{
		return;
	}

	cloakedUnitMap.clear();
	BWAPI::Unitset cloakedUnits;
	BWAPI::Unitset vultureUnits;
	tankUnitMap.clear();
	BWAPI::Unitset tankUnits;
	BWAPI::Unitset missileUnits;
	for (const auto unit : BWAPI::Broodwar->enemy()->getUnits())
	{
		if (unit->getType().hasPermanentCloak() ||     // dark templar, observer
			unit->getType().isCloakable() ||           // wraith, ghost
			unit->getType() == BWAPI::UnitTypes::Terran_Vulture_Spider_Mine ||
			unit->getType() == BWAPI::UnitTypes::Zerg_Lurker ||
			unit->isBurrowed() ||
			(unit->isVisible() && !unit->isDetected()))
		{
			cloakedUnits.insert(unit);
			cloakedUnitMap[unit] = false;
		}
		else if (unit->getType() == BWAPI::UnitTypes::Terran_Vulture && unit->exists())
		{
			vultureUnits.insert(unit);
		}
		else if (unit->getType() == BWAPI::UnitTypes::Terran_Siege_Tank_Siege_Mode ||
			unit->getType() == BWAPI::UnitTypes::Terran_Siege_Tank_Tank_Mode)
		{
			tankUnits.insert(unit);
			tankUnitMap[unit] = false;
		}
		else if (unit->getType() == BWAPI::UnitTypes::Terran_Missile_Turret && unit->exists() && unit->isVisible())
		{
			missileUnits.insert(unit);
		}
	}

	int i = -1;
	for (const auto detectorUnit : detectorUnits)
	{
		i++;
		if (!detectorUnit->exists())
		{
			continue;
		}
		if (detectorUnit->isDetected() && detectorUnit->isUnderAttack())
		{
			Micro::Move(detectorUnit, InformationManager::Instance().getMapMiddlePosition());
			continue;
		}
		if (missileUnits.size() > 0)
		{
			bool canBeAttacked = false;
			for (auto missle : missileUnits)
			{
				if (missle->isInWeaponRange(detectorUnit))
				{
					canBeAttacked = true;
					break;
				}
				if (detectorUnit->getDistance(missle) <= (7 + 1) * 32)
				{
					canBeAttacked = true;
					break;
				}
			}
			if (canBeAttacked)
			{
				Micro::Move(detectorUnit, InformationManager::Instance().getMapMiddlePosition());
				continue;
			}
		}

		// Move the detector toward the squadmate closest to the enemy.
		if (i == 4)
		{
			if (Config::Strategy::StrategyName != "anti-SAIDA-Carrier")
			{
				detectEnemyBases(detectorUnit, 1, i);
			}
			else
			{
				BWAPI::Unit vulture = closestVultureUnit(vultureUnits, detectorUnit);
				if (vulture)
				{
					Micro::Move(detectorUnit, vulture->getPosition());
					//BWAPI::Broodwar->printf("Found VultureUnit");

					if (Config::Debug::DrawUnitTargetInfo)
					{
						BWAPI::Broodwar->drawCircleMap(detectorUnit->getPosition(), 5, BWAPI::Colors::Red, true);
						BWAPI::Broodwar->drawCircleMap(vulture->getPosition(), 5, BWAPI::Colors::Red, true);
						BWAPI::Broodwar->drawLineMap(detectorUnit->getPosition(), vulture->getPosition(), BWAPI::Colors::Red);
					}
				}
				else
				{
					detectEnemyBases(detectorUnit, 0, i);
				}
			}	
		}
		// chaged spider
		else if (i == 2)
		{
			if (Config::Strategy::StrategyName != "anti-SAIDA-Carrier")
			{
				detectEnemyBases(detectorUnit, 1, i);
			}
			else
			{
				BWAPI::Unit close_spider = closestCloakedUnit(cloakedUnits, detectorUnit);
				if (close_spider)
				{
					Micro::Move(detectorUnit, close_spider->getPosition());
					//BWAPI::Broodwar->printf("Found close_spider");

					if (Config::Debug::DrawUnitTargetInfo)
					{
						BWAPI::Broodwar->drawCircleMap(detectorUnit->getPosition(), 5, BWAPI::Colors::Red, true);
						BWAPI::Broodwar->drawCircleMap(close_spider->getPosition(), 5, BWAPI::Colors::Red, true);
						BWAPI::Broodwar->drawLineMap(detectorUnit->getPosition(), close_spider->getPosition(), BWAPI::Colors::Red);
					}
				}
				else if (unitClosestToEnemy && unitClosestToEnemy->getPosition().isValid())
				{
					Micro::Move(detectorUnit, unitClosestToEnemy->getPosition());
				}
			}
			
		}
		else if (i == 0)
		{
			if (Config::Strategy::StrategyName != "anti-SAIDA-Carrier")
			{
				detectEnemyBases(detectorUnit, 1, i);
			}
			else if (unitClosestToEnemy && unitClosestToEnemy->getPosition().isValid())
				Micro::Move(detectorUnit, unitClosestToEnemy->getPosition());
		}
		else if (i == 3)
		{
			if (Config::Strategy::StrategyName != "anti-SAIDA-Carrier")
			{
				detectEnemyBases(detectorUnit, 0, i);
			}
			else
			{
				BWAPI::Unit close_tank = closestSigedTankUnit(tankUnits, detectorUnit);
				if (close_tank)
				{
					Micro::Move(detectorUnit, close_tank->getPosition());
				}
				else
				{
					detectEnemyBases(detectorUnit, 1, i);
				}
			}

		}
		else if (i == 1)
		{
			//BWAPI::Position explorePosition = MapGrid::Instance().getLeastExplored(true);
			detectEnemyBases(detectorUnit, 0, i);
		}
	}
}


void MicroObservers::detectEnemyBases(const BWAPI::Unit & detectorUnit, int direction, unsigned seed)
{
	std::vector<BWTA::BaseLocation *> result = InformationManager::Instance().getUnknownBases();

	// if there are no bases needed to detect, follow our army
	if (result.size() == 0)
	{
		if (unitClosestToEnemy && unitClosestToEnemy->getPosition().isValid())
			Micro::Move(detectorUnit, unitClosestToEnemy->getPosition());
		
		return;
	}

	// first go to middle of map, not apply in Destination and Andromeda
	if ("4e24f217d2fe4dbfa6799bc57f74d8dc939d425b" != BWAPI::Broodwar->mapHash() && "1e983eb6bcfa02ef7d75bd572cb59ad3aab49285" != BWAPI::Broodwar->mapHash())
	{
		BWAPI::Position middle = InformationManager::Instance().getMapMiddlePosition();
		if (middle.isValid())
			detectorUnit->move(middle, true);
	}

	
	BWTA::BaseLocation * enemyThird = InformationManager::Instance().getEnemyThirdLocation();
	if (enemyThird && enemyThird->getPosition().isValid())
		detectorUnit->move(enemyThird->getPosition(), true);
	
	/*
	Log().Get() << "Observer ID: " << seed;
	
	Log().Get() << "Before shuffle ";
	for (size_t j = 0; j < result.size(); j++)
	{
		Log().Get() << result[j]->getPosition();
	}
	*/

	std::shuffle(result.begin(), result.end(), std::default_random_engine(seed));
	
	//Log().Get() << "After shuffle ";
	
	for (size_t j = 0; j < result.size(); j++)
	{
		//Log().Get() << result[j]->getPosition();
		BWTA::BaseLocation * enemyNatural = InformationManager::Instance().getEnemyNaturalLocation();
		if (enemyNatural && result[j] == enemyNatural)
			continue;
		detectorUnit->move(result[j]->getPosition(), true);
	}


	/*
	if (direction == 0)
	{
		for (size_t j = 0; j < result.size(); j++)
		{
			BWTA::BaseLocation * enemyNatural = InformationManager::Instance().getEnemyNaturalLocation();
			if (enemyNatural && result[j] == enemyNatural)
				continue;
			detectorUnit->move(result[j]->getPosition(), true);
		}
	}
	else
	{
		for (size_t j = result.size()-1; j >= 0; j--)
		{
			BWTA::BaseLocation * enemyNatural = InformationManager::Instance().getEnemyNaturalLocation();
			if (enemyNatural && result[j] == enemyNatural)
				continue;
			detectorUnit->move(result[j]->getPosition(), true);
		}
	}
	*/


	return;
}

// NOTE Unused but potentially useful.
BWAPI::Unit MicroObservers::closestCloakedUnit(const BWAPI::Unitset & cloakedUnits, BWAPI::Unit detectorUnit)
{
	BWAPI::Unit closestCloaked = nullptr;
	double closestDist = 100000;

	for (const auto unit : cloakedUnits)
	{
		// if we haven't already assigned an detectorUnit to this cloaked unit
		if (!cloakedUnitMap[unit])
		{
			int dist = unit->getDistance(detectorUnit);

			if (dist < closestDist)
			{
				closestCloaked = unit;
				closestDist = dist;
			}
		}
	}

	return closestCloaked;
}


// NOTE Unused but potentially useful.
BWAPI::Unit MicroObservers::closestVultureUnit(const BWAPI::Unitset & vultureUnits, BWAPI::Unit detectorUnit)
{
	BWAPI::Unit closestVulture = nullptr;
	double closestDist = 100000;

	for (const auto unit : vultureUnits)
	{
		// if we haven't already assigned an detectorUnit to this cloaked unit
		if (true)
		{
			int dist = unit->getDistance(detectorUnit);

			if (dist < closestDist)
			{
				closestVulture = unit;
				closestDist = dist;
			}
		}
	}

	return closestVulture;
}

// for example, tansk.
BWAPI::Unit MicroObservers::closestSigedTankUnit(const BWAPI::Unitset & tankUnits, BWAPI::Unit detectorUnit)
{
	BWAPI::Unit closestSigedTank = nullptr;
	double closestDist = 100000;

	for (const auto unit : tankUnits)
	{
		// if we haven't already assigned an detectorUnit to this cloaked unit
		if (!tankUnitMap[unit])
		{
			int dist = unit->getDistance(detectorUnit);

			if (dist < closestDist)
			{
				closestSigedTank = unit;
				closestDist = dist;
			}
		}
	}

	return closestSigedTank;
}