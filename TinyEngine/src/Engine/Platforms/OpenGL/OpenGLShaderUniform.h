#pragma once

#include "Engine/Renderer/ShaderUniform.h"

namespace Engine
{
	class OpenGLShaderUniform : public ShaderUniform
	{
		friend class OpenGLShader;
		friend class OpenGLShaderUniformBuffer;

	public:
		enum class Type
		{
			None, Int, Float, Vec2, Vec3, Vec4, Mat3, Mat4, Bool, Struct
		};
	public:
		OpenGLShaderUniform(ShaderDomain domain, Type type, const std::string& name, uint32_t count = 1);
		OpenGLShaderUniform(ShaderDomain domain, ShaderStruct* uniformStruct, const std::string& name, uint32_t count = 1);

		virtual const std::string& GetName() const override { return m_Name; }
		virtual uint32_t GetSize() const override { return m_Size; }
		virtual uint32_t GetCount() const override { return m_Count; }
		virtual uint32_t GetOffset() const override { return m_Offset; }
		uint32_t GetAbsoluteOffset() const { return m_Struct ? m_Struct->GetOffset() + m_Offset : m_Offset; }
		ShaderDomain GetDomain() const { return m_Domain; }
		Type GetType() const { return m_Type; }
		int32_t GetLocation() const { return m_Location; }
		const ShaderStruct& GetShaderUniformStruct() const 
		{ 
			ENGINE_ASSERT(m_Struct, "ShaderStruct is nullptr!");
			return *m_Struct;
		}
		bool IsArray() const { return m_Count > 1; }

	public:
		static uint32_t SizeOfUniformType(Type type);
		static Type StringToType(const std::string& type);
		static std::string TypeToString(Type type);

	protected:
		void SetOffset(uint32_t offset) override;

	private:
		std::string m_Name;
		ShaderStruct* m_Struct;
		uint32_t m_Size = 0;
		uint32_t m_Count = 0;
		uint32_t m_Offset = 0;
		ShaderDomain m_Domain;
		Type m_Type;
		mutable int32_t m_Location = -1;
	};

	struct GLShaderUniformField
	{
		OpenGLShaderUniform::Type Type;
		std::string Name;
		uint32_t Count;
		mutable uint32_t Size;
		mutable int32_t Location;
	};

	class OpenGLShaderUniformBuffer : public ShaderUniformBuffer
	{
		friend class Shader;

	public:
		OpenGLShaderUniformBuffer(const std::string& name, ShaderDomain domain);

		virtual const std::string& GetName() const override { return m_Name; }
		virtual uint32_t GetRegister() const override { return m_Register; }
		virtual uint32_t GetSize() const override { return m_Size; }
		virtual ShaderDomain GetDomain() const { return m_Domain; }
		virtual const ShaderUniformList& GetUniforms() const override { return m_Uniforms; }

		void PushUniform(OpenGLShaderUniform* uniform);
		ShaderUniform* FindUniform(const std::string& name);

	private:
		std::string m_Name;
		uint32_t m_Register;
		uint32_t m_Size;
		ShaderDomain m_Domain;
		ShaderUniformList m_Uniforms;
	};

	class OpenGLShaderResource : public ShaderResource
	{
		friend class OpenGLShader;

	public:
		enum class Type
		{
			None, Texture2D, TextureCube
		};

	public:
		OpenGLShaderResource(Type type, const std::string& name, uint32_t count);

		virtual const std::string& GetName() const override { return m_Name; };
		virtual uint32_t GetRegister() const override { return m_Register; };
		virtual uint32_t GetCount() const override { return m_Count; };
		Type GetType() const { return m_Type; }

	public:
		static Type StringToType(const std::string& type);
		static std::string TypeToString(Type type);

	private:
		std::string m_Name;
		uint32_t m_Register = 0;
		uint32_t m_Count = 0;
		Type m_Type;
	};
}