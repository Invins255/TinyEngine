#pragma once

#include "TinyEngine.h"
#include "Editor/Panels/SceneHierarchyPanel.h"
#include "Editor/Panels/ContentBrowserPanel.h"
#include "Editor/Panels/MaterialEditorPanel.h"
#include "Editor/EditorCamera.h"

#include <glm/glm.hpp>

namespace Engine
{
	class EditorLayer : public Engine::Layer
	{
	private:
		enum class SelectionMode
		{
			None = 0, Entity, SubMesh
		};
		
		struct SelectedSubmesh
		{
			Engine::Entity Entity;
			Submesh* Mesh = nullptr;
			float Distance = 0.0f;
		};

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

		void SelectEntity(Entity entity);

	private:
		std::pair<float, float> GetMouseViewportSpace() const;
		std::pair<glm::vec3, glm::vec3> CastMouseRay(float mx, float my);

		void OnEntitySelected(SelectedSubmesh& selectionContext);
		void OnEntityDeleted(Entity e);

		float GetSnapValue();
	private:
		glm::vec2 m_ViewportBounds[2];
		bool m_ViewportFocused = false;
		bool m_ViewportHovered = false;
			
		EditorCamera m_EditorCamera;

		//Scene
		Ref<Scene> m_EditorScene;
		std::string m_SceneFilePath;

		//Panels
		SceneHierarchyPanel m_SceneHierarchyPanel;
		ContentBrowserPanel m_ContentBrowserPanel;
		MaterialEditorPanel m_MaterialEditorPanel;

		//Selection
		SelectionMode m_SelectionMode = SelectionMode::Entity;
		std::vector<SelectedSubmesh> m_SelectionContext;

		//Gizmo
		int m_GizmoType = -1; //No gizmo
	};

}