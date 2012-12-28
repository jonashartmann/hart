#include "../../bt/Behavior.h"
#include "gtest/gtest.h"

using namespace bt;

class MockBehavior: public Behavior {
public:
	MockBehavior(): Behavior() {
		m_CalledInit = false;
		m_CalledTerminate = false;
		m_CalledUpdate = false;
		m_CalledObserved = false;
		m_ReturnStatus = BH_SUCCESS;
	}

	virtual Status update() {
		m_CalledUpdate = true;
		return m_ReturnStatus;
	}

	virtual void onInitialize()	{
		m_CalledInit = true;
	}
	virtual void onTerminate(Status) {
		m_CalledTerminate = true;
	}

	void onObserved() {
		m_CalledObserved = true;
	}

	bool m_CalledInit;
	bool m_CalledTerminate;
	bool m_CalledUpdate;
	bool m_CalledObserved;
	Status m_ReturnStatus;
};

class MockSequence : public Sequence {
public:
	MockSequence(BehaviorTree& bt): Sequence(bt) {}
};

TEST(BehaviorTest, Constructor) {
	MockBehavior mock;
	EXPECT_EQ(BH_INVALID, mock.m_eStatus);
}

TEST(BehaviorTest, Tick) {
	MockBehavior mock;

	EXPECT_EQ(BH_SUCCESS, mock.tick());
	EXPECT_TRUE(mock.m_CalledInit);
	EXPECT_TRUE(mock.m_CalledUpdate);
	EXPECT_TRUE(mock.m_CalledTerminate);

	mock.m_ReturnStatus = BH_RUNNING;
	mock.m_CalledInit = false;
	mock.m_CalledTerminate = false;
	mock.m_CalledUpdate = false;
	
	EXPECT_EQ(BH_RUNNING, mock.tick());
	EXPECT_FALSE(mock.m_CalledInit);
	EXPECT_TRUE(mock.m_CalledUpdate);
	EXPECT_FALSE(mock.m_CalledTerminate);
}

TEST(BehaviorTreeTest, Insert){
	BehaviorTree bt;
	MockBehavior mock;
	bt.insert(mock);

	EXPECT_EQ(BH_INVALID, mock.m_eStatus);
}

TEST(BehaviorTreeTest, InsertWithObserver){
	BehaviorTree bt;
	MockBehavior mock;
	BehaviorObserver obs;
	obs.bind(&mock, &MockBehavior::onObserved);
	bt.insert(mock, &obs);

	EXPECT_EQ(BH_INVALID, mock.m_eStatus);
	EXPECT_FALSE(mock.m_Observer.empty());
}

TEST(BehaviorTreeTest, TickEmpty) {
	BehaviorTree bt;
	bt.tick();
}

TEST(BehaviorTreeTest, Tick) {
	BehaviorTree bt;
	MockBehavior mock;
	BehaviorObserver obs;
	obs.bind(&mock, &MockBehavior::onObserved);
	bt.insert(mock, &obs);

	// The first tick should initialize a behavior and terminate it after successful update
	bt.tick();

	EXPECT_TRUE(mock.m_eStatus == BH_SUCCESS);
	EXPECT_TRUE(mock.m_CalledInit);
	EXPECT_TRUE(mock.m_CalledUpdate);
	EXPECT_TRUE(mock.m_CalledTerminate);
	EXPECT_TRUE(mock.m_CalledObserved);

	// Another tick when the behavior is suspended, should not update nor terminate the behavior
	mock.m_eStatus = BH_SUSPENDED;
	mock.m_CalledInit = true;
	mock.m_CalledUpdate = false;
	mock.m_CalledTerminate = false;
	mock.m_CalledObserved = false;
	mock.m_ReturnStatus = BH_FAILURE;
	bt.insert(mock, &obs);

	bt.tick();

	EXPECT_TRUE(mock.m_eStatus == BH_SUSPENDED);
	EXPECT_TRUE(mock.m_CalledInit);
	EXPECT_FALSE(mock.m_CalledUpdate);
	EXPECT_FALSE(mock.m_CalledTerminate);
	EXPECT_TRUE(mock.m_CalledObserved);
}

class CompositeTest : public testing::Test
{
protected:
	BehaviorTree bt;
	Selector* selector;
	Sequence* sequence;

	virtual void SetUp() {
		selector = new Selector(bt);
		ASSERT_EQ(BH_INVALID, selector->m_eStatus);

		sequence = new Sequence(bt);
		ASSERT_EQ(BH_INVALID, sequence->m_eStatus);
	}

	virtual void TearDown() {
		delete selector;
		delete sequence;
	}
public:
	void OnBehaviorFinished()
	{
		// TODO: assert end status
	}
};

TEST_F(CompositeTest, Selector) {
	MockBehavior bhMockTrue;
	MockBehavior bhMockFalse;
	bhMockFalse.m_ReturnStatus = BH_FAILURE;
	MockBehavior bhMockTrue2;

	selector->add(&bhMockFalse);
	selector->add(&bhMockTrue);
	selector->add(&bhMockTrue2);

	BehaviorObserver obs(this, &CompositeTest::OnBehaviorFinished);
	bt.insert(*selector, &obs);

	bt.tick();

	EXPECT_EQ(BH_FAILURE, bhMockFalse.m_eStatus);
	EXPECT_TRUE(bhMockFalse.m_CalledInit);
	EXPECT_TRUE(bhMockFalse.m_CalledUpdate);
	EXPECT_TRUE(bhMockFalse.m_CalledTerminate);

	EXPECT_EQ(BH_SUCCESS, bhMockTrue.m_eStatus);
	EXPECT_TRUE(bhMockTrue.m_CalledInit);
	EXPECT_TRUE(bhMockTrue.m_CalledUpdate);
	EXPECT_TRUE(bhMockTrue.m_CalledTerminate);
	
	EXPECT_EQ(BH_INVALID, bhMockTrue2.m_eStatus);
	EXPECT_FALSE(bhMockTrue2.m_CalledInit);
	EXPECT_FALSE(bhMockTrue2.m_CalledUpdate);
	EXPECT_FALSE(bhMockTrue2.m_CalledTerminate);

	EXPECT_EQ(BH_SUCCESS, selector->m_eStatus);
}

TEST_F(CompositeTest, Sequence) {
	MockBehavior bhMockTrue;
	MockBehavior bhMockFalse;
	bhMockFalse.m_ReturnStatus = BH_FAILURE;
	MockBehavior bhMockTrue2;

	sequence->add(&bhMockTrue);
	sequence->add(&bhMockFalse);
	sequence->add(&bhMockTrue2);

	BehaviorObserver obs(this, &CompositeTest::OnBehaviorFinished);
	bt.insert(*sequence, &obs);

	bt.tick();

	EXPECT_EQ(BH_SUCCESS, bhMockTrue.m_eStatus);
	EXPECT_EQ(BH_FAILURE, bhMockFalse.m_eStatus);
	EXPECT_EQ(BH_INVALID, bhMockTrue2.m_eStatus);

	EXPECT_EQ(BH_FAILURE, sequence->m_eStatus);
}
//void demo()
//{
//	BehaviorTree bt;
//
//	Gun gun(6);
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
//}