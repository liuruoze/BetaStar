#include "MicroShuttles.h"
#include "MapTools.h"
#include "UnitUtil.h"
#include "CombatCommander.h"

using namespace UAlbertaBot;

namespace { auto & bwebMap = BWEB::Map::Instance(); }

// Distance between evenly-spaced waypoints, in tiles.
// Not all are evenly spaced.
const int WaypointSpacing = 5;

MicroShuttles::MicroShuttles()
	: _transportShip(nullptr)
	, _nextWaypointIndex(-1)
	, _lastWaypointIndex(-1)
	, _direction(0)
	, _target(BWAPI::Positions::Invalid)
{
}


// when enemy is terran, it is used
void MicroShuttles::executeMicro(const BWAPI::Unitset & targets)
{
	if (StrategyManager::Instance().getOpeningGroup() == "anti-terran-push")
	{
		const BWAPI::Unitset & tansportUnits = getUnits();

		if (tansportUnits.empty())
		{
			return;
		}

		std::vector<BWAPI::Unit> AttackingDragoons;
		Squad & pressingSquad = CombatCommander::Instance().getSquadData().getSquad("Pressing");
		if (!pressingSquad.isEmpty())
		{
			for (const auto unit : pressingSquad.getUnits())
			{
				if (unit->exists() &&
					unit->getType() == BWAPI::UnitTypes::Protoss_Dragoon)
				{
					AttackingDragoons.push_back(unit);
				}
			}
		}

		// if find there are 4 tanks sieging
		int siegTankCnt = 0;
		BWAPI::Unitset sigedTanks;
		for (const auto unit : BWAPI::Broodwar->enemy()->getUnits())
		{
			if (unit->isVisible() && unit->exists() &&
				(unit->getOrder() == BWAPI::Orders::Sieging ||
				unit->getOrder() == BWAPI::Orders::Unsieging ||
				unit->getType() == BWAPI::UnitTypes::Terran_Siege_Tank_Siege_Mode ||
				unit->getType() == BWAPI::UnitTypes::Terran_Siege_Tank_Tank_Mode))
			{
				sigedTanks.insert(unit);
				siegTankCnt++;
			}
		}

		std::vector<BWAPI::Unitset> groupTanks;
		if (siegTankCnt >= 2)
		{
			// assign tanks to groups
			int tank_i = 0;
			int group_i = 0;	
			for (size_t g = 0; g < size_t((float)siegTankCnt / 4) + 1; g++)
			{
				BWAPI::Unitset groupTank;
				groupTanks.push_back(groupTank);
			}
			for (const auto tank : sigedTanks)
			{
				group_i = size_t((float)tank_i / 4);
				groupTanks[group_i].insert(tank);
				tank_i++;
			}
		}

		// every shuttle has three state, load, follow, drop
		// if a shuttle is not full, it will find and load
		int g = -1;
		for (const auto trans : tansportUnits)
		{
			g++;
			if (!trans->exists())
			{
				continue;
			}

			// trans->getOrder() == BWAPI::Orders::MoveUnload || 
			if (trans->getOrder() == BWAPI::Orders::Unload)
			{
				//Log().Get() << "droping continue";
				//Log().Get() << trans->getOrder();
				continue;
			}

			// load transports
			if (trans->getSpaceRemaining() == 8)
			{
				BWAPI::Unit troop = nullptr;
				for (const auto unit : BWAPI::Broodwar->self()->getUnits())
				{
					// !unit->isUnderAttack() && unit->getOrder() != BWAPI::Orders::AttackUnit 
					if ((unit->getType() == BWAPI::UnitTypes::Protoss_Zealot ||
						unit->getType() == BWAPI::UnitTypes::Protoss_Dark_Templar) &&
						!unit->isLoaded() && unit->isCompleted() &&
						trans->getDistance(unit) < 32 * 3)
					{
						if (trans->getSpaceRemaining() == 0)
						{
							break;
						}
						troop = unit;
						break;
					}
				}
				if (troop)
				{
					Micro::Move(troop, trans->getPosition());
					if (Config::Debug::DrawCombatSimulationInfo)
					{
						BWAPI::Broodwar->drawTextScreen(200, 300, "load unit");
					}
					trans->load(troop);
				}
				else
				{
					//BWAPI::Broodwar->printf("Not full. Move to main base");
					Micro::Move(trans, order.getPosition());
				}
			}		
			else if (trans->getSpaceRemaining() > 0 && trans->getSpaceRemaining() < 8)
			{
				BWAPI::UnitCommand currentCommand(trans->getLastCommand());
				// Tf we've already ordered unloading, wait.
				if (currentCommand.getType() == BWAPI::UnitCommandTypes::Unload_All || currentCommand.getType() == BWAPI::UnitCommandTypes::Unload_All_Position)
				{
					continue;
				}

				BWAPI::Unit troop = nullptr;
				for (const auto unit : BWAPI::Broodwar->self()->getUnits())
				{				
					// !unit->isUnderAttack() && unit->getOrder() != BWAPI::Orders::AttackUnit 
					if ((unit->getType() == BWAPI::UnitTypes::Protoss_Zealot ||
						unit->getType() == BWAPI::UnitTypes::Protoss_High_Templar) &&
						!unit->isLoaded() && unit->isCompleted() && 
						trans->getDistance(unit) < 32 * 3)
					{
						if (trans->getSpaceRemaining() == 0)
						{
							break;
						}
						troop = unit;
						break;
					}
				}
				if (troop)
				{
					Micro::Move(troop, trans->getPosition());		
					if (Config::Debug::DrawCombatSimulationInfo)
					{
						BWAPI::Broodwar->drawTextScreen(200, 300, "load unit");
					}
					trans->load(troop);
				}
				else
				{
					//BWAPI::Broodwar->printf("Not full. Move to main base");
					Micro::Move(trans, order.getPosition());
				}		
			}
			else
			{		
				// we don't go too far into the enemy base if we are in pressing order
				if (order.isPressingOrder())
				{
					if (trans->getSpaceRemaining() == 0)
					{
						if (AttackingDragoons.size() >= 2 && g >= 0)
						{
							if (trans->getDistance(AttackingDragoons[g % AttackingDragoons.size()]->getPosition()) > order.getRadius() / 2 &&
								trans->getDistance(InformationManager::Instance().getEnemyMainBaseLocation()->getPosition()) <
								trans->getDistance(InformationManager::Instance().getMyMainBaseLocation()->getPosition())
								)
							{
								Micro::RightClick(trans, AttackingDragoons[g % AttackingDragoons.size()]);
								continue;
							}
						}
					
					
					}
					else
					{
						if (trans->getDistance(order.getPosition()) > order.getRadius() &&
							trans->getDistance(InformationManager::Instance().getEnemyMainBaseLocation()->getPosition()) <
							trans->getDistance(InformationManager::Instance().getMyMainBaseLocation()->getPosition())
							)
						{
							InformationManager::Instance().getLocutusUnit(trans).moveTo(order.getPosition(), order.getType() == SquadOrderTypes::Pressing);
							continue;
						}
					}

					/*
					if (trans->getDistance(order.getPosition()) > order.getRadius() &&
						trans->getDistance(InformationManager::Instance().getEnemyMainBaseLocation()->getPosition()) <
						trans->getDistance(InformationManager::Instance().getMyMainBaseLocation()->getPosition())
						)
					{
						if (trans->getSpaceRemaining() == 0)
						{
							if (AttackingDragoons.size() >= 2 && g >= 0)
							{
								Micro::RightClick(trans, AttackingDragoons[g % AttackingDragoons.size()]);
							}
						}
						else
						{
							InformationManager::Instance().getLocutusUnit(trans).moveTo(order.getPosition(), order.getType() == SquadOrderTypes::Pressing);
						}
						continue;
					}
					*/



				}

				if (siegTankCnt >= 2)
				{
					/*
					BWAPI::Unitset nearUnits = trans->getUnitsInRadius(16, BWAPI::Filter::IsEnemy);
					if (nearUnits.size() > 0)
					{
						for (auto unit : nearUnits)
						{
							if (unit->exists() && unit->isVisible() &&
								(unit->getOrder() == BWAPI::Orders::Sieging ||
								unit->getOrder() == BWAPI::Orders::Unsieging ||
								unit->getType() == BWAPI::UnitTypes::Terran_Siege_Tank_Siege_Mode))
							{
								BWAPI::Unitset dropunits = trans->getLoadedUnits();
								for (auto dropunit : dropunits)
								{
									trans->unload(dropunit);
									Log().Get() << "drop beside tanks";
									break;
								}
								break;
							}
						}
					}
					*/

					BWAPI::Broodwar->printf("Find many tanks. Begin to drop");						
					if (groupTanks.size() == 0)
					{
						continue;
					}

					BWAPI::Unitset targetTanks = groupTanks[g % groupTanks.size()];
					if (!targetTanks.empty())
					{	
						/*
						for (auto unit : targetTanks)
						{
							trans->move(unit->getPosition(), true);
						
							
							if (unit->exists() && unit->isVisible() &&
								(unit->getOrder() == BWAPI::Orders::Sieging ||
								unit->getOrder() == BWAPI::Orders::Unsieging ||
								unit->getType() == BWAPI::UnitTypes::Terran_Siege_Tank_Siege_Mode))
							{
								if (trans->getDistance(unit) < 16 * 1)
								{
									if (trans->canUnloadAtPosition(trans->getPosition()))
									{
										trans->unloadAll(trans->getPosition());
									}
									else
									{
										continue;
									}
								}
								else
								{
									trans->move(unit->getPosition(), false);
								}						
							}
							
						}
						*/
						
						//Log().Get() << "!targetTanks.empty";
						BWAPI::Position averagePosition = targetTanks.getPosition();
						if (averagePosition && averagePosition.isValid())
						{
							//Log().Get() << "averagePosition";
							BWAPI::Unit middleTank = closestSigedTankUnit(targetTanks, averagePosition);
							if (middleTank && middleTank->exists() && middleTank->isVisible())
							{
								//Log().Get() << "middleTank";
								BWAPI::Position target = middleTank->getPosition();
								if (target.isValid())
								{
									//Log().Get() << "Position isValid";
									if (Config::Debug::DrawCombatSimulationInfo)
									{
										BWAPI::Broodwar->drawTextScreen(200, 300, "drop beside tanks");
										BWAPI::Broodwar->drawLineMap(trans->getPosition(), target, BWAPI::Colors::White);
									}
									if (trans->canUnloadAtPosition(target))
									{
										trans->unloadAll(target);;
										//Log().Get() << "drop beside tanks";
									}
									else
									{
										//Log().Get() << "can not drop";
									}
								}
							}
						}
						

					}	// end target tanks
				}
				else
				{
					// Micro::Move(trans, order.getPosition());
					if (AttackingDragoons.size() >= 2 && g >= 0)
					{
						//BWAPI::Broodwar->printf("Full and dragoons");
						Micro::RightClick(trans, AttackingDragoons[g % AttackingDragoons.size()]);
					}
					else
					{
						//BWAPI::Broodwar->printf("Move to third base");
						Micro::Move(trans, order.getPosition());
					}			
				}			
			}
		}		
	}
}

