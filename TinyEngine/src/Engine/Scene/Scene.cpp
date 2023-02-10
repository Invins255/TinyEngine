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
	Scene::Scene(const std::string& name)
		:m_Name(name)
	{
		//Skybox texture
		m_SkyboxTexture = TextureCube::Create(
			"assets/textures/skybox/Sky/right.jpg",
			"assets/textures/skybox/Sky/left.jpg",
			"assets/textures/skybox/Sky/top.jpg",
			"assets/textures/skybox/Sky/bottom.jpg",
			"assets/textures/skybox/Sky/front.jpg",
			"assets/textures/skybox/Sky/back.jpg"
		);

		//Skybox material
		auto& skyboxShader = Renderer::GetShaderLibrary()->Get("Skybox");
		m_SkyboxMaterial = MaterialInstance::Create(Material::Create(skyboxShader));
		m_SkyboxMaterial->SetFlag(MaterialFlag::DepthTest, false);
	}

	Scene::~Scene()
	{
		m_Registry.clear();
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
		//Get main camera
		Entity cameraEntity = GetMainCameraEntity();
		if (!cameraEntity)
		{
			ENGINE_WARN("Scene '{0}' does not have main camera!", m_Name);
			return;
		}

		SceneCamera& camera = cameraEntity.GetComponent<CameraComponent>();
		glm::mat4 cameraViewMatrix = glm::inverse(cameraEntity.GetComponent<TransformComponent>().GetTransform());

		//Process lights
		m_LightEnvironment = LightEnvironment();
		auto& lights = m_Registry.group<DirectionalLightComponent>(entt::get<TransformComponent>);
		uint32_t directionalLightIndex = 0;
		for (auto entity : lights)
		{
			if (directionalLightIndex >= 4) {
				ENGINE_WARN("Too many directional lights!");
				break;
			}

			auto [transformComponent, lightComponent] = lights.get<TransformComponent, DirectionalLightComponent>(entity);
			glm::vec3 direction = -glm::normalize(glm::mat3(transformComponent.GetTransform()) * glm::vec3(1.0f));
			m_LightEnvironment.DirectionalLights[directionalLightIndex++] =
			{
				direction,
				lightComponent.Radiance,
				lightComponent.Intensity
			};
		}

		//Skybox
		SetSkybox(m_SkyboxTexture);

		SceneRenderer::BeginScene(this, {camera, cameraViewMatrix});
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

	void Scene::OnViewportResize(uint32_t width, uint32_t height)
	{
		m_ViewportWidth = width;
		m_ViewportHeight = height;

		//Resize non-FixedAspectRatio camera
		auto view = m_Registry.view<CameraComponent>();
		for (auto entity : view)
		{
			auto& cameraComponent = view.get<CameraComponent>(entity);
			if (!cameraComponent.FixedAspectRatio)
			{
				cameraComponent.Camera.SetViewportSize(width, height);
			}
		}
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
		m_SkyboxTexture = skybox;
		m_SkyboxMaterial->Set("u_Skybox", skybox);
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