#pragma once;

#include <Common.h>
#include <MathUtil.h> //by Ruo-Ze Liu, 20190613
#include "MicroManager.h"

namespace UAlbertaBot
{
class MicroHighTemplar : public MicroManager
{
public:

	MicroHighTemplar();
	void executeMicro(const BWAPI::Unitset & targets);
	void update();


	// by Ruo-Ze Liu, 20190604, the first judge condition
	BWAPI::Unit     getTarget(BWAPI::Unit unit, const BWAPI::Unitset & targets, std::vector<std::pair<BWAPI::Position, BWAPI::UnitType>> & enemyDefenses);
	int             getAttackPriority(BWAPI::Unit attacker, BWAPI::Unit unit) const;
	void judgeWhetherMerge();
	void attackMineingWorkers(BWAPI::Unit caster);
	void attackNearestEnemy(BWAPI::Unit caster, int id);
	int calculateScore(const BWAPI::Unit & target);
};
}