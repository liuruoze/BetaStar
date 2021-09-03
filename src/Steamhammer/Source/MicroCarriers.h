#pragma once;

#include <Common.h>
#include "MicroRanged.h"

namespace UAlbertaBot
{
class MicroCarriers : public MicroRanged
{

	BWAPI::Unit unitClosestToEnemy;
public:

    MicroCarriers();
	void setUnitClosestToEnemy(BWAPI::Unit unit) { unitClosestToEnemy = unit; }
	void executeMicro(const BWAPI::Unitset & targets);

	bool stayHomeUntilReady(const BWAPI::Unit u) const;
	bool notSafeHealthAndShield(const BWAPI::Unit u) const;  // added by ruo-ze
};
}