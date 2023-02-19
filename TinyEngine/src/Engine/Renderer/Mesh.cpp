#include "pch.h"
#include "Mesh.h"
#include "Engine/Renderer/VertexBuffer.h"
#include "Engine/Renderer/IndexBuffer.h"
#include "Engine/Renderer/Renderer.h"

#include <filesystem>
#include <locale>
#include <codecvt>

#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/Importer.hpp>
#include <assimp/DefaultLogger.hpp>
#include <assimp/LogStream.hpp>

namespace Engine
{
    static glm::mat4 AssimpMat4ToMat4(const aiMatrix4x4& matrix)
    {
        glm::mat4 result;
        //the a,b,c,d in assimp is the row ; the 1,2,3,4 is the column
        result[0][0] = matrix.a1; result[1][0] = matrix.a2; result[2][0] = matrix.a3; result[3][0] = matrix.a4;
        result[0][1] = matrix.b1; result[1][1] = matrix.b2; result[2][1] = matrix.b3; result[3][1] = matrix.b4;
        result[0][2] = matrix.c1; result[1][2] = matrix.c2; result[2][2] = matrix.c3; result[3][2] = matrix.c4;
        result[0][3] = matrix.d1; result[1][3] = matrix.d2; result[2][3] = matrix.d3; result[3][3] = matrix.d4;
        return result;
    }

    const std::string Mesh::m_InitShaderName = "BlinnPhong";

    struct LogStream : public Assimp::LogStream
    {
        static void Initialize()
        {
            if (Assimp::DefaultLogger::isNullLogger())
            {
                Assimp::DefaultLogger::create("", Assimp::Logger::VERBOSE);
                Assimp::DefaultLogger::get()->attachStream(new LogStream, Assimp::Logger::Err | Assimp::Logger::Warn);
            }
        }

        virtual void write(const char* message) override
        {
            ENGINE_WARN("Assimp: '{0}'", message);
        }
    };

