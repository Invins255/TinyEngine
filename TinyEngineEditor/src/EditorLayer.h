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

		void NewScene();
		void OpenScene();
		void OpenScene(const std::string& filepath);
		void SaveScene();
		void SaveSceneAs();

	private:
		glm::vec2 m_ViewportSize = {0.0f, 0.0f};
		bool m_ViewportFocused = false;
		bool m_ViewportHovered = false;

		//Scene
		Ref<Scene> m_EditorScene;
		Ref<Scene> m_RuntimeScene;
		std::string m_SceneFilePath;

		//Panels
		SceneHierarchyPanel m_SceneHierarchyPanel;
	};

}