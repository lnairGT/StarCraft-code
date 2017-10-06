//Read build order, object-affordance representation and execute in-game

#include "Build_orderAI.h"
#include <iostream>
#include <fstream>
#include <sstream>

using namespace BWAPI;
using namespace Filter;
using namespace std;

void Build_orderAI::onStart()
{
	// Hello World!
	Broodwar << "Beginning Build Order AI" << endl;

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

	//Initialize game categories
	Broodwar << "Initializing Object categories" << endl;
	Build_orderAI::obj_aff = "obj_aff.csv";
	Build_orderAI::obj_member = "obj_member.csv";
	init_categories(obj_aff);

}

void Build_orderAI::onEnd(bool isWinner)
{
	// Called when the game ends
	if (isWinner)
	{
		// Log your win here!
	}
}

void Build_orderAI::onFrame()
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

	//Plan file related variables
	static int last_check_plan = Broodwar->getFrameCount();
	static int complete_flag = 1; //Check if a line of a plan is complete
	static int plan_end = 0;
	static int action_found = 1;
	static Error action_error;

	//Flags to check the gather command and its parameter and other essential variables
	static int gather_flag = 0;
	static string gather_param = "";
	static string line, parameter, action;
	static string category;
	static int count = 1;
	static int file_checker = 0;
	static int pop_check = 0;
	static int start_pop = 0;

	//File related variables
	FILE* file;
	stringstream linestream;
	vector<string> tokens;
	//string obj_member = "Obj_member.csv";
	//string obj_aff = "Obj_aff.csv";

	//Read the plan
	file = fopen("Build_order.txt", "r");

	//Read each line of the plan per frame if conditions are met
	if (last_check_plan + 50 < Broodwar->getFrameCount() && file != NULL && complete_flag == 1 && plan_end == 0)
	{
		//Get line of plan
		last_check_plan = Broodwar->getFrameCount();
		line = get_build_order(file);

		//Check if end of plan
		if (line != "End")
		{
			parameter = line;
			parameter.erase(remove_if(parameter.begin(), parameter.end(), [](char c) { return (!isalpha(c) && c != '_'); }), parameter.end());
			//parameter.erase(parameter.size() - 1);
			Broodwar << "Reading line of build sequence: " << parameter << endl;

			//Retrieve category and action
			category = get_membership(obj_member, parameter);
			action = get_action(obj_aff, category);
			if (action != "None")
				Broodwar << "Valid action: " << action << endl;

			//If invalid action, try all actions
			if (action == "None")
			{
				action_found = 0;
				Broodwar << "Unencountered unit" << endl;
			}
			else //Obj encountered in human demo itself
				action_found = 1;

			complete_flag = 0;

		}
		else
		{
			//If plan reading is complete
			Broodwar << "Build sequence complete" << endl;
			parameter = "";
			complete_flag = 0;
			//End of plan
			plan_end = 1;
			fclose(file);
		}
	}

	if (!plan_end)
		fclose(file); //Important to close the file as there is a limit on number of times a file can be opened

	//If action not found, try different actions, then move on
	if (!action_found)
	{
		action = action_explore(parameter);
		if (action != "None")
			action_found = 1;
		else
		{
			Broodwar << "Unknown action" << endl;
			complete_flag = 1; //Move on with execution
			action_found = 1;
		}
	}
	//if (action == "None")
	//	complete_flag = 1; //No known actions

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

			if (u->isIdle())
			{
				action_error = unit_gather(u, mapper[gather_param]);
				if (action_error == Errors::Enum::None && action == "Gather")
					complete_flag = 1; 
			}

			//At least two workers gather gas if it is specified in the plan
			static int nGasWorkers = 0;
			if (gather_param == "Gas")
			{
				if (nGasWorkers < 2)
				{
					Unit nearest_refinery = u->getClosestUnit(IsRefinery);
					if ((u->isIdle() || u->isGatheringMinerals()) && nearest_refinery->isCompleted()) //Check if refinery exists
					{
						action_error = unit_gather(u, mapper[gather_param]);
						if (action_error == Errors::Enum::None)
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
			//A pseudo-wait function that waits for frame_wait number of frames before calling the function again
			static int frame_wait_build = 200;
			static int lastChecked_build = 0;

			static int frame_wait_train = 10;
			static int lastChecked_train = 0;

			//For plan reading build
			if (action == "Build" || action == "Morph")
			{
				map_bldg mapper;
				UnitType building = mapper[parameter];
				Unit builder = u->getClosestUnit(GetType == building.whatBuilds().first && IsOwned &&
					(IsIdle || IsGatheringMinerals));
				
				complete_flag = 0;

				//Record starting population count
				if (!pop_check)
				{
					start_pop = Broodwar->self()->allUnitCount(building); //Starting population
					pop_check = 1;
				}

				if (builder && Broodwar->self()->allUnitCount(building) < start_pop + count &&
					(builder->canBuild(building) || builder->canBuildAddon(building)) &&
					lastChecked_build + frame_wait_build < Broodwar->getFrameCount())
				{
					lastChecked_build = Broodwar->getFrameCount();
					action_error = unit_build(builder, building);
					//Check if error in execution
					if (action_error != Errors::Enum::None)
					{
						Broodwar << action_error << endl;
					}
				}//Could not execute command

				if (Broodwar->self()->allUnitCount(building) == start_pop + count)  //Change completedUnitCount to allUnitCount
				{
					Broodwar << "Building issued" << endl;

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

				complete_flag = 0;
				int num_eggs;

				//Record starting population count
				if (!pop_check)
				{
					//If there is a way to check number of eggs training a SPECIFIC unit, concurrent execution can be achieved
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
					Broodwar->self()->allUnitCount(trainee) + num_eggs < start_pop + count &&
					lastChecked_train + frame_wait_train < Broodwar->getFrameCount())
				{
					lastChecked_train = Broodwar->getFrameCount();
					action_error = unit_train(u, trainee);
					//Check if error in execution
					if (action_error != Errors::Enum::None)
					{
						Broodwar << action_error << endl;
					}
				}//Could not execute command

				if (Broodwar->self()->allUnitCount(trainee) == start_pop + count) //Change completedUnitCount to allUnitCount
				{
					Broodwar << "Training issued" << endl;

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

				if (u->getType() == research.whatResearches() && u->canResearch(research) && u->isIdle())
				{
					action_error = unit_research(u, research);

					if (action_error == Errors::Enum::None) {
						complete_flag = 1;
						Broodwar << "Research initiated" << endl;
						action = "";
					}
					else
						Broodwar << action_error << endl;
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

				if (u->getType() == upgrade.whatUpgrades() && u->canUpgrade(upgrade) && u->isIdle())
				{
					action_error = unit_upgrade(u, upgrade);

					if (action_error == Errors::Enum::None) {
						complete_flag = 1;
						Broodwar << "Upgrade initiated" << endl;
						action = "";
					}
					else
						Broodwar << action_error << endl;
				} //Could not execute command
				else
				{
					complete_flag = 0;
				}
			}

			//Check if upgrade or research has already been done and print that message
			if (Broodwar->getLastError() == Errors::Enum::Already_Researched)
			{
				Broodwar << "This tech has already been researched" << endl;
				complete_flag = 1; //Move onto next line
			}

		} //Non worker unit
	} // closure: unit iterator
}

void Build_orderAI::init_categories(string filename)
{
	ifstream file(filename);
	string line;
	string action, category;

	while (getline(file, line))
	{
		stringstream linestream(line);
		getline(linestream, category, ',');
		getline(linestream, action);

		Build_orderAI::categories.push_back(category);
	}
}

string Build_orderAI::get_build_order(FILE *file)
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

string Build_orderAI::get_membership(string filename, string member)
{
	ifstream file(filename);
	string line;
	string unit, category;

	while (getline(file, line))
	{
		stringstream linestream(line);
		getline(linestream, unit, ',');
		getline(linestream, category);
	
		if (member == unit)
		{
			file.close();
			return category;
		}
	}
	return "None";
}

string Build_orderAI::get_action(string filename, string category)
{
	ifstream file(filename);
	string line;
	string value, action;

	while (getline(file, line))
	{
		stringstream linestream(line);
		getline(linestream, value, ',');
		getline(linestream, action);

		action.erase(remove_if(action.begin(), action.end(), [](char c) { return (!isalpha(c) && c != '_'); }), action.end());

		if (category == value)
		{
			return action;
		}
	}
	return "None";
}

string Build_orderAI::action_explore(string parameter)
{
	vector<string> category_list = Build_orderAI::categories;
	string action = "None";
	for (std::vector<string>::iterator it = category_list.begin(); it != category_list.end(); ++it)
	{
		action = get_action(obj_aff, *it); //Assumes one action per parameter
		Broodwar << "Attempting action: " << action << endl;
		string value = action_validate(action, parameter);
		if (value != "None")
		{
			Build_orderAI::learned_object[parameter] = *it;

			//Integrate learned info into existing representation
			ofstream file;
			file.open(obj_member, ios::app);
			file << parameter << "," << *it << endl;
			file.close();

			action = value;
			//break;
			return action;
		}
		else
			action = "None";
	}
	return action;
}

string Build_orderAI::action_validate(string action, string parameter)
{
	if (action == "Gather")
	{
		map_rsrc mapper;
		UnitType value = mapper[parameter];
		if (value != UnitTypes::Enum::None)
			return action;
		else
			return "None";
	}
	else if (action == "Build" || action == "Morph")
	{
		map_bldg mapper;
		UnitType value = mapper[parameter];
		if (value != UnitTypes::Enum::None)
			return action;
		else
			return "None";
	}
	else if (action == "Train" || action == "Hatch")
	{
		map_unit mapper;
		UnitType value = mapper[parameter];
		if (value != UnitTypes::Enum::None)
			return action;
		else
			return "None";
	}
	else if (action == "Research")
	{
		map_research mapper;
		TechType value = mapper[parameter];
		if (value != TechTypes::Enum::None)
			return action;
		else
			return "None";
	}
	else if (action == "Upgrade")
	{
		map_upgrade mapper;
		UpgradeType value = mapper[parameter];
		if (value != UpgradeTypes::Enum::None)
			return action;
		else
			return "None";
	}
	else
		return "None";
}

Error Build_orderAI::unit_gather(BWAPI::Unit unit, BWAPI::UnitType resource)
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

Error Build_orderAI::unit_build(BWAPI::Unit unit, BWAPI::UnitType building, int count)
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
		if (Broodwar->canBuildHere(targetBuildLocation, building)) //Check if building can be built at target location
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

Error Build_orderAI::unit_train(BWAPI::Unit unit, BWAPI::UnitType toTrain)
{
	if (!unit->train(toTrain))
		Broodwar << Broodwar->getLastError() << std::endl;

	Error last_err = Broodwar->getLastError(); //Returns None if no errors
	return last_err;
}

Error Build_orderAI::unit_research(BWAPI::Unit unit, BWAPI::TechType tech)
{
	if (!unit->research(tech))
		Broodwar << Broodwar->getLastError() << std::endl;

	Error last_err = Broodwar->getLastError(); //Returns None if no error
	return last_err;
}

Error Build_orderAI::unit_upgrade(BWAPI::Unit unit, BWAPI::UpgradeType upgrade)
{
	if (!unit->upgrade(upgrade))
		Broodwar << Broodwar->getLastError() << std::endl;

	Error last_err = Broodwar->getLastError(); //Returns None if no error
	return last_err;
}