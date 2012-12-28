#include <boost/optional.hpp>
#include <gtest/gtest.h>

#include "../../../api/Vector2.h"
#include "../../../api/Commands.h"
#include "../../DecisionMaking.h"

using namespace dms;

class MockDecisionMaking : public DecisionMaking
{
public:
	MockDecisionMaking(): calledTick(0) {}

	int calledTick;

	virtual const Command* tick()
	{
		calledTick++;
		return nullptr;
	}
};

class MockProfileManager : public ProfileManager
{
public:
	MockProfileManager(vector<DecisionMaking*> dms): ProfileManager(dms), m_CalledInit(0), m_CalledTick(0) {}

	virtual int performanceUpdate(DecisionMaking& decisionMaker, int currentWeight)
	{
		return 10;
	}

	virtual void init()
	{
		m_CalledInit++;
		ProfileManager::init();
	}

	virtual void tick()
	{
		m_CalledTick++;
		ProfileManager::tick();
	}

	int m_CalledTick;
	int m_CalledInit;
};

class MockAC : public ActivationCondition
{
public:
	MockAC(): m_Condition(false) {}

	bool m_Condition;

	virtual bool checkCondition() { return m_Condition; }
};

class MockMonitor: public Monitor
{
public:
	MockMonitor(ActivationCondition& ac, ProfileManager& manager): Monitor(ac, manager), m_CalledUpdate(0) {}

	virtual void update()
	{
		m_CalledUpdate++;
		Monitor::update();
	}

	int m_CalledUpdate;
};

TEST(PolymorphTest, Tick)
{
	DecisionMaking* mock;
	mock = new MockDecisionMaking();
	mock->tick();

	EXPECT_EQ(1, ((MockDecisionMaking*)mock)->calledTick);
	delete mock;
}

// Test fixture for sharing the same initialization among tests
class DecisionMakingTest : public testing::Test
{
protected:
	MockDecisionMaking* m_decisionMakingMock;
	MockProfileManager* m_managerMock;
	vector<DecisionMaking*> m_dms;
	MockAC* m_mockAC;
	MockMonitor* m_monitorMock;
	CompositeDecisionMakingSystem* m_cdms;

	virtual void SetUp() {	
		m_decisionMakingMock = new MockDecisionMaking();
		m_dms.push_back(m_decisionMakingMock);

		m_managerMock = new MockProfileManager(m_dms);
		m_mockAC = new MockAC();
		m_monitorMock = new MockMonitor(*m_mockAC, *m_managerMock);

		m_cdms = new CompositeDecisionMakingSystem(*m_managerMock, *m_monitorMock);
	}

	virtual void TearDown() {
		delete m_decisionMakingMock;
		delete m_managerMock;
		delete m_mockAC;
		delete m_monitorMock;
		delete m_cdms;
	}
};

TEST_F(DecisionMakingTest, CDMS_init)
{
	m_cdms->init();

	EXPECT_EQ(1, m_managerMock->m_CalledInit);
}

//TEST_F(DecisionMakingTest, CDMS_tick){
//	m_cdms->tick();
//
//	EXPECT_EQ(1, m_monitorMock->m_CalledUpdate);
//	EXPECT_EQ(1, m_managerMock->m_CalledTick);
//	EXPECT_EQ(1, m_decisionMakingMock->calledTick);
//}