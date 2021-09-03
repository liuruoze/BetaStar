#pragma once;

#include <Common.h>
#include "MicroManager.h"

namespace UAlbertaBot
{
class MicroDragoons : public MicroManager
{
private:

	// Ranged ground weapon does splash damage, so it works under dark swarm.
	bool goodUnderDarkSwarm(BWAPI::UnitType type);

    void kite(BWAPI::Unit rangedUnit, BWAPI::Unit target);
	void kiteNearTanks(BWAPI::Unit rangedUnit, BWAPI::Unit target, BWAPI::Unitset tanks);
	BWAPI::Unit closestSigedTankUnit(BWAPI::Unitset & tankUnits, BWAPI::Position position);
	BWAPI::Position center(BWAPI::TilePosition tile);

public:

	MicroDragoons();

	void getTargets(BWAPI::Unitset & targets) const;
    virtual void executeMicro(const BWAPI::Unitset & targets);
	void assignTargets(const BWAPI::Unitset & targets);

	int getAttackPriority(BWAPI::Unit rangedUnit, BWAPI::Unit target);
	BWAPI::Unit getTarget(BWAPI::Unit rangedUnit, const BWAPI::Unitset & targets);
};
}