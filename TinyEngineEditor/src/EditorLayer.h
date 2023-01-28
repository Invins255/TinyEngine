#pragma once

#include "TinyEngine.h"
#include "Panels/SceneHierarchyPanel.h"
#include <glm/glm.hpp>

namespace Engine
{
	class EditorLayer : public Engine::Layer
	{
	public:
		EditorLayer();
		virtual ~EditorLayer() = default;

		virtual void OnAttach() override;
		virtual void OnDetach() override;

		virtual void OnUpdate(Engine::Timestep ts) override;
		virtual void OnEvent(Engine::Event& e) override;
		virtual void OnImGuiRender() override;
	private:
		glm::vec2 m_ViewportSize = {0.0f, 0.0f};
		bool m_ViewportFocused = false;
		bool m_ViewportHovered = false;

		Ref<FrameBuffer> m_FrameBuffer;

		Ref<Scene> m_ActiveScene;

		//Panels
		SceneHierarchyPanel m_SceneHierarchyPanel;
	};

}