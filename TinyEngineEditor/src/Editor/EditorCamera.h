#pragma once

#include "Engine/Core/TimeStep.h"
#include "Engine/Renderer/Camera.h"
#include "Engine/Events/KeyEvent.h"
#include "Engine/Events/MouseEvent.h"

//TODO: Add Camera mode, such as free perspective

namespace Engine
{
	enum class CameraMode
	{
		None, Flycam, Arcball
	};

	class EditorCamera : public Camera
	{
	public:
		EditorCamera() = default;
		EditorCamera(const float degFov, const float width, const float height, const float nearP, const float farP);

		void Focus(const glm::vec3& focusPoint);
		void OnUpdate(Timestep ts);
		void OnEvent(Event& e);

		CameraMode GetCameraMode() const { return m_CameraMode; }
		void SetCameraMode(CameraMode mode) { m_CameraMode = mode; }

		void SetDistance(float distance) { m_Distance = distance; }
		float GetDistance() const { return m_Distance; }
		
		void SetViewportSize(uint32_t width, uint32_t height);

		const glm::mat4& GetViewMatrix() const { return m_ViewMatrix; }
		glm::mat4 GetViewProjection() const { return m_Projection * m_ViewMatrix; }
		glm::vec3 GetUpDirection() const;
		glm::vec3 GetRightDirection() const;
		glm::vec3 GetForwardDirection() const;
		glm::vec3 GetPosition() const { return m_Position; }
		glm::quat GetOrientation() const;
		float GetVerticalFOV() const { return m_FOV; }
		float GetNearClip() const { return m_NearClip; }
		float GetFarClip() const { return m_FarClip; }
		float GetPitch() const { return m_Pitch; }
		float GetYaw() const { return m_Yaw; }
		float GetCameraSpeed() const;

	private:
		void UpdateCameraView();

		bool OnKeyPressed(KeyPressedEvent& e);
		bool OnMouseScroll(MouseScrolledEvent& e);

		void MousePan(const glm::vec2& delta);
		void MouseRotate(const glm::vec2& delta);
		void MouseZoom(float delta);

		glm::vec3 CalculatePosition();

		std::pair<float, float> PanSpeed() const;
		float RotationSpeed() const;
		float ZoomSpeed() const;

	private:
		CameraMode m_CameraMode = CameraMode::Arcball;

		glm::mat4 m_ViewMatrix;
		float m_FOV, m_NearClip, m_FarClip;
		glm::vec3 m_Position, m_Direction, m_RightDirection, m_FocalPoint;
		bool m_Panning, m_Rotating;

		glm::vec2 m_InitialMousePosition;
		glm::vec3 m_InitialFocalPoint;
		glm::vec3 m_InitialRotation;

		float m_Distance;
		float m_Pitch, m_Yaw;

		float m_PitchDelta = 0.0f, m_YawDelta = 0.0f;
		glm::vec3 m_PositionDelta = glm::vec3(0.0f);

		float m_MinFocusDistance = 100.0f;

		uint32_t m_ViewportWidth = 1600, m_ViewportHeight = 900;

		float m_NormalSpeed = 0.005f;
		constexpr static float MIN_SPEED = 0.0005f, MAX_SPEED = 2.0f;
	};
}
