#ifndef DECISION_MAKING_H
#define DECISION_MAKING_H

#include "bt/Behavior.h"

class DecisionMaking
{
public:
	virtual void initialize() {}
	virtual void terminate() {}
};

class Profile
{
private:
	DecisionMaking *decisionMaker;
};

/**
	* Responsible for recovering or creating profiles that match the opponent behavior.
	* It also uses Reinforcement Learning to manage the relative weigths of DMSs connected to a profile.
	*/
class ProfileManager
{
private:
	vector<Profile> profiles;
};

/**
* Logical expression that identifies a bad performance.
*/
class ActivationCondition
{
public:
	/**
	* Checks if this condition is met.
	* True if it is active, false otherwise.
	*/
	virtual bool checkCondition() = 0;
};

/**
* Responsible for watching the world and producing alerts for the Profile Manager.
* An alert is generated every time the Activation Condition (AC) is met or the game session ends.
* The AC is a logical expression that identifies a bad performance.
*/
class Monitor
{
private:
	ProfileManager *profileManager;
};

/**
* Responsible for making decisions based upon observations of the world's state.
* It is composed from different Decision Making Systems that are choosen based on the current profile.
*/
class CompositeDecisionMakingSystem
{
private:
	/**
	* Responsible for recovering or creating profiles that match the opponent behavior.
	* It also uses Reinforcement Learning to manage the relative weigths of DMSs connected to a profile.
	*/
	ProfileManager *m_ProfileManager;

	/**
	* Responsible for watching the world and producing alerts for the Profile Manager.
	*/
	Monitor *m_Monitor;

	/**
	* Is chosen upon the highest weighted connection between the current profile and all available DMSs.
	* It will choose the best action to perform based on the world's state and send it to the associated NPC.
	*/
	DecisionMaking *m_CurrentDecisionMaker;

	/**
	* Profile that best suits the opponent style.
	* It is defined by the Profile Manager.
	*/
	Profile *m_CurrentProfile;
};

#endif // !defined (DECISION_MAKING_H)