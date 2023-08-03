#include "pch.h"
#include "ScriptWrappers.h"

#include "Engine/Scene/Scene.h"
#include "Engine/Scene/Entity.h"
#include "Engine/Physics/PhysicsUtil.h"
#include "Engine/Physics/PhysicsActor.h"
#include "Engine/Physics/PXPhysicsWrappers.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/common.hpp>

#include <mono/jit/jit.h>

#include <PhysX/PxPhysicsAPI.h>

namespace Engine {
	extern std::unordered_map<MonoType*, std::function<bool(Entity&)>> s_HasComponentFuncs;
	extern std::unordered_map<MonoType*, std::function<void(Entity&)>> s_CreateComponentFuncs;
}

namespace Engine
{
namespace Script
{
	//-------------------------------------------------------------------------------------
	// Input
	//-------------------------------------------------------------------------------------
	bool Engine::Script::Engine_Input_IsKeyPressed(int key)
	{
		return Input::IsKeyPressed(key);
	}

	bool Engine::Script::Engine_Input_IsMouseButtonPressed(int button)
	{
		return Input::IsMouseButtonPressed(button);
	}

	void Engine::Script::Engine_Input_GetMousePosition(glm::vec2* outPosition)
	{
		auto [x, y] = Input::GetMousePosition();
		*outPosition = { x, y };
	}

	void Engine::Script::Engine_Input_SetCursorMode(CursorMode mode)
	{
		Input::SetCursorMode(mode);
	}

	CursorMode Engine::Script::Engine_Input_GetCursorMode()
	{
		return Input::GetCursorMode();
	}

	//-------------------------------------------------------------------------------------
	// Physics
	//-------------------------------------------------------------------------------------
	bool Engine::Script::Engine_Physics_Raycast(glm::vec3* origin, glm::vec3* direction, float maxDistance, RaycastHit* hit)
	{
		return PXPhysicsWrappers::Raycast(*origin, *direction, maxDistance, hit);
	}

	// Helper function for the Overlap functions below
	static void AddCollidersToArray(MonoArray* array, const std::array<physx::PxOverlapHit, OVERLAP_MAX_COLLIDERS>& hits, uint32_t count, uint32_t arrayLength)
	{
		uint32_t arrayIndex = 0;
		for (uint32_t i = 0; i < count; i++)
		{
			Entity& entity = *(Entity*)hits[i].actor->userData;

			if (entity.HasComponent<BoxColliderComponent>() && arrayIndex < arrayLength)
			{
				auto& boxCollider = entity.GetComponent<BoxColliderComponent>();

				void* data[] = {
					&entity.ID(),
					&boxCollider.IsTrigger,
					&boxCollider.Size,
					&boxCollider.Offset
				};

				MonoObject* obj = ScriptEngine::Construct("Engine.BoxCollider:.ctor(ulong,bool,Vector3,Vector3)", true, data);
				mono_array_set(array, MonoObject*, arrayIndex++, obj);
			}

			if (entity.HasComponent<SphereColliderComponent>() && arrayIndex < arrayLength)
			{
				auto& sphereCollider = entity.GetComponent<SphereColliderComponent>();

				void* data[] = {
					&entity.ID(),
					&sphereCollider.IsTrigger,
					&sphereCollider.Radius
				};

				MonoObject* obj = ScriptEngine::Construct("Engine.SphereCollider:.ctor(ulong,bool,float)", true, data);
				mono_array_set(array, MonoObject*, arrayIndex++, obj);
			}

			if (entity.HasComponent<CapsuleColliderComponent>() && arrayIndex < arrayLength)
			{
				auto& capsuleCollider = entity.GetComponent<CapsuleColliderComponent>();

				void* data[] = {
					&entity.ID(),
					&capsuleCollider.IsTrigger,
					&capsuleCollider.Radius,
					&capsuleCollider.Height
				};

				MonoObject* obj = ScriptEngine::Construct("Engine.CapsuleCollider:.ctor(ulong,bool,float,float)", true, data);
				mono_array_set(array, MonoObject*, arrayIndex++, obj);
			}

			if (entity.HasComponent<MeshColliderComponent>() && arrayIndex < arrayLength)
			{
				auto& meshCollider = entity.GetComponent<MeshColliderComponent>();

				Ref<Mesh>* mesh = new Ref<Mesh>(meshCollider.CollisionMesh);
				void* data[] = {
					&entity.ID(),
					&meshCollider.IsTrigger,
					&mesh
				};

				MonoObject* obj = ScriptEngine::Construct("Engine.MeshCollider:.ctor(ulong,bool,intptr)", true, data);
				mono_array_set(array, MonoObject*, arrayIndex++, obj);
			}
		}
	}

