#include "pch.h"
#include "ScriptEngineRegistry.h"

#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Engine/Scene/Entity.h"
#include "Engine/Script/ScriptWrappers.h"

namespace Engine
{
	std::unordered_map<MonoType*, std::function<bool(Entity&)>> s_HasComponentFuncs;
	std::unordered_map<MonoType*, std::function<void(Entity&)>> s_CreateComponentFuncs;

	extern MonoImage* s_CoreAssemblyImage;

#define Component_RegisterType(Type) \
	{\
		MonoType* type = mono_reflection_type_from_name("Engine." #Type, s_CoreAssemblyImage);\
		if (type) {\
			uint32_t id = mono_type_get_type(type);\
			s_HasComponentFuncs[type] = [](Entity& entity) { return entity.HasComponent<Type>(); };\
			s_CreateComponentFuncs[type] = [](Entity& entity) { entity.AddComponent<Type>(); };\
		} else {\
			ENGINE_ERROR("No C# component class found for " #Type "!");\
		}\
	}

	static void InitComponentTypes()
	{
		Component_RegisterType(TagComponent);
		Component_RegisterType(TransformComponent);
		Component_RegisterType(MeshComponent);
		Component_RegisterType(ScriptComponent);
		Component_RegisterType(CameraComponent);
		Component_RegisterType(RigidBodyComponent);
		Component_RegisterType(BoxColliderComponent);
		Component_RegisterType(SphereColliderComponent);
	}


	void ScriptEngineRegistry::RegisterAll()
	{
		InitComponentTypes();

		mono_add_internal_call("Engine.Physics::Raycast_Native",				Engine::Script::Engine_Physics_Raycast);
		mono_add_internal_call("Engine.Physics::OverlapBox_Native",				Engine::Script::Engine_Physics_OverlapBox);
		mono_add_internal_call("Engine.Physics::OverlapCapsule_Native",			Engine::Script::Engine_Physics_OverlapCapsule);
		mono_add_internal_call("Engine.Physics::OverlapSphere_Native",			Engine::Script::Engine_Physics_OverlapSphere);
		mono_add_internal_call("Engine.Physics::OverlapBoxNonAlloc_Native",		Engine::Script::Engine_Physics_OverlapBoxNonAlloc);
		mono_add_internal_call("Engine.Physics::OverlapCapsuleNonAlloc_Native", Engine::Script::Engine_Physics_OverlapCapsuleNonAlloc);
		mono_add_internal_call("Engine.Physics::OverlapSphereNonAlloc_Native",	Engine::Script::Engine_Physics_OverlapSphereNonAlloc);

		mono_add_internal_call("Engine.Entity::CreateComponent_Native",	Engine::Script::Engine_Entity_CreateComponent);
		mono_add_internal_call("Engine.Entity::HasComponent_Native",	Engine::Script::Engine_Entity_HasComponent);
		mono_add_internal_call("Engine.Entity::FindEntityByTag_Native", Engine::Script::Engine_Entity_FindEntityByTag);
								
		mono_add_internal_call("Engine.TransformComponent::GetTransform_Native", Engine::Script::Engine_TransformComponent_GetTransform);
		mono_add_internal_call("Engine.TransformComponent::SetTransform_Native", Engine::Script::Engine_TransformComponent_SetTransform);
								
		mono_add_internal_call("Engine.MeshComponent::GetMesh_Native", Engine::Script::Engine_MeshComponent_GetMesh);
		mono_add_internal_call("Engine.MeshComponent::SetMesh_Native", Engine::Script::Engine_MeshComponent_SetMesh);
																
		mono_add_internal_call("Engine.RigidBodyComponent::GetBodyType_Native",			Engine::Script::Engine_RigidBodyComponent_GetBodyType);
		mono_add_internal_call("Engine.RigidBodyComponent::AddForce_Native",			Engine::Script::Engine_RigidBodyComponent_AddForce);
		mono_add_internal_call("Engine.RigidBodyComponent::AddTorque_Native",			Engine::Script::Engine_RigidBodyComponent_AddTorque);
		mono_add_internal_call("Engine.RigidBodyComponent::GetLinearVelocity_Native",	Engine::Script::Engine_RigidBodyComponent_GetLinearVelocity);
		mono_add_internal_call("Engine.RigidBodyComponent::SetLinearVelocity_Native",	Engine::Script::Engine_RigidBodyComponent_SetLinearVelocity);
		mono_add_internal_call("Engine.RigidBodyComponent::GetAngularVelocity_Native",	Engine::Script::Engine_RigidBodyComponent_GetAngularVelocity);
		mono_add_internal_call("Engine.RigidBodyComponent::SetAngularVelocity_Native",	Engine::Script::Engine_RigidBodyComponent_SetAngularVelocity);
		mono_add_internal_call("Engine.RigidBodyComponent::Rotate_Native",				Engine::Script::Engine_RigidBodyComponent_Rotate);
		mono_add_internal_call("Engine.RigidBodyComponent::GetLayer_Native",			Engine::Script::Engine_RigidBodyComponent_GetLayer);
		mono_add_internal_call("Engine.RigidBodyComponent::GetMass_Native",				Engine::Script::Engine_RigidBodyComponent_GetMass);
		mono_add_internal_call("Engine.RigidBodyComponent::SetMass_Native",				Engine::Script::Engine_RigidBodyComponent_SetMass);
								
		mono_add_internal_call("Engine.Input::IsKeyPressed_Native",			Engine::Script::Engine_Input_IsKeyPressed);
		mono_add_internal_call("Engine.Input::IsMouseButtonPressed_Native", Engine::Script::Engine_Input_IsMouseButtonPressed);
		mono_add_internal_call("Engine.Input::GetMousePosition_Native",		Engine::Script::Engine_Input_GetMousePosition);
		mono_add_internal_call("Engine.Input::SetCursorMode_Native",		Engine::Script::Engine_Input_SetCursorMode);
		mono_add_internal_call("Engine.Input::GetCursorMode_Native",		Engine::Script::Engine_Input_GetCursorMode);
								
		mono_add_internal_call("Engine.Texture2D::Constructor_Native",	Engine::Script::Engine_Texture2D_Constructor);
		mono_add_internal_call("Engine.Texture2D::Destructor_Native",	Engine::Script::Engine_Texture2D_Destructor);
		mono_add_internal_call("Engine.Texture2D::SetData_Native",		Engine::Script::Engine_Texture2D_SetData);
								
		mono_add_internal_call("Engine.Material::Destructor_Native",	Engine::Script::Engine_Material_Destructor);
		mono_add_internal_call("Engine.Material::SetFloat_Native",		Engine::Script::Engine_Material_SetFloat);
		mono_add_internal_call("Engine.Material::SetTexture_Native",	Engine::Script::Engine_Material_SetTexture);
								
		mono_add_internal_call("Engine.MaterialInstance::Destructor_Native",	Engine::Script::Engine_MaterialInstance_Destructor);
		mono_add_internal_call("Engine.MaterialInstance::SetFloat_Native",		Engine::Script::Engine_MaterialInstance_SetFloat);
		mono_add_internal_call("Engine.MaterialInstance::SetVector3_Native",	Engine::Script::Engine_MaterialInstance_SetVector3);
		mono_add_internal_call("Engine.MaterialInstance::SetVector4_Native",	Engine::Script::Engine_MaterialInstance_SetVector4);
		mono_add_internal_call("Engine.MaterialInstance::SetTexture_Native",	Engine::Script::Engine_MaterialInstance_SetTexture);
								
		mono_add_internal_call("Engine.Mesh::Constructor_Native",			Engine::Script::Engine_Mesh_Constructor);
		mono_add_internal_call("Engine.Mesh::Destructor_Native",			Engine::Script::Engine_Mesh_Destructor);
		mono_add_internal_call("Engine.Mesh::GetMaterial_Native",			Engine::Script::Engine_Mesh_GetMaterial);
		mono_add_internal_call("Engine.Mesh::GetMaterialByIndex_Native",	Engine::Script::Engine_Mesh_GetMaterialByIndex);
		mono_add_internal_call("Engine.Mesh::GetMaterialCount_Native",		Engine::Script::Engine_Mesh_GetMaterialCount);
								
		mono_add_internal_call("Engine.MeshFactory::CreatePlane_Native", Engine::Script::Engine_MeshFactory_CreatePlane);

	}
}
 