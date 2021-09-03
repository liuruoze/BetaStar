#pragma once;

#include <Common.h>
#include "MicroManager.h"

namespace UAlbertaBot
{
class MicroManager;

class MicroObservers : public MicroManager
{

	// The cloakedUnitMap is unused, but code exists to fill in its values.
	// For each enemy cloaked unit, it keeps a flag "have we assigned a detector to watch it?"
	// No code exists to assign detectors or move them toward their assigned positions.
	std::map<BWAPI::Unit, bool>	cloakedUnitMap;


	// For each enemy tank unit, it keeps a flag "have we assigned a observer to watch it?"
	std::map<BWAPI::Unit, bool>	tankUnitMap;

	BWAPI::Unit unitClosestToEnemy;

public:

	MicroObservers();
	~MicroObservers() {}

	void setUnitClosestToEnemy(BWAPI::Unit unit) { unitClosestToEnemy = unit; }
	void setClosestEnemy(BWAPI::Unit unit) { unitClosestToEnemy = unit; }
	
	void executeMicro(const BWAPI::Unitset & targets);
	void detectEnemyBases(const BWAPI::Unit & detectorUnit, int direction, unsigned seed);

	BWAPI::Unit closestCloakedUnit(const BWAPI::Unitset & cloakedUnits, BWAPI::Unit detectorUnit);
	BWAPI::Unit closestSigedTankUnit(const BWAPI::Unitset & tankUnits, BWAPI::Unit detectorUnit);
	BWAPI::Unit closestVultureUnit(const BWAPI::Unitset & vultureUnit, BWAPI::Unit detectorUnit);
};
}