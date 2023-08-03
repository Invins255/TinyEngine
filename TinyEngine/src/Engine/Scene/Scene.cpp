#include "pch.h"
#include "Scene.h"
#include "Engine/Renderer/Renderer.h"
#include "Engine/Renderer/SceneRenderer.h"
#include "Engine/Scene/Component.h"
#include "Engine/Scene/Entity.h"

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Engine/Physics/Physics.h"
#include "Engine/Physics/PhysicsActor.h"

#include "Engine/Script/ScriptEngine.h"

namespace Engine
{
	std::unordered_map<UUID, Scene*> Scene::s_ActiveScenes;

	Scene* Scene::GetScene(UUID uuid)
	{
		if (s_ActiveScenes.find(uuid) != s_ActiveScenes.end())
			return s_ActiveScenes[uuid];
		return {};
	}

	static void OnScriptComponentConstruct(entt::registry& registry, entt::entity entity)
	{
		auto sceneView = registry.view<SceneComponent>();
		UUID sceneID = registry.get<SceneComponent>(sceneView.front()).SceneID;

		auto scene = Scene::s_ActiveScenes[sceneID];
					   
		auto entityID = registry.get<IDComponent>(entity).ID;
		ENGINE_ASSERT(scene->m_EntityIDMap.find(entityID) != scene->m_EntityIDMap.end(),"");
		ScriptEngine::InitScriptEntity(scene->m_EntityIDMap.at(entityID));
	}

	static void OnScriptComponentDestroy(entt::registry& registry, entt::entity entity)
	{
		auto sceneView = registry.view<SceneComponent>();
		UUID sceneID = registry.get<SceneComponent>(sceneView.front()).SceneID;

		auto scene = Scene::s_ActiveScenes[sceneID];

		auto entityID = registry.get<IDComponent>(entity).ID;
		ScriptEngine::OnScriptComponentDestroyed(sceneID, entityID);
	}


	Scene::Scene(const std::string& name, bool isEditorScene)
		:m_Name(name)
	{
		m_Registry.on_construct<ScriptComponent>().connect<&OnScriptComponentConstruct>();
		m_Registry.on_destroy<ScriptComponent>().connect<&OnScriptComponentDestroy>();

		m_SceneID = UUID();
		m_SceneEntity = m_Registry.create();
		m_Registry.emplace<SceneComponent>(m_SceneEntity, m_SceneID);

		s_ActiveScenes[m_SceneID] = this;

		if (!isEditorScene)
			Physics::CreateScene();

		Init();
	}

	Scene::~Scene()
	{
		m_Registry.on_destroy<ScriptComponent>().disconnect();

		m_Registry.clear();
		s_ActiveScenes.erase(m_SceneID);

		ScriptEngine::OnSceneDestruct(m_SceneID);
	}

	void Scene::Init()
	{
		//Skybox material
		auto& skyboxShader = Renderer::GetShaderLibrary().Get("Skybox");
		m_SkyboxMaterial = MaterialInstance::Create(Material::Create(skyboxShader), "Skybox");
		m_SkyboxMaterial->SetFlag(MaterialFlag::DepthTest, false);
	}
	 
	Entity Scene::CreateEntity(const std::string& name)
	{
		return CreateEntity(Entity(), name);
	}

	Entity Scene::CreateEntity(UUID uuid, const std::string& name)
	{
		Entity entity{ m_Registry.create(), this };
		entity.AddComponent<IDComponent>(uuid);
		entity.AddComponent<TagComponent>(name);
		entity.AddComponent<RelationshipComponent>();
		entity.AddComponent<TransformComponent>();
		
		ENGINE_ASSERT(m_EntityIDMap.find(uuid) == m_EntityIDMap.end(), "Entity already exists!");
		m_EntityIDMap[entity.ID()] = entity;
		ENGINE_INFO("Scene '{0}': Create entity '{1}'", m_Name, name);
		return entity;
	}

	Entity Scene::CreateEntity(Entity parent, const std::string& name)
	{
		Entity entity{ m_Registry.create(), this };
		entity.AddComponent<IDComponent>();
		entity.AddComponent<TagComponent>(name);
		entity.AddComponent<RelationshipComponent>();
		entity.AddComponent<TransformComponent>();

		if (parent)
			entity.SetParent(parent);

		m_EntityIDMap[entity.ID()] = entity;
		ENGINE_INFO("Scene '{0}': Create entity '{1}'", m_Name, name);
		return entity;
	}

	void Scene::DestroyEntity(Entity entity)
	{
		if (entity.HasComponent<ScriptComponent>())
			ScriptEngine::OnScriptComponentDestroyed(m_SceneID, entity.ID());

		m_Registry.destroy(entity);

		std::string tag = entity.GetComponent<TagComponent>().Tag;
		ENGINE_INFO("Scene '{0}': Destroy entity '{1}'", m_Name, tag);
	}

