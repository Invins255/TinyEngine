#pragma once

#include "Engine/Core/Core.h"
#include "Engine/Renderer/Shader.h"
#include "Engine/Renderer/Texture.h"
#include <unordered_set>

namespace Engine
{
	enum class MaterialFlag
	{
		None		= BIT(0),
		DepthTest	= BIT(1),
		Blend		= BIT(2),
		TwoSided	= BIT(3)
	};

	class Material
	{
		friend class MaterialInstance;

	public:
		static Ref<Material> Create(const Ref<Shader>& shader);

		Material(const Ref<Shader>& shader);
		virtual ~Material();

		void Bind();

		uint32_t GetFlags() const { return m_MaterialFlags; }
		void SetFlags(MaterialFlag flag) { m_MaterialFlags |= (uint32_t)flag; }

		Ref<Shader> GetShader() { return m_Shader; }

		template<typename T>
		void Set(const std::string& name, const T& value)
		{
			auto uniform = FindShaderUniform(name);
			ENGINE_ASSERT(uniform, "Conld not find uniform in shader!");
	
			auto& buffer = GetUniformBufferTarget(uniform);
			buffer.Write((uint8_t*)&value, uniform->GetSize(), uniform->GetOffset());

			for (auto mi : m_MaterialInstances)
				mi->OnMaterialValueUpdated(uniform);
		}
		void Set(const std::string& name, const Ref<Texture>& texture)
		{
			auto resource = FindShaderResource(name);
			ENGINE_ASSERT(resource, "Conld not find uniform in shader!");
			uint32_t slot = resource->GetRegister();
			if (m_Textures.size() <= slot)
				m_Textures.resize((size_t)slot + 1);
			m_Textures[slot] = texture;
		}
		void Set(const std::string& name, const Ref<Texture2D>& texture)
		{
			Set(name, (const Ref<Texture>&)texture);
		}
		void Set(const std::string& name, const Ref<TextureCube>& texture)
		{
			Set(name, (const Ref<Texture>&)texture);
		}

		template<typename T>
		T& Get(const std::string& name)
		{
			auto uniform = FindShaderUniform(name);
			ENGINE_ASSERT(uniform, "Conld not find uniform in shader!");
			auto& buffer = GetUniformBufferTarget(uniform);
			return buffer.Read<T>(uniform->GetOffset());
		}

		template<typename T>
		Ref<T> GetResource(const std::string& name)
		{
			auto resource = FindShaderResource(name);
			ENGINE_ASSERT(resource, "Conld not find uniform in shader!");
			uint32_t slot = resource->GetRegister();
			ENGINE_ASSERT(slot < m_Textures.size(), "Texture slot is invalid!");
			return m_Textures[slot];
		}

	private:
		void AllocateStorage();
		void OnShaderReloaded();
		void BindTextures();

		ShaderUniform* FindShaderUniform(const std::string& name);
		ShaderResource* FindShaderResource(const std::string& name);
		Buffer& GetUniformBufferTarget(ShaderUniform* uniform);

	private:
		Ref<Shader> m_Shader;
		uint32_t m_MaterialFlags;

		//该Material所拥有的MaterialInstance
		std::unordered_set<MaterialInstance*> m_MaterialInstances;

		Buffer m_VSUniformStorageBuffer;
		Buffer m_PSUniformStorageBuffer;
		std::vector<Ref<Texture>> m_Textures;
	};

	/// <summary>
	/// Material实例。Material与MaterialInstance之间构成一对多关系，用于具体的Mesh
	/// </summary>
	class MaterialInstance
	{
		friend class Material;

	public:
		static Ref<MaterialInstance> Create(const Ref<Material>& material, const std::string& name = "");

		MaterialInstance(const Ref<Material>& material, const std::string& name = "");
		virtual ~MaterialInstance();

		void Bind();
		const std::string& GetName() const { return m_Name; }
		Ref<Shader> GetShader() { return m_Material->GetShader(); }
		uint32_t GetFlags() const { return m_Material->GetFlags(); }
		bool GetFlag(MaterialFlag flag) const { return (uint32_t)flag & m_Material->GetFlags(); }
		void SetFlag(MaterialFlag flag, bool value = true);

		template <typename T>
		void Set(const std::string& name, const T& value)
		{
			auto uniform = m_Material->FindShaderUniform(name);
			//ENGINE_ASSERT(uniform, "Conld not find uniform in shader!");
			if (!uniform) 
			{
				ENGINE_WARN("Material: Shader {0} does not have uniform {1}", m_Material->GetShader()->GetName(), name);
				return;
			}
			auto& buffer = GetUniformBufferTarget(uniform);
			buffer.Write((uint8_t*)&value, uniform->GetSize(), uniform->GetOffset());

			m_OverriddenValues.insert(name);
		}
		void Set(const std::string& name, const Ref<Texture>& texture)
		{
			auto resource = m_Material->FindShaderResource(name);
			//ENGINE_ASSERT(resource, "Conld not find uniform in shader!");
			if (!resource)
			{
				ENGINE_WARN("Material: Shader {0} does not have resource {1}", m_Material->GetShader()->GetName(), name);
				return;
			}
			uint32_t slot = resource->GetRegister();
			if (m_Textures.size() <= slot)
				m_Textures.resize((size_t)slot + 1);
			m_Textures[slot] = texture;
		}
		void Set(const std::string& name, const Ref<Texture2D>& texture)
		{
			Set(name, (const Ref<Texture>&)texture);
		}
		void Set(const std::string& name, const Ref<TextureCube>& texture)
		{
			Set(name, (const Ref<Texture>&)texture);
		}

		template<typename T>
		T& Get(const std::string& name)
		{
			auto uniform = m_Material->FindShaderUniform(name);
			ENGINE_ASSERT(uniform, "Conld not find uniform in shader!");
			auto& buffer = GetUniformBufferTarget(uniform);
			return buffer.Read<T>(uniform->GetOffset());
		}

		template<typename T>
		Ref<T> GetResource(const std::string& name)
		{
			auto resource = m_Material->FindShaderResource(name);
			ENGINE_ASSERT(resource, "Conld not find uniform in shader!");
			uint32_t slot = resource->GetRegister();
			ENGINE_ASSERT(slot < m_Textures.size(), "Texture slot is invalid!");
			return m_Textures[slot];
		}

		template<typename T>
		Ref<T> TryGetResource(const std::string& name)
		{
			auto resource = m_Material->FindShaderResource(name);
			if (!resource)
				return nullptr;

			uint32_t slot = resource->GetRegister();
			if (slot >= m_Textures.size())
				return nullptr;

			return m_Textures[slot];
		}

	private:
		void AllocateStorage();
		void OnShaderReloaded();
		void OnMaterialValueUpdated(ShaderUniform* uniform);
		Buffer& GetUniformBufferTarget(ShaderUniform* uniform);

	private:
		std::string m_Name;

		//该MaterialInstance所属的Material
		Ref<Material> m_Material;

		Buffer m_VSUniformStorageBuffer;
		Buffer m_PSUniformStorageBuffer;
		std::vector<Ref<Texture>> m_Textures;

		// TODO: This is temporary; come up with a proper system to track overrides
		std::unordered_set<std::string> m_OverriddenValues;
	};
}