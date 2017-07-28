#pragma once
#include <BWAPI.h>
#include <string>
using namespace std;

// Remember not to use "Broodwar" in any global class constructor!

class TestAI : public BWAPI::AIModule
{
public:
	// Virtual functions for callbacks, leave these as they are.
	virtual void onStart();
	virtual void onEnd(bool isWinner);
	virtual void onFrame();
	// Everything below this line is safe to modify.
	string get_plan(FILE *file);

	BWAPI::Error unit_gather(BWAPI::Unit unit, BWAPI::UnitType resource);
	BWAPI::Error unit_build(BWAPI::Unit unit, BWAPI::UnitType building, int count = 1);
	BWAPI::Error unit_train(BWAPI::Unit unit, BWAPI::UnitType toTrain);
	BWAPI::Error unit_research(BWAPI::Unit unit, BWAPI::TechType research);
	BWAPI::Error unit_upgrade(BWAPI::Unit unit, BWAPI::UpgradeType upgrade);
};

// Following are the maps from strings to enum types

//Resources - gather
struct map_rsrc : map<string, BWAPI::UnitType> 
{
	map_rsrc()
	{
		this->operator[]("Minerals") = BWAPI::UnitTypes::Enum::Resource_Mineral_Field;
		this->operator[]("Gas") = BWAPI::UnitTypes::Enum::Resource_Vespene_Geyser;
	};
	~map_rsrc() {};
};

//Buildings - build
struct map_bldg : map<string, BWAPI::UnitType>
{
	map_bldg()
	{
		//Tier I buildings 
		//Terran
		this->operator[]("Refinery") = BWAPI::UnitTypes::Enum::Terran_Refinery;
		this->operator[]("Supply_Depot") = BWAPI::UnitTypes::Enum::Terran_Supply_Depot;
		this->operator[]("Command_Center") = BWAPI::UnitTypes::Enum::Terran_Command_Center;

		//Protoss
		this->operator[]("Assimilator") = BWAPI::UnitTypes::Enum::Protoss_Assimilator;
		this->operator[]("Nexus") = BWAPI::UnitTypes::Enum::Protoss_Nexus;
		this->operator[]("Pylon") = BWAPI::UnitTypes::Enum::Protoss_Pylon;

		//Zerg
		this->operator[]("Extractor") = BWAPI::UnitTypes::Enum::Zerg_Extractor;
		this->operator[]("Hatchery") = BWAPI::UnitTypes::Enum::Zerg_Hatchery;

		////////////////////////////////////////////////////////////////////////////////

		//Tier II buildings
		//Terran
		this->operator[]("Barracks") = BWAPI::UnitTypes::Enum::Terran_Barracks;
		this->operator[]("Engineering_Bay") = BWAPI::UnitTypes::Enum::Terran_Engineering_Bay;

		//Protoss
		this->operator[]("Gateway") = BWAPI::UnitTypes::Enum::Protoss_Gateway;
		this->operator[]("Forge") = BWAPI::UnitTypes::Enum::Protoss_Forge;

		//Zerg
		this->operator[]("Spawning_Pool") = BWAPI::UnitTypes::Enum::Zerg_Spawning_Pool;
		this->operator[]("Evolution_Chamber") = BWAPI::UnitTypes::Enum::Zerg_Evolution_Chamber;

		///////////////////////////////////////////////////////////////////////////////

		//Tier III buildings
		//Terran
		this->operator[]("Factory") = BWAPI::UnitTypes::Enum::Terran_Factory;
		this->operator[]("Academy") = BWAPI::UnitTypes::Enum::Terran_Academy;
		this->operator[]("Bunker") = BWAPI::UnitTypes::Enum::Terran_Bunker;
		this->operator[]("Missile_Turret") = BWAPI::UnitTypes::Enum::Terran_Missile_Turret;

		//Protoss
		this->operator[]("Cybernetics_Core") = BWAPI::UnitTypes::Enum::Protoss_Cybernetics_Core;
		this->operator[]("Shield_Battery") = BWAPI::UnitTypes::Enum::Protoss_Shield_Battery;
		this->operator[]("Photon_Cannon") = BWAPI::UnitTypes::Enum::Protoss_Photon_Cannon;

		//Zerg
		this->operator[]("Hydralisk_Den") = BWAPI::UnitTypes::Enum::Zerg_Hydralisk_Den;
		this->operator[]("Creep_Colony") = BWAPI::UnitTypes::Enum::Zerg_Creep_Colony;
		this->operator[]("Lair") = BWAPI::UnitTypes::Enum::Zerg_Lair;
		this->operator[]("Sunken_Colony") = BWAPI::UnitTypes::Enum::Zerg_Sunken_Colony;
		this->operator[]("Spore_Colony") = BWAPI::UnitTypes::Enum::Zerg_Spore_Colony;

		///////////////////////////////////////////////////////////////////////////////

		//Tier IV buildings
		//Terrans
		this->operator[]("Starport") = BWAPI::UnitTypes::Enum::Terran_Starport;
		this->operator[]("Armory") = BWAPI::UnitTypes::Enum::Terran_Armory;
		this->operator[]("Machine_Shop") = BWAPI::UnitTypes::Enum::Terran_Machine_Shop; //Addons
		this->operator[]("Comsat_Station") = BWAPI::UnitTypes::Enum::Terran_Comsat_Station; //Addons
		this->operator[]("Control_Tower") = BWAPI::UnitTypes::Enum::Terran_Control_Tower; //Addons - Tier V

		//Protoss
		this->operator[]("Robotics_Facility") = BWAPI::UnitTypes::Enum::Protoss_Robotics_Facility;
		this->operator[]("Stargate") = BWAPI::UnitTypes::Enum::Protoss_Stargate;
		this->operator[]("Citadel_Of_Adun") = BWAPI::UnitTypes::Enum::Protoss_Citadel_of_Adun;
		this->operator[]("Templar_Archives") = BWAPI::UnitTypes::Enum::Protoss_Templar_Archives;

		//Zerg
		this->operator[]("Queens_Nest") = BWAPI::UnitTypes::Enum::Zerg_Queens_Nest;
		this->operator[]("Spire") = BWAPI::UnitTypes::Enum::Zerg_Spire;

	};
	~map_bldg() {};
};

