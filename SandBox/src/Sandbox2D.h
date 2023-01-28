#pragma once

#include "TinyEngine.h"
#include <glm/glm.hpp>

class Sandbox2D : public Engine::Layer
{
public:
	Sandbox2D();
	virtual ~Sandbox2D() = default;

	virtual void OnAttach() override;
	virtual void OnDetach() override;

	virtual void OnUpdate(Engine::Timestep ts) override;
	virtual void OnEvent(Engine::Event& e) override;
	virtual void OnImGuiRender() override;
private:
	Engine::OrthographicCameraController m_CameraController;
	//Temp
	Engine::Ref<Engine::Shader> m_Shader;
	Engine::Ref<Engine::VertexArray> m_VertexArray;
	Engine::Ref<Engine::Texture2D> m_Texture;

	glm::vec4 m_Color = { 1.0f, 1.0f, 1.0f, 1.0f };
};

