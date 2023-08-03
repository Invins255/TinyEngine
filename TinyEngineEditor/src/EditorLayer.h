#pragma once

#include "TinyEngine.h"
#include "Editor/Panels/SceneHierarchyPanel.h"
#include "Editor/Panels/ContentBrowserPanel.h"
#include "Editor/Panels/MaterialEditorPanel.h"
#include "Editor/Panels/PhysicsSettingsPanel.h"
#include "Editor/EditorCamera.h"

#include <glm/glm.hpp>

namespace Engine
{
	class EditorLayer : public Engine::Layer
	{
	private:
		enum class SceneState
		{
			Edit, Play, Pause
		};

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
		void ClearSelectContext();

	private:
		std::pair<float, float> GetMouseViewportSpace() const;
		std::pair<glm::vec3, glm::vec3> CastMouseRay(float mx, float my);

		void OnEntitySelected(SelectedSubmesh& selectionContext);
		void OnEntityDeleted(Entity e);

		void OnScenePlay();
		void OnSceneStop();

		float GetSnapValue();

		void UI_MenuBar();
		void UI_GizmosToolBar();
		void UI_ToolBar();
	private:
		glm::vec2 m_ViewportBounds[2];
		bool m_ViewportFocused = false;
		bool m_ViewportHovered = false;
			
		EditorCamera m_EditorCamera;

		SceneState m_SceneState = SceneState::Edit;

		//Scene
		Ref<Scene> m_EditorScene;
		Ref<Scene> m_RuntimeScene;
		std::string m_SceneFilePath;

		//Script
		bool m_ReloadScriptOnPlay = true;

		//Panels
		SceneHierarchyPanel m_SceneHierarchyPanel;
		ContentBrowserPanel m_ContentBrowserPanel;
		MaterialEditorPanel m_MaterialEditorPanel;

		bool m_ShowPhysicsSettings = false;

		//Selection
		SelectionMode m_SelectionMode = SelectionMode::Entity;
		std::vector<SelectedSubmesh> m_SelectionContext;

		//Gizmo
		int m_GizmoType = -1; //No gizmo
		Ref<Texture2D> m_SelectIcon;
		Ref<Texture2D> m_MoveIcon;
		Ref<Texture2D> m_RotateIcon;
		Ref<Texture2D> m_ScaleIcon;

		Ref<Texture2D> m_PlayIcon;
		Ref<Texture2D> m_StopIcon;
		Ref<Texture2D> m_PauseIcon;
	};

}