	void Scene::OnUpdate(Timestep ts)
	{
		// Update all entities' scripts		
		auto view = m_Registry.view<ScriptComponent>();
		for (auto entity : view)
		{
			Entity e = { entity, this };
			if (ScriptEngine::ModuleExists(e.GetComponent<ScriptComponent>().ModuleName))
				ScriptEngine::OnUpdateEntity(e, ts);
		}
		
		// Update physics
		Physics::Simulate(ts);
	}

	void Scene::OnRenderRuntime(Timestep ts)
	{
		//Get main camera
		Entity cameraEntity = GetMainCameraEntity();
		if (!cameraEntity)
		{
			ENGINE_WARN("Scene '{0}' does not have main camera!", m_Name);
			return;
		}

		SceneCamera& camera = cameraEntity.GetComponent<CameraComponent>();
		glm::mat4 cameraViewMatrix = glm::inverse(cameraEntity.GetComponent<TransformComponent>().GetTransform());
		camera.SetViewportSize(m_ViewportWidth, m_ViewportHeight);

		//Process directional lights
		m_LightEnvironment = LightEnvironment();
		auto& lights = m_Registry.group<DirectionalLightComponent>(entt::get<TransformComponent>);
		uint32_t directionalLightIndex = 0;
		for (auto entity : lights)
		{
			if (directionalLightIndex >= 4) {
				ENGINE_WARN("Too many directional lights! Only support the first 4 lights!");
				break;
			}

			auto [transformComponent, lightComponent] = lights.get<TransformComponent, DirectionalLightComponent>(entity);
			glm::vec3 direction = -glm::normalize(glm::mat3(transformComponent.GetTransform()) * glm::vec3(1.0f));			
			m_LightEnvironment.DirectionalLights[directionalLightIndex++] =
			{
				direction,
				lightComponent.Radiance,
				lightComponent.Intensity,
				static_cast<int>(lightComponent.shadowsType),
				lightComponent.CastShadows
			};
		}

		SceneRenderer::BeginScene(this, { camera, cameraViewMatrix });
		auto group = m_Registry.group<MeshComponent>(entt::get<TransformComponent>);
		for (auto entity : group)
		{
			auto& [meshComponent, transformComponent] = group.get<MeshComponent, TransformComponent>(entity);
			if (meshComponent.Mesh)
			{
				SceneRenderer::SubmitMesh(meshComponent.Mesh, transformComponent.GetTransform());
			}
		}
		SceneRenderer::EndScene();
	}

	void Scene::OnRenderEditor(Timestep ts, const Camera& editorCamera, const glm::mat4& viewMatrix)
	{
		//Process directional lights
		m_LightEnvironment = LightEnvironment();
		auto& lights = m_Registry.group<DirectionalLightComponent>(entt::get<TransformComponent>);
		uint32_t directionalLightIndex = 0;
		for (auto entity : lights)
		{
			if (directionalLightIndex >= 4) {
				ENGINE_WARN("Too many directional lights! Only support the first 4 lights!");
				break;
			}

			auto [transformComponent, lightComponent] = lights.get<TransformComponent, DirectionalLightComponent>(entity);
			glm::vec3 direction = -glm::normalize(glm::mat3(transformComponent.GetTransform()) * glm::vec3(1.0f));
			m_LightEnvironment.DirectionalLights[directionalLightIndex++] =
			{
				direction,
				lightComponent.Radiance,
				lightComponent.Intensity,
				static_cast<int>(lightComponent.shadowsType),
				lightComponent.SamplingRadius,
				lightComponent.CastShadows
			};
		}

		SceneRenderer::BeginScene(this, {editorCamera, viewMatrix});	
		//---------------------------------------------------
		//Render mesh
		//---------------------------------------------------
		auto group = m_Registry.group<MeshComponent>(entt::get<TransformComponent>);
		for (auto entity : group)
		{
			auto& [meshComponent, transformComponent] = group.get<MeshComponent, TransformComponent>(entity);
			if (meshComponent.Mesh)
			{
				SceneRenderer::SubmitMesh(meshComponent.Mesh, transformComponent.GetTransform());
			}
		}	
		//---------------------------------------------------
		//Render debug mesh
		//---------------------------------------------------			
		{
			//Box collider debug mesh
			auto view = m_Registry.view<BoxColliderComponent>();
			for (auto entity : view)
			{
				Entity e = { entity, this };
				auto& collider = e.GetComponent<BoxColliderComponent>();

				if (m_SelectedEntityHandle == entity)
					SceneRenderer::SubmitColliderMesh(collider, e.GetComponent<TransformComponent>().GetTransform());
			}
		}
		{
			//Sphere collider debug mesh
			auto view = m_Registry.view<SphereColliderComponent>();
			for (auto entity : view)
			{
				Entity e = { entity, this };
				auto& collider = e.GetComponent<SphereColliderComponent>();

				if (m_SelectedEntityHandle == entity)
					SceneRenderer::SubmitColliderMesh(collider, e.GetComponent<TransformComponent>().GetTransform());
			}
		}
		{
			//Capsule collider debug mesh
			auto view = m_Registry.view<CapsuleColliderComponent>();
			for (auto entity : view)
			{
				Entity e = { entity, this };
				auto& collider = e.GetComponent<CapsuleColliderComponent>();

				if (m_SelectedEntityHandle == entity)
					SceneRenderer::SubmitColliderMesh(collider, e.GetComponent<TransformComponent>().GetTransform());
			}
		}
		{
			//Mesh collider debug mesh
			auto view = m_Registry.view<MeshColliderComponent>();
			for (auto entity : view)
			{
				Entity e = { entity, this };
				auto& collider = e.GetComponent<MeshColliderComponent>();

				if (m_SelectedEntityHandle == entity)
					SceneRenderer::SubmitColliderMesh(collider, e.GetComponent<TransformComponent>().GetTransform());
			}
		}
		SceneRenderer::EndScene();
	}

