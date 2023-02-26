#pragma once

#include "entt.hpp"

#include "Engine/Core/TimeStep.h"
#include "Engine/Core/UUID.h"
#include "Engine/Renderer/Light.h"
#include "Engine/Renderer/Texture.h"
#include "Engine/Renderer/Material.h"
#include "Engine/Scene/Environment.h"
#include "Engine/Renderer/Camera.h"

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
		Scene(const std::string& name = "Untitled Scene");
		~Scene();

		//Entity operations
		Entity CreateEntity(const std::string& name = "Empty Entity");
		Entity CreateEntity(UUID uuid, const std::string& name = "Empty Entity");
		void DestroyEntity(Entity entity);	
		void SetSelectedEntity(entt::entity entity) { m_SelectedEntityHandle = entity; }
		Entity GetSelectedEntity();
		template<typename T>
		auto GetAllEntitiesWith()
		{
			return m_Registry.view<T>();
		}

		void OnUpdate(Timestep ts);
		void OnRenderRuntime(Timestep ts);
		void OnRenderEditor(Timestep ts, const Camera& editorCamera, const glm::mat4& viewMatrix);
	
		const std::string GetName() const { return m_Name; }
		void SetName(const std::string& name) { m_Name = name; }

		void SetViewportSize(uint32_t width, uint32_t height)
		{
			m_ViewportWidth = width;
			m_ViewportHeight = height;
		}

		const EntityMap& GetEntityMap() const { return m_EntityIDMap; }
		Entity GetMainCameraEntity();

		const Environment& GetEnvironment() const { return m_Environment; }
		void SetEnvironment(Environment& environment) { m_Environment = environment; }
		void SetSkybox(const Ref<TextureCube>& skybox);

	private:
		template<typename T>
		void OnComponentAdded(Entity entity, T& component);
	
	private:
		std::string m_Name;
		entt::registry m_Registry;
		uint32_t m_ViewportWidth = 0, m_ViewportHeight = 0;

		entt::entity m_SelectedEntityHandle = entt::null;
		EntityMap m_EntityIDMap;

		//Lights
		LightEnvironment m_LightEnvironment;

		//Environment
		Environment m_Environment;
		Ref<MaterialInstance> m_SkyboxMaterial;
	};
}