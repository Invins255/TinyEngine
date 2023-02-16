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

		//Entity ID, Tag
		uint64_t uuid = entity.GetComponent<IDComponent>().ID;
		std::string tag = entity.GetComponent<TagComponent>().Tag;
		SERIALIZER_INFO("Serialize entity: ID = {0}, tag = {1}", uuid, tag);

		out << YAML::Key << "Entity" << YAML::Value << uuid;

		out << YAML::Key << "TagComponent";
		out << YAML::BeginMap; 			
		out << YAML::Key << "Tag" << YAML::Value << tag;
		out << YAML::EndMap; 

		if (entity.HasComponent<TransformComponent>())
		{
			out << YAML::Key << "TransformComponent";
			out << YAML::BeginMap; 
			auto& transform = entity.GetComponent<TransformComponent>();
			out << YAML::Key << "Position" << YAML::Value << transform.Translation;
			out << YAML::Key << "Rotation" << YAML::Value << transform.Rotation;
			out << YAML::Key << "Scale" << YAML::Value << transform.Scale;
			out << YAML::EndMap; 

			SERIALIZER_INFO("	Entity Transform:");
			SERIALIZER_INFO("		Position: {0}, {1}, {2}", transform.Translation.x, transform.Translation.y, transform.Translation.z);
			SERIALIZER_INFO("		Rotation: {0}, {1}, {2}", transform.Rotation.x, transform.Rotation.y, transform.Rotation.z);
			SERIALIZER_INFO("		Scale:	  {0}, {1}, {2}", transform.Scale.x, transform.Scale.y, transform.Scale.z);
		}
		if (entity.HasComponent<MeshComponent>())
		{
			//TODO: 对程序化生成的Mesh进行序列化

			out << YAML::Key << "MeshComponent";
			out << YAML::BeginMap; 
			auto mesh = entity.GetComponent<MeshComponent>().Mesh;
			out << YAML::Key << "AssetPath" << YAML::Value << mesh->GetFilePath();
			out << YAML::EndMap; 

			SERIALIZER_INFO("	Mesh:");
			SERIALIZER_INFO("		Asset Path: {0}", mesh->GetFilePath());
		}
		if (entity.HasComponent<CameraComponent>())
		{
			out << YAML::Key << "CameraComponent";
			out << YAML::BeginMap;
			auto& cameraComponent = entity.GetComponent<CameraComponent>();
			out << YAML::Key << "Camera"; 
			out << YAML::BeginMap; // Camera Data

			auto& camera = cameraComponent.Camera;
			std::string type;
			switch (camera.GetProjectionType())
			{
			case SceneCamera::ProjectionType::Perspective:
				type = "Perspective";
				break;
			case SceneCamera::ProjectionType::Orthographic:
				type = "Orthographic";
				break;
			}
			out << YAML::Key << "ProjectionType" << YAML::Value << type;
			out << YAML::Key << "Orthographic";
			out << YAML::BeginMap; // Orthographic
			out << YAML::Key << "Size" << YAML::Value << camera.GetOrthographicSize();
			out << YAML::Key << "Near" << YAML::Value << camera.GetOrthographicNearClip();
			out << YAML::Key << "Far" << YAML::Value << camera.GetOrthographicFarClip();
			out << YAML::EndMap; // Orthographic
			out << YAML::Key << "Perspective";
			out << YAML::BeginMap; // Perspective
			out << YAML::Key << "FOV" << YAML::Value << camera.GetDegPerspectiveFOV();
			out << YAML::Key << "Near" << YAML::Value << camera.GetPerspectiveNearClip();
			out << YAML::Key << "Far" << YAML::Value << camera.GetPerspectiveFarClip();
			out << YAML::EndMap; // Perspective
			out << YAML::Key << "AspectRatio" << YAML::Value << camera.GetAspectRatio();
			out << YAML::EndMap; // Camera Data
			out << YAML::Key << "Primary" << YAML::Value << cameraComponent.Primary;
			out << YAML::EndMap;

			SERIALIZER_INFO("	Camera:");
			SERIALIZER_INFO("		Type: {0}", type);
			SERIALIZER_INFO("		Orthographic:");
			SERIALIZER_INFO("			Size: {0}", camera.GetOrthographicSize());
			SERIALIZER_INFO("			Near: {0}", camera.GetOrthographicNearClip());
			SERIALIZER_INFO("			Far:  {0}", camera.GetOrthographicFarClip());
			SERIALIZER_INFO("		Perspective:");
			SERIALIZER_INFO("			FOV:  {0}", camera.GetDegPerspectiveFOV());
			SERIALIZER_INFO("			Near: {0}", camera.GetPerspectiveNearClip());
			SERIALIZER_INFO("			Far:  {0}", camera.GetPerspectiveFarClip());
			SERIALIZER_INFO("		AspectRatio: {0}", camera.GetAspectRatio());
			SERIALIZER_INFO("		Primary: {0}", cameraComponent.Primary);
		}
		if (entity.HasComponent<DirectionalLightComponent>())
		{
			out << YAML::Key << "DirectionalLightComponent";
			out << YAML::BeginMap;
			auto& light = entity.GetComponent<DirectionalLightComponent>();
			out << YAML::Key << "Radiance" << YAML::Value << light.Radiance;
			out << YAML::Key << "Intensity" << YAML::Value << light.Intensity;
			out << YAML::Key << "CastShadows" << YAML::Value << light.CastShadows;
			out << YAML::EndMap; 

			SERIALIZER_INFO("	DirectionalLight:");
			SERIALIZER_INFO("		Radiance: {0}, {1}, {2}", light.Radiance.x, light.Radiance.y, light.Radiance.z);
			SERIALIZER_INFO("		Intensity: {0}", light.Intensity);
			SERIALIZER_INFO("		CastShadows: {0}", light.CastShadows);
		}

		out << YAML::EndMap;
		
	}

	static void SerializeEnvironment(YAML::Emitter& out, const Ref<Scene>& scene)
	{
		out << YAML::Key << "Environment"; 
		out << YAML::Value;
		out << YAML::BeginMap; // Environment
		out << YAML::Key << "Skybox";
		out << YAML::Value;
		out << YAML::BeginMap; // Skybox
		out << YAML::Key << "AssetPath";
		out << YAML::Value;
		out << YAML::BeginMap;// path
		auto skyboxPath = scene->GetEnvironment().SkyboxMap->GetPath();
		out << YAML::Key << "Left" << YAML::Value << skyboxPath[1];
		out << YAML::Key << "Right" << YAML::Value << skyboxPath[0];
		out << YAML::Key << "Top" << YAML::Value << skyboxPath[2];
		out << YAML::Key << "Bottom" << YAML::Value << skyboxPath[3];
		out << YAML::Key << "Front" << YAML::Value << skyboxPath[4];
		out << YAML::Key << "Back" << YAML::Value << skyboxPath[5];
		out << YAML::EndMap; // Path
		out << YAML::EndMap; // Skybox		

		//TODO: Environment data
		out << YAML::EndMap; //Environment		

		SERIALIZER_INFO("Environment:");
		SERIALIZER_INFO("	Skybox:");
		SERIALIZER_INFO("		Left:	{0}", skyboxPath[1]);
		SERIALIZER_INFO("		Right:	{0}", skyboxPath[0]);
		SERIALIZER_INFO("		Top:	{0}", skyboxPath[2]);
		SERIALIZER_INFO("		Bottom: {0}", skyboxPath[3]);
		SERIALIZER_INFO("		Front:	{0}", skyboxPath[4]);
		SERIALIZER_INFO("		Back:	{0}", skyboxPath[5]);
	}

	void SceneSerializer::Serialize(const std::string& filepath)
	{
		YAML::Emitter out;
		out << YAML::BeginMap;
		out << YAML::Key << "Scene";
		out << YAML::Value << m_Scene->GetName();
		SERIALIZER_INFO("Serialize scene '{0}'", m_Scene->GetName());

		//Serialize environment
		SerializeEnvironment(out, m_Scene);

		//Serialize entities
		out << YAML::Key << "Entities";
		out << YAML::Value << YAML::BeginSeq;
		m_Scene->m_Registry.each([&](auto entityID)
			{
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
		m_Scene->m_Name = sceneName;
		SERIALIZER_INFO("Deserialize scene '{0}'", sceneName);

		//Deserialize environment
		auto environment = data["Environment"];
		if (environment)
		{
			auto skybox = environment["Skybox"];
			auto skyboxPath = skybox["AssetPath"];
			auto skyboxTexture = TextureCube::Create(
				skyboxPath["Right"].as<std::string>(),
				skyboxPath["Left"].as<std::string>(),
				skyboxPath["Top"].as<std::string>(),
				skyboxPath["Bottom"].as<std::string>(),
				skyboxPath["Front"].as<std::string>(),
				skyboxPath["Back"].as<std::string>()
			);
			m_Scene->SetSkybox(skyboxTexture);
		}

		//Deserialize entities
		auto entities = data["Entities"];
		if (entities)
		{
			for (auto entity : entities)
			{
				uint64_t uuid = entity["Entity"].as<uint64_t>();

				std::string tag;
				auto tagComponent = entity["TagComponent"];
				if (tagComponent)
					tag = tagComponent["Tag"].as<std::string>();

				Entity deserializedEntity = m_Scene->CreateEntity(uuid, tag);
				SERIALIZER_INFO("Deserialize entity: ID = {0}, tag = {1}", uuid, tag);

				auto transformComponent = entity["TransformComponent"];
				if (transformComponent)
				{
					auto& transform = deserializedEntity.GetComponent<TransformComponent>();
					transform.Translation = transformComponent["Position"].as<glm::vec3>();
					transform.Rotation = transformComponent["Rotation"].as<glm::vec3>();
					transform.Scale = transformComponent["Scale"].as<glm::vec3>();
				
					SERIALIZER_INFO("	Entity Transform:");
					SERIALIZER_INFO("		Position: {0}, {1}, {2}", transform.Translation.x, transform.Translation.y, transform.Translation.z);
					SERIALIZER_INFO("		Rotation: {0}, {1}, {2}", transform.Rotation.x, transform.Rotation.y, transform.Rotation.z);
					SERIALIZER_INFO("		Scale:	  {0}, {1}, {2}", transform.Scale.x, transform.Scale.y, transform.Scale.z);
				}

				auto meshComponent = entity["MeshComponent"];
				if (meshComponent)
				{
					std::string meshPath = meshComponent["AssetPath"].as<std::string>();
					if (!deserializedEntity.HasComponent<MeshComponent>())
						deserializedEntity.AddComponent<MeshComponent>(CreateRef<Mesh>(meshPath));

					SERIALIZER_INFO("	Mesh:");
					SERIALIZER_INFO("		Asset Path: {0}", meshPath);
				}

				auto cameraComponent = entity["CameraComponent"];
				if (cameraComponent)
				{
					auto& component = deserializedEntity.AddComponent<CameraComponent>();
					component.Camera = SceneCamera();

					auto cameraData = cameraComponent["Camera"];
					std::string typeStr = cameraData["ProjectionType"].as<std::string>();
					SceneCamera::ProjectionType type;
					if (typeStr == "Perspective")	
						type = SceneCamera::ProjectionType::Perspective;
					else							
						type = SceneCamera::ProjectionType::Orthographic;
					component.Camera.SetProjectionType(type);
					auto orthoData = cameraData["Orthographic"];
					float orthoSize = orthoData["Size"].as<float>();
					float orthoNear = orthoData["Near"].as<float>();
					float orthoFar = orthoData["Far"].as<float>();
					component.Camera.SetOrthographicSize(orthoSize);
					component.Camera.SetOrthographicNearClip(orthoNear);
					component.Camera.SetOrthographicFarClip(orthoFar);
					auto persData = cameraData["Perspective"];
					float persFOV = persData["FOV"].as<float>();
					float persNear = persData["Near"].as<float>();
					float persFar = persData["Far"].as<float>();
					component.Camera.SetDegPerspectiveFOV(persFOV);
					component.Camera.SetOrthographicNearClip(persFOV);
					component.Camera.SetPerspectiveFarClip(persFar);
					float aspectRatio = cameraData["AspectRatio"].as<float>();
					component.Camera.SetAspectRatio(aspectRatio);
					component.Primary = cameraComponent["Primary"].as<bool>();
					
					if (type == SceneCamera::ProjectionType::Perspective)
						component.Camera.SetPerspectiveProjection(glm::radians(persFOV), aspectRatio, persNear, persFar);
					else
						component.Camera.SetOrthographicProjection(orthoSize, orthoSize, orthoNear, orthoFar);

					SERIALIZER_INFO("	Camera:");
					SERIALIZER_INFO("		Type: {0}", typeStr);
					SERIALIZER_INFO("		Orthographic:");
					SERIALIZER_INFO("			Size: {0}", orthoData["Size"].as<float>());
					SERIALIZER_INFO("			Near: {0}", orthoData["Near"].as<float>());
					SERIALIZER_INFO("			Far:  {0}", orthoData["Far"].as<float>());
					SERIALIZER_INFO("		Perspective:");
					SERIALIZER_INFO("			FOV:  {0}", persData["FOV"].as<float>());
					SERIALIZER_INFO("			Near: {0}", persData["Near"].as<float>());
					SERIALIZER_INFO("			Far:  {0}", persData["Far"].as<float>());
					SERIALIZER_INFO("		AspectRatio: {0}", cameraData["AspectRatio"].as<float>());
					SERIALIZER_INFO("		Primary: {0}", component.Primary);
				}

				auto directionalLightComponent = entity["DirectionalLightComponent"];
				if (directionalLightComponent)
				{
					auto& light = deserializedEntity.AddComponent<DirectionalLightComponent>();
					light.Radiance = directionalLightComponent["Radiance"].as<glm::vec3>();
					light.Intensity = directionalLightComponent["Intensity"].as<float>();
					light.CastShadows = directionalLightComponent["CastShadows"].as<bool>();

					SERIALIZER_INFO("	DirectionalLight:");
					SERIALIZER_INFO("		Radiance: {0}, {1}, {2}", light.Radiance.x, light.Radiance.y, light.Radiance.z);
					SERIALIZER_INFO("		Intensity: {0}", light.Intensity);
					SERIALIZER_INFO("		CastShadows: {0}", light.CastShadows);
				}
			}
		}

		return true;
	}
}
