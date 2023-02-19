#pragma once

#include "Engine/Core/Log.h"
#include "Engine/Core/Core.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Scene/Entity.h"
#include "Engine/Renderer/Mesh.h"

namespace Engine
{
	class SceneHierarchyPanel
	{
	public:
		SceneHierarchyPanel() = default;
		SceneHierarchyPanel(const Ref<Scene>& context);

		void SetContext(const Ref<Scene>& context);
		void OnImGuiRender();

		Ref<Scene> GetContext() const { return m_Context; }
		Entity GetSelectedEntity() const { return m_SelectionContext; }

	private:
		void DrawEntityNode(Entity entity);
		void DrawComponents(Entity entity);
		void DrawAddComponentMenu();
	private:
		template<typename T, typename UIFunction>
		void DrawComponent(const std::string& name, Entity entity, UIFunction uiFunction);

		template<typename T>
		void DrawAddComponentButton(const std::string& name);

	private:
		Ref<Scene> m_Context;
		Entity m_SelectionContext;
	};
}
