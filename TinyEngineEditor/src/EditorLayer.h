#pragma once

#include "TinyEngine.h"
#include "Editor/Panels/SceneHierarchyPanel.h"
#include "Editor/Panels/ContentBrowserPanel.h"
#include "Editor/EditorCamera.h"

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
		bool OnKeyPressedEvent(KeyPressedEvent& e);
		bool OnMouseButtonPressed(MouseButtonPressedEvent& e);

		void NewScene();
		void OpenScene();
		void OpenScene(const std::string& filepath);
		void SaveScene();
		void SaveSceneAs();

	private:
		glm::vec2 m_ViewportSize = {0.0f, 0.0f};
		bool m_ViewportFocused = false;
		bool m_ViewportHovered = false;
			
		EditorCamera m_EditorCamera;

		//Scene
		Ref<Scene> m_EditorScene;
		std::string m_SceneFilePath;

		//Panels
		SceneHierarchyPanel m_SceneHierarchyPanel;
		ContentBrowserPanel m_ContentBrowserPanel;

		//Selection
		enum class SelectionMode
		{
			None = 0, Entity, SubMesh
		};
		SelectionMode m_SelectionMode = SelectionMode::Entity;

		struct SelectedSubmesh
		{
			Engine::Entity Entity;
			Submesh* Mesh = nullptr;
		};
		std::vector<SelectedSubmesh> m_SelectionContext;
	};

}