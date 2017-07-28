//Read plan and execute in-game

#include "TestAI.h"
#include <iostream>
#include <fstream>
#include <sstream>

using namespace BWAPI;
using namespace Filter;
using namespace std;

void TestAI::onStart()
{
	Broodwar->sendText("Plan Reader for StarCraft");

	// Print the map name.
	// BWAPI returns std::string when retrieving a string, don't forget to add .c_str() when printing!
	Broodwar << "The map is " << Broodwar->mapName() << "!" << std::endl;

	// Enable the UserInput flag, which allows us to control the bot and type messages.
	Broodwar->enableFlag(Flag::UserInput);

	// Uncomment the following line and the bot will know about everything through the fog of war (cheat).
	//Broodwar->enableFlag(Flag::CompleteMapInformation);

	// Set the command optimization level so that common commands can be grouped
	// and reduce the bot's APM (Actions Per Minute).
	Broodwar->setCommandOptimizationLevel(2);

	// Check if this is a replay
	if (Broodwar->isReplay())
	{

		// Announce the players in the replay
		Broodwar << "The following players are in this replay:" << std::endl;

		// Iterate all the players in the game using a std:: iterator
		Playerset players = Broodwar->getPlayers();
		for (auto p : players)
		{
			// Only print the player if they are not an observer
			if (!p->isObserver())
				Broodwar << p->getName() << ", playing as " << p->getRace() << std::endl;
		}

	}
	else // if this is not a replay
	{
		// Retrieve you and your enemy's races. enemy() will just return the first enemy.
		// If you wish to deal with multiple enemies then you must use enemies().
		if (Broodwar->enemy()) // First make sure there is an enemy
			Broodwar << "The matchup is " << Broodwar->self()->getRace() << " vs " << Broodwar->enemy()->getRace() << std::endl;
	}
}

void TestAI::onEnd(bool isWinner)
{
	// Called when the game ends
	if (isWinner)
	{
		// Log your win here!
	}
}

