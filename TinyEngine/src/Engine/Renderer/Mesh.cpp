#include "pch.h"
#include "Mesh.h"
#include "Engine/Renderer/Buffer.h"
#include "Engine/Renderer/Renderer.h"

namespace Engine
{
    Mesh::Mesh(const std::string& filename)
    {
        //TODO: Load asset
    }

    Mesh::Mesh(const std::vector<Vertex>& vertices, const std::vector<Index>& indices, const glm::mat4& transform)
        : m_StaticVertices(vertices), m_Indices(indices)
    {
        Submesh submesh;
        submesh.BaseVertex = 0;
        submesh.BaseIndex = 0;
        submesh.IndexCount = indices.size() * 3;
        submesh.Transform = transform;
        m_Submeshes.push_back(submesh);

        m_VertexBuffer = VertexBuffer::Create(m_StaticVertices.data(), m_StaticVertices.size() * sizeof(Vertex));
        m_IndexBuffer = IndexBuffer::Create(m_Indices.data(), m_Indices.size() * sizeof(Index));

        PipelineSpecification pipelineSpecification;
        pipelineSpecification.Layout = {
            { ShaderDataType::Float3, "a_Position" },
            { ShaderDataType::Float3, "a_Normal" },
            { ShaderDataType::Float3, "a_Tangent" },
            { ShaderDataType::Float3, "a_Binormal" },
            { ShaderDataType::Float2, "a_TexCoord" },
        };
        m_Pipeline = Pipeline::Create(pipelineSpecification);

        //TEMP
        m_MeshShader = Renderer::GetShaderLibrary()->Get("FlatColor3D");
        m_BaseMaterial = Material::Create(m_MeshShader);
        m_Materials.resize(1);
        m_Materials[0] = CreateRef<MaterialInstance>(m_BaseMaterial);
    }

    Mesh::~Mesh()
    {
    }

    void Mesh::OnUpdate(Timestep ts)
    {
        //TODO: Animate
    }

    void Mesh::DumpVertexBuffer()
    {
    }
}