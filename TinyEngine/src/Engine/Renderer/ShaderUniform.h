#pragma once

#include <string>
#include <vector>
#include "Engine/Core/Core.h"
#include "Engine/Core/Log.h"

namespace Engine
{
	enum class ShaderDomain
	{
		Vertex = 0,
		Pixel = 1
	};

	/// <summary>
	/// Shader中的Uniform变量
	/// </summary>
	class ShaderUniform
	{
		friend class Shader;
		friend class ShaderStruct;

	public:
		virtual const std::string& GetName() const = 0;
		virtual uint32_t GetSize() const = 0;
		virtual uint32_t GetCount() const = 0;
		virtual uint32_t GetOffset() const = 0;
		virtual ShaderDomain GetDomain() const = 0;

	protected:
		virtual void SetOffset(uint32_t offset) = 0;
	};

	using ShaderUniformList = std::vector<ShaderUniform*>;

	class ShaderUniformBuffer
	{
	public:
		virtual const std::string& GetName() const = 0;
		virtual uint32_t GetRegister() const = 0;
		virtual uint32_t GetSize() const = 0;
		virtual const ShaderUniformList& GetUniforms() const = 0;
		virtual ShaderUniform* FindUniform(const std::string& name) = 0;
	};

	/// <summary>
	/// Shader中的Struct
	/// </summary>
	class ShaderStruct
	{
		friend class Shader;

	public:
		ShaderStruct(const std::string& name) :
			m_Name(name), m_Size(0), m_Offset(0)
		{
		}

		void AddField(ShaderUniform* field)
		{
			m_Size += field->GetSize();
			uint32_t offset = 0;
			if (m_Fields.size())
			{
				auto previous = m_Fields.back();
				offset = previous->GetOffset() + previous->GetSize();
			}
			field->SetOffset(offset);
			m_Fields.push_back(field);
		}
		
		const std::string& GetName() const { return m_Name; }
		const std::vector<ShaderUniform*>& GetFields() const { return m_Fields; }
		uint32_t GetSize() const { return m_Size; }
		void SetOffset(uint32_t offset) { m_Offset = offset; }
		uint32_t GetOffset() const { return m_Offset; }

	private:
		std::string m_Name;
		uint32_t m_Size;
		uint32_t m_Offset;

		//Struct内部变量
		std::vector<ShaderUniform*> m_Fields;
	};

	using ShaderStructList = std::vector<ShaderStruct*>;

	class ShaderResource
	{
	public:
		virtual const std::string& GetName() const = 0;
		virtual uint32_t GetRegister() const = 0;
		virtual uint32_t GetCount() const = 0;
	};

	using ShaderResourceList = std::vector<ShaderResource*>;
}