#ifndef CHARACTER_DECISION_MAKING_H
#define CHARACTER_DECISION_MAKING_H

#include "../../api/GameInfo.h"
#include "DecisionMaking.h"

using namespace dms;

class AttackerDMS : public DecisionMaking
{
private:
	const BotInfo* m_Bot;
	GameInfo* m_GameInfo;
	LevelInfo *m_LevelInfo;
public:
	AttackerDMS(const BotInfo& bot, GameInfo& gameInfo, LevelInfo& levelInfo)
		: m_Bot(&bot), m_GameInfo(&gameInfo), m_LevelInfo(&levelInfo) {}

	virtual const Command* tick()
	{
		// Determine a place to run randomly...
		Vector2 target;
		switch((int)((float)rand()/RAND_MAX*3))
		{
		case 0: // Either a random choice of *current* flag locations, ours or theirs.
			target = (((float)rand()/RAND_MAX > 0.5f) ? m_GameInfo->team : m_GameInfo->enemyTeam)->flag->position;
			break;

		case 1: // Or a random choice of the goal locations for returning flags.
			target = (((float)rand()/RAND_MAX > 0.5f) ? m_GameInfo->team : m_GameInfo->enemyTeam)->flagScoreLocation;
			break; 

		case 2: // Or a random position in the entire level, one that's not blocked.
			target = *m_Bot->position;
			m_LevelInfo->findRandomFreePositionInBox(target, Vector2(0.0f,0.0f), Vector2((float)m_LevelInfo->width, (float)m_LevelInfo->height));
			break;
		}

		return new AttackCommand(m_Bot->name, target, boost::none, "AttackerDMS: random attack");
	}
};


#endif // !defined (CHARACTER_DECISION_MAKING_H)