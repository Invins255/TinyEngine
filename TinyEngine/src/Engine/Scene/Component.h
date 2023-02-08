#pragma once

#include <string>
#include <functional>

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
		UUID ID = 0;
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

	enum class LightType
	{
		None = 0, Directional = 1, Point = 2, Spot = 3
	};

	struct DirectionalLightComponent
	{
		glm::vec3 Radiance = { 1.0f, 1.0f, 1.0f };
		float Intensity = 1.0f;
	};
}