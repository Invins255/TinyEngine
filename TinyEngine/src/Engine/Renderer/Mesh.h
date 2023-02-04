#pragma once

#include <vector>
#include <glm/glm.hpp>
#include "Engine/Core/TimeStep.h"
#include "Engine/Renderer/Pipeline.h"
#include "Engine/Renderer/Material.h"

namespace Engine
{
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
		uint32_t V1, V2, V3;
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

		uint32_t BaseVertex;
		uint32_t BaseIndex;
		uint32_t MaterialIndex = 0;
		uint32_t IndexCount = 0;
		uint32_t VertexCount = 0;

		glm::mat4 Transform;	
	};

	class Mesh
	{
		friend class Renderer;

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
	};
}