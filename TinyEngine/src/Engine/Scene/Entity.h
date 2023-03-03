#pragma once

#include "entt.hpp"
#include "Engine/Core/Core.h"
#include "Engine/Core/Ref.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Scene/Entity.h"
#include "Engine/Scene/Component.h"
#include <glm/glm.hpp>

namespace Engine
{
	class Entity
	{
	public:
		friend class Scene;
		friend class SceneSerializer;

	public:
		Entity() = default;
		Entity(entt::entity, Scene* scene);

		template<typename T>
		bool HasComponent()
		{
			return m_Scene->m_Registry.has<T>(m_EntityHandle);
		}

		template<typename T, typename... Args>
		T& AddComponent(Args&&... args)
		{
			ENGINE_ASSERT(!HasComponent<T>(), "Entity already has component!");
			T& component = m_Scene->m_Registry.emplace<T>(m_EntityHandle, std::forward<Args>(args)...);
			m_Scene->OnComponentAdded<T>(*this, component);
			return component;
		}

		template<typename T>
		T& GetComponent()
		{
			ENGINE_ASSERT(HasComponent<T>(), "Entity does not have component!");
			return m_Scene->m_Registry.get<T>(m_EntityHandle);
		}

		template<typename T>
		void RemoveComponent()
		{
			ENGINE_ASSERT(HasComponent<T>(), "Entity does not have component!");
			m_Scene->m_Registry.remove<T>(m_EntityHandle);
		}

		UUID ID() { return GetIDComponent().ID; }
		std::string Tag() { return GetTagComponent().Tag; }
		glm::mat4 Transform() { return GetTransformComponent().GetTransform(); }
		IDComponent& GetIDComponent() { return GetComponent<IDComponent>(); }
		TagComponent& GetTagComponent() { return GetComponent<TagComponent>(); }
		TransformComponent& GetTransformComponent() { return GetComponent<TransformComponent>(); }

		operator bool() const { return m_EntityHandle != entt::null; }
		operator uint32_t() const { return (uint32_t)m_EntityHandle; }
		operator entt::entity() const { return m_EntityHandle; }

		bool operator==(const Entity& other) const
		{
			return m_Scene == other.m_Scene && m_EntityHandle == other.m_EntityHandle;
		}
		bool operator!=(const Entity& other) const
		{
			return !(*this == other);
		}
	private:
		entt::entity m_EntityHandle = entt::null;
		Scene* m_Scene = nullptr;
	};
}