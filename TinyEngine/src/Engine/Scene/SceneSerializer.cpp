#include "pch.h"
#include "SceneSerializer.h"

#include "Engine/Scene/Entity.h"
#include "Engine/Scene/Component.h"

#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "yaml-cpp/yaml.h"

namespace YAML
{
	template<>
	struct convert<glm::vec2>
	{
		static Node encode(const glm::vec2& rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			return node;
		}

		static bool decode(const Node& node, glm::vec2& rhs)
		{
			if (!node.IsSequence() || node.size() != 2)
				return false;

			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			return true;
		}
	};

	template<>
	struct convert<glm::vec3>
	{
		static Node encode(const glm::vec3& rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.push_back(rhs.z);
			return node;
		}

		static bool decode(const Node& node, glm::vec3& rhs)
		{
			if (!node.IsSequence() || node.size() != 3)
				return false;

			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			rhs.z = node[2].as<float>();
			return true;
		}
	};

	template<>
	struct convert<glm::vec4>
	{
		static Node encode(const glm::vec4& rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.push_back(rhs.z);
			node.push_back(rhs.w);
			return node;
		}

		static bool decode(const Node& node, glm::vec4& rhs)
		{
			if (!node.IsSequence() || node.size() != 4)
				return false;

			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			rhs.z = node[2].as<float>();
			rhs.w = node[3].as<float>();
			return true;
		}
	};

	template<>
	struct convert<glm::quat>
	{
		static Node encode(const glm::quat& rhs)
		{
			Node node;
			node.push_back(rhs.w);
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.push_back(rhs.z);
			return node;
		}

		static bool decode(const Node& node, glm::quat& rhs)
		{
			if (!node.IsSequence() || node.size() != 4)
				return false;

			rhs.w = node[0].as<float>();
			rhs.x = node[1].as<float>();
			rhs.y = node[2].as<float>();
			rhs.z = node[3].as<float>();
			return true;
		}
	};
}

