#pragma once

#include <glm/glm.hpp>

namespace Engine
{
	struct DirectionalLight
	{
		glm::vec3 Direction = { 0.0f, 0.0f, 0.0f };
		glm::vec3 Radiance = { 0.0f, 0.0f, 0.0f };
		float Intensity = 0.0f;
		bool CastShadows = true;
	};

	struct LightEnvironment
	{
		DirectionalLight DirectionalLights[4];
	};
}