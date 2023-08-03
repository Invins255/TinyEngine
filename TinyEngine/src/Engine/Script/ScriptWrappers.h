#pragma once

#include "Engine/Script/ScriptEngine.h"
#include "Engine/Core/Input.h"
#include "Engine/Physics/Physics.h"

#include <glm/glm.hpp>

extern "C" {
	typedef struct _MonoString MonoString;
	typedef struct _MonoArray MonoArray;
}

namespace Engine
{ 
	namespace Script 
	{
		struct ScriptTransform
		{
			glm::vec3 Translation;
			glm::vec3 Rotation;
			glm::vec3 Scale;
			glm::vec3 Up, Right, Forward;
		};

		//---------------------------------------------------------------------------------
		//C#脚本函数实现
		//---------------------------------------------------------------------------------
		
		// Input
		bool Engine_Input_IsKeyPressed(int key);
		bool Engine_Input_IsMouseButtonPressed(int button);
		void Engine_Input_GetMousePosition(glm::vec2* outPosition);
		void Engine_Input_SetCursorMode(CursorMode mode);
		CursorMode Engine_Input_GetCursorMode();

		// Physics
		bool Engine_Physics_Raycast(glm::vec3* origin, glm::vec3* direction, float maxDistance, RaycastHit* hit);
		MonoArray* Engine_Physics_OverlapBox(glm::vec3* origin, glm::vec3* halfSize);
		MonoArray* Engine_Physics_OverlapCapsule(glm::vec3* origin, float radius, float halfHeight);
		MonoArray* Engine_Physics_OverlapSphere(glm::vec3* origin, float radius);
		int32_t Engine_Physics_OverlapBoxNonAlloc(glm::vec3* origin, glm::vec3* halfSize, MonoArray* outColliders);
		int32_t Engine_Physics_OverlapCapsuleNonAlloc(glm::vec3* origin, float radius, float halfHeight, MonoArray* outColliders);
		int32_t Engine_Physics_OverlapSphereNonAlloc(glm::vec3* origin, float radius, MonoArray* outColliders);

		// Entity
		void Engine_Entity_CreateComponent(uint64_t entityID, void* type);
		bool Engine_Entity_HasComponent(uint64_t entityID, void* type);
		uint64_t Engine_Entity_FindEntityByTag(MonoString* tag);

		void Engine_TransformComponent_GetTransform(uint64_t entityID, ScriptTransform* outTransform);
		void Engine_TransformComponent_SetTransform(uint64_t entityID, ScriptTransform* inTransform);

		void* Engine_MeshComponent_GetMesh(uint64_t entityID);
		void Engine_MeshComponent_SetMesh(uint64_t entityID, Ref<Mesh>* inMesh);
	 
		RigidBodyComponent::Type Engine_RigidBodyComponent_GetBodyType(uint64_t entityID);
		void Engine_RigidBodyComponent_AddForce(uint64_t entityID, glm::vec3* force, ForceMode forceMode);
		void Engine_RigidBodyComponent_AddTorque(uint64_t entityID, glm::vec3* torque, ForceMode forceMode);
		void Engine_RigidBodyComponent_GetLinearVelocity(uint64_t entityID, glm::vec3* outVelocity);
		void Engine_RigidBodyComponent_SetLinearVelocity(uint64_t entityID, glm::vec3* velocity);
		void Engine_RigidBodyComponent_GetAngularVelocity(uint64_t entityID, glm::vec3* outVelocity);
		void Engine_RigidBodyComponent_SetAngularVelocity(uint64_t entityID, glm::vec3* velocity);
		void Engine_RigidBodyComponent_Rotate(uint64_t entityID, glm::vec3* rotation);
		uint32_t Engine_RigidBodyComponent_GetLayer(uint64_t entityID);
		float Engine_RigidBodyComponent_GetMass(uint64_t entityID);
		void Engine_RigidBodyComponent_SetMass(uint64_t entityID, float mass);

		// Renderer
		// Texture2D
		void* Engine_Texture2D_Constructor(uint32_t width, uint32_t height);
		void Engine_Texture2D_Destructor(Ref<Texture2D>* _this);
		void Engine_Texture2D_SetData(Ref<Texture2D>* _this, MonoArray* inData, int32_t count);

		// Material
		void Engine_Material_Destructor(Ref<Material>* _this);
		void Engine_Material_SetFloat(Ref<Material>* _this, MonoString* uniform, float value);
		void Engine_Material_SetTexture(Ref<Material>* _this, MonoString* uniform, Ref<Texture2D>* texture);
			 
		void Engine_MaterialInstance_Destructor(Ref<MaterialInstance>* _this);
		void Engine_MaterialInstance_SetFloat(Ref<MaterialInstance>* _this, MonoString* uniform, float value);
		void Engine_MaterialInstance_SetVector3(Ref<MaterialInstance>* _this, MonoString* uniform, glm::vec3* value);
		void Engine_MaterialInstance_SetVector4(Ref<MaterialInstance>* _this, MonoString* uniform, glm::vec4* value);
		void Engine_MaterialInstance_SetTexture(Ref<MaterialInstance>* _this, MonoString* uniform, Ref<Texture2D>* texture);

		// Mesh
		Ref<Mesh>* Engine_Mesh_Constructor(MonoString* filepath);
		void Engine_Mesh_Destructor(Ref<Mesh>* _this);
		Ref<Material>* Engine_Mesh_GetMaterial(Ref<Mesh>* inMesh);
		Ref<MaterialInstance>* Engine_Mesh_GetMaterialByIndex(Ref<Mesh>* inMesh, int index);
		int Engine_Mesh_GetMaterialCount(Ref<Mesh>* inMesh);

		void* Engine_MeshFactory_CreatePlane(float width, float height);

	}
}
