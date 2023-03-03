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

namespace Engine
{
	std::unordered_map<UUID, Scene*> Scene::s_ActiveScenes;

	Ref<Scene> Scene::GetScene(UUID uuid)
	{
		if (s_ActiveScenes.find(uuid) != s_ActiveScenes.end())
			return Ref<Scene>(s_ActiveScenes.at(uuid));
		return {};
	}

	Scene::Scene(const std::string& name)
		:m_Name(name)
	{
		m_SceneID = UUID();
		m_SceneEntity = m_Registry.create();
		m_Registry.emplace<SceneComponent>(m_SceneEntity, m_SceneID);

		s_ActiveScenes[m_SceneID] = this;

		Init();
	}

	Scene::~Scene()
	{
		m_Registry.clear();
		s_ActiveScenes.erase(m_SceneID);
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
		Entity entity { m_Registry.create(), this };
		auto idComponent = entity.AddComponent<IDComponent>();
		entity.AddComponent<TransformComponent>();
		entity.AddComponent<TagComponent>(name);
		m_EntityIDMap[idComponent.ID] = entity;
		return entity;
	}

	Entity Scene::CreateEntity(UUID uuid, const std::string& name)
	{
		Entity entity{ m_Registry.create(), this };
		auto idComponent = entity.AddComponent<IDComponent>(uuid);
		entity.AddComponent<TransformComponent>();
		entity.AddComponent<TagComponent>(name);
		ENGINE_ASSERT(m_EntityIDMap.find(uuid) == m_EntityIDMap.end(), "Entity already exists!");
		m_EntityIDMap[uuid] = entity;
		ENGINE_INFO("Scene '{0}': Create entity '{1}'", m_Name, name);
		return entity;
	}

	void Scene::DestroyEntity(Entity entity)
	{
		std::string tag = entity.GetComponent<TagComponent>().Tag;
		m_Registry.destroy(entity);
		ENGINE_INFO("Scene '{0}': Destroy entity '{1}'", m_Name, tag);
	}

	void Scene::OnUpdate(Timestep ts)
	{
		//TODO: Update physics, scripts
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