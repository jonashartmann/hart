#ifndef BEHAVIOR_H
#define BEHAVIOR_H
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <vector>
#include <deque>
#include <iostream>
#include <FastDelegate.h>
//#include "../tools/asserts.h"

using namespace std;
using namespace fastdelegate;

namespace bt
{
	typedef FastDelegate<void ()> BehaviorObserver;

	enum Status {
		/**
		* Not initialized.
		*/
		BH_INVALID,
		/**
		* The behavior succeeded.
		*/
		BH_SUCCESS,
		/**
		* The behavior failed.
		*/
		BH_FAILURE,
		/**
		* Is currently running.
		*/
		BH_RUNNING,
		/**
		* Is waiting for something to happen.
		*/
		BH_SUSPENDED,
	};

	/*
	 * Convenience function to retrieve the name of the enumeration by its value
	 */
	/*const char* getStatusName(Status s)
	{
		switch (s)
		{
		case BH_INVALID:
			return "INVALID";
		case BH_SUCCESS:
			return "SUCCESS";
		case BH_FAILURE:
			return "FAILURE";
		case BH_RUNNING:
			return "RUNNING";
		case BH_SUSPENDED:
			return "SUSPENDED";
		}

		return "NOT RECOGNIZED STATUS: " + s;
	}*/
	/******************************************************************************/

	/*
	* Abstract base class for actions, conditions and composites.
	*/
	class Behavior 
	{
	public:
		Behavior() {
			m_eStatus = BH_INVALID;
		}

		/**
		* Helper wrapper function, which helps to make sure that we call our
		* functions correctly.
		* 
		* @return Returns the status after execution.
		*/
		Status tick() 
		{
			if (m_eStatus == BH_INVALID)
			{
				onInitialize();
			}

			m_eStatus = update();

			if (m_eStatus != BH_RUNNING)
			{
				onTerminate(m_eStatus);
			}
			return m_eStatus;
		}

		/**
		* Update function which is called at every frame.
		* 
		* @return the status of this behavior after execution.
		*/
		virtual Status update()				= 0;

		/**
		* Called only once before the {@link #update()} is called.
		*/
		virtual void onInitialize()			{}

		/**
		* Called once the update is finished.
		* 
		* @param status
		*            - the status which was returned by the {@link #update()}
		*            function.
		*/
		virtual void onTerminate(Status)	{}

		Status m_eStatus;
		BehaviorObserver m_Observer;
	};

	/* Controls the main execution of the behavior tree */
	class BehaviorTree
	{
	public :
		/**
		* Insert active behavior.
		* 
		* @param bh
		*            - active behavior
		*/
		void insert(Behavior& bh, BehaviorObserver* observer = NULL) 
		{
			if (observer != NULL)
			{
				bh.m_Observer = *observer;
			}
			m_Behaviors.push_front(&bh);
		}

		/*void insert(Behavior& bh) 
		{
		m_Behaviors.push_front(&bh);
		}*/

		void terminate(Behavior& bh, Status result) 
		{
			cout << "BT: Terminating behavior " << addressof(bh) << endl; // << " with status " << getStatusName(result) << endl;
			//ASSERT(result != BH_RUNNING);
			bh.m_eStatus = result;

			if (!bh.m_Observer.empty())
			{
				cout << "Notifying observer of behavior: " << &bh << endl;
				bh.m_Observer();
			}

		}

		/**
		* Main entry-point function. 
		* Loop through all behaviors.
		*/
		void tick() 
		{
			// Insert an end-of-update marker into the list of tasks.
			m_Behaviors.push_back(NULL);

			while (step()) 
			{
				continue;
			}
		}

		/**
		* Single stepping, where the magic happens.
		* 
		* @return
		*/
		bool step() 
		{
			// Get the first element of the deque and remove it.
			Behavior* current = m_Behaviors.front();
			m_Behaviors.pop_front();

			// If this is the end-of-update marker, stop processing.
			if (current == NULL) {
				return false;
			}

			// Perform the update on this individual task.
			if (current->m_eStatus != BH_SUSPENDED)
			{
				cout << "BT: Updating current task:" << endl;
				cout << "--------------------------" << endl;
				Status status = current->tick();
				cout << "BT: task updated."; // Status = " << getStatusName(status);
				cout << endl;
			}


			// Process the observer if the task is terminated.
			if (current->m_eStatus != BH_RUNNING && (current->m_Observer != NULL))
			{
				// Call the observer to notify the parent
				cout << "Notifying observer of behavior: " << current << endl;
				current->m_Observer();
			}
			// Otherwise drop it into the queue for the next tick().
			else 
			{
				m_Behaviors.push_back(current);
			}
			return true;
		}