//Units - train
struct map_unit : map<string, BWAPI::UnitType>
{
	map_unit()
	{
		//Terran
		this->operator[]("SCV") = BWAPI::UnitTypes::Enum::Terran_SCV;
		this->operator[]("Marine") = BWAPI::UnitTypes::Enum::Terran_Marine;
		this->operator[]("Medic") = BWAPI::UnitTypes::Enum::Terran_Medic;
		this->operator[]("Vulture") = BWAPI::UnitTypes::Enum::Terran_Vulture;
		this->operator[]("Firebat") = BWAPI::UnitTypes::Enum::Terran_Firebat;
		this->operator[]("Wraith") = BWAPI::UnitTypes::Enum::Terran_Wraith;
		this->operator[]("Goliath") = BWAPI::UnitTypes::Enum::Terran_Goliath;
		this->operator[]("Dropship") = BWAPI::UnitTypes::Enum::Terran_Dropship; //Transport Unit

		//Protoss
		this->operator[]("Probe") = BWAPI::UnitTypes::Enum::Protoss_Probe;
		this->operator[]("Zealot") = BWAPI::UnitTypes::Enum::Protoss_Zealot;
		this->operator[]("Dragoon") = BWAPI::UnitTypes::Enum::Protoss_Dragoon;
		this->operator[]("Shuttle") = BWAPI::UnitTypes::Enum::Protoss_Shuttle;  //Transport unit
		this->operator[]("Corsair") = BWAPI::UnitTypes::Enum::Protoss_Corsair;
		this->operator[]("Scout") = BWAPI::UnitTypes::Enum::Protoss_Scout;
		this->operator[]("High_Templar") = BWAPI::UnitTypes::Enum::Protoss_High_Templar;
		this->operator[]("Dark_Templar") = BWAPI::UnitTypes::Enum::Protoss_Dark_Templar;
		
		//Zerg
		this->operator[]("Drone") = BWAPI::UnitTypes::Enum::Zerg_Drone;
		this->operator[]("Overlord") = BWAPI::UnitTypes::Enum::Zerg_Overlord;   //Transport unit
		this->operator[]("Zergling") = BWAPI::UnitTypes::Enum::Zerg_Zergling;
		this->operator[]("Hydralisk") = BWAPI::UnitTypes::Enum::Zerg_Hydralisk;
		this->operator[]("Broodling") = BWAPI::UnitTypes::Enum::Zerg_Broodling;
		this->operator[]("Queen") = BWAPI::UnitTypes::Enum::Zerg_Queen;
		this->operator[]("Mutalisk") = BWAPI::UnitTypes::Enum::Zerg_Mutalisk;
	};
	~map_unit() {};
};

