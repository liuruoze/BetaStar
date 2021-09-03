#include "MicroDetectors.h"
#include <random>

using namespace UAlbertaBot;

MicroDetectors::MicroDetectors()
	: unitClosestToEnemy(nullptr)
{
}

void MicroDetectors::executeMicro(const BWAPI::Unitset & targets) 
{
	const BWAPI::Unitset & detectorUnits = getUnits();

	if (detectorUnits.empty())
	{
		return;
	}

	// NOTE targets is a list of nearby enemies.
	// Currently unused. Could use it to avoid enemy fire, among other possibilities.
	for (size_t i(0); i<targets.size(); ++i)
	{
		// do something here if there are targets
	}

	cloakedUnitMap.clear();
	BWAPI::Unitset cloakedUnits;

	// Find enemy cloaked units.
	// NOTE This code is unused, but it is potentially useful.
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
	}

	int i = -1;
	for (const auto detectorUnit : detectorUnits)
	{
		i++;
		// Move the detector toward the squadmate closest to the enemy.
		if (i == 0)
		{
			if (unitClosestToEnemy && unitClosestToEnemy->getPosition().isValid())
			{
				Micro::Move(detectorUnit, unitClosestToEnemy->getPosition());
			}
		}
		else if (i == 1)
		{
			BWAPI::Unit close_cloak = closestCloakedUnit(cloakedUnits, detectorUnit);
			if (close_cloak)
				Micro::Move(detectorUnit, close_cloak->getPosition());
			else
				detectEnemyBases(detectorUnit, 1, i);
		}
		else if (i == 2)
		{
			BWAPI::Unit close_cloak = closestCloakedUnit(cloakedUnits, detectorUnit);
			if (close_cloak)
				Micro::Move(detectorUnit, close_cloak->getPosition());
			else
				detectEnemyBases(detectorUnit, 1, i);
		}
		else if (i == 3)
		{
			detectEnemyBases(detectorUnit, 1, i);
		}
		else if (i == 4)
		{
			detectEnemyBases(detectorUnit, 1, i);
		}
		// otherwise there is no unit closest to enemy so we don't want our detectorUnit to die
		// send it to scout around the map
		// no, don't - not so smart for overlords
		// readd by ruo-ze, it is convient for against protoss
		else
		{
			BWAPI::Position explorePosition = MapGrid::Instance().getLeastExplored(true);
			Micro::Move(detectorUnit, explorePosition);
		}
		
	}
}

void MicroDetectors::detectEnemyBases(const BWAPI::Unit & detectorUnit, int direction, unsigned seed)
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

	std::shuffle(result.begin(), result.end(), std::default_random_engine(seed));

	for (size_t j = 0; j < result.size(); j++)
	{
		//Log().Get() << result[j]->getPosition();
		BWTA::BaseLocation * enemyNatural = InformationManager::Instance().getEnemyNaturalLocation();
		if (enemyNatural && result[j] == enemyNatural)
			continue;
		detectorUnit->move(result[j]->getPosition(), true);
	}

	return;
}

// NOTE Unused but potentially useful.
BWAPI::Unit MicroDetectors::closestCloakedUnit(const BWAPI::Unitset & cloakedUnits, BWAPI::Unit detectorUnit)
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