	void Scene::OnEvent(Event& e)
	{
	}

	void Scene::OnRuntimeStart()
	{
		//Init runtime scene

		//Init scripts engine scene context
		{
			auto view = m_Registry.view<ScriptComponent>();
			for (auto entity : view)
			{
				Entity e = { entity, this };
				if (ScriptEngine::ModuleExists(e.GetComponent<ScriptComponent>().ModuleName))
					ScriptEngine::InstantiateEntityClass(e);
			}
		}

		//Init physics rigidbody
		{
			auto view = m_Registry.view<RigidBodyComponent>();
			for (auto entity : view)
			{
				Entity e = { entity, this };
				Physics::CreateActor(e);
			}
		}

		m_IsPlaying = true;
	}

	void Scene::OnRuntimeStop()
	{
		//Clear runtime scene
		Physics::DestroyScene();

		m_IsPlaying = false;
	}

	Entity Scene::GetMainCameraEntity()
	{
		auto view = m_Registry.view<CameraComponent>();
		for (auto entity : view)
		{
			auto& camera = view.get<CameraComponent>(entity);
			if (camera.Primary)
				return { entity, this };
		}
		ENGINE_WARN("Conld not find main camera!");
		return {};
	}

	void Scene::SetSkybox(const Ref<TextureCube>& skybox)
	{
		m_Environment.SkyboxMap = skybox;
		m_SkyboxMaterial->Set("u_Skybox", skybox);
	}

	template<typename T> 
	static void CopyComponent(entt::registry& dstRegistry, entt::registry& srcRegistry, const std::unordered_map<UUID, entt::entity>& enttMap)
	{
		auto components = srcRegistry.view<T>();
		for (auto srcEntity : components)
		{
			entt::entity destEntity = enttMap.at(srcRegistry.get<IDComponent>(srcEntity).ID);

			auto& srcComponent = srcRegistry.get<T>(srcEntity);
			auto& destComponent = dstRegistry.emplace_or_replace<T>(destEntity, srcComponent);
		}
	}

	void Scene::CopeTo(Ref<Scene>& target)
	{
		target->m_LightEnvironment = m_LightEnvironment;
		target->m_Environment = m_Environment;

		target->SetViewportSize(m_ViewportWidth, m_ViewportHeight);

		std::unordered_map<UUID, entt::entity> enttMap;
		auto idComponents = m_Registry.view<IDComponent>();
		for (auto entity : idComponents)
		{
			auto uuid = m_Registry.get<IDComponent>(entity).ID;
			Entity e = target->CreateEntity(uuid, "Runtime Entity");
			enttMap[uuid] = e.m_EntityHandle;
		}

		CopyComponent<TagComponent>(target->m_Registry, m_Registry, enttMap);
		CopyComponent<TransformComponent>(target->m_Registry, m_Registry, enttMap);
		CopyComponent<MeshComponent>(target->m_Registry, m_Registry, enttMap);
		CopyComponent<DirectionalLightComponent>(target->m_Registry, m_Registry, enttMap);
		CopyComponent<CameraComponent>(target->m_Registry, m_Registry, enttMap);
		CopyComponent<RigidBodyComponent>(target->m_Registry, m_Registry, enttMap);
		CopyComponent<PhysicsMaterialComponent>(target->m_Registry, m_Registry, enttMap);
		CopyComponent<BoxColliderComponent>(target->m_Registry, m_Registry, enttMap);
		CopyComponent<SphereColliderComponent>(target->m_Registry, m_Registry, enttMap);
		CopyComponent<CapsuleColliderComponent>(target->m_Registry, m_Registry, enttMap);
		CopyComponent<MeshColliderComponent>(target->m_Registry, m_Registry, enttMap);
		CopyComponent<ScriptComponent>(target->m_Registry, m_Registry, enttMap);
		//TODO: copy components

		const auto& entityInstanceMap = ScriptEngine::GetEntityInstanceMap();
		if (entityInstanceMap.find(target->GetUUID()) != entityInstanceMap.end())
			ScriptEngine::CopyEntityScriptData(target->GetUUID(), m_SceneID);
	}