// for example, tansk.
BWAPI::Unit MicroShuttles::closestSigedTankUnit(const BWAPI::Unitset & tankUnits, BWAPI::Position position)
{
	BWAPI::Unit closestSigedTank = nullptr;
	double closestDist = 1000; // max width is 1000

	for (const auto unit : tankUnits)
	{
		// if we haven't already assigned an detectorUnit to this cloaked unit
		if (!tankUnitMap[unit])
		{
			int dist = unit->getDistance(position);

			if (dist < closestDist)
			{
				closestSigedTank = unit;
				closestDist = dist;
			}
		}
	}

	return closestSigedTank;
}



void MicroShuttles::calculateWaypoints()
{
	// Tile coordinates.
	int minX = 0;
	int minY = 0;
	int maxX = BWAPI::Broodwar->mapWidth() - 1;
	int maxY = BWAPI::Broodwar->mapHeight() - 1;

	// Add vertices down the left edge.
	for (int y = minY; y <= maxY; y += WaypointSpacing)
	{
		_waypoints.push_back(BWAPI::Position(BWAPI::TilePosition(minX, y)) + BWAPI::Position(16, 16));
	}
	// Add vertices across the bottom.
	for (int x = minX; x <= maxX; x += WaypointSpacing)
	{
		_waypoints.push_back(BWAPI::Position(BWAPI::TilePosition(x, maxY)) + BWAPI::Position(16, 16));
	}
	// Add vertices up the right edge.
	for (int y = maxY; y >= minY; y -= WaypointSpacing)
	{
		_waypoints.push_back(BWAPI::Position(BWAPI::TilePosition(maxX, y)) + BWAPI::Position(16, 16));
	}
	// Add vertices across the top back to the origin.
	for (int x = maxX; x >= minX; x -= WaypointSpacing)
	{
		_waypoints.push_back(BWAPI::Position(BWAPI::TilePosition(x, minY)) + BWAPI::Position(16, 16));
	}
}

