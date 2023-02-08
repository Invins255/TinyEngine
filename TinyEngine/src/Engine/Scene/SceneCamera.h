#pragma once

#include "Engine/Renderer/Camera.h"

namespace Engine
{
	class SceneCamera : public Camera
	{
	public:
		enum class ProjectionType { Perspective = 0, Orthographic = 1 };
	public:
		SceneCamera();
		virtual ~SceneCamera() = default;

		ProjectionType GetProjectionType() const { return m_ProjectionType; }
		void SetProjectionType(ProjectionType type) { m_ProjectionType = type; }

		void SetOrthographic(float size, float nearClip, float farClip);
		void SetPerspective(float verticalFOV, float nearClip, float farClip);
		
		void SetViewportSize(uint32_t width, uint32_t height);	
		
		//Orthographic
		void SetOrthographicSize(float size) { m_OrthographicSize = size; RecalculateProjection(); }	
		float GetOrthographicSize() const { return m_OrthographicSize; }
		void SetOrthographicNearClip(float nearClip) { m_OrthographicNear = nearClip; RecalculateProjection(); }
		float GetOrthographicNearClip() { return m_OrthographicNear; }
		void SetOrthographicFarClip(float farClip) { m_OrthographicFar = farClip; RecalculateProjection(); }
		float GetOrthographicFarClip() { return m_OrthographicFar; }

		//Perspective
		void SetDegPerspectiveFOV(float degFov) { m_DegPerspectiveFOV = degFov; RecalculateProjection(); }
		float GetDegPerspectiveFOV() { return m_DegPerspectiveFOV; }
		void SetRadPerspectiveFOV(float radFov) { m_DegPerspectiveFOV = glm::degrees(radFov); RecalculateProjection(); }
		float GetRadPerspectiveFOV() { return glm::radians(m_DegPerspectiveFOV); }
		void SetPerspectiveNearClip(float nearClip) { m_PerspectiveNear = nearClip; RecalculateProjection(); }
		float GetPerspectiveNearClip() { return m_PerspectiveNear; }
		void SetPerspectiveFarClip(float farClip) { m_PerspectiveFar = farClip; RecalculateProjection(); }
		float GetPerspectiveFarClip() { return m_PerspectiveFar; }

		void SetAspectRatio(float aspectRatio) { m_AspectRatio = aspectRatio; }
		float GetAspectRatio() const { return m_AspectRatio; }
	private:
		void RecalculateProjection();
	
	private:
		ProjectionType m_ProjectionType = ProjectionType::Perspective;

		float m_OrthographicSize = 10.0f;
		float m_OrthographicNear = -1.0f, m_OrthographicFar = 1.0f;

		float m_DegPerspectiveFOV = 45.0f;
		float m_PerspectiveNear = 0.01f, m_PerspectiveFar = 1000.0f;
		
		float m_AspectRatio = 0.0f;
	};
}