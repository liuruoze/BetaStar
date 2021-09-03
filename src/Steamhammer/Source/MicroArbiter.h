#pragma once
#include <Common.h>
#include <MathUtil.h>
#include "MicroManager.h"

// by Yuanhao Zheng, 2019-07-11

namespace UAlbertaBot
{
	class MicroArbiter : public MicroManager
	{
	public:

		MicroArbiter();
		void executeMicro(const BWAPI::Unitset & targets);
		BWAPI::Unit     getTarget(BWAPI::Unit unit, const BWAPI::Unitset & targets, std::vector<std::pair<BWAPI::Position, BWAPI::UnitType>> & enemyDefenses);
		int getAttackPriority(BWAPI::Unit attacker, BWAPI::Unit unit) const;
		void followArrmy(BWAPI::Unit caster);
		void attackNearestEnemy(BWAPI::Unit caster);
		int calculateScore1(const BWAPI::Unit & target);
		int calculateScore2(const BWAPI::Unit & target);
	};
}