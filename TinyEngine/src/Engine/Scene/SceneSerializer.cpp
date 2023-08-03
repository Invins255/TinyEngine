#include "pch.h"
#include "SceneSerializer.h"

#include "Engine/Scene/Entity.h"
#include "Engine/Scene/Component.h"
#include "Engine/Renderer/MeshFactory.h"
#include "Engine/Physics/Physics.h"
#include "Engine/Physics/PhysicsLayer.h"
#include "Engine/Physics/PXPhysicsWrappers.h"
#include "Engine/Script/ScriptEngine.h"

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
		
		out << YAML::BeginMap;	//Entity

		//Entity ID, Tag
		uint64_t uuid = entity.GetComponent<IDComponent>().ID;
		std::string tag = entity.GetComponent<TagComponent>().Tag;
		SERIALIZER_INFO("Serialize entity: ID = {0}, tag = {1}", uuid, tag);

		out << YAML::Key << "Entity" << YAML::Value << uuid;
		if(entity.HasComponent<TagComponent>())
		{ 
			out << YAML::Key << "TagComponent";
			out << YAML::BeginMap; //TagComponent
			out << YAML::Key << "Tag" << YAML::Value << tag;
			out << YAML::EndMap; //TagComponent
		}
		if (entity.HasComponent<TransformComponent>())
		{
			out << YAML::Key << "TransformComponent";
			out << YAML::BeginMap; //TransformComponent
			auto& transform = entity.GetComponent<TransformComponent>();
			out << YAML::Key << "Position" << YAML::Value << transform.Translation;
			out << YAML::Key << "Rotation" << YAML::Value << transform.Rotation;
			out << YAML::Key << "Scale" << YAML::Value << transform.Scale;
			out << YAML::EndMap; //TransformComponent
		}
		if (entity.HasComponent<MeshComponent>())
		{
			//TODO: 对程序化生成的Mesh进行序列化

			out << YAML::Key << "MeshComponent";
			out << YAML::BeginMap; //MeshComponent
			auto mesh = entity.GetComponent<MeshComponent>().Mesh;
			out << YAML::Key << "AssetPath" << YAML::Value << mesh->GetFilePath();
			out << YAML::EndMap; //MeshComponent
		}
		if (entity.HasComponent<CameraComponent>())
		{
			out << YAML::Key << "CameraComponent";
			out << YAML::BeginMap; //CameraComponent
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
			out << YAML::EndMap; //CameraComponent
		}
		if (entity.HasComponent<DirectionalLightComponent>())
		{
			out << YAML::Key << "DirectionalLightComponent";
			out << YAML::BeginMap; //DirectionalLightComponent
			auto& light = entity.GetComponent<DirectionalLightComponent>();
			out << YAML::Key << "Radiance" << YAML::Value << light.Radiance;
			out << YAML::Key << "Intensity" << YAML::Value << light.Intensity;
			out << YAML::Key << "CastShadows" << YAML::Value << light.CastShadows;
			out << YAML::EndMap; //DirectionalLightComponent
		}
		if (entity.HasComponent<RigidBodyComponent>())
		{
			out << YAML::Key << "RigidBodyComponent";
			out << YAML::BeginMap; //RigidBodyComponent
			auto& rbc = entity.GetComponent<RigidBodyComponent>();
			out << YAML::Key << "BodyType" << YAML::Value << (int)rbc.BodyType;
			out << YAML::Key << "Mass" << YAML::Value << rbc.Mass;
			out << YAML::Key << "LinearDrag" << YAML::Value << rbc.LinearDrag;
			out << YAML::Key << "AngularDrag" << YAML::Value << rbc.AngularDrag;
			out << YAML::Key << "IsKinematic" << YAML::Value << rbc.IsKinematic;
			out << YAML::Key << "Layer" << YAML::Value << rbc.Layer;

			out << YAML::Key << "Constraints";
			out << YAML::BeginMap;	// Constraints
			out << YAML::Key << "LockPositionX" << YAML::Value << rbc.LockPositionX;
			out << YAML::Key << "LockPositionY" << YAML::Value << rbc.LockPositionY;
			out << YAML::Key << "LockPositionZ" << YAML::Value << rbc.LockPositionZ;
			out << YAML::Key << "LockRotationX" << YAML::Value << rbc.LockRotationX;
			out << YAML::Key << "LockRotationY" << YAML::Value << rbc.LockRotationY;
			out << YAML::Key << "LockRotationZ" << YAML::Value << rbc.LockRotationZ;
			out << YAML::EndMap;	// Constraints

			out << YAML::EndMap; //RigidBodyComponent
		}
		if (entity.HasComponent<PhysicsMaterialComponent>())
		{
			out << YAML::Key << "PhysicsMaterialComponent";
			out << YAML::BeginMap; //PhysicsMaterialComponent
			auto& pmc = entity.GetComponent<PhysicsMaterialComponent>();
			out << YAML::Key << "StaticFriction" << YAML::Value << pmc.StaticFriction;
			out << YAML::Key << "DynamicFriction" << YAML::Value << pmc.DynamicFriction;
			out << YAML::Key << "Bounciness" << YAML::Value << pmc.Bounciness;
			out << YAML::EndMap; //PhysicsMaterialComponent
		}
		if (entity.HasComponent<BoxColliderComponent>())
		{
			out << YAML::Key << "BoxColliderComponent";
			out << YAML::BeginMap; //BoxColliderComponent
			auto& bcc = entity.GetComponent<BoxColliderComponent>();
			out << YAML::Key << "Offset" << YAML::Value << bcc.Offset;
			out << YAML::Key << "Size" << YAML::Value << bcc.Size;
			out << YAML::Key << "IsTrigger" << YAML::Value << bcc.IsTrigger;
			out << YAML::EndMap; //BoxColliderComponent
		}
		if (entity.HasComponent<SphereColliderComponent>())
		{
			out << YAML::Key << "SphereColliderComponent";
			out << YAML::BeginMap; //SphereColliderComponent
			auto& scc = entity.GetComponent<SphereColliderComponent>();
			out << YAML::Key << "Radius" << YAML::Value << scc.Radius;
			out << YAML::Key << "IsTrigger" << YAML::Value << scc.IsTrigger;
			out << YAML::EndMap; //SphereColliderComponent
		}
		if (entity.HasComponent<CapsuleColliderComponent>())
		{
			out << YAML::Key << "CapsuleColliderComponent";
			out << YAML::BeginMap; //CapsuleColliderComponent
			auto& ccc = entity.GetComponent<CapsuleColliderComponent>();
			out << YAML::Key << "Radius" << YAML::Value << ccc.Radius;
			out << YAML::Key << "Height" << YAML::Value << ccc.Height;
			out << YAML::Key << "IsTrigger" << YAML::Value << ccc.IsTrigger;
			out << YAML::EndMap; //CapsuleColliderComponent
		}
		if (entity.HasComponent<MeshColliderComponent>())
		{
			out << YAML::Key << "MeshColliderComponent";
			out << YAML::BeginMap; //MeshColliderComponent
			auto& mcc = entity.GetComponent<MeshColliderComponent>();
			if (mcc.OverrideMesh)
				out << YAML::Key << "AssetPath" << YAML::Value << mcc.CollisionMesh->GetFilePath();
			out << YAML::Key << "IsConvex" << YAML::Value << mcc.IsConvex;
			out << YAML::Key << "IsTrigger" << YAML::Value << mcc.IsTrigger;
			out << YAML::Key << "OverrideMesh" << YAML::Value << mcc.OverrideMesh;
			out << YAML::EndMap; //MeshColliderComponent
		}
		if (entity.HasComponent<ScriptComponent>())
		{
			out << YAML::Key << "ScriptComponent";
			out << YAML::BeginMap; // ScriptComponent

			auto& moduleName = entity.GetComponent<ScriptComponent>().ModuleName;
			out << YAML::Key << "ModuleName" << YAML::Value << moduleName;

			EntityInstanceData& data = ScriptEngine::GetEntityInstanceData(entity.GetSceneUUID(), uuid);
			const auto& moduleFieldMap = data.ModuleFieldMap;
			if (moduleFieldMap.find(moduleName) != moduleFieldMap.end())
			{
				const auto& fields = moduleFieldMap.at(moduleName);
				out << YAML::Key << "StoredFields" << YAML::Value;
				out << YAML::BeginSeq;
				for (const auto& [name, field] : fields)
				{
					out << YAML::BeginMap; // Field
					out << YAML::Key << "Name" << YAML::Value << name;
					out << YAML::Key << "Type" << YAML::Value << (uint32_t)field.Type;
					out << YAML::Key << "Data" << YAML::Value;

					switch (field.Type)
					{
					case FieldType::Int:
						out << field.GetStoredValue<int>();
						break;
					case FieldType::UnsignedInt:
						out << field.GetStoredValue<uint32_t>();
						break;
					case FieldType::Float:
						out << field.GetStoredValue<float>();
						break;
					case FieldType::Vec2:
						out << field.GetStoredValue<glm::vec2>();
						break;
					case FieldType::Vec3:
						out << field.GetStoredValue<glm::vec3>();
						break;
					case FieldType::Vec4:
						out << field.GetStoredValue<glm::vec4>();
						break;
					}
					out << YAML::EndMap; // Field
				}
				out << YAML::EndSeq;
			}

			out << YAML::EndMap; // ScriptComponent
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
		auto skyboxPath = scene->GetEnvironment().SkyboxMap->GetPath();
		out << YAML::Value << skyboxPath;
		out << YAML::EndMap; // Skybox		

		//TODO: Environment data
		out << YAML::EndMap; //Environment		
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
		out << YAML::Value << YAML::BeginSeq;	//Entities seq
 		m_Scene->m_Registry.each([&](auto entityID)
			{
				Entity entity = { entityID, m_Scene.get()};
				if (!entity || !entity.HasComponent<IDComponent>())
					return;

				SerializeEntity(out, entity);
			});
		out << YAML::EndSeq;	//Entities seq

		//Serialize physics
		out << YAML::Key << "PhysicsLayers";
		out << YAML::Value << YAML::BeginSeq;	//Layers seq
		for (const auto& layer : PhysicsLayerManager::GetLayers())
		{
			// Never serialize the Default layer
			if (layer.LayerID == 0)
				continue;

			out << YAML::BeginMap;
			out << YAML::Key << "Name" << YAML::Value << layer.Name;

			out << YAML::Key << "CollidesWith" << YAML::Value;
			out << YAML::BeginSeq;
			for (const auto& collidingLayer : PhysicsLayerManager::GetLayerCollisions(layer.LayerID))
			{
				out << YAML::BeginMap;
				out << YAML::Key << "Name" << YAML::Value << collidingLayer.Name;
				out << YAML::EndMap;
			}
			out << YAML::EndSeq;
			out << YAML::EndMap;
		}
		out << YAML::EndSeq;	//Layers seq

		//TODO: More

		out << YAML::EndMap;
		
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
			auto envAssetPath = skybox["AssetPath"].as<std::string>();
			auto environment = Environment::Create(envAssetPath);

			m_Scene->SetEnvironment(environment);
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
				}

				auto meshComponent = entity["MeshComponent"];
				if (meshComponent)
				{
					std::string meshPath = meshComponent["AssetPath"].as<std::string>();
					if (!deserializedEntity.HasComponent<MeshComponent>())
						deserializedEntity.AddComponent<MeshComponent>(CreateRef<Mesh>(meshPath));
				
					SERIALIZER_INFO("	MeshComponent");
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
				
					SERIALIZER_INFO("	CameraComponent");
				}

				auto directionalLightComponent = entity["DirectionalLightComponent"];
				if (directionalLightComponent)
				{
					auto& light = deserializedEntity.AddComponent<DirectionalLightComponent>();
					light.Radiance = directionalLightComponent["Radiance"].as<glm::vec3>();
					light.Intensity = directionalLightComponent["Intensity"].as<float>();
					light.CastShadows = directionalLightComponent["CastShadows"].as<bool>();

					SERIALIZER_INFO("	DirectionalLightComponent");
				}

				auto rigidBodyComponent = entity["RigidBodyComponent"];
				if (rigidBodyComponent)
				{
					auto& component = deserializedEntity.AddComponent<RigidBodyComponent>();
					component.BodyType = (RigidBodyComponent::Type)rigidBodyComponent["BodyType"].as<int>();
					component.Mass = rigidBodyComponent["Mass"].as<float>();
					component.LinearDrag = rigidBodyComponent["LinearDrag"] ? rigidBodyComponent["LinearDrag"].as<float>() : 0.0f;
					component.AngularDrag = rigidBodyComponent["AngularDrag"] ? rigidBodyComponent["AngularDrag"].as<float>() : 0.05f;
					component.DisableGravity = rigidBodyComponent["DisableGravity"] ? rigidBodyComponent["DisableGravity"].as<bool>() : false;
					component.IsKinematic = rigidBodyComponent["IsKinematic"] ? rigidBodyComponent["IsKinematic"].as<bool>() : false;
					component.Layer = rigidBodyComponent["Layer"] ? rigidBodyComponent["Layer"].as<uint32_t>() : 0;

					component.LockPositionX = rigidBodyComponent["Constraints"]["LockPositionX"].as<bool>();
					component.LockPositionY = rigidBodyComponent["Constraints"]["LockPositionY"].as<bool>();
					component.LockPositionZ = rigidBodyComponent["Constraints"]["LockPositionZ"].as<bool>();
					component.LockRotationX = rigidBodyComponent["Constraints"]["LockRotationX"].as<bool>();
					component.LockRotationY = rigidBodyComponent["Constraints"]["LockRotationY"].as<bool>();
					component.LockRotationZ = rigidBodyComponent["Constraints"]["LockRotationZ"].as<bool>();

					SERIALIZER_INFO("	RigidBodyComponent");
				}

				auto physicsMaterialComponent = entity["PhysicsMaterialComponent"];
				if (physicsMaterialComponent)
				{
					auto& component = deserializedEntity.AddComponent<PhysicsMaterialComponent>();
					component.StaticFriction = physicsMaterialComponent["StaticFriction"].as<float>();
					component.DynamicFriction = physicsMaterialComponent["DynamicFriction"].as<float>();
					component.Bounciness = physicsMaterialComponent["Bounciness"].as<float>();

					SERIALIZER_INFO("	PhysicsMaterialComponent");
				}

				auto boxColliderComponent = entity["BoxColliderComponent"];
				if (boxColliderComponent)
				{
					auto& component = deserializedEntity.AddComponent<BoxColliderComponent>();
					component.Offset = boxColliderComponent["Offset"].as<glm::vec3>();
					component.Size = boxColliderComponent["Size"].as<glm::vec3>();
					component.IsTrigger = boxColliderComponent["IsTrigger"] ? boxColliderComponent["IsTrigger"].as<bool>() : false;
					component.DebugMesh = MeshFactory::CreateBox(component.Size);

					SERIALIZER_INFO("	BoxColliderComponent");
				}

				auto sphereColliderComponent = entity["SphereColliderComponent"];
				if (sphereColliderComponent)
				{
					auto& component = deserializedEntity.AddComponent<SphereColliderComponent>();
					component.Radius = sphereColliderComponent["Radius"].as<float>();
					component.IsTrigger = sphereColliderComponent["IsTrigger"] ? sphereColliderComponent["IsTrigger"].as<bool>() : false;
					component.DebugMesh = MeshFactory::CreateSphere(component.Radius);
					
					SERIALIZER_INFO("	SphereColliderComponent");
				}

				auto capsuleColliderComponent = entity["CapsuleColliderComponent"];
				if (capsuleColliderComponent)
				{
					auto& component = deserializedEntity.AddComponent<CapsuleColliderComponent>();
					component.Radius = capsuleColliderComponent["Radius"].as<float>();
					component.Height = capsuleColliderComponent["Height"].as<float>();
					component.IsTrigger = capsuleColliderComponent["IsTrigger"] ? capsuleColliderComponent["IsTrigger"].as<bool>() : false;
					component.DebugMesh = MeshFactory::CreateCapsule(component.Radius, component.Height);

					SERIALIZER_INFO("	CapsuleColliderComponent");
				}

				auto meshColliderComponent = entity["MeshColliderComponent"];
				if (meshColliderComponent)
				{
					Ref<Mesh> collisionMesh = deserializedEntity.HasComponent<MeshComponent>() ? deserializedEntity.GetComponent<MeshComponent>().Mesh : nullptr;
					bool overrideMesh = meshColliderComponent["OverrideMesh"] ? meshColliderComponent["OverrideMesh"].as<bool>() : false;

					if (overrideMesh)
					{
						std::string meshPath = meshColliderComponent["AssetPath"].as<std::string>();
						collisionMesh = CreateRef<Mesh>(meshPath);
					}

					if (collisionMesh)
					{
						auto& component = deserializedEntity.AddComponent<MeshColliderComponent>(collisionMesh);
						component.IsConvex = meshColliderComponent["IsConvex"] ? meshColliderComponent["IsConvex"].as<bool>() : false;
						component.IsTrigger = meshColliderComponent["IsTrigger"] ? meshColliderComponent["IsTrigger"].as<bool>() : false;
						component.OverrideMesh = overrideMesh;

						if (component.IsConvex)
							PXPhysicsWrappers::CreateConvexMesh(component, deserializedEntity.GetTransformComponent().Scale);
						else
							PXPhysicsWrappers::CreateTriangleMesh(component, deserializedEntity.GetTransformComponent().Scale);
					}
					else
					{
						SERIALIZER_WARN("MeshColliderComponent in use without valid mesh!");
					}

					SERIALIZER_INFO("	MeshColliderComponent");
				}

				auto scriptComponent = entity["ScriptComponent"];
				if (scriptComponent)
				{
					std::string moduleName = scriptComponent["ModuleName"].as<std::string>();
					deserializedEntity.AddComponent<ScriptComponent>(moduleName);

					if (ScriptEngine::ModuleExists(moduleName))
					{
						auto storedFields = scriptComponent["StoredFields"];
						if (storedFields)
						{
							for (auto field : storedFields)
							{
								std::string name = field["Name"].as<std::string>();
								FieldType type = (FieldType)field["Type"].as<uint32_t>();
								EntityInstanceData& data = ScriptEngine::GetEntityInstanceData(m_Scene->GetUUID(), uuid);
								auto& moduleFieldMap = data.ModuleFieldMap;
								auto& publicFields = moduleFieldMap[moduleName];
								if (publicFields.find(name) == publicFields.end())
								{
									PublicField pf = { name, type };
									publicFields.emplace(name, std::move(pf));
								}
								auto dataNode = field["Data"];
								switch (type)
								{
								case FieldType::Float:
								{
									publicFields.at(name).SetStoredValue(dataNode.as<float>());
									break;
								}
								case FieldType::Int:
								{
									publicFields.at(name).SetStoredValue(dataNode.as<int32_t>());
									break;
								}
								case FieldType::UnsignedInt:
								{
									publicFields.at(name).SetStoredValue(dataNode.as<uint32_t>());
									break;
								}
								case FieldType::String:
								{
									ENGINE_ASSERT(false, "Unimplemented", "");
									break;
								}
								case FieldType::Vec2:
								{
									publicFields.at(name).SetStoredValue(dataNode.as<glm::vec2>());
									break;
								}
								case FieldType::Vec3:
								{
									publicFields.at(name).SetStoredValue(dataNode.as<glm::vec3>());
									break;
								}
								case FieldType::Vec4:
								{
									publicFields.at(name).SetStoredValue(dataNode.as<glm::vec4>());
									break;
								}
								}
							}
						}
					}

					SERIALIZER_INFO("	ScriptComponent");
				}
			}
		}

		//Deserialize physics
		auto physicsLayers = data["PhysicsLayers"];
		if (physicsLayers)
		{
			for (auto layer : physicsLayers)
			{
				PhysicsLayerManager::AddLayer(layer["Name"].as<std::string>(), false);
			}

			for (auto layer : physicsLayers)
			{
				const PhysicsLayer& layerInfo = PhysicsLayerManager::GetLayer(layer["Name"].as<std::string>());

				auto collidesWith = layer["CollidesWith"];
				if (collidesWith)
				{
					for (auto collisionLayer : collidesWith)
					{
						const auto& otherLayer = PhysicsLayerManager::GetLayer(collisionLayer["Name"].as<std::string>());
						PhysicsLayerManager::SetLayerCollision(layerInfo.LayerID, otherLayer.LayerID, true);
					}
				}
			}
		}

		return true;
	}
}
