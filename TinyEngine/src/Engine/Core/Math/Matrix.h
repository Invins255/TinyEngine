#pragma once

#include <glm/glm.hpp>
#include <string>

namespace Engine::Math
{
	std::string MatrixToString(const glm::mat4& mat);

	bool DecomposeTransform(const glm::mat4& transform, glm::vec3& translation, glm::vec3& rotation, glm::vec3& scale);
}