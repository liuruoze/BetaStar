#pragma once;

#include <Common.h>
#include "MicroRanged.h"

// by Yuzhou Wu, 2019-07-15

namespace UAlbertaBot
{
class MicroReavers : public MicroRanged
{
	BWAPI::Unit unitClosestToEnemy;

public:

    MicroReavers();

	void executeMicro(const BWAPI::Unitset & targets);
	void setUnitClosestToEnemy(BWAPI::Unit unit) { unitClosestToEnemy = unit; }
	bool stayHomeUntilReady(const BWAPI::Unit u) const;
	bool notSafeHealthAndShield(const BWAPI::Unit u) const;  // added by ruo-ze
};
}