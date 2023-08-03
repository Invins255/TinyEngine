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
		friend class Scene;
		friend class SceneSerializer;
		friend class ScriptEngine;

	public:
		Entity() = default;
		Entity(entt::entity, Scene* scene);

		template<typename T>
		bool HasComponent()
		{
			return m_Scene->m_Registry.has<T>(m_EntityHandle);
		}

		template<typename T>
		bool HasComponent() const
		{
			return m_Scene->m_Registry.has<T>(m_EntityHandle);
		}

		template<typename T, typename... Args>
		T& AddComponent(Args&&... args)
		{
			ENGINE_ASSERT(!HasComponent<T>(), "Entity already has component!");
			T& component = m_Scene->m_Registry.emplace<T>(m_EntityHandle, std::forward<Args>(args)...);
			//m_Scene->OnComponentAdded<T>(*this, component);
			return component;
		}

		template<typename T>
		T& GetComponent()
		{
			ENGINE_ASSERT(HasComponent<T>(), "Entity does not have component!");
			return m_Scene->m_Registry.get<T>(m_EntityHandle);
		}

		template<typename T>
		T& GetComponent() const
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

		void SetParentUUID(UUID parent)
		{
			GetComponent<RelationshipComponent>().ParentHandle = parent;
		}
		UUID GetParentUUID() const { return GetComponent<RelationshipComponent>().ParentHandle; }
		void SetParent(Entity parent)
		{
			Entity currentParent = GetParent();
			if (currentParent == parent)
				return;

			// If changing parent, remove child from existing parent
			if (currentParent)
				currentParent.RemoveChild(*this);

			// Setting to null is okay
			if (!parent)
				SetParentUUID(0);
			else
				SetParentUUID(parent.ID());

			if (parent)
			{
				auto& parentChildren = parent.GetChildrenUUID();
				UUID uuid = ID();
				if (std::find(parentChildren.begin(), parentChildren.end(), uuid) == parentChildren.end())
					parentChildren.emplace_back(ID());
			}
		}
		Entity GetParent() const { return m_Scene->TryGetEntityWithUUID(GetParentUUID()); }
		std::vector<UUID>& GetChildrenUUID() const { return GetComponent<RelationshipComponent>().Children; }
		bool RemoveChild(Entity child)
		{
			UUID childId = child.ID();
			std::vector<UUID>& children = GetChildrenUUID();
			auto it = std::find(children.begin(), children.end(), childId);
			if (it != children.end())
			{
				children.erase(it);
				return true;
			}
			return false;
		}

		bool IsAncesterOf(Entity entity) const
		{
			const auto& children = GetChildrenUUID();
			if (children.empty())
				return false;
			for (UUID child : children)
			{
				if (child == entity.ID())
					return true;
			}
			for (UUID child : children)
			{
				if (m_Scene->GetEntityWithUUID(child).IsAncesterOf(entity))
					return true;
			}
			return false;
		}
		bool IsDescendantOf(Entity entity) const
		{
			return entity.IsAncesterOf(*this);
		}

		UUID GetSceneUUID() const { return m_Scene->GetUUID(); }

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