	static std::array<physx::PxOverlapHit, OVERLAP_MAX_COLLIDERS> s_OverlapBuffer;


	MonoArray* Engine::Script::Engine_Physics_OverlapBox(glm::vec3* origin, glm::vec3* halfSize)
	{
		MonoArray* outColliders = nullptr;
		memset(s_OverlapBuffer.data(), 0, OVERLAP_MAX_COLLIDERS * sizeof(physx::PxOverlapHit));

		uint32_t count;
		if (PXPhysicsWrappers::OverlapBox(*origin, *halfSize, s_OverlapBuffer, &count))
		{
			outColliders = mono_array_new(mono_domain_get(), ScriptEngine::GetCoreClass("Engine.Collider"), count);
			AddCollidersToArray(outColliders, s_OverlapBuffer, count, count);
		}

		return outColliders;
	}

	MonoArray* Engine::Script::Engine_Physics_OverlapCapsule(glm::vec3* origin, float radius, float halfHeight)
	{
		MonoArray* outColliders = nullptr;
		memset(s_OverlapBuffer.data(), 0, OVERLAP_MAX_COLLIDERS * sizeof(physx::PxOverlapHit));

		uint32_t count;
		if (PXPhysicsWrappers::OverlapCapsule(*origin, radius, halfHeight, s_OverlapBuffer, &count))
		{
			outColliders = mono_array_new(mono_domain_get(), ScriptEngine::GetCoreClass("Engine.Collider"), count);
			AddCollidersToArray(outColliders, s_OverlapBuffer, count, count);
		}

		return outColliders;
	}

	MonoArray* Engine::Script::Engine_Physics_OverlapSphere(glm::vec3* origin, float radius)
	{
		MonoArray* outColliders = nullptr;
		memset(s_OverlapBuffer.data(), 0, OVERLAP_MAX_COLLIDERS * sizeof(physx::PxOverlapHit));

		uint32_t count;
		if (PXPhysicsWrappers::OverlapSphere(*origin, radius, s_OverlapBuffer, &count))
		{
			outColliders = mono_array_new(mono_domain_get(), ScriptEngine::GetCoreClass("Engine.Collider"), count);
			AddCollidersToArray(outColliders, s_OverlapBuffer, count, count);
		}

		return outColliders;
	}

	int32_t Engine::Script::Engine_Physics_OverlapBoxNonAlloc(glm::vec3* origin, glm::vec3* halfSize, MonoArray* outColliders)
	{
		memset(s_OverlapBuffer.data(), 0, OVERLAP_MAX_COLLIDERS * sizeof(physx::PxOverlapHit));

		uint64_t arrayLength = mono_array_length(outColliders);

		uint32_t count;
		if (PXPhysicsWrappers::OverlapBox(*origin, *halfSize, s_OverlapBuffer, &count))
		{
			if (count > arrayLength)
				count = arrayLength;

			AddCollidersToArray(outColliders, s_OverlapBuffer, count, arrayLength);
		}

		return count;
	}

	int32_t Engine::Script::Engine_Physics_OverlapCapsuleNonAlloc(glm::vec3* origin, float radius, float halfHeight, MonoArray* outColliders)
	{
		memset(s_OverlapBuffer.data(), 0, OVERLAP_MAX_COLLIDERS * sizeof(physx::PxOverlapHit));

		uint64_t arrayLength = mono_array_length(outColliders);
		uint32_t count;
		if (PXPhysicsWrappers::OverlapCapsule(*origin, radius, halfHeight, s_OverlapBuffer, &count))
		{
			if (count > arrayLength)
				count = arrayLength;

			AddCollidersToArray(outColliders, s_OverlapBuffer, count, arrayLength);
		}

		return count;
	}

