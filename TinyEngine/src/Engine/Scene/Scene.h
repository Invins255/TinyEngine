#pragma once

#include "entt.hpp"
#include "Engine/Core/TimeStep.h"
#include "Engine/Core/UUID.h"
#include "Engine/Scene/Light.h"
#include "Engine/Renderer/Texture.h"
#include "Engine/Renderer/Material.h"

#include <unordered_map>

namespace Engine
{
	class Entity;
	using EntityMap = std::unordered_map<UUID, Entity>;

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
		void SetName(const std::string& name) { m_Name = name; }

		const EntityMap& GetEntityMap() const { return m_EntityIDMap; }
		Entity GetMainCameraEntity();
		Light& GetLight() { return m_Light; }
		const Light& GetLight() const { return m_Light; }
		
		void SetSkybox(const Ref<TextureCube>& skybox);

	private:
		template<typename T>
		void OnComponentAdded(Entity entity, T& component);
	
	private:
		std::string m_Name;
		entt::registry m_Registry;
		uint32_t m_ViewportWidth = 0, m_ViewportHeight = 0;

		EntityMap m_EntityIDMap;

		//Lights
		Light m_Light;
		float m_LightMultiplier = 1.0f;
		LightEnvironment m_LightEnvironment;

		//Environment
		Ref<TextureCube> m_SkyboxTexture;
		Ref<MaterialInstance> m_SkyboxMaterial;
	};
}