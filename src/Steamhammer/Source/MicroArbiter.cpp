#include "MicroArbiter.h"

#include "InformationManager.h"
#include "UnitUtil.h"

// by Yuanhao Zheng, 2019-07-11

using namespace UAlbertaBot;

MicroArbiter::MicroArbiter() {

}

void MicroArbiter::executeMicro(const BWAPI::Unitset & target)
{
	if (!order.isCombatOrder()) return;

	const BWAPI::Unitset & arbiterUnits = getUnits();
	for (const auto arbiter : arbiterUnits)
	{
		if (unstickStuckUnit(arbiter))
			continue;

		if (arbiter->exists() && arbiter->isCompleted())
		{
			if (arbiter->getEnergy() > 100)
				attackNearestEnemy(arbiter);
			//followArrmy(arbiter);

			if (arbiter->getDistance(order.getPosition()) > 100)
			{
				InformationManager::Instance().getLocutusUnit(arbiter).moveTo(order.getPosition());
			}
		}
	}
}

int MicroArbiter::calculateScore2(const BWAPI::Unit & target)
{
	BWAPI::Unitset unitsNear = target->getUnitsInRadius(48, !BWAPI::Filter::IsBuilding);
	int score = 0;
	for (auto unit : unitsNear)
	{
		if (unit->getPlayer() == BWAPI::Broodwar->enemy())
		{
			if (unit->isVisible() && unit->exists() &&
				unit->getType() == BWAPI::UnitTypes::Terran_Siege_Tank_Siege_Mode )
			{
				score += 10;
			}
			else if(unit->isVisible() && unit->exists() && unit->getType() == BWAPI::UnitTypes::Terran_Siege_Tank_Tank_Mode)
			{
				score += 5;
			}
			else if (unit->isVisible() && unit->exists() && unit->isStasised())
			{
				score -= 10;
			}
		}
	}
	return score;
}

void MicroArbiter::attackNearestEnemy(BWAPI::Unit caster)
{
	BWAPI::Unitset enemysNear = caster->getUnitsInRadius(9 * 32, !BWAPI::Filter::IsBuilding && BWAPI::Filter::IsEnemy);

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
				int score = calculateScore2(unit);

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
			Micro::CastSpellonPosition(caster, target->getPosition(), BWAPI::TechTypes::Stasis_Field);
		}
		else
		{
			Log().Get() << "attackNearestEnemy::No target or targetPosition is not valid!";
		}
	}
	else
	{
		Log().Get() << "attackNearestEnemy::No enemys found!";
	}
}

int MicroArbiter::calculateScore1(const BWAPI::Unit & target)
{
	BWAPI::Unitset unitsNear = target->getUnitsInRadius(48, !BWAPI::Filter::IsBuilding);
	int score = 0;
	for(auto unit : unitsNear)
	{
		if (unit->getPlayer() == BWAPI::Broodwar->self())
		{
			if (unit->getType() == BWAPI::UnitTypes::Protoss_Dragoon)
			{
				score += 2;
			}
			else
			{
				score += 1;
			}
		}
	}
	return score;
}
void MicroArbiter::followArrmy(BWAPI::Unit caster)
{
	BWAPI::Unitset FriNear = caster->getUnitsInRadius(32 * 32, !BWAPI::Filter::IsBuilding && !BWAPI::Filter::IsEnemy);
	if (!FriNear.empty())
	{
		BWAPI::Unit target = nullptr;
		int maxScore = 0;
		for (auto unit : FriNear)
		{
			if (Config::Debug::DrawUnitTargetInfo)
			{
				BWAPI::Broodwar->drawCircleMap(unit->getPosition(), 5, BWAPI::Colors::Blue, false);
			}
			if (unit->isVisible())
			{
				int score = calculateScore1(unit);

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
			Log().Get() << "Following!!!";
			caster->follow(target);
		}
		else
		{
			Log().Get() << "follow::No target or targetPosition is not valid!";
		}
	}
}