	int32_t Engine::Script::Engine_Physics_OverlapSphereNonAlloc(glm::vec3* origin, float radius, MonoArray* outColliders)
	{
		memset(s_OverlapBuffer.data(), 0, OVERLAP_MAX_COLLIDERS * sizeof(physx::PxOverlapHit));

		uint64_t arrayLength = mono_array_length(outColliders);

		uint32_t count;
		if (PXPhysicsWrappers::OverlapSphere(*origin, radius, s_OverlapBuffer, &count))
		{
			if (count > arrayLength)
				count = arrayLength;

			AddCollidersToArray(outColliders, s_OverlapBuffer, count, arrayLength);
		}

		return count;
	}

	//-------------------------------------------------------------------------------------
	// Entiny
	//-------------------------------------------------------------------------------------
	void Engine::Script::Engine_Entity_CreateComponent(uint64_t entityID, void* type)
	{
		Ref<Scene> scene = ScriptEngine::GetCurrentSceneContext();
		ENGINE_ASSERT(scene, "No active scene!");
		const auto& entityMap = scene->GetEntityMap();
		ENGINE_ASSERT(entityMap.find(entityID) != entityMap.end(), "Invalid entity ID or entity doesn't exist in scene!");

		Entity entity = entityMap.at(entityID);
		MonoType* monoType = mono_reflection_type_get_type((MonoReflectionType*)type);
		s_CreateComponentFuncs[monoType](entity);
	}

	bool Engine::Script::Engine_Entity_HasComponent(uint64_t entityID, void* type)
	{
		Ref<Scene> scene = ScriptEngine::GetCurrentSceneContext();
		ENGINE_ASSERT(scene, "No active scene!");
		const auto& entityMap = scene->GetEntityMap();
		ENGINE_ASSERT(entityMap.find(entityID) != entityMap.end(), "Invalid entity ID or entity doesn't exist in scene!");

		Entity entity = entityMap.at(entityID);
		MonoType* monoType = mono_reflection_type_get_type((MonoReflectionType*)type);
		bool result = s_HasComponentFuncs[monoType](entity);
		return result;
	}

	uint64_t Engine::Script::Engine_Entity_FindEntityByTag(MonoString* tag)
	{
		Ref<Scene> scene = ScriptEngine::GetCurrentSceneContext();
		ENGINE_ASSERT(scene, "No active scene!");

		Entity entity = scene->TryGetEntityWithTag(mono_string_to_utf8(tag));
		if (entity)
			return entity.GetComponent<IDComponent>().ID;

		return 0;
	}

	void Engine::Script::Engine_TransformComponent_GetTransform(uint64_t entityID, ScriptTransform* outTransform)
	{
		Ref<Scene> scene = ScriptEngine::GetCurrentSceneContext();
		ENGINE_ASSERT(scene, "No active scene!");
		const auto& entityMap = scene->GetEntityMap();
		ENGINE_ASSERT(entityMap.find(entityID) != entityMap.end(), "Invalid entity ID or entity doesn't exist in scene!");

		Entity entity = entityMap.at(entityID);
		TransformComponent& transform = entity.GetComponent<TransformComponent>();

		glm::quat rotation = glm::quat(transform.Rotation);
		glm::vec3 right = glm::normalize(glm::rotate(rotation, glm::vec3(1.0F, 0.0F, 0.0F)));
		glm::vec3 up = glm::normalize(glm::rotate(rotation, glm::vec3(0.0F, 1.0F, 0.0F)));
		glm::vec3 forward = glm::normalize(glm::rotate(rotation, glm::vec3(0.0F, 0.0F, -1.0F)));

		*outTransform = {
			transform.Translation, glm::degrees(transform.Rotation), transform.Scale,
			up, right, forward
		};
	}

