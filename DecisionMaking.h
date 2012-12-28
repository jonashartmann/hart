#ifndef DECISION_MAKING_H
#define DECISION_MAKING_H

#include <iostream>
#include <memory>
#include "../../api/Commands.h"

using namespace std;

namespace dms{

	/**
	* Abstract base class for a Decision Making System (DMS). 
	* Responsible for choosing the best action to perform based on the world's state and send it to the associated NPC.
	*/
	class DecisionMaking
	{
	public:
		virtual const Command* tick() = 0;
	};

	/**
	* Represents a player feature, like 'movement', 'physical attacks', 'defensive actions' and so on...
	*/
	class Feature
	{
	private:
		string m_Name;
		int m_Value;
	public:
		Feature(string name, int value)
		{
			m_Name = name;
			m_Value = value;
		}

		string getName() { return m_Name; }
		int getValue() { return m_Value; }
		void setValue(int value) { m_Value = value; }
	};

	/**
	* Represents a weighted connection with a DMS.
	*/
	class ProfileConnection
	{
	private:
		DecisionMaking* m_DecisionMaker;
		int m_Weight;
	
	public:
		ProfileConnection (DecisionMaking& decisionMaker, int weight)
			: m_DecisionMaker(&decisionMaker), m_Weight(weight) {}

		int getWeight() { return m_Weight; }
		void setWeight(int newWeight) { m_Weight = newWeight; }

		DecisionMaking* getDecisionMaker() { return m_DecisionMaker; }
	};

	/**
	* It is composed by a set of features that identifies a specific player style
	* and a set of weighted connections to all available DMSs in the CDMS.
	* A greater weight means a better performance against a specific style.
	*/
	class Profile
	{
	private:
		/**
		* Responsible for choosing the best action to perform based on the world's state and send it to the associated NPC.
		* It is chosen upon the highest weighted connection between the current profile and all available DMSs.
		*/
		ProfileConnection *m_CurrentDMS;
		vector<ProfileConnection*> m_Connections;
		vector<Feature> m_Features; // TODO: change to set

	public:
		~Profile() {}
		/*
		* Constructs a matrix of connections with every DMS with default weight values.
		* The first DMS in the list will receive the biggest value and will be selected as the current one.
		*/
		virtual void init(vector<DecisionMaking*> availableDecisionMakers) 
		{
			if (availableDecisionMakers.empty()) {
				return;
			}

			vector<ProfileConnection*> connections;
			// Starts with 1000 and go down
			int w = 1000;
			for (auto iter = availableDecisionMakers.begin(); iter != availableDecisionMakers.end(); ++iter)
			{
				// Create matrix of connections
				connections.push_back(new ProfileConnection((**iter), w--));
			}

			m_CurrentDMS = *(&connections.front());
			m_Connections = connections;
		}

		ProfileConnection* getCurrentConnection() { return m_CurrentDMS; }
	};

	/**
	* Abstract base class for a Profile Manager.
	* Responsible for recovering or creating profiles that match the opponent behavior.
	* It also uses Reinforcement Learning to manage the relative weigths of DMSs connected to a profile.
	*
	* Subclasses should implement the performance update function.
	*/
	class ProfileManager
	{
	private:
		bool m_Initialized;

		Profile *m_DefaultProfile;
		Profile *m_LastUsedProfile;
		Profile *m_CurrentProfile;

		vector<Profile> m_Profiles;
		vector<DecisionMaking*> m_DecisionMakers;

		void resetInternalFeatures() 
		{
			// TODO reset internal player features set (internal features)
		}
		/**
		* Updates weight between the current profile and the current DMS.
		*/
		void updateCurrentWeight()
		{
			// Updates the weight based on a performance update function
			ProfileConnection* connection = m_CurrentProfile->getCurrentConnection();
			connection->setWeight(performanceUpdate(*(connection->getDecisionMaker()), connection->getWeight()));
		}

