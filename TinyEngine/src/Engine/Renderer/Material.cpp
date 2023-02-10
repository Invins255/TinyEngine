#include "pch.h"
#include "Material.h"

namespace Engine
{
    //--------------------------------------------------------------------------------
    //Material
    //--------------------------------------------------------------------------------
    Ref<Material> Material::Create(const Ref<Shader>& shader)
    {
        return CreateRef<Material>(shader);
    }

    Material::Material(const Ref<Shader>& shader)
        :m_Shader(shader)
    {
        m_MaterialFlags |= (uint32_t)MaterialFlag::DepthTest;

        m_Shader->AddShaderReloadedCallback(std::bind(&Material::OnShaderReloaded, this));
        AllocateStorage();
    }

    Material::~Material()
    {
    }

    void Material::Bind() 
    {
        m_Shader->Bind();

        if (m_VSUniformStorageBuffer)
            m_Shader->SetVSMaterialUniformBuffer(m_VSUniformStorageBuffer);
        if (m_PSUniformStorageBuffer)
            m_Shader->SetPSMaterialUniformBuffer(m_PSUniformStorageBuffer);

        BindTextures();
    }

    void Material::AllocateStorage()
    {
        if (m_Shader->HasVSMaterialUniformBuffer())
        {
            const auto& vsBuffer = m_Shader->GetVSMaterialUniformBuffer();
            m_VSUniformStorageBuffer.Allocate(vsBuffer.GetSize());
            m_VSUniformStorageBuffer.ZeroInitialize();
        }
        if (m_Shader->HasPSMaterialUniformBuffer())
        {
            const auto& psBuffer = m_Shader->GetPSMaterialUniformBuffer();
            m_PSUniformStorageBuffer.Allocate(psBuffer.GetSize());
            m_PSUniformStorageBuffer.ZeroInitialize();
        }
    }

    void Material::OnShaderReloaded()
    {
        AllocateStorage();
        for (auto mi : m_MaterialInstances)
            mi->OnShaderReloaded();
    }

    void Material::BindTextures()
    {
        for (uint32_t i = 0; i < m_Textures.size(); i++)
        {
            auto& texture = m_Textures[i];
            if (texture)
                texture->Bind(i);
        }
    }

    ShaderUniform* Material::FindShaderUniform(const std::string& name)
    {
        if (m_VSUniformStorageBuffer)
        {
            auto& uniforms = m_Shader->GetVSMaterialUniformBuffer().GetUniforms();
            for (ShaderUniform* uniform : uniforms)
            {
                if (uniform->GetName() == name)
                    return uniform;
            }
        }
        if (m_PSUniformStorageBuffer)
        {
            auto& uniforms = m_Shader->GetPSMaterialUniformBuffer().GetUniforms();
            for (ShaderUniform* uniform : uniforms)
            {
                if (uniform->GetName() == name)
                    return uniform;
            }
        }
        return nullptr;
    }

    ShaderResource* Material::FindShaderResource(const std::string& name)
    {
        auto& resources = m_Shader->GetResources();
        for (ShaderResource* resource : resources)
        {
            if (resource->GetName() == name)
                return resource;
        }
        return nullptr;
    }

    Buffer& Material::GetUniformBufferTarget(ShaderUniform* uniform)
    {
        switch (uniform->GetDomain())
        {
        case ShaderDomain::Vertex:    return m_VSUniformStorageBuffer;
        case ShaderDomain::Pixel:     return m_PSUniformStorageBuffer;
        }
        ENGINE_ASSERT(false, "Material does not support this shader type!");
        return m_VSUniformStorageBuffer;
    }

    //--------------------------------------------------------------------------------
    //MaterialInstance
    //--------------------------------------------------------------------------------
    Ref<MaterialInstance> MaterialInstance::Create(const Ref<Material>& material, const std::string& name)
    {
        return CreateRef<MaterialInstance>(material, name);
    }

    MaterialInstance::MaterialInstance(const Ref<Material>& material, const std::string& name)
        :m_Name(name), m_Material(material)
    {
        m_Material->m_MaterialInstances.insert(this);
        AllocateStorage();
    }

    MaterialInstance::~MaterialInstance()
    {
        m_Material->m_MaterialInstances.erase(this);
    }

    void MaterialInstance::Bind()
    {
        m_Material->GetShader()->Bind();

        if (m_VSUniformStorageBuffer)
            m_Material->m_Shader->SetVSMaterialUniformBuffer(m_VSUniformStorageBuffer);
        if (m_PSUniformStorageBuffer)
            m_Material->m_Shader->SetPSMaterialUniformBuffer(m_PSUniformStorageBuffer);

        m_Material->BindTextures();
        for (uint32_t i = 0; i < m_Textures.size(); i++)
        {
            auto& texture = m_Textures[i];
            if (texture)
                texture->Bind(i);
        }
    }

    void MaterialInstance::SetFlag(MaterialFlag flag, bool value)
    {
        if (value)
        {
            m_Material->m_MaterialFlags |= (uint32_t)flag;
        }
        else
        {
            m_Material->m_MaterialFlags &= ~(uint32_t)flag;
        }
    }

    void MaterialInstance::AllocateStorage()
    {
        if (m_Material->GetShader()->HasVSMaterialUniformBuffer())
        {
            const auto& vsBuffer = m_Material->GetShader()->GetVSMaterialUniformBuffer();
            m_VSUniformStorageBuffer.Allocate(vsBuffer.GetSize());
            memcpy(m_VSUniformStorageBuffer.Data, m_Material->m_VSUniformStorageBuffer.Data, vsBuffer.GetSize());
        }
        if (m_Material->GetShader()->HasPSMaterialUniformBuffer())
        {
            const auto& psBuffer = m_Material->GetShader()->GetPSMaterialUniformBuffer();
            m_PSUniformStorageBuffer.Allocate(psBuffer.GetSize());
            memcpy(m_PSUniformStorageBuffer.Data, m_Material->m_PSUniformStorageBuffer.Data, psBuffer.GetSize());
        }
    }

    void MaterialInstance::OnShaderReloaded()
    {
        AllocateStorage();
        m_OverriddenValues.clear();
    }

    void MaterialInstance::OnMaterialValueUpdated(ShaderUniform* uniform)
    {
        if (m_OverriddenValues.find(uniform->GetName()) == m_OverriddenValues.end())
        {
            auto& buffer = GetUniformBufferTarget(uniform);
            auto& materialBuffer = m_Material->GetUniformBufferTarget(uniform);
            buffer.Write(materialBuffer.Data + uniform->GetOffset(), uniform->GetSize(), uniform->GetOffset());
        }
    }

    Buffer& MaterialInstance::GetUniformBufferTarget(ShaderUniform* uniform)
    {
        switch (uniform->GetDomain())
        {
        case ShaderDomain::Vertex:    return m_VSUniformStorageBuffer;
        case ShaderDomain::Pixel:     return m_PSUniformStorageBuffer;
        }
        ENGINE_ASSERT(false, "Material does not support this shader type!");
        return m_VSUniformStorageBuffer;
    }

}