	void Engine::Script::Engine_TransformComponent_SetTransform(uint64_t entityID, ScriptTransform* inTransform)
	{
		Ref<Scene> scene = ScriptEngine::GetCurrentSceneContext();
		ENGINE_ASSERT(scene, "No active scene!");
		const auto& entityMap = scene->GetEntityMap();
		ENGINE_ASSERT(entityMap.find(entityID) != entityMap.end(), "Invalid entity ID or entity doesn't exist in scene!");

		Entity entity = entityMap.at(entityID);
		TransformComponent& transform = entity.GetComponent<TransformComponent>();
		transform.Translation = inTransform->Translation;
		transform.Rotation = glm::radians(inTransform->Rotation);
		transform.Scale = inTransform->Scale;
	}

	void* Engine::Script::Engine_MeshComponent_GetMesh(uint64_t entityID)
	{
		Ref<Scene> scene = ScriptEngine::GetCurrentSceneContext();
		ENGINE_ASSERT(scene, "No active scene!");
		const auto& entityMap = scene->GetEntityMap();
		ENGINE_ASSERT(entityMap.find(entityID) != entityMap.end(), "Invalid entity ID or entity doesn't exist in scene!");

		Entity entity = entityMap.at(entityID);
		auto& meshComponent = entity.GetComponent<MeshComponent>();
		return new Ref<Mesh>(meshComponent.Mesh);
	}

	void Engine::Script::Engine_MeshComponent_SetMesh(uint64_t entityID, Ref<Mesh>* inMesh)
	{
		Ref<Scene> scene = ScriptEngine::GetCurrentSceneContext();
		ENGINE_ASSERT(scene, "No active scene!");
		const auto& entityMap = scene->GetEntityMap();
		ENGINE_ASSERT(entityMap.find(entityID) != entityMap.end(), "Invalid entity ID or entity doesn't exist in scene!");

		Entity entity = entityMap.at(entityID);
		auto& meshComponent = entity.GetComponent<MeshComponent>();
		meshComponent.Mesh = inMesh ? *inMesh : nullptr;
	}

	Engine::RigidBodyComponent::Type Engine::Script::Engine_RigidBodyComponent_GetBodyType(uint64_t entityID)
	{
		return RigidBodyComponent::Type();
	}

	void Engine::Script::Engine_RigidBodyComponent_AddForce(uint64_t entityID, glm::vec3* force, ForceMode forceMode)
	{
		Ref<Scene> scene = ScriptEngine::GetCurrentSceneContext();
		ENGINE_ASSERT(scene, "No active scene!");
		const auto& entityMap = scene->GetEntityMap();
		ENGINE_ASSERT(entityMap.find(entityID) != entityMap.end(), "Invalid entity ID or entity doesn't exist in scene!");

		Entity entity = entityMap.at(entityID);
		ENGINE_ASSERT(entity.HasComponent<RigidBodyComponent>(),"");
		auto& component = entity.GetComponent<RigidBodyComponent>();

		if (component.IsKinematic)
		{
			ENGINE_WARN("Cannot add a force to a kinematic actor! EntityID({0})", entityID);
			return;
		}

		Ref<PhysicsActor> actor = Physics::GetActorForEntity(entity);
		actor->AddForce(*force, forceMode);
	}

	void Engine::Script::Engine_RigidBodyComponent_AddTorque(uint64_t entityID, glm::vec3* torque, ForceMode forceMode)
	{
		Ref<Scene> scene = ScriptEngine::GetCurrentSceneContext();
		ENGINE_ASSERT(scene, "No active scene!");
		const auto& entityMap = scene->GetEntityMap();
		ENGINE_ASSERT(entityMap.find(entityID) != entityMap.end(), "Invalid entity ID or entity doesn't exist in scene!");

		Entity entity = entityMap.at(entityID);
		ENGINE_ASSERT(entity.HasComponent<RigidBodyComponent>(),"");
		auto& component = entity.GetComponent<RigidBodyComponent>();

		if (component.IsKinematic)
		{
			ENGINE_WARN("Cannot add torque to a kinematic actor! EntityID({0})", entityID);
			return;
		}

		Ref<PhysicsActor> actor = Physics::GetActorForEntity(entity);
		actor->AddTorque(*torque, forceMode);
	}

