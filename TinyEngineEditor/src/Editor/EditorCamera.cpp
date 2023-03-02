#include "EditorCamera.h"

#include "Engine/Core/Input.h"
#include "Engine/Core/KeyCodes.h"
#include "Engine/Core/MouseButtonCodes.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

#define _USE_MATH_DEFINES
#include <math.h>

namespace Engine
{
	EditorCamera::EditorCamera(const float degFov, const float width, const float height, const float nearP, const float farP)
		:Camera(glm::perspectiveFov(glm::radians(degFov), width, height, farP, nearP)), m_FocalPoint(0.0f), m_FOV(glm::radians(degFov)), m_NearClip(nearP), m_FarClip(farP)
	{
		//Initialize 
		glm::vec3 position = { 50, 50, 50 };
		m_Distance = glm::distance(position, m_FocalPoint);

		m_Yaw = -(float)M_PI / 4.0f;
		m_Pitch = M_PI / 4.0f;

		m_Position = CalculatePosition();
		const glm::quat orientation = GetOrientation();
		m_Direction = glm::eulerAngles(orientation) * (180.0f / glm::pi<float>());
		m_ViewMatrix = glm::translate(glm::mat4(1.0f), m_Position) * glm::toMat4(orientation);
		m_ViewMatrix = glm::inverse(m_ViewMatrix);
	}

	/// <summary>
	/// Camera operation
	/// 
	/// Mouse left button:		Pan
	/// Mouse middle button:	Rotate
	/// Mouse right button:		Zoom
	/// Mouse scroll:			Zoom
	/// </summary>

	void EditorCamera::Focus(const glm::vec3& focusPoint)
	{
		m_FocalPoint = focusPoint;
		m_CameraMode = CameraMode::Flycam;
		if (m_Distance > m_MinFocusDistance)
		{
			m_Distance -= m_Distance - m_MinFocusDistance;
			m_Position = m_FocalPoint - GetForwardDirection() * m_Distance;
		}
		m_Position = m_FocalPoint - GetForwardDirection() * m_Distance;
		UpdateCameraView();
	}

	void EditorCamera::OnUpdate(Timestep ts)
	{
		const glm::vec2& mouse{ Input::GetMouseX(), Input::GetMouseY() };
		glm::vec2 delta = (mouse - m_InitialMousePosition) * 0.003f;
		m_InitialMousePosition = mouse;

		if(m_CameraMode == CameraMode::Arcball)
		{
			if (Input::IsMouseButtonPressed(ENGINE_MOUSE_BUTTON_MIDDLE))
				MousePan(delta);
			else if (Input::IsMouseButtonPressed(ENGINE_MOUSE_BUTTON_LEFT))
				MouseRotate(delta);
			else if (Input::IsMouseButtonPressed(ENGINE_MOUSE_BUTTON_RIGHT))
				MouseZoom(delta.y);

			m_Position += m_PositionDelta;
			m_Yaw += m_YawDelta;
			m_Pitch += m_PitchDelta;

			m_Position = CalculatePosition();
		}
		else if (m_CameraMode == CameraMode::Flycam)
		{
			const float yawSign = GetUpDirection().y < 0 ? -1.0f : 1.0f;
			const float speed = GetCameraSpeed();

			if (Input::IsKeyPressed(ENGINE_KEY_Q))
				m_PositionDelta -= ts.GetMilliseconds() * speed * glm::vec3{ 0.f, yawSign, 0.f };
			if (Input::IsKeyPressed(ENGINE_KEY_E))
				m_PositionDelta += ts.GetMilliseconds() * speed * glm::vec3{ 0.f, yawSign, 0.f };
			if (Input::IsKeyPressed(ENGINE_KEY_S))
				m_PositionDelta -= ts.GetMilliseconds() * speed * m_Direction;
			if (Input::IsKeyPressed(ENGINE_KEY_W))
				m_PositionDelta += ts.GetMilliseconds() * speed * m_Direction;
			if (Input::IsKeyPressed(ENGINE_KEY_A))
				m_PositionDelta -= ts.GetMilliseconds() * speed * m_RightDirection;
			if (Input::IsKeyPressed(ENGINE_KEY_D))
				m_PositionDelta += ts.GetMilliseconds() * speed * m_RightDirection;

			constexpr float maxRate{ 0.12f };
			m_YawDelta += glm::clamp(yawSign * delta.x * RotationSpeed(), -maxRate, maxRate);
			m_PitchDelta += glm::clamp(delta.y * RotationSpeed(), -maxRate, maxRate);

			m_RightDirection = glm::cross(m_Direction, glm::vec3{ 0.f, yawSign, 0.f });

			m_Direction = glm::rotate(glm::normalize(glm::cross(glm::angleAxis(-m_PitchDelta, m_RightDirection),
				glm::angleAxis(-m_YawDelta, glm::vec3{ 0.f, yawSign, 0.f }))), m_Direction);

			const float distance = glm::distance(m_FocalPoint, m_Position);
			m_FocalPoint = m_Position + GetForwardDirection() * distance;
			m_Distance = distance;

			m_Position += m_PositionDelta;
			m_Yaw += m_YawDelta;
			m_Pitch += m_PitchDelta;
		}

		UpdateCameraView();
	}

	void EditorCamera::OnEvent(Event& e)
	{
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<KeyPressedEvent>(ENGINE_BIND_EVENT_FN(EditorCamera::OnKeyPressed));
		dispatcher.Dispatch<MouseScrolledEvent>(ENGINE_BIND_EVENT_FN(EditorCamera::OnMouseScroll));
	}

