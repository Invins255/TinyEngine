#pragma once

#include <memory>
#include <vector>
#include <glm/glm.hpp>
#include "Engine/Core/TimeStep.h"
#include "Engine/Asset/Asset.h"
#include "Engine/Renderer/Pipeline.h"
#include "Engine/Renderer/Material.h"
#include "Engine/Renderer/Shader.h"
#include "Engine/Renderer/VertexArray.h"
#include "Engine/Renderer/VertexBuffer.h"
#include "Engine/Renderer/IndexBuffer.h"
#include "Engine/Core/Math/AABB.h"

struct aiScene;
struct aiNode;

namespace Assimp {
	class Importer;
}

namespace Engine
{

#define MESH_DEBUG 0
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
		AABB BoundingBox;
	};

	//------------------------------------------------------------------------------------
	//Mesh
	//------------------------------------------------------------------------------------
	class Mesh : public Asset
	{
		friend class Renderer;
		friend class SceneHierarchyPanel;

	public:
		static Ref<Mesh> Create(const std::string& filename);
		static Ref<Mesh> Create(const std::vector<Vertex>& vertices, const std::vector<Index>& indices, const glm::mat4& transform = glm::mat4(1.0f));

		static AssetType GetStaticType() { return AssetType::Mesh; }
		virtual AssetType GetAssetType() const override { return GetStaticType(); }

	public:
		Mesh(const std::string& filename);
		Mesh(const std::vector<Vertex>& vertices, const std::vector<Index>& indices, const glm::mat4& transform = glm::mat4(1.0f));
		~Mesh();

		const std::string& GetFilePath() const { return m_FilePath; }
		std::vector<Submesh>& GetSubmeshes() { return m_Submeshes; }
		const VertexBufferLayout& GetBaseVertexLayout() { return m_BaseVertexLayout; }
		const std::vector<Submesh>& GetSubmeshes() const { return m_Submeshes; }
		const std::vector<Vertex>& GetStaticVertices() const { return m_StaticVertices; }
		const std::vector<Index>& GetIndices() const { return m_Indices; }

		/// <summary>
		/// 获取某个Submesh的所有Triangle
		/// </summary>
		const std::vector<Triangle> GetTriangleCache(uint32_t index) const { return m_TriangleCache.at(index); }

		/// <summary>
		/// 获取Material
		/// </summary>
		Ref<Material> GetMaterial() { return m_BaseMaterial; }
		/// <summary>
		/// 获取Material Instances
		/// </summary>
		std::vector<Ref<MaterialInstance>> GetMaterials() { return m_Materials; }

	private:
		void TraverseNodes(aiNode* node, const glm::mat4& parentTransform = glm::mat4(1.0f), uint32_t level = 0);

	private:
		std::string m_FilePath;

		std::vector<Submesh> m_Submeshes;

		//Buffer
		Ref<VertexBuffer> m_VertexBuffer;
		Ref<IndexBuffer> m_IndexBuffer;
		Ref<VertexArray> m_VertexArray;

		VertexBufferLayout m_BaseVertexLayout;

		std::vector<Vertex> m_StaticVertices;
		std::vector<Index> m_Indices;

		std::unordered_map<uint32_t, std::vector<Triangle>> m_TriangleCache;

		//Material
		Ref<Shader> m_MeshShader;
		Ref<Material> m_BaseMaterial;
		std::vector<Ref<MaterialInstance>> m_Materials;
		std::vector<Ref<Texture2D>> m_Textures;
		std::vector<Ref<Texture2D>> m_NormalMaps;
		std::vector<Ref<Texture2D>> m_RoughnessMaps;
		std::vector<Ref<Texture2D>> m_MetalnessMaps;

		//Assimp
		std::unique_ptr<Assimp::Importer> m_Importer;
		const aiScene* m_Scene;

	private:
		static const std::string m_InitShaderName;
	};
}