	void Engine::Script::Engine_RigidBodyComponent_GetLinearVelocity(uint64_t entityID, glm::vec3* outVelocity)
	{
		Ref<Scene> scene = ScriptEngine::GetCurrentSceneContext();
		ENGINE_ASSERT(scene, "No active scene!");
		const auto& entityMap = scene->GetEntityMap();
		ENGINE_ASSERT(entityMap.find(entityID) != entityMap.end(), "Invalid entity ID or entity doesn't exist in scene!");

		Entity entity = entityMap.at(entityID);
		ENGINE_ASSERT(entity.HasComponent<RigidBodyComponent>(), "");
		auto& component = entity.GetComponent<RigidBodyComponent>();
		ENGINE_ASSERT(outVelocity, "");
		Ref<PhysicsActor> actor = Physics::GetActorForEntity(entity);
		*outVelocity = actor->GetLinearVelocity();
	}

	void Engine::Script::Engine_RigidBodyComponent_SetLinearVelocity(uint64_t entityID, glm::vec3* velocity)
	{
		Ref<Scene> scene = ScriptEngine::GetCurrentSceneContext();
		ENGINE_ASSERT(scene, "No active scene!");
		const auto& entityMap = scene->GetEntityMap();
		ENGINE_ASSERT(entityMap.find(entityID) != entityMap.end(), "Invalid entity ID or entity doesn't exist in scene!");

		Entity entity = entityMap.at(entityID);
		ENGINE_ASSERT(entity.HasComponent<RigidBodyComponent>(), "");
		auto& component = entity.GetComponent<RigidBodyComponent>();
		ENGINE_ASSERT(velocity, "");
		Ref<PhysicsActor> actor = Physics::GetActorForEntity(entity);
		actor->SetLinearVelocity(*velocity);
	}

	void Engine::Script::Engine_RigidBodyComponent_GetAngularVelocity(uint64_t entityID, glm::vec3* outVelocity)
	{
		Ref<Scene> scene = ScriptEngine::GetCurrentSceneContext();
		ENGINE_ASSERT(scene, "No active scene!");
		const auto& entityMap = scene->GetEntityMap();
		ENGINE_ASSERT(entityMap.find(entityID) != entityMap.end(), "Invalid entity ID or entity doesn't exist in scene!");

		Entity entity = entityMap.at(entityID);
		ENGINE_ASSERT(entity.HasComponent<RigidBodyComponent>(), "");
		auto& component = entity.GetComponent<RigidBodyComponent>();
		ENGINE_ASSERT(outVelocity, "");
		Ref<PhysicsActor> actor = Physics::GetActorForEntity(entity);
		*outVelocity = actor->GetAngularVelocity();
	}

	void Engine::Script::Engine_RigidBodyComponent_SetAngularVelocity(uint64_t entityID, glm::vec3* velocity)
	{
		Ref<Scene> scene = ScriptEngine::GetCurrentSceneContext();
		ENGINE_ASSERT(scene, "No active scene!");
		const auto& entityMap = scene->GetEntityMap();
		ENGINE_ASSERT(entityMap.find(entityID) != entityMap.end(), "Invalid entity ID or entity doesn't exist in scene!");

		Entity entity = entityMap.at(entityID);
		ENGINE_ASSERT(entity.HasComponent<RigidBodyComponent>(), "");
		auto& component = entity.GetComponent<RigidBodyComponent>();
		ENGINE_ASSERT(velocity, "");
		Ref<PhysicsActor> actor = Physics::GetActorForEntity(entity);
		actor->SetAngularVelocity(*velocity);
	}

