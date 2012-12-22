#include <string>
#include <boost/none.hpp>
#include <boost/thread.hpp>
#include <map>
#include <cassert>

#include "../../api/GameInfo.h"
#include "../../api/Commands.h"
#include "../../api/Commander.h"
#include "../../api/CommanderFactory.h"
#include "StrategyPlaner.hpp"

using namespace std;

class HartCommander : public Commander
{
public:
    virtual string getName() const;
    virtual void initialize();
    virtual void tick();
    virtual void shutdown();

	map<string, bool> m_botAliveState;
};

REGISTER_COMMANDER(HartCommander);


string
HartCommander::getName() const
{
    // change this to return the commander name
    return "HartCommander";
}


void
HartCommander::initialize()
{
    // Use this function to setup your bot before the game starts.
	for (auto iter = m_game->bots.begin(); iter != m_game->bots.end(); ++iter)
	{
		const BotInfo& bot = *iter->second;
		m_botAliveState[bot.name] = false;
	}
}


void
HartCommander::tick()
{
    // Use this function to do stuff each time a game update is received.
    // Here you can access all the information in m_level (information about the level being played)
    // and m_game (information about the current game state).
    // You can send commands to your bots using the issue member function
    // Warning: don't spam commands. It will probably not have the effect you want as bots 
    // pause their behavior each time they get a new command.

	StrategyPlaner planer(*m_game, *m_level);
	planer.init();
	vector<Command*> commands = planer.tick();

	for (auto i = commands.begin(); i != commands.end() ; i++)
	{
		issue(*i);
	}

	//"""Process all the bots that are done with their orders and available for taking commands."""

	// The 'bots_available' list is a dynamically calculated list of bots that are done with their commands.
	//for (auto i = m_game->bots_available.begin(), end = m_game->bots_available.end(); i!=end; ++i)
	//{
	//	// Determine a place to run randomly...
	//	Vector2 target;
	//	switch((int)((float)rand()/RAND_MAX*3))
	//	{
	//	case 0: // Either a random choice of *current* flag locations, ours or theirs.
	//		target = (((float)rand()/RAND_MAX > 0.5f) ? m_game->team:m_game->enemyTeam)->flag->position;
	//		break;

	//	case 1: // Or a random choice of the goal locations for returning flags.
	//		target = (((float)rand()/RAND_MAX > 0.5f) ? m_game->team:m_game->enemyTeam)->flagScoreLocation;
	//		break; 

	//	case 2: // Or a random position in the entire level, one that's not blocked.
	//		target = *(*i)->position;
	//		m_level->findRandomFreePositionInBox(target, Vector2(0.0f,0.0f), Vector2((float)m_level->width, (float)m_level->height));
	//		break;
	//	}

	//	switch((int)((float)rand()/RAND_MAX*2))
	//	{
	//	case 0: issue(new AttackCommand((*i)->name, target, boost::none, "random")); break;
	//	case 1: issue(new ChargeCommand((*i)->name, target, "random")); break;
	//	}
	//}

	for (auto iter = m_game->match->combatEvents.begin(); iter != m_game->match->combatEvents.end(); ++iter)
	{
		const MatchCombatEvent& event = *iter;
		if (event.type == MatchCombatEvent::TYPE_RESPAWN)
		{
			m_botAliveState[event.respawnEventData.subject->name] = true;
		}
		else if (event.type == MatchCombatEvent::TYPE_KILLED)
		{
			m_botAliveState[event.killedEventData.subject->name] = false;
		}
	}
	for (auto iter = m_game->team->members.begin(); iter != m_game->team->members.end(); ++iter)
	{
		const BotInfo& bot = **iter;
		assert(m_botAliveState[bot.name] == *bot.health > 0);
	}
}


void
HartCommander::shutdown()
{
    // Use this function to do stuff after the game finishes.
}

