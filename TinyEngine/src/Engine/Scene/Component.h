#pragma once

#include <string>
#include <functional>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>

#include "Engine/Core/UUID.h"
#include "Engine/Scene/SceneCamera.h"
#include "Engine/Renderer/Mesh.h"

namespace Engine
{
	struct IDComponent
	{
		UUID ID;
	};

	struct TagComponent
	{
		std::string Tag;

		TagComponent() = default;
		TagComponent(const std::string& tag) :
			Tag(tag) {}

		operator std::string& () { return Tag; }
		operator const std::string& () const { return Tag; }
	};

	struct RelationshipComponent
	{
		UUID ParentHandle = 0;
		std::vector<UUID> Children;

		RelationshipComponent() = default;
		RelationshipComponent(UUID parent)
			: ParentHandle(parent) {}
	};

	struct TransformComponent
	{
		glm::vec3 Translation = { 0.0f, 0.0f, 0.0f };
		glm::vec3 Rotation = { 0.0f, 0.0f, 0.0f };
		glm::vec3 Scale = { 1.0f, 1.0f, 1.0f };

		TransformComponent() = default;
		TransformComponent(const glm::vec3& translation) :
			Translation(translation) {}

		glm::mat4 GetTransform()
		{
			return	glm::translate(glm::mat4(1.0f), Translation) *
					glm::toMat4(glm::quat(Rotation)) *
					glm::scale(glm::mat4(1.0f), Scale);
		}
	};

	struct MeshComponent
	{
		Ref<Engine::Mesh> Mesh;
		MeshComponent() = default;
		MeshComponent(const Ref<Engine::Mesh>& mesh)
			:Mesh(mesh) {}
	};

	struct SpriteRendererComponent
	{
		glm::vec4 Color{ 1.0f,1.0f,1.0f,1.0f };

		SpriteRendererComponent() = default;
		SpriteRendererComponent(const glm::vec4& color) :
			Color(color) {} 
	};

	struct CameraComponent
	{
		SceneCamera Camera;
		bool Primary = true;
		bool FixedAspectRatio = false;

		CameraComponent() = default;

		operator SceneCamera& () { return Camera; }
		operator const SceneCamera& () const { return Camera; }
	};

	//--------------------------------------------------------
	// Light
	//--------------------------------------------------------

	enum class LightType
	{
		None = 0, Directional = 1, Point = 2, Spot = 3
	};

	enum class ShadowsType
	{
		HardShadows = 0, PCF = 1, PCSS = 2
	};

	struct DirectionalLightComponent
	{
		glm::vec3 Radiance = { 1.0f, 1.0f, 1.0f };
		float Intensity = 1.0f;
		ShadowsType shadowsType = ShadowsType::HardShadows;
		int SamplingRadius = 5;
		bool CastShadows = true;
	};

	//--------------------------------------------------------
	// Physics
	//--------------------------------------------------------

	struct BoxColliderComponent
	{
		glm::vec3 Size = { 1.0f, 1.0f, 1.0f };
		glm::vec3 Offset = { 0.0f, 0.0f, 0.0f };

		bool IsTrigger = false;

		//Collision bounds
		Ref<Mesh> DebugMesh;
	};

	struct SphereColliderComponent
	{
		float Radius = 0.5f;
		bool IsTrigger = false;

		//Collision bounds
		Ref<Mesh> DebugMesh;
	};

	struct CapsuleColliderComponent
	{
		float Radius = 0.5f;
		float Height = 1.0f;
		bool IsTrigger = false;

		//Collision bounds
		Ref<Mesh> DebugMesh;
	};

	struct MeshColliderComponent
	{
		Ref<Mesh> CollisionMesh;
		std::vector<Ref<Mesh>> ProcessedMeshes;
		bool IsConvex = false;
		bool IsTrigger = false;
		bool OverrideMesh = false;

		MeshColliderComponent(const Ref<Mesh>& mesh)
			: CollisionMesh(mesh)
		{
		}

		operator Ref<Mesh>() { return CollisionMesh; }
	};

	struct RigidBodyComponent
	{
		enum class Type { Static, Dynamic };
		Type BodyType;
		float Mass = 1.0f;
		float LinearDrag = 0.0f;
		float AngularDrag = 0.05f;
		bool DisableGravity = false;
		bool IsKinematic = false;
		uint32_t Layer = 0;

		bool LockPositionX = false;
		bool LockPositionY = false;
		bool LockPositionZ = false;
		bool LockRotationX = false;
		bool LockRotationY = false;
		bool LockRotationZ = false;
	};

	struct PhysicsMaterialComponent
	{
		float StaticFriction = 1.0F;
		float DynamicFriction = 1.0F;
		float Bounciness = 1.0F;
	};


	//--------------------------------------------------------
	// Script
	//--------------------------------------------------------
	struct ScriptComponent
	{
		std::string ModuleName;

		ScriptComponent() = default;
		ScriptComponent(const ScriptComponent& other) = default;
		ScriptComponent(const std::string& moduleName)
			: ModuleName(moduleName) {}
	};
}