	void Engine::Script::Engine_RigidBodyComponent_Rotate(uint64_t entityID, glm::vec3* rotation)
	{
		Ref<Scene> scene = ScriptEngine::GetCurrentSceneContext();
		ENGINE_ASSERT(scene, "No active scene!");
		const auto& entityMap = scene->GetEntityMap();
		ENGINE_ASSERT(entityMap.find(entityID) != entityMap.end(), "Invalid entity ID or entity doesn't exist in scene!");

		Entity entity = entityMap.at(entityID);
		ENGINE_ASSERT(entity.HasComponent<RigidBodyComponent>(), "");
		auto& component = entity.GetComponent<RigidBodyComponent>();
		ENGINE_ASSERT(rotation, "");
		Ref<PhysicsActor> actor = Physics::GetActorForEntity(entity);
		actor->Rotate(*rotation);
	}

	uint32_t Engine::Script::Engine_RigidBodyComponent_GetLayer(uint64_t entityID)
	{
		Ref<Scene> scene = ScriptEngine::GetCurrentSceneContext();
		ENGINE_ASSERT(scene, "No active scene!");
		const auto& entityMap = scene->GetEntityMap();
		ENGINE_ASSERT(entityMap.find(entityID) != entityMap.end(), "Invalid entity ID or entity doesn't exist in scene!");

		Entity entity = entityMap.at(entityID);
		ENGINE_ASSERT(entity.HasComponent<RigidBodyComponent>(), "");
		auto& component = entity.GetComponent<RigidBodyComponent>();
		return component.Layer;
	}

	float Engine::Script::Engine_RigidBodyComponent_GetMass(uint64_t entityID)
	{
		Ref<Scene> scene = ScriptEngine::GetCurrentSceneContext();
		ENGINE_ASSERT(scene, "No active scene!");
		const auto& entityMap = scene->GetEntityMap();
		ENGINE_ASSERT(entityMap.find(entityID) != entityMap.end(), "Invalid entity ID or entity doesn't exist in scene!");

		Entity entity = entityMap.at(entityID);
		ENGINE_ASSERT(entity.HasComponent<RigidBodyComponent>(),"");
		auto& component = entity.GetComponent<RigidBodyComponent>();
		Ref<PhysicsActor>& actor = Physics::GetActorForEntity(entity);
		return actor->GetMass();
	}

	void Engine::Script::Engine_RigidBodyComponent_SetMass(uint64_t entityID, float mass)
	{
		Ref<Scene> scene = ScriptEngine::GetCurrentSceneContext();
		ENGINE_ASSERT(scene, "No active scene!");
		const auto& entityMap = scene->GetEntityMap();
		ENGINE_ASSERT(entityMap.find(entityID) != entityMap.end(), "Invalid entity ID or entity doesn't exist in scene!");

		Entity entity = entityMap.at(entityID);
		ENGINE_ASSERT(entity.HasComponent<RigidBodyComponent>(),"");
		auto& component = entity.GetComponent<RigidBodyComponent>();
		Ref<PhysicsActor>& actor = Physics::GetActorForEntity(entity);
		actor->SetMass(mass);
	}

	//---------------------------------------------------------------------------------
	// Render
	//---------------------------------------------------------------------------------
	void* Engine::Script::Engine_Texture2D_Constructor(uint32_t width, uint32_t height)
	{
		auto result = Texture2D::Create(TextureFormat::RGBA, width, height);
		return new Ref<Texture2D>(result);
	}

	void Engine::Script::Engine_Texture2D_Destructor(Ref<Texture2D>* _this)
	{
		delete _this;
	}

	void Engine::Script::Engine_Texture2D_SetData(Ref<Texture2D>* _this, MonoArray* inData, int32_t count)
	{
		Ref<Texture2D>& instance = *_this;

		uint32_t dataSize = count * sizeof(glm::vec4) / 4;

		instance->Lock();
		Buffer buffer = instance->GetWritableBuffer();
		ENGINE_ASSERT(dataSize <= buffer.Size,"");
		// Convert RGBA32F color to RGBA8
		uint8_t* pixels = (uint8_t*)buffer.Data;
		uint32_t index = 0;
		for (int i = 0; i < instance->GetWidth() * instance->GetHeight(); i++)
		{
			glm::vec4& value = mono_array_get(inData, glm::vec4, i);
			*pixels++ = (uint32_t)(value.x * 255.0f);
			*pixels++ = (uint32_t)(value.y * 255.0f);
			*pixels++ = (uint32_t)(value.z * 255.0f);
			*pixels++ = (uint32_t)(value.w * 255.0f);
		}

		instance->Unlock();
	}