	void EditorCamera::SetViewportSize(uint32_t width, uint32_t height)
	{
		if (width == m_ViewportWidth && height == m_ViewportHeight)
			return;
		m_ViewportWidth = width;
		m_ViewportHeight = height;
		SetPerspectiveProjection(m_FOV, m_ViewportWidth, m_ViewportHeight, m_NearClip, m_FarClip);
	}

	glm::vec3 EditorCamera::GetUpDirection() const
	{
		return glm::rotate(GetOrientation(), glm::vec3(0.0f, 1.0f, 0.0f));
	}

	glm::vec3 EditorCamera::GetRightDirection() const
	{
		return glm::rotate(GetOrientation(), glm::vec3(1.0f, 0.0f, 0.0f));
	}

	glm::vec3 EditorCamera::GetForwardDirection() const
	{
		return glm::rotate(GetOrientation(), glm::vec3(0.0f, 0.0f, -1.0f));
	}

	glm::quat EditorCamera::GetOrientation() const
	{
		return glm::quat(glm::vec3(-m_Pitch - m_PitchDelta, -m_Yaw - m_YawDelta, 0.0f));
	}

	float EditorCamera::GetCameraSpeed() const
	{
		float speed = m_NormalSpeed;
		if (Input::IsKeyPressed(ENGINE_KEY_LEFT_CONTROL))
			speed /= 2 - glm::log(m_NormalSpeed);
		if (Input::IsKeyPressed(ENGINE_KEY_LEFT_SHIFT))
			speed *= 2 - glm::log(m_NormalSpeed);

		return glm::clamp(speed, MIN_SPEED, MAX_SPEED);
	}

	void EditorCamera::UpdateCameraView()
	{
		const float yawSign = GetUpDirection().y < 0 ? -1.0f : 1.0f;
		const float cosAngle = glm::dot(GetForwardDirection(), GetUpDirection());
		if (cosAngle * yawSign > 0.99f)
			m_PitchDelta = 0.f;

		const glm::vec3 lookAt = m_Position + GetForwardDirection();
		m_Direction = glm::normalize(lookAt - m_Position);
		m_Distance = glm::distance(m_Position, m_FocalPoint);
		m_ViewMatrix = glm::lookAt(m_Position, lookAt, glm::vec3{ 0.f, yawSign, 0.f });

		//damping for smooth camera
		m_YawDelta *= 0.6f;
		m_PitchDelta *= 0.6f;
		m_PositionDelta *= 0.8f;
	}

	bool EditorCamera::OnKeyPressed(KeyPressedEvent& e)
	{
		if (Input::IsKeyPressed(ENGINE_KEY_LEFT_CONTROL) && Input::IsKeyPressed(ENGINE_KEY_LEFT_ALT))
		{
			if (m_CameraMode == CameraMode::Flycam)
				m_CameraMode = CameraMode::Arcball;
			else
				m_CameraMode = CameraMode::Flycam;

			APP_TRACE("Camera mode: {0}", m_CameraMode == CameraMode::Arcball ? "Arcball" : "Flycam");
		}
		return false;
	}

	bool EditorCamera::OnMouseScroll(MouseScrolledEvent& e)
	{
		float delta = e.GetYOffset() * 0.1f;
		MouseZoom(delta);
		UpdateCameraView();
		
		return false;
	}

	void EditorCamera::MousePan(const glm::vec2& delta)
	{
		auto [xSpeed, ySpeed] = PanSpeed();
		m_FocalPoint += -GetRightDirection() * delta.x * xSpeed * m_Distance;
		m_FocalPoint += GetUpDirection() * delta.y * ySpeed * m_Distance;
	}

	void EditorCamera::MouseRotate(const glm::vec2& delta)
	{
		float yawSign = GetUpDirection().y < 0 ? -1.0f : 1.0f;
		m_YawDelta += yawSign * delta.x * RotationSpeed();
		m_PitchDelta += delta.y * RotationSpeed();
	}

	void EditorCamera::MouseZoom(float delta)
	{
		m_Distance -= delta * ZoomSpeed();
		const glm::vec3 forwardDir = GetForwardDirection();
		m_Position = m_FocalPoint - forwardDir * m_Distance;
		if (m_Distance < 1.0f)
		{
			m_FocalPoint += GetForwardDirection();
			m_Distance = 1.0f;
		}
		m_PositionDelta += delta * ZoomSpeed() * forwardDir;
	}

	glm::vec3 EditorCamera::CalculatePosition()
	{
		return m_FocalPoint - GetForwardDirection() * m_Distance + m_PositionDelta;
	}

	std::pair<float, float> EditorCamera::PanSpeed() const
	{
		float x = std::min(m_ViewportWidth / 1000.0f, 2.4f); // max = 2.4f
		float xFactor = 0.0366f * (x * x) - 0.1778f * x + 0.3021f;

		float y = std::min(m_ViewportHeight / 1000.0f, 2.4f); // max = 2.4f
		float yFactor = 0.0366f * (y * y) - 0.1778f * y + 0.3021f;

		return { xFactor, yFactor };
	}

	float EditorCamera::RotationSpeed() const
	{
		return 0.8f;
	}

	float EditorCamera::ZoomSpeed() const
	{
		float distance = m_Distance * 0.2f;
		distance = std::max(distance, 0.0f);
		float speed = distance * distance;
		speed = std::min(speed, 100.0f); // max speed = 100
		return speed;
	}
}