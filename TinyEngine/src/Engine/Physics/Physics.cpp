#include "pch.h"
#include "Physics.h"
#include "PXPhysicsWrappers.h"
#include "PhysicsLayer.h"
#include "PhysicsActor.h"
#include <PhysX/extensions/PxBroadPhaseExt.h>


namespace Engine
{
    static physx::PxScene* s_Scene;
    static std::vector<Ref<PhysicsActor>> s_Actors;
    static float s_SimulationTime = 0.0F;

    static PhysicsSettings s_Settings;

    void Physics::Init()
    {
        PXPhysicsWrappers::Initialize();
        PhysicsLayerManager::AddLayer("Default");
    }

    void Physics::Shutdown()
    {
        PXPhysicsWrappers::Shutdown();
    }

    void Physics::CreateScene()
    {
        ENGINE_ASSERT(s_Scene == nullptr, "Scene already has a Physics Scene!");
        s_Scene = PXPhysicsWrappers::CreateScene();

        if (s_Settings.BroadphaseAlgorithm != BroadphaseType::AutomaticBoxPrune)
        {
            physx::PxBounds3* regionBounds;
            physx::PxBounds3 globalBounds(ToPhysXVector(s_Settings.WorldBoundsMin), ToPhysXVector(s_Settings.WorldBoundsMax));
            uint32_t regionCount = physx::PxBroadPhaseExt::createRegionsFromWorldBounds(regionBounds, globalBounds, s_Settings.WorldBoundsSubdivisions);

            for (uint32_t i = 0; i < regionCount; i++)
            {
                physx::PxBroadPhaseRegion region;
                region.bounds = regionBounds[i];
                s_Scene->addBroadPhaseRegion(region);
            }
        }

        PHYSICS_TRACE("Create physics scene");
    }

    Ref<PhysicsActor> Physics::CreateActor(Entity e)
    {
        ENGINE_ASSERT(s_Scene, "");
        Ref<PhysicsActor> actor = CreateRef<PhysicsActor>(e);
        s_Actors.push_back(actor);
        actor->Spawn();
        return actor;
    }

    Ref<PhysicsActor> Physics::GetActorForEntity(Entity entity)
    {
        for (auto& actor : s_Actors)
        {
            if (actor->GetEntity() == entity)
                return actor;
        }
        return nullptr;
    }

    void Physics::Simulate(Timestep ts)
    {
        //更新物理模拟时间，达到更新步长时进行物理计算
        s_SimulationTime += ts.GetMilliseconds();
        if (s_SimulationTime < s_Settings.FixedTimestep)
            return;
        s_SimulationTime -= s_Settings.FixedTimestep;

        //物理计算
        for (auto& actor : s_Actors)
            actor->Update(s_Settings.FixedTimestep);
        s_Scene->simulate(s_Settings.FixedTimestep);
        s_Scene->fetchResults(true);

        //完成更新后同步各个物体的transform属性
        for (auto& actor : s_Actors)
            actor->SynchronizeTransform();
    }
    
    void Physics::DestroyScene()
    {
        ENGINE_ASSERT(s_Scene,"");

        s_Actors.clear();
        s_Scene->release();
        s_Scene = nullptr;

        PHYSICS_TRACE("Destroy physics scene");
    }
    
    void* Physics::GetPhysicsScene()
    {
        return s_Scene;
    }

    PhysicsSettings& Physics::GetSettings()
    {
        return s_Settings;
    }
}