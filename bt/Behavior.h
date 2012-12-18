#include <vector>
#include <deque>
#include <iostream>
#include "../tools/Delegate.h"
#include "../tools/Asserts.h"

using namespace std;

namespace bt
{

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

	//typedef Delegate<void, void ()> BehaviorObserver;

	/******************************************************************************/
	/*
	* Base class for actions, conditions and composites.
	* 
	* @author Jonas Hartmann
	* 
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
		//BehaviorObserver m_Observer;
	};

	class BehaviorTree
	{
	public :
		/*void insert(Behavior& bh, BehaviorObserver* observer = NULL) 
		{
		if (observer != NULL)
		{
		bh.m_Observer = observer;
		}
		this->insert(bh);
		}*/

		/**
		* Insert active behavior.
		* 
		* @param bh
		*            - active behavior
		*/
		void insert(Behavior& bh) 
		{
			m_Behaviors.push_front(&bh);
		}

		void terminate(Behavior& bh, Status result) 
		{
			ASSERT(result != BH_RUNNING);
			bh.m_eStatus = result;

			/*if (!bh.m_Observer.empty())
			{
			bh.m_Observer();
			}*/

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
			cout << "BT: Updating current task:" << endl;
			cout << "--------------------------" << endl;
			current->tick();
			cout << endl;
			// Process the observer if the task is terminated.
			/*if (current->m_eStatus != BH_RUNNING && !current->m_Observer.empty())
			{
			// Call the observer to notify the parent
			current->m_Observer();
			}
			// Otherwise drop it into the queue for the next tick().
			else 
			{*/
			m_Behaviors.push_back(current);
			//}
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
			//BehaviorObserver observer = BehaviorObserver::FROM_METHOD(Sequence, onChildComplete);
			//m_pBehaviorTree->insert(**m_Current); //, &observer);
		}

		virtual void onChildComplete() 
		{
			Behavior& child = **m_Current;
			if (child.m_eStatus == BH_FAILURE)
			{
				m_pBehaviorTree->terminate(*this, BH_FAILURE);
			}

			//ASSERT(behavior, child.m_eStatus == BH_SUCCESS);
			if (++m_Current == m_Children.end()) 
			{
				m_pBehaviorTree->terminate(*this, BH_SUCCESS);
			}
			else
			{
				//BehaviorObserver observer = BehaviorObserver::FROM_METHOD(Sequence, onChildComplete);
				m_pBehaviorTree->insert(**m_Current);//, &observer);
			}

		}

		virtual Status update() 
		{
			while (true)
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
			}

			// ASSERT_FAIL(behavior, "Unexpected loop exit.");
			return BH_INVALID;
		}

		Behaviors::iterator m_Current;
	};

	class Selector : public Composite
	{
	protected:
		virtual void onInitialize()
		{
			m_CurrentChild = m_Children.begin();
		}

		virtual Status update() 
		{
			while (true)
			{
				Status s = (*m_CurrentChild)->tick();

				if (s != BH_FAILURE)
				{
					return s;
				}

				if (++m_CurrentChild == m_Children.end())
				{
					return BH_FAILURE;
				}
			}

			// ASSERT_FAIL(behavior, "Unexpected loop exit.");
			return BH_INVALID;
		}

		Behaviors::iterator m_CurrentChild;
	};

	/******************************************************************************/
}

using namespace bt;

class Gun 
{
public:
	Gun(int bullets) { bulletsLeft = bullets; }
	void fire() throw(char*)
	{ 
		if (hasBullets()) 
		{
			cout << "Fire! \n" ;
			bulletsLeft = bulletsLeft - 1; 
		}
		else
		{
			throw("There are no bullets left!");
		}
	}
	bool hasBullets() { return bulletsLeft > 0; }
private:
	int bulletsLeft;
};

class FireGun : public Behavior
{
public:
	FireGun(Gun& gun)
	{ 
		myGun = &gun;
	}

	void onInitialize() 
	{
		cout << "onInitialize(): ";
		debugPrint();
		cout << endl;

		m_eStatus = BH_RUNNING;
	}
	Status update() 
	{
		// Fire once the gun
		try	{
			myGun->fire();
			return BH_SUCCESS;
		} catch (char*) {
			cout << "Catched exception!\n";
			return BH_FAILURE;
		}
	}
	void onTerminate(Status status) 
	{
		cout << "onTerminate: " << status;
		cout << " ";
		debugPrint();
		cout << endl;
	}

private:
	void debugPrint() { cout << "FireGun [" << this << "]"; }

	Gun* myGun;
};

void main()
{
	BehaviorTree* bt = new BehaviorTree;

	Gun gun(6);
	cout << "Gun " << (gun.hasBullets() ? "has" : "doesnt have") << " bullets" << endl;

	Sequence seq(*bt);
	seq.add(new FireGun(gun));
	seq.add(new FireGun(gun));
	seq.add(new FireGun(gun));
	seq.add(new FireGun(gun));
	seq.add(new FireGun(gun));

	bt->insert(seq);

	bt->tick();

	cout << "Gun " << (gun.hasBullets() ? "has" : "doesnt have") << " bullets" << endl;

	system("pause");
}