namespace Engine
{
	YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec2& v)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << v.x << v.y << YAML::EndSeq;
		return out;
	}

	YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec3& v)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << v.x << v.y << v.z << YAML::EndSeq;
		return out;
	}


	YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec4& v)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << v.x << v.y << v.z << v.w << YAML::EndSeq;
		return out;
	}

	YAML::Emitter& operator<<(YAML::Emitter& out, const glm::quat& v)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << v.w << v.x << v.y << v.z << YAML::EndSeq;
		return out;
	}

	//--------------------------------------------------------------------------------
	// SceneSerializer
	//--------------------------------------------------------------------------------
	SceneSerializer::SceneSerializer(const Ref<Scene>& scene)
		: m_Scene(scene)
	{
	}

	static void SerializeEntity(YAML::Emitter& out, Entity entity)
	{
		
		out << YAML::BeginMap;

		//Entity ID
		out << YAML::Key << "Entity" << YAML::Value << entity.GetComponent<IDComponent>().ID;

		//TODO:More components
		if (entity.HasComponent<TagComponent>())
		{
			out << YAML::Key << "TagComponent";
			out << YAML::BeginMap; 			
			out << YAML::Key << "Tag" << YAML::Value << entity.GetComponent<TagComponent>().Tag;
			out << YAML::EndMap; 
		}
		if (entity.HasComponent<TransformComponent>())
		{
			out << YAML::Key << "TransformComponent";
			out << YAML::BeginMap; 
			auto& transform = entity.GetComponent<TransformComponent>();
			out << YAML::Key << "Position" << YAML::Value << transform.Translation;
			out << YAML::Key << "Rotation" << YAML::Value << transform.Rotation;
			out << YAML::Key << "Scale" << YAML::Value << transform.Scale;
			out << YAML::EndMap; 
		}
		if (entity.HasComponent<MeshComponent>())
		{
			out << YAML::Key << "MeshComponent";
			out << YAML::BeginMap; 
			auto mesh = entity.GetComponent<MeshComponent>().Mesh;
			out << YAML::Key << "AssetPath" << YAML::Value << mesh->GetFilePath();
			out << YAML::EndMap; 
		}
		if (entity.HasComponent<CameraComponent>())
		{
			out << YAML::Key << "CameraComponent";
			out << YAML::BeginMap;
			auto& cameraComponent = entity.GetComponent<CameraComponent>();
			out << YAML::Key << "Camera" << YAML::Value << "some camera data..."; //TODO: Camera data
			out << YAML::Key << "Primary" << YAML::Value << cameraComponent.Primary;
			out << YAML::EndMap;
		}
		if (entity.HasComponent<DirectionalLightComponent>())
		{
			out << YAML::Key << "DirectionalLightComponent";
			out << YAML::BeginMap;
			auto& directionalLightComponent = entity.GetComponent<DirectionalLightComponent>();
			out << YAML::Key << "Radiance" << YAML::Value << directionalLightComponent.Radiance;
			out << YAML::EndMap; 
		}

		out << YAML::EndMap;
		
	}

	static void SerializeEnvironment(YAML::Emitter& out, const Ref<Scene>& scene)
	{
		out << YAML::Key << "Environment"; 
		out << YAML::Value;
		out << YAML::BeginMap; // Environment
		//TODO: Environment data
		out << YAML::EndMap; //Environment		
	}

	void SceneSerializer::Serialize(const std::string& filepath)
	{
		YAML::Emitter out;
		out << YAML::BeginMap;
		out << YAML::Key << "Scene";
		out << YAML::Value << m_Scene->GetName();
		SerializeEnvironment(out, m_Scene);

		//Serialize entities
		out << YAML::Key << "Entities";
		out << YAML::Value << YAML::BeginSeq;
		m_Scene->m_Registry.each([&](auto entityID)
			{
				//BUG: ¿ÉÄÜ´íÎó
				Entity entity = { entityID, m_Scene.get()};
				if (!entity || !entity.HasComponent<IDComponent>())
					return;

				SerializeEntity(out, entity);
			});
		out << YAML::EndSeq;

		//TODO: More

		
		std::ofstream fout(filepath);
		fout << out.c_str();
	}

	bool SceneSerializer::Deserialize(const std::string& filepath)
	{
		std::ifstream stream(filepath);
		std::stringstream strStream;
		strStream << stream.rdbuf();

		YAML::Node data = YAML::Load(strStream.str());
		if (!data["Scene"])
			return false;

		std::string sceneName = data["Scene"].as<std::string>();
		ENGINE_INFO("Deserialize scene '{0}'", sceneName);

		//Deserialize entities
		auto entities = data["Entities"];
		if (!entities)
		{
			for (auto entity : entities)
			{
				uint64_t uuid = entity["Entity"].as<uint64_t>();

				std::string tag;
				auto tagComponent = entity["TagComponent"];
				if (tagComponent)
					tag = tagComponent["Tag"].as<std::string>();

				Entity deserializedEntity = m_Scene->CreateEntity(uuid, tag);
				ENGINE_INFO("Deserialize entity: ID = {0}, tag = {1}", uuid, tag);

				auto transformComponent = entity["TransformComponent"];
				if (transformComponent)
				{
					auto& transform = deserializedEntity.GetComponent<TransformComponent>();
					transform.Translation = transformComponent["Position"].as<glm::vec3>();
					transform.Rotation = transformComponent["Rotation"].as<glm::vec3>();
					transform.Scale = transformComponent["Scale"].as<glm::vec3>();
				
					ENGINE_INFO("	Entity Transform:");
					ENGINE_INFO("		Position: {0}, {1}, {2}", transform.Translation.x, transform.Translation.y, transform.Translation.z);
					ENGINE_INFO("		Rotation: {0}, {1}, {2}", transform.Rotation.x, transform.Rotation.y, transform.Rotation.z);
					ENGINE_INFO("		Scale:	  {0}, {1}, {2}", transform.Scale.x, transform.Scale.y, transform.Scale.z);
				}

				auto meshComponent = entity["MeshComponent"];
				if (meshComponent)
				{
					std::string meshPath = meshComponent["AssetPath"].as<std::string>();
					if (!deserializedEntity.HasComponent<MeshComponent>())
						deserializedEntity.AddComponent<MeshComponent>(CreateRef<Mesh>(meshPath));

					ENGINE_INFO("	Mesh:");
					ENGINE_INFO("		Asset Path: {0}", meshPath);
				}

				auto cameraComponent = entity["CameraComponent"];
				if (cameraComponent)
				{
					auto& component = deserializedEntity.AddComponent<CameraComponent>();
					component.Camera = SceneCamera(); //TODO: Camera data
					component.Primary = cameraComponent["Primary"].as<bool>();

					ENGINE_INFO("	Camera:");
					ENGINE_INFO("		Primary: {0}", component.Primary);
				}

				auto directionalLightComponent = entity["DirectionalLightComponent"];
				if (directionalLightComponent)
				{
					auto& light = deserializedEntity.AddComponent<DirectionalLightComponent>();
					light.Radiance = directionalLightComponent["Radiance"].as<glm::vec3>();

					ENGINE_INFO("	DirectionalLight:");
					ENGINE_INFO("		Radiance: {0}, {1}, {2}", light.Radiance.x, light.Radiance.y, light.Radiance.z);
				}
			}
		}

		return true;
	}
}
