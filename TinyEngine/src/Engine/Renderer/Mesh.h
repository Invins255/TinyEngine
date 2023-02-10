#pragma once

#include <memory>
#include <vector>
#include <glm/glm.hpp>
#include "Engine/Core/TimeStep.h"
#include "Engine/Renderer/Pipeline.h"
#include "Engine/Renderer/Material.h"
#include "Engine/Renderer/Shader.h"

struct aiScene;
struct aiNode;

namespace Assimp {
	class Importer;
}

namespace Engine
{

#define MESH_DEBUG 1
#if MESH_DEBUG
#define MESH_INFO(...) ENGINE_INFO(__VA_ARGS__)
#else
#define MESH_INFO(...)
#endif

	struct Vertex
	{
		glm::vec3 Position	= glm::vec3(0.0f);
		glm::vec3 Normal	= glm::vec3(0.0f);
		glm::vec3 Tangent	= glm::vec3(0.0f);
		glm::vec3 Binormal	= glm::vec3(0.0f);
		glm::vec2 Texcoord	= glm::vec2(0.0f);
	};

	struct Index
	{
		uint32_t V1 = 0, V2 = 0, V3 = 0;
	};

	struct Triangle
	{
		Vertex V0, V1, V2;

		Triangle(const Vertex& v0, const Vertex& v1, const Vertex& v2)
			: V0(v0), V1(v1), V2(v2)
		{}
	};

	struct Submesh
	{
		std::string NodeName;
		std::string MeshName;

		uint32_t BaseVertex		= 0;
		uint32_t BaseIndex		= 0;
		uint32_t MaterialIndex	= 0;
		uint32_t IndexCount		= 0;
		uint32_t VertexCount	= 0;

		glm::mat4 Transform = glm::mat4(1.0f);
	};

	class Mesh
	{
		friend class Renderer;
		friend class SceneHierarchyPanel;

	public:
		Mesh(const std::string& filename);
		Mesh(const std::vector<Vertex>& vertices, const std::vector<Index>& indices, const glm::mat4& transform = glm::mat4(1.0f));
		~Mesh();

		void OnUpdate(Timestep ts);
		void DumpVertexBuffer();

		const std::string& GetFilePath() const { return m_FilePath; }
		std::vector<Submesh>& GetSubmeshes() { return m_Submeshes; }
		const std::vector<Submesh>& GetSubmeshes() const { return m_Submeshes; }
		const std::vector<Vertex>& GetStaticVertices() const { return m_StaticVertices; }
		const std::vector<Index>& GetIndices() const { return m_Indices; }

		Ref<Material> GetMaterial() { return m_BaseMaterial; }
		std::vector<Ref<MaterialInstance>> GetMaterials() { return m_Materials; }

	private:
		void TraverseNodes(aiNode* node, const glm::mat4& parentTransform = glm::mat4(1.0f), uint32_t level = 0);

	private:
		std::string m_FilePath;

		std::vector<Submesh> m_Submeshes;

		//TODO: Use VertexArrayBuffer
		Ref<Pipeline> m_Pipeline;
		Ref<VertexBuffer> m_VertexBuffer;
		Ref<IndexBuffer> m_IndexBuffer;

		std::vector<Vertex> m_StaticVertices;
		std::vector<Index> m_Indices;

		//Material
		Ref<Shader> m_MeshShader;
		Ref<Material> m_BaseMaterial;
		std::vector<Ref<MaterialInstance>> m_Materials;
		std::vector<Ref<Texture2D>> m_Textures;

		std::unique_ptr<Assimp::Importer> m_Importer;
		const aiScene* m_Scene;
	};
}