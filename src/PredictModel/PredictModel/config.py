

ACTIONS = ['Created', 'Destroyed', 'Discovered',
           'ChangedOwnership', 'Morph',
           'StartResearch', 'FinishUpgrade', 'CancelResearch',
           'StartUpgrade', 'FinishResearch', 'CancelUpgrade'
           'NuclearLaunch']


PROTOSS_UNITS = ['Protoss Dragoon',      'Protoss Zealot',       'Protoss Probe',
                 'Protoss Reaver',       'Protoss Observer',     'Protoss Carrier',
                 'Protoss Arbiter',      'Protoss Corsair',      'Protoss Shuttle',
                 'Protoss Scout',        'Protoss High Templar', 'Protoss Dark Templar',
                 'Protoss Archon',      'Protoss Dark Archon']

PROTOSS_BUILDINGS = ['Protoss Nexus',            'Protoss Pylon',                'Protoss Assimilator',
                     'Protoss Gateway',          'Protoss Forge',                'Protoss Cybernetics Core',
                     'Protoss Photon Cannon',    'Protoss Robotics Facility',    'Protoss Stargate',
                     'Protoss Citadel of Adun',  'Protoss Robotics Support Bay', 'Protoss Fleet Beacon',
                     'Protoss Templar Archives', 'Protoss Observatory',          'Protoss Shield Battery',
                     'Protoss Arbiter Tribunal']

PROTOSS_UPGRADES = ['Protoss Ground Armor',   'Protoss Air Armor',  'Protoss Ground Weapons',
                    'Protoss Air Weapons',    'Singularity Charge', 'Leg Enhancements',
                    'Protoss Plasma Shields', 'Carrier Capacity',   'Scarab Damage',
                    'Reaver Capacity',        'Gravitic Drive',     'Sensor Array',
                    'Gravitic Boosters',      'Khaydarin Amulet',   'Apial Sensors',
                    'Gravitic Thrusters',     'Khaydarin Core',     'Argus Jewel',
                    'Argus Talisman']

PROTOSS_RESEARCH = ['Psionic Storm',  'Hallucination', 'Recall',
                    'Disruption Web', 'Mind Control',  'Maelstrom',
                    'Stasis Field']

TERRAN_UNITS = ['Terran Firebat',               'Terran Ghost',                'Terran Goliath',
                'Terran Marine',                'Terran Medic',                'Terran SCV',
                'Terran Siege Tank',            'Terran Vulture',              'Terran Vulture Spider Mine',
                'Terran Battlecruiser',         'Terran Dropship',             'Terran Nuclear Missile',
                'Terran Science Vessel',        'Terran Valkyrie',             'Terran Wraith',
                'Spell Scanner Sweep']

TERRAN_BUILDINGS = ['Terran Academy',          'Terran Armory',         'Terran Barracks',
                    'Terran Bunker',           'Terran Command Center', 'Terran Engineering Bay',
                    'Terran Factory',          'Terran Missile Turret', 'Terran Refinery',
                    'Terran Science Facility', 'Terran Starport',       'Terran Supply Depot',
                    'Terran Comsat Station',   'Terran Control Tower',  'Terran Covert Ops',
                    'Terran Machine Shop',     'Terran Nuclear Silo',   'Terran Physics Lab']

ZERG_UNITS = ['Zerg Broodling', 'Zerg Defiler',  'Zerg Drone',
              'Zerg Hydralisk', 'Zerg Lurker',   'Zerg Ultralisk',
              'Zerg Zergling',  'Zerg Cocoon',   'Zerg Devourer',
              'Zerg Guardian',  'Zerg Mutalisk', 'Zerg Overlord',
              'Zerg Queen',     'Zerg Scourge']

ZERG_BUILDINGS = ['Zerg Creep Colony',     'Zerg Defiler Mound', 'Zerg Evolution Chamber',
                  'Zerg Extractor',        'Zerg Greater Spire', 'Zerg Hatchery',
                  'Zerg Hive',             'Zerg Hydralisk Den', 'Zerg Lair',
                  'Zerg Nydus Canal',      'Zerg Queens Nest',   'Zerg Spawning Pool',
                  'Zerg Spire',            'Zerg Spore Colony',  'Zerg Sunken Colony',
                  'Zerg Ultralisk Cavern']

TERRAN_PREDICTIONS = ['Terran SCV',          'Terran Marine',      'Terran Medic',
                      'Terran Firebat',      'Terran Ghost',       'Terran Vulture',     
                      'Terran Goliath',      'Terran Siege Tank',  'Terran Dropship',    
                      'Terran Wraith',       'Terran Valkyrie',    'Terran Science Vessel', 
                      'Terran Battlecruiser']

ZERG_PREDICTIONS = ['Zerg Drone',     'Zerg Overlord', 'Zerg Zergling', 
                    'Zerg Hydralisk', 'Zerg Lurker',   'Zerg Ultralisk',
                    'Zerg Devourer',  'Zerg Guardian', 'Zerg Mutalisk', 
                    'Zerg Queen',     'Zerg Scourge',  'Zerg Defiler']

PROTOSS_PREDICTONS = ['Protoss Dragoon',      'Protoss Zealot',       'Protoss Probe',
                      'Protoss Reaver',       'Protoss Observer',     'Protoss Carrier',
                      'Protoss Arbiter',      'Protoss Corsair',      'Protoss Shuttle',
                      'Protoss Scout',        'Protoss High Templar', 'Protoss Archon', 
                      'Protoss Dark Templar', 'Protoss Dark Archon']

MORPHED_UNITS = ['Protoss Archon', 'Protoss Dark Archon', 'Protoss Assimilator'] + \
                ZERG_UNITS + ZERG_BUILDINGS

MORPHED_ZERG = ['Zerg Lair', 'Zerg Hive', 'Zerg Lurker',
                'Zerg Devourer',  'Zerg Guardian'] + ZERG_BUILDINGS