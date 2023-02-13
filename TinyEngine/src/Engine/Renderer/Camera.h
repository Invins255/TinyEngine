#pragma once

#include <glm/glm.hpp>

namespace Engine
{
	class Camera
	{
	public:
		Camera() = default;
		Camera(const glm::mat4& projection) :
			m_Projection(projection) {}
		virtual ~Camera() = default;

		const glm::mat4& GetProjection() const { return m_Projection; }

		void SetProjection(const glm::mat4& projection) { m_Projection = projection; }
		void SetPerspectiveProjection(float radFov, float width, float height, float nearClip, float farClip)
		{
			m_Projection = glm::perspectiveFov(radFov, width, height, nearClip, farClip);
		}
		void SetPerspectiveProjection(float radFov, float aspectRatio, float nearClip, float farClip)
		{
			m_Projection = glm::perspective(radFov, aspectRatio, nearClip, farClip);
		}
		void SetOrthographicProjection(float width, float height, float nearClip, float farClip)
		{
			m_Projection = glm::ortho(-width * 0.5f, width * 0.5f, -height * 0.5f, height * 0.5f, nearClip, farClip);
		}
		void SetOrthographicProjection(float size, float nearClip, float farClip)
		{
			m_Projection = glm::ortho(-size * 0.5f, size * 0.5f, -size * 0.5f, size * 0.5f, nearClip, farClip);
		}
	protected:
		glm::mat4 m_Projection = glm::mat4(1.0f);
	};
}