void TestAI::onFrame()
{
	// Called once every game frame

	// Display the game frame rate as text in the upper left area of the screen
	Broodwar->drawTextScreen(200, 0, "FPS: %d", Broodwar->getFPS());
	Broodwar->drawTextScreen(200, 20, "Average FPS: %f", Broodwar->getAverageFPS());

	// Return if the game is a replay or is paused
	if (Broodwar->isReplay() || Broodwar->isPaused() || !Broodwar->self())
		return;

	// Prevent spamming by only running our onFrame once every number of latency frames.
	// Latency frames are the number of frames before commands are processed.
	if (Broodwar->getFrameCount() % Broodwar->getLatencyFrames() != 0)
		return;

	//Plan related variables
	static int last_check_plan = Broodwar->getFrameCount(); //Frame when plan last checked
	static int complete_flag = 1; //Check if a line of a plan is complete
	static int plan_end = 0; //Check if end of plan
	static string line, action, parameter;
	static int count = 1; //Number of units to train or build

	//Flags to check the gather command and its parameters
	static int gather_flag = 0;
	static string gather_param = "";

	//Variables for maintaining population count
	static int pop_check = 0;
	static int start_pop = 0;

	//File related variables
	FILE* file;
	stringstream linestream;
	vector<string> tokens;

	//Read the plan
	file = fopen("Plan.txt", "r");

	//Read each line of the plan per frame if conditions are met
	if (last_check_plan + 50 < Broodwar->getFrameCount() && file != NULL && complete_flag == 1 && plan_end == 0)
	{
		//Get line of plan
		last_check_plan = Broodwar->getFrameCount();
		line = get_plan(file);

		if (line != "End")
		{
			std::istringstream split(line);
			for (std::string each; std::getline(split, each, ' '); tokens.push_back(each));

			action = tokens[0];
			parameter = tokens[1];

			//Remove extraneous characters from parameter
			parameter.erase(remove_if(parameter.begin(), parameter.end(), [](char c) { return (!isalpha(c) && c != '_'); }), parameter.end());

			Broodwar << "Reading line of plan:" << endl;

			if (action == "Build" || action == "Train" || action == "Morph" || action == "Hatch")
			{
				count = stoi(tokens[2]);
				Broodwar << action << " " << parameter << " " << count << endl;
			}
			else 
				Broodwar << action << " " << parameter << endl;

			//Plan line incomplete
			complete_flag = 0;
		}
		else //End of plan
		{	
			Broodwar << "Plan reading complete" << endl;
			action = "";
			parameter = "";
			complete_flag = 0;
			plan_end = 1;
			fclose(file);
		}
	}

	if (!plan_end)
		fclose(file); //Limit on number of times a file can be opened

	// Iterate through all the units that we own
	for (auto &u : Broodwar->self()->getUnits())
	{
		// Ignore the unit if it no longer exists
		// Make sure to include this block when handling any Unit pointer!
		if (!u->exists())
			continue;
		if (u->isLockedDown() || u->isMaelstrommed() || u->isStasised())
			continue;
		if (u->isLoaded() || !u->isPowered() || u->isStuck())
			continue;
		if (!u->isCompleted() || u->isConstructing())
			continue;
		if (u->isBurrowed()) //For Zerg units
			continue;

		//Plan specifies gathering, then always gather as default
		if (action == "Gather")
		{	
			gather_flag = 1;
			gather_param = parameter; //Resource to be gathered
		}

		//Check if unit type is worker
		if (u->getType().isWorker() && gather_flag == 1)
		{
			map_rsrc mapper;
			Error gather_error;

			if (u->isIdle())
			{
				unit_gather(u, mapper[gather_param]);
				complete_flag = 1;
			}

			//At least two workers gather gas if specified in the plan
			static int nGasWorkers = 0;
			if (gather_param == "Gas")
			{
				if (nGasWorkers < 2)
				{
					if (u->isIdle() || u->isGatheringMinerals())
					{
						gather_error = unit_gather(u, mapper[gather_param]);
						if (gather_error == Errors::Enum::None)
						{
							++nGasWorkers;
							complete_flag = 0;
						}
					}
				}
				else if (nGasWorkers == 2)
				{
					complete_flag = 1;
					gather_param = "Minerals"; //Gather minerals by default after assigning gas workers
				}
			} //Refinery not available
		}

		else //For any other unit
		{
			//Waits for frame_wait number of frames before calling the function again
			static int frame_wait = 200;
			static int lastChecked = 0;

			//For plan reading build
			if (action == "Build" || action == "Morph")
			{
				map_bldg mapper;
				UnitType building = mapper[parameter];
				Unit builder = u->getClosestUnit(GetType == building.whatBuilds().first && IsOwned &&
					                             (IsIdle || IsGatheringMinerals || IsGatheringGas));
				Error build_error;
				complete_flag = 0;

				//Record starting population count
				if (!pop_check)
				{
					start_pop = Broodwar->self()->allUnitCount(building); //Starting population
					pop_check = 1;
				}

				if (builder && Broodwar->self()->allUnitCount(building) < start_pop + count &&
					(builder->canBuild(building) || builder->canBuildAddon(building)) &&
					lastChecked + frame_wait < Broodwar->getFrameCount())
				{
					lastChecked = Broodwar->getFrameCount();
					build_error = unit_build(builder, building);
					//Check if error in execution
					if (build_error != Errors::Enum::None) 
					{
						Broodwar << build_error << endl;
					}
				}//Could not execute command

				if (Broodwar->self()->completedUnitCount(building) == start_pop + count)
				{
					Broodwar << "Building complete" << endl;
					
					action = "";
					complete_flag = 1;
					count = 1;
					//Reinitialize population variables
					pop_check = 0;
					start_pop = 0;
				}
			}

			//For plan reading train
			if (action == "Train" || action == "Hatch")
			{
				map_unit mapper;
				UnitType trainee = mapper[parameter];

				Error train_error;
				complete_flag = 0;
				int num_eggs; 

				//Record starting population count
				if (!pop_check)
				{
					start_pop = Broodwar->self()->allUnitCount(trainee); //Starting population
					pop_check = 1;
				}

				//Check Zerg eggs for training
				if (u->getType().getRace() == Races::Zerg)
					num_eggs = Broodwar->self()->allUnitCount(UnitTypes::Enum::Zerg_Egg);
				else
					num_eggs = 0;

				//Execute command
				if (u->getType() == trainee.whatBuilds().first && u->canTrain(trainee) && u->isIdle() &&
					Broodwar->self()->allUnitCount(trainee) + num_eggs < start_pop + count)
				{
					train_error = unit_train(u, trainee);
					//Check if error in execution
					if (train_error != Errors::Enum::None)
					{
						Broodwar << train_error << endl;
					}
				}//Could not execute command

				if (Broodwar->self()->completedUnitCount(trainee) == start_pop + count)
				{
					Broodwar << "Training complete" << endl;

					action = "";
					complete_flag = 1;
					count = 1;
					//Reinitialize population variables
					pop_check = 0;
					start_pop = 0;
				}
			}

			//Plan reading research
			if (action == "Research")
			{
				map_research mapper;
				TechType research = mapper[parameter];
				Error rsrch_error;

				if (u->getType() == research.whatResearches() && u->canResearch(research) && u->isIdle())
				{
					rsrch_error = unit_research(u, research);

					if (rsrch_error == Errors::Enum::None) {
						complete_flag = 1;
						Broodwar << "Research initiated" << endl;
						action = "";
					}
					else
						Broodwar << rsrch_error << endl;
				} //Could not execute command
				else
				{
					complete_flag = 0;
				}
			}

			//Plan reading upgrade
			if (action == "Upgrade")
			{
				map_upgrade mapper;
				UpgradeType upgrade = mapper[parameter];
				Error upgrade_error;

				if (u->getType() == upgrade.whatUpgrades() && u->canUpgrade(upgrade) && u->isIdle())
				{
					upgrade_error = unit_upgrade(u, upgrade);

					if (upgrade_error == Errors::Enum::None) {
						complete_flag = 1;
						Broodwar << "Upgrade initiated" << endl;
						action = "";
					}
					else
						Broodwar << upgrade_error << endl;
				} //Could not execute command
				else
				{
					complete_flag = 0;
				}
			}
		} //Non worker unit
	} // closure: unit iterator
}

