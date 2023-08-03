#pragma once

#include "entt.hpp"

#include "Engine/Core/TimeStep.h"
#include "Engine/Core/UUID.h"
#include "Engine/Renderer/Light.h"
#include "Engine/Renderer/Texture.h"
#include "Engine/Renderer/Material.h"
#include "Engine/Scene/Environment.h"
#include "Engine/Renderer/Camera.h"
#include "Engine/Events/Event.h"
#include <unordered_map>

namespace Engine
{
	class Entity;
	using EntityMap = std::unordered_map<UUID, Entity>;

	struct SceneComponent
	{
		UUID SceneID;
	};

	class Scene
	{
		friend class Entity;
		friend class SceneHierarchyPanel;
		friend class SceneRenderer;
		friend class SceneSerializer;

		friend void OnScriptComponentConstruct(entt::registry& registry, entt::entity entity);
		friend void OnScriptComponentDestroy(entt::registry& registry, entt::entity entity);

	public:
		static Scene* GetScene(UUID uuid);

	public:
		Scene(const std::string& name = "Untitled Scene", bool isEditorScene = false);
		~Scene();

		void Init();

		UUID GetUUID() const { return m_SceneID; }

		//Entity operations
		Entity CreateEntity(const std::string& name = "Empty Entity");
		Entity CreateEntity(UUID uuid, const std::string& name = "Empty Entity");
		Entity CreateEntity(Entity parent, const std::string& name = "Empty Entity");
		void DestroyEntity(Entity entity);
		void DuplicateEntity(Entity entity);
		void SetSelectedEntity(entt::entity entity) { m_SelectedEntityHandle = entity; }
		Entity GetSelectedEntity();
		template<typename... T>
		auto GetAllEntitiesWith()
		{
			return m_Registry.view<T...>();
		}
		Entity GetEntityWithUUID(UUID id) const;
		Entity TryGetEntityWithUUID(UUID id) const;
		Entity TryGetEntityWithTag(const std::string& tag);

		void OnUpdate(Timestep ts);
		void OnRenderRuntime(Timestep ts);
		void OnRenderEditor(Timestep ts, const Camera& editorCamera, const glm::mat4& viewMatrix);
		void OnEvent(Event& e);

		void OnRuntimeStart();
		void OnRuntimeStop();

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

		void CopeTo(Ref<Scene>& target);



	private:
		template<typename T>
		void OnComponentAdded(Entity entity, T& component);
	
	private:
		static std::unordered_map<UUID, Scene*> s_ActiveScenes;

	private:
		std::string m_Name;
		entt::registry m_Registry;
		
		UUID m_SceneID;
		entt::entity m_SceneEntity;

		uint32_t m_ViewportWidth = 0, m_ViewportHeight = 0;

		entt::entity m_SelectedEntityHandle = entt::null;
		EntityMap m_EntityIDMap;

		//Lights
		LightEnvironment m_LightEnvironment;

		//Environment
		Environment m_Environment;
		Ref<MaterialInstance> m_SkyboxMaterial;

		//Runtime
		bool m_IsPlaying = false;
	};
}