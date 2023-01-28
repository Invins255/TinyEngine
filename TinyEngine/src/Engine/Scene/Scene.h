#pragma once

#include "entt.hpp"
#include "Engine/Core/TimeStep.h"

namespace Engine
{
	class Entity;

	class Scene
	{
	public:
		Scene();
		~Scene();

		Entity CreateEntity(const std::string& name = std::string());
		void DestroyEntity(Entity entity);
		void OnUpdate(Timestep ts);
		void OnViewportResize(uint32_t width, uint32_t height);
	
	private:
		template<typename T>
		void OnComponentAdded(Entity entity, T& component);
	
	private:
		entt::registry m_Registry;
		uint32_t m_ViewportWidth, m_ViewportHeight;

		friend class Entity;
		friend class SceneHierarchyPanel;
	};
}