	protected:
		deque<Behavior*> m_Behaviors;
	};

	/******************************************************************************/
	class Composite : public Behavior
	{
	public:
		void add(Behavior *bh) { m_Children.push_back(bh); };
	protected:
		typedef vector<Behavior*> Behaviors;
		Behaviors m_Children;
	};

	class Sequence : public Composite
	{
	public:
		Sequence(BehaviorTree& bt) 
		{
			m_pBehaviorTree = &bt;
		}

	protected:
		BehaviorTree* m_pBehaviorTree;

		virtual void onInitialize()
		{
			m_Current = m_Children.begin();
			BehaviorObserver observer;
			observer.bind(this, &Sequence::onChildComplete);
			m_pBehaviorTree->insert(**m_Current, &observer);
		}

		virtual void onChildComplete() 
		{
			cout << "Sequence: Child is complete!" << endl;
			Behavior& child = **m_Current;
			if (child.m_eStatus == BH_FAILURE)
			{
				cout << "Sequence: terminating with FAILURE" << endl << endl;
				m_pBehaviorTree->terminate(*this, BH_FAILURE);
				return;
			}

			//ASSERT(child.m_eStatus == BH_SUCCESS);
			if (++m_Current == m_Children.end()) 
			{
				cout << "Sequence: terminating with SUCCESS" << endl << endl;
				m_pBehaviorTree->terminate(*this, BH_SUCCESS);
				return;
			}
			else
			{
				cout << "Sequence: Inserting new child" << endl << endl;
				BehaviorObserver observer;
				observer.bind(this, &Sequence::onChildComplete);
				m_pBehaviorTree->insert((**m_Current), &observer);
			}

		}

		virtual Status update() 
		{
			/*while (true)
			{
			cout << "Sequence: updating current child:" << endl;
			cout << "---------------------------------" << endl; 
			Status s = (*m_Current)->tick();
			cout << endl;

			if (s != BH_SUCCESS)
			{
			return s;
			}

			if (++m_Current == m_Children.end())
			{
			return BH_SUCCESS;
			}
			}*/

			// ASSERT_FAIL(behavior, "Unexpected loop exit.");
			//return BH_INVALID;
			return BH_SUSPENDED;
		}

		Behaviors::iterator m_Current;
	};

	class Selector : public Composite
	{
	public:
		Selector(BehaviorTree& bt) 
		{
			m_pBehaviorTree = &bt;
		}
	protected:
		BehaviorTree* m_pBehaviorTree;

		virtual void onInitialize()
		{
			m_Current = m_Children.begin();
			BehaviorObserver observer;
			observer.bind(this, &Selector::onChildComplete);
			m_pBehaviorTree->insert((**m_Current), &observer);
		}
		
		virtual void onChildComplete() 
		{
			cout << "SELECTOR: Child is complete!" << endl;
			Behavior& child = **m_Current;
			if (child.m_eStatus != BH_FAILURE)
			{
				// cout << "SELECTOR: terminating with " << getStatusName(child.m_eStatus) << endl << endl;
				m_pBehaviorTree->terminate(*this, child.m_eStatus);
				return;
			}

			//ASSERT(child.m_eStatus == BH_FAILURE);
			if (++m_Current == m_Children.end()) 
			{
				cout << "SELECTOR: terminating with FAILURE" << endl << endl;
				m_pBehaviorTree->terminate(*this, BH_FAILURE);
				return;
			}
			else
			{
				cout << "SELECTOR: Inserting new child" << endl << endl;
				BehaviorObserver observer;
				observer.bind(this, &Selector::onChildComplete);
				m_pBehaviorTree->insert((**m_Current), &observer);
			}

		}

		virtual Status update() 
		{
			/*while (true)
			{
				cout << "SELECTOR: updating child -> " << *m_CurrentChild << endl;
				Status s = (*m_CurrentChild)->tick();

				if (s != BH_FAILURE)
				{
					cout << "SELECTOR: finished with " << getStatusName(s) << endl << endl;
					return s;
				}

				if (++m_CurrentChild == m_Children.end())
				{
					cout << "SELECTOR: finished with FAILURE" << endl << endl;
					return BH_FAILURE;
				}
			}*/

			/*ASSERT_FAIL("Unexpected loop exit.");
			return BH_INVALID;*/
			return BH_SUSPENDED;
		}

		Behaviors::iterator m_Current;
	};

	/******************************************************************************/
}
#endif // !defined (BEHAVIOR_H)