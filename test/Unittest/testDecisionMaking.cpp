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
	MockProfileManager(vector<DecisionMaking*> dms): ProfileManager(dms), m_CalledInit(0), m_CalledTick(0), m_CalledAlert(0) {}

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

	virtual void alert() {
		m_CalledAlert++;
		ProfileManager::alert();
	}

	void addProfile(Profile& profile) {
		m_Profiles.push_back(profile);
	}

	int m_CalledTick;
	int m_CalledInit;
	int m_CalledAlert;
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

TEST_F(DecisionMakingTest, ProfileManagerInit) {
	m_managerMock->init();

	EXPECT_EQ(1, m_managerMock->m_CalledInit);
	EXPECT_TRUE(m_managerMock->getCurrentProfile() != nullptr);
}

TEST_F(DecisionMakingTest, ProfileManagerTick) {
	m_managerMock->tick();

	EXPECT_EQ(1, m_managerMock->m_CalledTick);
	// TODO: extend test after tick function is implemented
}

TEST_F(DecisionMakingTest, ProfileManagerAlert) {
	m_managerMock->init();

	Profile* currentProfile = m_managerMock->getCurrentProfile();
	EXPECT_TRUE(currentProfile != nullptr);
	
	m_managerMock->addProfile(*currentProfile);

	ProfileConnection* connection = currentProfile->getCurrentConnection();
	EXPECT_FALSE(connection->getWeight() == 10);

	// Make sure the weight is correctly updated after an alert
	m_managerMock->alert();
	EXPECT_TRUE(connection->getWeight() == 10);

}

TEST_F(DecisionMakingTest, MonitorUpdate) {
	m_mockAC->m_Condition = false;
	m_monitorMock->update();

	EXPECT_EQ(0, m_managerMock->m_CalledAlert);

	m_mockAC->m_Condition = true;
	m_monitorMock->update();

	EXPECT_EQ(1, m_managerMock->m_CalledAlert);
}

TEST_F(DecisionMakingTest, CDMS_init)
{
	m_cdms->init();

	EXPECT_EQ(1, m_managerMock->m_CalledInit);
}

TEST_F(DecisionMakingTest, CDMS_tick){
	m_cdms->tick();
	
	EXPECT_EQ(1, m_managerMock->m_CalledInit);
	EXPECT_EQ(1, m_managerMock->m_CalledTick);
	EXPECT_EQ(1, m_monitorMock->m_CalledUpdate);
	EXPECT_EQ(1, m_decisionMakingMock->calledTick);
}