	void Engine::Script::Engine_Material_Destructor(Ref<Material>* _this)
	{
		delete _this;
	}

	void Engine::Script::Engine_Material_SetFloat(Ref<Material>* _this, MonoString* uniform, float value)
	{
		Ref<Material>& instance = *(Ref<Material>*)_this;
		instance->Set(mono_string_to_utf8(uniform), value);
	}

	void Engine::Script::Engine_Material_SetTexture(Ref<Material>* _this, MonoString* uniform, Ref<Texture2D>* texture)
	{
		Ref<Material>& instance = *(Ref<Material>*)_this;
		instance->Set(mono_string_to_utf8(uniform), *texture);
	}

	void Engine::Script::Engine_MaterialInstance_Destructor(Ref<MaterialInstance>* _this)
	{
		delete _this;
	}

	void Engine::Script::Engine_MaterialInstance_SetFloat(Ref<MaterialInstance>* _this, MonoString* uniform, float value)
	{
		Ref<MaterialInstance>& instance = *(Ref<MaterialInstance>*)_this;
		instance->Set(mono_string_to_utf8(uniform), value);
	}

	void Engine::Script::Engine_MaterialInstance_SetVector3(Ref<MaterialInstance>* _this, MonoString* uniform, glm::vec3* value)
	{
		Ref<MaterialInstance>& instance = *(Ref<MaterialInstance>*)_this;
		instance->Set(mono_string_to_utf8(uniform), *value);
	}

	void Engine::Script::Engine_MaterialInstance_SetVector4(Ref<MaterialInstance>* _this, MonoString* uniform, glm::vec4* value)
	{
		Ref<MaterialInstance>& instance = *(Ref<MaterialInstance>*)_this;
		instance->Set(mono_string_to_utf8(uniform), *value);
	}

	void Engine::Script::Engine_MaterialInstance_SetTexture(Ref<MaterialInstance>* _this, MonoString* uniform, Ref<Texture2D>* texture)
	{
		Ref<MaterialInstance>& instance = *(Ref<MaterialInstance>*)_this;
		instance->Set(mono_string_to_utf8(uniform), *texture);
	}

	Engine::Ref<Engine::Mesh>* Engine::Script::Engine_Mesh_Constructor(MonoString* filepath)
	{
		return new Ref<Mesh>(new Mesh(mono_string_to_utf8(filepath)));
	}

	void Engine::Script::Engine_Mesh_Destructor(Ref<Mesh>* _this)
	{
		Ref<Mesh>* instance = (Ref<Mesh>*)_this;
		delete _this;
	}

	Engine::Ref<Engine::Material>* Engine::Script::Engine_Mesh_GetMaterial(Ref<Mesh>* inMesh)
	{
		Ref<Mesh>& mesh = *(Ref<Mesh>*)inMesh;
		return new Ref<Material>(mesh->GetMaterial());
	}

	Engine::Ref<Engine::MaterialInstance>* Engine::Script::Engine_Mesh_GetMaterialByIndex(Ref<Mesh>* inMesh, int index)
	{
		Ref<Mesh>& mesh = *(Ref<Mesh>*)inMesh;
		const auto& materials = mesh->GetMaterials();

		ENGINE_ASSERT(index < materials.size(),"");
		return new Ref<MaterialInstance>(materials[index]);
	}

	int Engine::Script::Engine_Mesh_GetMaterialCount(Ref<Mesh>* inMesh)
	{
		Ref<Mesh>& mesh = *(Ref<Mesh>*)inMesh;
		const auto& materials = mesh->GetMaterials();
		return materials.size();
	}

	void* Engine::Script::Engine_MeshFactory_CreatePlane(float width, float height)
	{
		// TODO: Implement properly with MeshFactory class
		return new Ref<Mesh>(new Mesh("assets/models/Plane/Plane.fbx"));
	}
}
}