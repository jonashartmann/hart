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
	class Behavior 
	{
	public:
		Behavior() {
			m_eStatus = BH_INVALID;
		}

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

		virtual Status update()				= 0;

		virtual void onInitialize()			{}
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

		void tick() 
		{
			m_Behaviors.push_back(NULL);

			while (step()) 
			{
				continue;
			}
		}

		bool step() 
		{
			Behavior* current = m_Behaviors.front();
			m_Behaviors.pop_front();

			if (current == NULL) {
				return false;
			}

			current->tick();

			/*if (current->m_eStatus != BH_RUNNING && !current->m_Observer.empty())
			{
			current->m_Observer();
			}
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
			m_pBehaviorTree->insert(**m_Current); //, &observer);
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
				Status s = (*m_Current)->tick();

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

//using namespace bt;
//class FireGun : public Behavior
//{
//public:
//	void onInitialize() 
//	{
//		cout << "onInitialize() \n";
//		bulletsLeft = 6;
//		m_eStatus = BH_RUNNING;
//	}
//
//	Status update() 
//	{
//		// Fire once
//		if (bulletsLeft > 0)
//		{
//			cout << "Fire! " ;
//			bulletsLeft = bulletsLeft - 1;
//			cout << "Bullets left: " << bulletsLeft << "\n";
//			return BH_RUNNING;
//		}
//		else 
//		{
//			cout << "No bullets left! \n";
//			return BH_FAILURE;
//		}
//	}
//
//	void onTerminate(Status status) 
//	{
//		cout << "onTerminate: " << status << "\n";
//	}
//
//private:
//	int bulletsLeft;
//};
//
//void main()
//{
//	BehaviorTree* bt = new BehaviorTree;
//	FireGun* fireGun = new FireGun();
//
//	bt->insert(*fireGun);
//	bt->insert(*fireGun);
//	bt->insert(*fireGun);
//	bt->insert(*fireGun);
//	bt->insert(*fireGun);
//	bt->insert(*fireGun);
//	bt->insert(*fireGun);
//
//	bt->tick();
//
//	system("pause");
//}