//Research types
struct map_research : map<string, BWAPI::TechType>
{
	map_research()
	{
		//Terran
		this->operator[]("Stim_Packs") = BWAPI::TechTypes::Enum::Stim_Packs;
		this->operator[]("Spider_Mines") = BWAPI::TechTypes::Enum::Spider_Mines;
		this->operator[]("Scanner_Sweep") = BWAPI::TechTypes::Enum::Scanner_Sweep;
		this->operator[]("Cloaking_Field") = BWAPI::TechTypes::Enum::Cloaking_Field;
		this->operator[]("Restoration") = BWAPI::TechTypes::Enum::Restoration;
		this->operator[]("Optical_Flare") = BWAPI::TechTypes::Enum::Optical_Flare;

		//Protoss
		this->operator[]("Psionic_Storm") = BWAPI::TechTypes::Enum::Psionic_Storm;
		this->operator[]("Hallucination") = BWAPI::TechTypes::Enum::Hallucination;
		this->operator[]("Mind_Control") = BWAPI::TechTypes::Enum::Mind_Control;
		this->operator[]("Maelstrom") = BWAPI::TechTypes::Enum::Maelstrom;

		//Zerg
		this->operator[]("Burrowing") = BWAPI::TechTypes::Enum::Burrowing;
		this->operator[]("Ensnare") = BWAPI::TechTypes::Enum::Ensnare;
		this->operator[]("Lurker_Aspect") = BWAPI::TechTypes::Enum::Lurker_Aspect;

	};
	~map_research() {};
};

//Upgrade types
struct map_upgrade : map<string, BWAPI::UpgradeType>
{
	map_upgrade()
	{
		//Terran
		this->operator[]("Infantry_Armor") = BWAPI::UpgradeTypes::Enum::Terran_Infantry_Armor;
		this->operator[]("Vehicle_Plating") = BWAPI::UpgradeTypes::Enum::Terran_Vehicle_Plating;
		this->operator[]("Infantry_Weapons") = BWAPI::UpgradeTypes::Enum::Terran_Infantry_Weapons;

		//Protoss
		this->operator[]("Ground_Weapons") = BWAPI::UpgradeTypes::Enum::Protoss_Ground_Weapons;
		this->operator[]("Ground_Armor") = BWAPI::UpgradeTypes::Enum::Protoss_Ground_Armor;
		this->operator[]("Plasma_Shields") = BWAPI::UpgradeTypes::Enum::Protoss_Plasma_Shields;

		//Zerg
		this->operator[]("Carapace") = BWAPI::UpgradeTypes::Enum::Zerg_Carapace;
		this->operator[]("Missile_Attacks") = BWAPI::UpgradeTypes::Enum::Zerg_Missile_Attacks;
		this->operator[]("Melee_Attacks") = BWAPI::UpgradeTypes::Enum::Zerg_Melee_Attacks;

	};
	~map_upgrade() {};
};