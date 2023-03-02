#pragma once

#include "Engine/Core/TimeStep.h"
#include "Engine/Renderer/Camera.h"
#include "Engine/Events/MouseEvent.h"

//TODO: Add Camera mode, such as free perspective

namespace Engine
{
	class EditorCamera : public Camera
	{
	public:
		EditorCamera() = default;
		EditorCamera(const glm::mat4& projection);

		void Focus();
		void OnUpdate(Timestep ts);
		void OnEvent(Event& e);

		void SetDistance(float distance) { m_Distance = distance; }
		float GetDistance() const { return m_Distance; }
		
		void SetViewportSize(uint32_t width, uint32_t height) { m_ViewportWidth = width; m_ViewportHeight = height; }

		const glm::mat4& GetViewMatrix() const { return m_ViewMatrix; }
		glm::mat4 GetViewProjection() const { return m_Projection * m_ViewMatrix; }
		glm::vec3 GetUpDirection() const;
		glm::vec3 GetRightDirection() const;
		glm::vec3 GetForwardDirection() const;
		glm::vec3 GetPosition() const { return m_Position; }
		glm::quat GetOrientation() const;

	private:
		void UpdateCameraView();

		bool OnMouseScroll(MouseScrolledEvent& e);

		void MousePan(const glm::vec2& delta);
		void MouseRotate(const glm::vec2& delta);
		void MouseZoom(float delta);

		glm::vec3 CalculatePosition();

		std::pair<float, float> PanSpeed() const;
		float RotationSpeed() const;
		float ZoomSpeed() const;

	private:
		glm::mat4 m_ViewMatrix;
		glm::vec3 m_Position, m_Rotation, m_FocalPoint;
		bool m_Panning, m_Rotating;

		glm::vec2 m_InitialMousePosition;
		glm::vec3 m_InitialFocalPoint;
		glm::vec3 m_InitialRotation;

		float m_Distance;
		float m_Pitch, m_Yaw;

		uint32_t m_ViewportWidth = 1600, m_ViewportHeight = 900;
	};
}
