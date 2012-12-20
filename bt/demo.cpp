//#include <iostream>
//#include "Behavior.h"
//
//using namespace std;
//using namespace bt;
//
///*
// * Model class
// */
//class Gun 
//{
//public:
//	Gun(int bullets) { bulletsLeft = bullets; }
//	void fire() throw(char*)
//	{ 
//		if (hasBullets()) 
//		{
//			cout << "Fire! \n" ;
//			bulletsLeft = bulletsLeft - 1; 
//		}
//		else
//		{
//			throw("There are no bullets left!");
//		}
//	}
//	bool hasBullets() { return bulletsLeft > 0; }
//	void setBulletsLeft(int bullets) { bulletsLeft = bullets; }
//private:
//	int bulletsLeft;
//};
//
//class FireGun : public Behavior
//{
//public:
//	FireGun(Gun& gun)
//	{ 
//		myGun = &gun;
//	}
//
//	void onInitialize() 
//	{
//		cout << "onInitialize(): ";
//		debugPrint();
//		cout << endl;
//
//		m_eStatus = BH_RUNNING;
//	}
//	Status update() 
//	{
//		// Fire once the gun
//		try	{
//			myGun->fire();
//			return BH_SUCCESS;
//		} catch (char*) {
//			cout << "Catched exception!\n";
//			return BH_FAILURE;
//		}
//	}
//	void onTerminate(Status status) 
//	{
//		debugPrint();
//		cout << ": terminating with status " << getStatusName(status);
//		cout << endl << endl;
//	}
//
//private:
//	void debugPrint() { cout << "FireGun [" << this << "]"; }
//
//	Gun* myGun;
//};
//
//class ReloadGun : public Behavior
//{
//public:
//	ReloadGun(Gun& gun)
//	{
//		myGun = &gun;
//	}
//
//	Status update()
//	{
//		cout << endl;
//		debugPrint();
//		cout << ": reloading" << endl << endl;
//		myGun->setBulletsLeft(6);
//		return BH_SUCCESS;
//	}
//
//private:
//	void debugPrint() { cout << "ReloadGun [" << this << "]"; }
//
//	Gun* myGun;
//};
//
//Selector *root;
//
//void OnBehaviorFinished()
//{
//	cout << endl << "FINISHED EXECUTION OF BEHAVIOR TREE. RESULT: " << getStatusName(root->m_eStatus) << endl << endl;
//}
//
//void demo()
//{
//	BehaviorTree bt;
//
//	Gun gun(6);
//	cout << "Gun " << (gun.hasBullets() ? "has" : "doesnt have") << " bullets" << endl;
//
//	Sequence seq(bt);
//	for (int i = 0; i < 7; i++)
//	{
//		FireGun f(gun);
//		seq.add(&f);
//	}
//	
//	Selector sel(bt);
//	sel.add(&seq);
//	
//	ReloadGun reload(gun);
//	sel.add(&reload);
//
//	BehaviorObserver observer;
//	observer.bind(&OnBehaviorFinished);
//	bt.insert(sel, &observer);
//
//	root = &sel;
//
//	/* Starts the magic */
//	bt.tick();
//
//	cout << "Gun " << (gun.hasBullets() ? "has" : "doesnt have") << " bullets" << endl;
//}