    //-----------------------------------------------------------------------------------
    //Mesh
    //-----------------------------------------------------------------------------------
    Mesh::Mesh(const std::string& filename)
        :m_FilePath(filename)
    {
        //BUG: ÖÐÎÄ×Ö·û¶ÁÈ¡´íÎó

        MESH_INFO("Mesh: Loading mesh '{0}'", filename);

        LogStream::Initialize();
        
        m_Importer = std::make_unique<Assimp::Importer>();
        const uint32_t meshImportFlags =
            aiProcess_CalcTangentSpace |        // Create binormals/tangents just in case
            aiProcess_Triangulate |             // Make sure we're triangles
            aiProcess_SortByPType |             // Split meshes by primitive type
            aiProcess_GenNormals |              // Make sure we have legit normals
            aiProcess_GenUVCoords |             // Convert UVs if required 
            aiProcess_OptimizeMeshes |          // Batch draws where possible
            aiProcess_ValidateDataStructure;    // Validation
        const aiScene* scene = m_Importer->ReadFile(filename, meshImportFlags);
        if (!scene || !scene->HasMeshes())
        {
            ENGINE_ERROR("Failed to load mesh file: '{0}'", filename);
            return;
        }
        m_Scene = scene;

        m_MeshShader = Renderer::GetShaderLibrary().Get(m_InitShaderName);
        m_BaseMaterial = Material::Create(m_MeshShader);
        m_BaseMaterial->SetFlags(MaterialFlag::DepthTest);

        uint32_t vertexCount = 0;
        uint32_t indexCount = 0;

        //TODO: Load animated mesh 
        //Load meshes
        m_Submeshes.reserve(m_Scene->mNumMeshes);
        for (uint32_t i = 0; i < m_Scene->mNumMeshes; i++)
        {
            aiMesh* mesh = m_Scene->mMeshes[i];

            Submesh& submesh = m_Submeshes.emplace_back();
            submesh.BaseVertex = vertexCount;
            submesh.BaseIndex = indexCount;
            submesh.MaterialIndex = mesh->mMaterialIndex;
            submesh.IndexCount = mesh->mNumFaces * 3;
            submesh.VertexCount = mesh->mNumVertices;
            submesh.MeshName = mesh->mName.C_Str();

            vertexCount += submesh.VertexCount;
            indexCount += submesh.IndexCount;

            ENGINE_ASSERT(mesh->HasPositions(), "Mesh has no positions!");
            ENGINE_ASSERT(mesh->HasNormals(), "Mesh has no normals!");

            //Vertices
            for (uint32_t j = 0; j < mesh->mNumVertices; j++)
            {
                Vertex vertex;
                vertex.Position = { mesh->mVertices[j].x,mesh->mVertices[j].y ,mesh->mVertices[j].z };
                vertex.Normal = { mesh->mNormals[j].x,mesh->mNormals[j].y, mesh->mNormals[j].z };
                if (mesh->HasTangentsAndBitangents())
                {
                    vertex.Tangent = { mesh->mTangents[j].x, mesh->mTangents[j].y ,mesh->mTangents[j].z };
                    vertex.Binormal = { mesh->mBitangents[j].x, mesh->mBitangents[j].y ,mesh->mBitangents[j].z};
                }
                if (mesh->HasTextureCoords(0))
                    vertex.Texcoord = { mesh->mTextureCoords[0][j].x, mesh->mTextureCoords[0][j].y};
            
                m_StaticVertices.push_back(vertex);
            }
            //Indices
            for (uint32_t j = 0; j < mesh->mNumFaces; j++)
            {
                ENGINE_ASSERT(mesh->mFaces[j].mNumIndices == 3, "Face must have 3 indices!");
                Index index = { mesh->mFaces[j].mIndices[0], mesh->mFaces[j].mIndices[1] ,mesh->mFaces[j].mIndices[2] };
                m_Indices.push_back(index);
            }
        }

        TraverseNodes(m_Scene->mRootNode);
        
        //Load materials
        if (m_Scene->HasMaterials())
        {
            MESH_INFO("Mesh: ({0}) materials", filename);

            m_Textures.resize(scene->mNumMaterials);
            m_NormalMaps.resize(scene->mNumMaterials);
            m_RoughnessMaps.resize(scene->mNumMaterials);
            m_MetalnessMaps.resize(scene->mNumMaterials);
            m_Materials.resize(scene->mNumMaterials);
            
            for (uint32_t i = 0; i < scene->mNumMaterials; i++)
            {
                auto aiMaterial = m_Scene->mMaterials[i];
                auto aiMaterialName = aiMaterial->GetName();

                auto mi = MaterialInstance::Create(m_BaseMaterial, aiMaterialName.data);
                m_Materials[i] = mi;

                uint32_t textureCount = aiMaterial->GetTextureCount(aiTextureType_DIFFUSE);

                aiColor3D aiColor;
                aiMaterial->Get(AI_MATKEY_COLOR_DIFFUSE, aiColor);

                float shininess, metalness;
                if (aiMaterial->Get(AI_MATKEY_SHININESS, shininess) != aiReturn_SUCCESS)
                    shininess = 80.0f;
                if (aiMaterial->Get(AI_MATKEY_REFLECTIVITY, metalness) != aiReturn_SUCCESS)
                    metalness = 0.0f;
                float roughness = 1.0f - glm::sqrt(shininess / 100.0f);

                MESH_INFO("Mesh: Load material {0}, index = {1}", mi->GetName(), i);
                MESH_INFO("     TextureCount = {0}", textureCount);
                MESH_INFO("     Color = {0}, {1}, {2}", aiColor.r, aiColor.g, aiColor.b);
                MESH_INFO("     Shininess = {0}", shininess);
                MESH_INFO("     Metalness = {0}", metalness);
                MESH_INFO("     Roughness = {0}", roughness);

                aiString aiTexPath;
                
                //Albedo map
                mi->Set("u_AlbedoTexToggle", 0.0f);
                bool hasAlbedoMap = aiMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &aiTexPath) == aiReturn_SUCCESS;
                if (hasAlbedoMap)
                {
                    std::filesystem::path path = filename;
                    auto parentPath = path.parent_path();
                    parentPath /= std::string(aiTexPath.data);
                    std::string texturePath = parentPath.string();
                    MESH_INFO("     Albedo map path = {0}", texturePath);
                    
                    auto texture = Texture2D::Create(texturePath, true);
                    if (texture->IsLoaded())
                    {
                        m_Textures[i] = texture;
                        mi->Set("u_AlbedoTexture", m_Textures[i]);
                        mi->Set("u_AlbedoTexToggle", 1.0f);
                    }
                    else
                    {
                        ENGINE_ERROR("Could not load texture {0}", texturePath);
                        mi->Set("u_AlbedoColor", glm::vec3(aiColor.r, aiColor.g, aiColor.b));
                    }
                    
                }
                else
                {
                    mi->Set("u_AlbedoColor", glm::vec3{ aiColor.r, aiColor.g, aiColor.b });
                    MESH_INFO("     No albedo map. Set albedo: {0}, {1}, {2}", aiColor.r, aiColor.g, aiColor.b);
                }

                //Normal map
                mi->Set("u_NormalTexToggle", 0.0f);
                bool hasNormalMap = aiMaterial->GetTexture(aiTextureType_NORMALS, 0, &aiTexPath) == aiReturn_SUCCESS;
                if (hasNormalMap)
                {
                    std::filesystem::path path = filename;
                    auto parentPath = path.parent_path();
                    parentPath /= std::string(aiTexPath.data);
                    std::string texturePath = parentPath.string();
                    MESH_INFO("     Normal map path = {0}", texturePath);
                    
                    auto texture = Texture2D::Create(texturePath); //TODO: Manage textures
                    if (texture->IsLoaded())
                    {
                        m_NormalMaps[i] = texture;
                        mi->Set("u_NormalTexture", texture);
                        mi->Set("u_NormalTexToggle", 1.0f);
                    }
                    else
                    {
                        ENGINE_ERROR("Could not load texture {0}", texturePath);
                    }
                    
                }
                else
                {
                    MESH_INFO("     No normal map");
                }
                
                //Roughness map
                mi->Set("u_RoughnessTexToggle", 0.0f);
                bool hasRoughnessMap = aiMaterial->GetTexture(aiTextureType_SHININESS, 0, &aiTexPath) == aiReturn_SUCCESS;
                if (hasRoughnessMap)
                {
                    std::filesystem::path path = filename;
                    auto parentPath = path.parent_path();
                    parentPath /= std::string(aiTexPath.data);
                    std::string texturePath = parentPath.string();
                    MESH_INFO("     Roughness map path = {0}", texturePath);
                    
                    auto texture = Texture2D::Create(texturePath);
                    if (texture->IsLoaded())
                    {
                        m_RoughnessMaps[i] = texture;
                        mi->Set("u_RoughnessTexture", texture);
                        mi->Set("u_RoughnessTexToggle", 1.0f);
                    }
                    else
                    {
                        ENGINE_ERROR("Could not load texture {0}", texturePath);
                    }
                    
                }
                else
                {
                    mi->Set("u_Roughness", roughness);
                    MESH_INFO("     No roughness map. Set roughness: {0}", roughness);
                }
                
                //Metalness map
                mi->Set("u_MetalnessTexToggle", 0.0f);
                bool hasMetalnessMap = false;
                for (uint32_t j = 0; j < aiMaterial->mNumProperties; j++)
                {
                    auto prop = aiMaterial->mProperties[j];
                    if (prop->mType == aiPTI_String)
                    {
                        uint32_t strLength = *(uint32_t*)prop->mData;
                        std::string str(prop->mData + 4, strLength);

                        std::string key = prop->mKey.data;
                        if (key == "$raw.ReflectionFactor|file")
                        {
                            hasMetalnessMap = true;

                            std::filesystem::path path = filename;
                            auto parentPath = path.parent_path();
                            parentPath /= str;
                            std::string texturePath = parentPath.string();
                            MESH_INFO("    Metalness map path = {0}", texturePath);
                            auto texture = Texture2D::Create(texturePath);
                            if (texture->IsLoaded())
                            {
                                m_MetalnessMaps[i] = texture;
                                mi->Set("u_MetalnessTexture", texture);
                                mi->Set("u_MetalnessTexToggle", 1.0f);
                            }
                            else
                            {
                                MESH_INFO("    Could not load texture: {0}", texturePath);
                                mi->Set("u_Metalness", metalness);
                                mi->Set("u_MetalnessTexToggle", 0.0f);
                            }
                            break;
                        }
                    }
                }

                if (!hasMetalnessMap)
                {
                    mi->Set("u_Metalness", metalness);
                    mi->Set("u_MetalnessTexToggle", 0.0f);
                    MESH_INFO("    No metalness map. Set metalness: {0}", metalness);
                }

                //TODO: Other textures
            }
        }
        