// Turn an integer (possibly negative) into a valid waypoint index.
// The waypoints form a loop. so moving to the next or previous one is always possible.
// This calculation is also used in finding the shortest path around the map. Then
// i may be as small as -_waypoints.size() + 1.
int MicroShuttles::waypointIndex(int i)
{
	UAB_ASSERT(_waypoints.size(), "no waypoints");
	const int m = int(_waypoints.size());
	return ((i % m) + m) % m;
}

// The index can be any integer. It gets mapped to a correct index first.
const BWAPI::Position & MicroShuttles::waypoint(int i)
{
	return _waypoints[waypointIndex(i)];
}

void MicroShuttles::drawTransportInformation()
{
	if (!Config::Debug::DrawUnitTargetInfo)
	{
		return;
	}

	for (size_t i = 0; i < _waypoints.size(); ++i)
	{
		BWAPI::Broodwar->drawCircleMap(_waypoints[i], 4, BWAPI::Colors::Green, false);
		BWAPI::Broodwar->drawTextMap(_waypoints[i] + BWAPI::Position(-4, 4), "%d", i);
	}
	BWAPI::Broodwar->drawCircleMap(waypoint(_lastWaypointIndex), 5, BWAPI::Colors::Red, false);
	BWAPI::Broodwar->drawCircleMap(waypoint(_lastWaypointIndex), 6, BWAPI::Colors::Red, false);
	if (_target.isValid())
	{
		BWAPI::Broodwar->drawCircleMap(_target, 8, BWAPI::Colors::Purple, true);
		BWAPI::Broodwar->drawCircleMap(_target, order.getRadius(), BWAPI::Colors::Purple, false);
	}
}


bool MicroShuttles::isNotLoadOrDrop(BWAPI::Unit unit)
{
	if (unit->getLastCommand().getType() == BWAPI::UnitCommandTypes::Load)
	{
		return false;
	}
	if (unit->getLastCommand().getType() == BWAPI::UnitCommandTypes::Unload_All)
	{
		return false;
	}
	if (unit->getLastCommand().getType() == BWAPI::UnitCommandTypes::Unload_All_Position)
	{
		return false;
	}
	return true;
}

void MicroShuttles::update()
{
	
}

// Called when the transport exists and is not full.
void MicroShuttles::loadTroops()
{
}

// Only called when the transport exists and is loaded.
void MicroShuttles::maybeUnloadTroops()
{
}

// Called when the transport exists and is loaded.
void MicroShuttles::moveTransport()
{
}

// Decide which direction to go, then follow the perimeterto the destination.
// Called only when the transport exists and is loaded.
void MicroShuttles::followPerimeter()
{
}

bool MicroShuttles::hasTransportShip() const
{
	return UnitUtil::IsValidUnit(_transportShip);
}
