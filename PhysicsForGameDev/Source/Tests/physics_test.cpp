#include <gtest/gtest.h>

#include "physics_system.h"

static bool ListenerCalled = false;

static void UpdateListener(void* pObject, const glm::vec3& Pos)
{
	ListenerCalled = true;
}


TEST(TestCaseName, TestName) 
{
	Physics::System PhysicsSystem;

	PhysicsSystem.Init(10, UpdateListener);

	int foo;

	Physics::PointMass* pm = PhysicsSystem.AllocPointMass();

	glm::vec3 Force(1.0f, 0.0f, 0.0f);
	pm->Init(1.0f, glm::vec3(0.0f), Force, &foo);

	PhysicsSystem.Update(1);
	
	EXPECT_TRUE(ListenerCalled);
}



int main(int argc, char** argv) 
{
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}