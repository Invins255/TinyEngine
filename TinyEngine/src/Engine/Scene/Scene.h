#pragma once

#include "entt.hpp"
#include "Engine/Core/TimeStep.h"
#include "Engine/Core/UUID.h"
#include "Engine/Scene/Light.h"


namespace Engine
{
	class Entity;

	class Scene
	{
		friend class Entity;
		friend class SceneHierarchyPanel;
		friend class SceneRenderer;
		friend class SceneSerializer;

	public:
		Scene(const std::string& name = "Empty Scene");
		~Scene();

		Entity CreateEntity(const std::string& name = "Empty Entity");
		Entity CreateEntity(UUID uuid, const std::string& name = "Empty Entity");
		void DestroyEntity(Entity entity);

		void OnUpdate(Timestep ts);
		void OnViewportResize(uint32_t width, uint32_t height);
	
		const std::string GetName() const { return m_Name; }
		Entity GetMainCameraEntity();
		Light& GetLight() { return m_Light; }
		const Light& GetLight() const { return m_Light; }

	private:
		template<typename T>
		void OnComponentAdded(Entity entity, T& component);
	
	private:
		std::string m_Name;
		entt::registry m_Registry;
		uint32_t m_ViewportWidth, m_ViewportHeight;

		//Lights
		Light m_Light;
		float m_LightMultiplier = 1.0f;
		LightEnvironment m_LightEnvironment;
	};
}