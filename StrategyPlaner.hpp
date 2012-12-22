#include "../../api/GameInfo.h"
#include "../../api/Commands.h"
#include "DecisionMaking.h"
#include "CharacterDecisionMaking.hpp"

using namespace dms;

class AliveCondition: public ActivationCondition
{
private:
	int m_AliveOnStart;
	int m_AliveCurrent;

public:
	AliveCondition(int botsAlive) 
	{
		m_AliveOnStart = m_AliveCurrent = botsAlive;
	}

	virtual bool checkCondition()
	{
		return m_AliveCurrent < (m_AliveOnStart / 2);
	}

	void setAliveCurrent(int botsAlive)
	{
		m_AliveCurrent = botsAlive;
	}
};

class StrategyPlaner;

class CommandProfileManager: public ProfileManager
{
private:
	StrategyPlaner* m_Planer;

public:
	CommandProfileManager(StrategyPlaner& planer, vector<DecisionMaking*> decisionMakers)
		: ProfileManager(decisionMakers), m_Planer(&planer) {}

	virtual int performanceUpdate(DecisionMaking& decisionMaker, int currentWeight)
	{
		// TODO: change the function
		return 0;
	}
};

class StrategyPlaner
{
private:
	AliveCondition* m_CurrentAC;

	// to make it easier to write the code
	typedef CompositeDecisionMakingSystem CDMS;
	vector<CDMS*> m_Cdms;


public:
	StrategyPlaner(GameInfo& gameInfo, LevelInfo& levelInfo)
	{
		int botsAlive = gameInfo.bots_alive.size();
		if (m_CurrentAC == NULL)
		{
			AliveCondition ac(botsAlive);
			m_CurrentAC = &ac;
		} else 
		{
			m_CurrentAC->setAliveCurrent(botsAlive);
		}

		// The 'bots_available' list is a dynamically calculated list of bots that are done with their commands.
		for (auto i = gameInfo.bots_available.begin(), end = gameInfo.bots_available.end(); i!=end; ++i)
		{
			vector<DecisionMaking*> decisionMakers;

			const BotInfo& bot = **i;
			AttackerDMS attacker(bot, gameInfo, levelInfo);
			decisionMakers.push_back(&attacker);

			CommandProfileManager manager(*this, decisionMakers);
			Monitor monitor(*m_CurrentAC, manager);
			CDMS cdms(manager, monitor);
			m_Cdms.push_back(&cdms);
		}
	}
	~StrategyPlaner()
	{
		m_CurrentAC = NULL;
	}

	void init() 
	{
		for (auto i = m_Cdms.begin(); i != m_Cdms.end(); i++)
		{
			(*i)->init();
		}
	}

	vector<Command*> tick()
	{
		vector<Command*> commands;
		for (auto i = m_Cdms.begin(); i != m_Cdms.end(); i++)
		{
			Command* command = (*i)->tick();
			if (NULL != command) 
			{
				commands.push_back(command);
			}
		}

		return commands;
	}

};