        m_VertexBuffer = VertexBuffer::Create(m_StaticVertices.data(), m_StaticVertices.size() * sizeof(Vertex));
        m_IndexBuffer = IndexBuffer::Create(m_Indices.data(), m_Indices.size() * sizeof(Index));
        m_VertexArray = VertexArray::Create();

        m_BaseVertexLayout = {
            { ShaderDataType::Float3, "a_Position" },
            { ShaderDataType::Float3, "a_Normal" },
            { ShaderDataType::Float3, "a_Tangent" },
            { ShaderDataType::Float3, "a_Binormal" },
            { ShaderDataType::Float2, "a_TexCoord" },
        };
    }

    Mesh::Mesh(const std::vector<Vertex>& vertices, const std::vector<Index>& indices, const glm::mat4& transform)
        : m_StaticVertices(vertices), m_Indices(indices)
    {
        MESH_INFO("Mesh: Construct basic mesh");

        m_Importer = nullptr;
        m_Scene = nullptr;

        Submesh& submesh = m_Submeshes.emplace_back();
        submesh.MeshName = "Basic mesh";
        submesh.BaseVertex = 0;
        submesh.BaseIndex = 0;
        submesh.MaterialIndex = 0;
        submesh.IndexCount = indices.size() * 3;
        submesh.VertexCount = vertices.size();
        submesh.Transform = transform;

        //TEMP
        m_MeshShader = Renderer::GetShaderLibrary().Get(m_InitShaderName);
        
        m_BaseMaterial = Material::Create(m_MeshShader);
        m_BaseMaterial->SetFlags(MaterialFlag::DepthTest);

        auto mi = MaterialInstance::Create(m_BaseMaterial);
        mi->Set("u_AlbedoColor", glm::vec3(0.6f, 0.6f, 0.6f));
        m_Materials.push_back(mi);

        m_VertexBuffer = VertexBuffer::Create(m_StaticVertices.data(), m_StaticVertices.size() * sizeof(Vertex));
        m_IndexBuffer = IndexBuffer::Create(m_Indices.data(), m_Indices.size() * sizeof(Index));
        m_VertexArray = VertexArray::Create();

        m_BaseVertexLayout = {
            { ShaderDataType::Float3, "a_Position" },
            { ShaderDataType::Float3, "a_Normal" },
            { ShaderDataType::Float3, "a_Tangent" },
            { ShaderDataType::Float3, "a_Binormal" },
            { ShaderDataType::Float2, "a_TexCoord" },
        };
    }

    Mesh::~Mesh()
    {
    }

    void Mesh::TraverseNodes(aiNode* node, const glm::mat4& parentTransform, uint32_t level)
    {
        glm::mat4 transform = parentTransform * AssimpMat4ToMat4(node->mTransformation);
        for (uint32_t i = 0; i < node->mNumMeshes; i++)
        {
            uint32_t mesh = node->mMeshes[i];
            Submesh& submesh = m_Submeshes[mesh];
            submesh.NodeName = node->mName.C_Str();
            submesh.Transform = transform;
        }

        for (uint32_t i = 0; i < node->mNumChildren; i++)
            TraverseNodes(node->mChildren[i], transform, level + 1);
    }
}