	template<typename T>
	static void CopyComponentIfExists(entt::entity dst, entt::entity src, entt::registry& registry)
	{
		if (registry.has<T>(src))
		{
			auto& srcComponent = registry.get<T>(src);
			registry.emplace_or_replace<T>(dst, srcComponent);
		}
	}

	void Scene::DuplicateEntity(Entity entity)
	{
		Entity newEntity;
		if (entity.HasComponent<TagComponent>())
			newEntity = CreateEntity(entity.GetComponent<TagComponent>().Tag);
		else
			newEntity = CreateEntity();

		CopyComponentIfExists<TransformComponent>(newEntity.m_EntityHandle, entity.m_EntityHandle, m_Registry);
		CopyComponentIfExists<MeshComponent>(newEntity.m_EntityHandle, entity.m_EntityHandle, m_Registry);
		CopyComponentIfExists<DirectionalLightComponent>(newEntity.m_EntityHandle, entity.m_EntityHandle, m_Registry);
		CopyComponentIfExists<ScriptComponent>(newEntity.m_EntityHandle, entity.m_EntityHandle, m_Registry);
		CopyComponentIfExists<CameraComponent>(newEntity.m_EntityHandle, entity.m_EntityHandle, m_Registry);
		CopyComponentIfExists<SpriteRendererComponent>(newEntity.m_EntityHandle, entity.m_EntityHandle, m_Registry);
		CopyComponentIfExists<RigidBodyComponent>(newEntity.m_EntityHandle, entity.m_EntityHandle, m_Registry);
		CopyComponentIfExists<PhysicsMaterialComponent>(newEntity.m_EntityHandle, entity.m_EntityHandle, m_Registry);
		CopyComponentIfExists<BoxColliderComponent>(newEntity.m_EntityHandle, entity.m_EntityHandle, m_Registry);
		CopyComponentIfExists<SphereColliderComponent>(newEntity.m_EntityHandle, entity.m_EntityHandle, m_Registry);
		CopyComponentIfExists<CapsuleColliderComponent>(newEntity.m_EntityHandle, entity.m_EntityHandle, m_Registry);
		CopyComponentIfExists<MeshColliderComponent>(newEntity.m_EntityHandle, entity.m_EntityHandle, m_Registry);
	}

	Entity Scene::GetEntityWithUUID(UUID id) const
	{
		ENGINE_ASSERT(m_EntityIDMap.find(id) != m_EntityIDMap.end(), "Invalid entity ID or entity doesn't exist in scene!");
		return m_EntityIDMap.at(id);
	}

	Entity Scene::TryGetEntityWithUUID(UUID id) const
	{
		if (const auto iter = m_EntityIDMap.find(id); iter != m_EntityIDMap.end())
			return iter->second;
		return Entity{};
	}

	Entity Scene::TryGetEntityWithTag(const std::string& tag)
	{
		auto entities = GetAllEntitiesWith<TagComponent>();
		for (auto e : entities)
		{
			if (entities.get<TagComponent>(e).Tag == tag)
				return Entity(e, this);
		}
		return Entity{};
	}

	Entity Scene::GetSelectedEntity()
	{
		return Entity(m_SelectedEntityHandle, this);
	}

	template<typename T>
	void Scene::OnComponentAdded(Entity entity, T& component)
	{
	}

	template<>
	void Scene::OnComponentAdded<TransformComponent>(Entity entity, TransformComponent& component)
	{
	}

	template<>
	void Scene::OnComponentAdded<MeshComponent>(Entity entity, MeshComponent& component)
	{
	}

	template<>
	void Scene::OnComponentAdded<CameraComponent>(Entity entity, CameraComponent& component)
	{
		component.Camera.SetViewportSize(m_ViewportWidth, m_ViewportHeight);
	}

	template<>
	void Scene::OnComponentAdded<SpriteRendererComponent>(Entity entity, SpriteRendererComponent& component)
	{
	}

	template<>
	void Scene::OnComponentAdded<TagComponent>(Entity entity, TagComponent& component)
	{
	}

	template<>
	void Scene::OnComponentAdded<DirectionalLightComponent>(Entity entity, DirectionalLightComponent& component)
	{
	}
}