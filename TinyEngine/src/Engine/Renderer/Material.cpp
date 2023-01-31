#include "pch.h"
#include "Material.h"

namespace Engine
{
    Ref<Material> Material::Create(const Ref<Shader>& shader)
    {
        return CreateRef<Material>(shader);
    }

    Material::Material(const Ref<Shader>& shader)
        :m_Shader(shader)
    {
        m_MaterialFlags = (uint32_t)MaterialFlag::DepthTest;
        m_MaterialFlags = (uint32_t)MaterialFlag::Blend;
    }

    Material::~Material()
    {
    }

    void Material::Bind() const
    {
    }

}