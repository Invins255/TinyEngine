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

	private:
		void DrawEntityNode(Entity entity);
		void DrawComponents(Entity entity);
		void DrawAddComponentMenu();
		
		//Unused
		//void DrawMeshNode(const Ref<Mesh>& mesh, uint32_t& imguiMeshID);
		//void MeshNodeHierarchy(const Ref<Mesh>& mesh, aiNode* node, const glm::mat4& parentTransform = glm::mat4(1.0f), uint32_t level = 0);
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