string TestAI::get_plan(FILE *file)
{
	char line[256];
	static long pos = 0;

	if (!feof(file))
	{
		fseek(file, pos, SEEK_CUR);
		fgets(line, sizeof(line), file);
		pos = ftell(file);
		return line;
	}
	else
		return "";
}

Error TestAI::unit_gather(BWAPI::Unit unit, BWAPI::UnitType resource)
{
	//Check if unit already carrying some resource
	if (unit->isCarryingGas() || unit->isCarryingMinerals())
	{
		unit->returnCargo();
	}

	if (resource == UnitTypes::Enum::Resource_Mineral_Field) {
		//Gather minerals from nearest mineral patch
		if (!unit->gather(unit->getClosestUnit(IsMineralField)))
		{
			// If the call fails, then print the last error message
			Broodwar << Broodwar->getLastError() << std::endl;
		}
	}

	else if (resource == UnitTypes::Enum::Resource_Vespene_Geyser) {
		//Gather gas from nearest refinery
		if (!unit->gather(unit->getClosestUnit(IsRefinery)))
		{
			// If the call fails, then print the last error message
			Broodwar << Broodwar->getLastError() << std::endl;
		}	
	}

	Error last_err = Broodwar->getLastError(); //Returns None if no errors
	return last_err;
}

Error TestAI::unit_build(BWAPI::Unit unit, BWAPI::UnitType building, int count) 
{
	//Building is refinery
	if (building.isRefinery()) 
	{
		Unit vespene = unit->getClosestUnit(GetType == UnitTypes::Enum::Resource_Vespene_Geyser);
		TilePosition vespene_pos = vespene->getTilePosition();

		if (vespene_pos) {
			Broodwar->registerEvent([vespene_pos, building](Game*)
			{
				Broodwar->drawBoxMap(Position(vespene_pos),
					                 Position(vespene_pos + building.tileSize()),
					                 Colors::Blue);
			},
			nullptr,  // condition
			building.buildTime() + 100);  // frames to run

			unit->build(building, vespene_pos);
		} //No valid vespene geyser position
	} 

	//Builder is a moving unit/worker and building is not addon
	else if (!unit->getType().isBuilding() && !building.isAddon()) 
	{
		TilePosition targetBuildLocation = Broodwar->getBuildLocation(building, unit->getTilePosition());
		if (Broodwar->canBuildHere(targetBuildLocation, building, unit)) //Check if building can be built at target location
		{
			// Register an event that draws the target build location
			Broodwar->registerEvent([targetBuildLocation, building](Game*)
			{
			 Broodwar->drawBoxMap(Position(targetBuildLocation),
					              Position(targetBuildLocation + building.tileSize()),
						          Colors::Blue);
		    },
	        nullptr,  // condition
	        building.buildTime() + 100);  // frames to run

			unit->build(building, targetBuildLocation);
		} //No valid target position
		else 
		{
			Broodwar << "Finding a valid build location.." << endl;
			//Can try increasing max range ... CHECK!!
		}
	}
	
	//Building is addon
	else if (building.isAddon())
	{
		TilePosition builder_pos = unit->getTilePosition();
		Broodwar->registerEvent([builder_pos, building](Game*)
		{
			Broodwar->drawBoxMap(Position(builder_pos),
				Position(builder_pos + building.tileSize()),
				Colors::Blue);
		},
		nullptr,  // condition
		building.buildTime() + 100);  // frames to run

		unit->buildAddon(building);
	}

	//builder is a building Eg. Zerg lair from Hatchery
	else 
	{
		TilePosition builder_pos = unit->getTilePosition();
		Broodwar->registerEvent([builder_pos, building](Game*)
		{
			Broodwar->drawBoxMap(Position(builder_pos),
				Position(builder_pos + building.tileSize()),
				Colors::Blue);
		},
		nullptr,  // condition
		building.buildTime() + 100);  // frames to run

		unit->build(building); //Same as: unit->morph(building); 
	}

	Error last_err = Broodwar->getLastError(); //Returns None if no errors
	return last_err;
}

Error TestAI::unit_train(BWAPI::Unit unit, BWAPI::UnitType toTrain)
{
	if (!unit->train(toTrain))
		Broodwar << Broodwar->getLastError() << std::endl;

	Error last_err = Broodwar->getLastError(); //Returns None if no errors
	return last_err;
}

Error TestAI::unit_research(BWAPI::Unit unit, BWAPI::TechType tech)
{
	if (!unit->research(tech))
		Broodwar << Broodwar->getLastError() << std::endl;

	Error last_err = Broodwar->getLastError(); //Returns None if no error
	return last_err;
}

Error TestAI::unit_upgrade(BWAPI::Unit unit, BWAPI::UpgradeType upgrade)
{
	if (!unit->upgrade(upgrade))
		Broodwar << Broodwar->getLastError() << std::endl;

	Error last_err = Broodwar->getLastError(); //Returns None if no error
	return last_err;
}
