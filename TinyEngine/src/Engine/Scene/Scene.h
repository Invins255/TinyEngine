#pragma once

#include "entt.hpp"
#include "Engine/Core/TimeStep.h"


namespace Engine
{
	class Entity;

	class Scene
	{
	public:
		Scene(const std::string& name = "Scene");
		~Scene();

		Entity CreateEntity(const std::string& name = "");
		void DestroyEntity(Entity entity);

		void OnUpdate(Timestep ts);
		void OnViewportResize(uint32_t width, uint32_t height);
	
		Entity GetMainCameraEntity();

	private:
		template<typename T>
		void OnComponentAdded(Entity entity, T& component);
	
	private:
		std::string m_Name;
		entt::registry m_Registry;
		uint32_t m_ViewportWidth, m_ViewportHeight;

		friend class Entity;
		friend class SceneHierarchyPanel;
	};
}