	protected:
		/**
		* Performance update function.
		* >= 0 if performance is satisfying
		* < 0 if performance is unsatisfying
		*
		* Returns the updated performance value (the new weight)
		*/
		virtual int performanceUpdate(DecisionMaking& decisionMaker, int currentWeight) = 0;

	public:
		ProfileManager(vector<DecisionMaking*> decisionMakers)
		{ 
			m_Initialized = false;
			m_DecisionMakers = decisionMakers;

			m_DefaultProfile = new Profile();
			m_DefaultProfile->init(m_DecisionMakers);
		}

		~ProfileManager() { 
			delete m_DefaultProfile;
		}

		virtual void init() 
		{	
			if (m_Initialized)
			{
				// Prevent loading the profiles more than once
				return;
			}

			// TODO: load available profiles

			if (m_Profiles.empty())
			{
				m_CurrentProfile = m_DefaultProfile;
			} else
			{
				m_CurrentProfile = m_LastUsedProfile;
			}

			m_Initialized = true;
		}

		virtual void tick() 
		{
			// TODO: check world state
			// TODO: if its a new game session then 
			resetInternalFeatures();
			// TODO: if state changed then update internal features
		}

		/**
		* Produces an alert on this Profile Manager.
		*/
		virtual void alert()
		{
			// Alert received!
			if (!m_Profiles.empty())
			{
				// TODO: search similar profile on stored profiles
				// TODO: set current profile as similar profile
				// TODO: if similarity > similarity threshold then 
				// TODO:	update weight of connection between current profile and current DMS
				updateCurrentWeight();
				resetInternalFeatures();
				// TODO: else 
				createProfileFromCurrent();
			}
			else
				// There are no stored profiles
			{
				createProfileFromCurrent();
			}
		}

		void createProfileFromCurrent()
		{
			// TODO: set features of current profile with internal features
			// TODO: update weight of connection between current profile and current DMS
			// TODO: store/save current profile
			resetInternalFeatures();
		}

		Profile* getCurrentProfile() { return m_CurrentProfile; }
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
		ActivationCondition* m_AC;
		ProfileManager *m_ProfileManager;
	public:
		Monitor(ActivationCondition& AC, ProfileManager& profileManager)
			: m_AC(&AC), m_ProfileManager(&profileManager) {}
		~Monitor() { m_AC = NULL; m_ProfileManager = NULL; }

		virtual void update() 
		{
			if (m_AC->checkCondition())
			{
				m_ProfileManager->alert();
			}
		}
	};

	/**
	* Responsible for making decisions based upon observations of the world's state.
	* It is composed by different Decision Making Systems that are choosen based on the current profile.
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

		///**
		//* Responsible for choosing the best action to perform based on the world's state and send it to the associated NPC.
		//* It is chosen upon the highest weighted connection between the current profile and all available DMSs.
		//*/
		//DecisionMaking *m_CurrentDecisionMaker;

		///**
		//* Profile that best suits the opponent style.
		//* It is defined by the Profile Manager.
		//*/
		//Profile *m_CurrentProfile;

	public:
		CompositeDecisionMakingSystem(ProfileManager& profileManager, Monitor& monitor)
			: m_ProfileManager(&profileManager), m_Monitor(&monitor) {}

		void init() 
		{
			m_ProfileManager->init();
		}

		const Command* tick()
		{
			m_Monitor->update();
			m_ProfileManager->tick();
			
			Profile* m_CurrentProfile = m_ProfileManager->getCurrentProfile();
			ProfileConnection* connection = m_CurrentProfile->getCurrentConnection();
			if (NULL != connection) 
			{
				DecisionMaking* m_CurrentDecisionMaker = connection->getDecisionMaker();
				if (m_CurrentDecisionMaker != NULL) {
					return m_CurrentDecisionMaker->tick();
				}
			}

			return NULL;
		}
	};

}

#endif // !defined (DECISION_MAKING_H)