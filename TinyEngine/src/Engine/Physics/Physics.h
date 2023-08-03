#pragma once

#include "Engine/Core/Ref.h"
#include "Engine/Core/TimeStep.h"
#include "Engine/Scene/Entity.h"

#define PHYSICS_DEBUG 1
#if PHYSICS_DEBUG
#define PHYSICS_TRACE(...) ENGINE_TRACE(__VA_ARGS__)
#define PHYSICS_WARN(...) ENGINE_WARN(__VA_ARGS__)
#else
#define PHYSICS_TRACE(...)
#define PHYSICS_WARN(...)
#endif

namespace Engine
{
	class PhysicsActor;

	enum class ForceMode
	{
		Force = 0,
		Impulse,
		VelocityChange,
		Acceleration,
	};

	enum class FilterGroup
	{
		Static = BIT(0),
		Dynamic = BIT(1),
		Kinematic = BIT(2),
		All = Static | Dynamic | Kinematic
	};

	enum class BroadphaseType
	{
		SweepAndPrune,
		MultiBoxPrune,
		AutomaticBoxPrune
	};

	enum class FrictionType
	{
		Patch,
		OneDirectional,
		TwoDirectional
	};

	struct PhysicsSettings
	{
		float FixedTimestep = 0.02f;
		glm::vec3 Gravity = { 0.0f, -9.81f, 0.0f };
		BroadphaseType BroadphaseAlgorithm = BroadphaseType::AutomaticBoxPrune;
		glm::vec3 WorldBoundsMin = glm::vec3(0.0f);
		glm::vec3 WorldBoundsMax = glm::vec3(1.0f);
		uint32_t WorldBoundsSubdivisions = 2;
		FrictionType FrictionModel = FrictionType::Patch;
		uint32_t SolverIterations = 6;
		uint32_t SolverVelocityIterations = 1;
	};

	struct RaycastHit
	{
		uint64_t EntityID;
		glm::vec3 Position;
		glm::vec3 Normal;
		float Distance;
	};

	class Physics
	{
	public:
		static void Init();
		static void Shutdown();

		static void CreateScene();
		static Ref<PhysicsActor> CreateActor(Entity e);

		static Ref<PhysicsActor> GetActorForEntity(Entity entity);

		static void Simulate(Timestep ts);

		static void DestroyScene();

		static void* GetPhysicsScene();

	public:
		static PhysicsSettings& GetSettings();
	};
}