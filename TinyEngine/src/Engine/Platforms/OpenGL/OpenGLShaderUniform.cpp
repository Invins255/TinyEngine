#include "pch.h"
#include "OpenGLShaderUniform.h"

namespace Engine
{

	OpenGLShaderUniform::OpenGLShaderUniform(ShaderDomain domain, Type type, const std::string& name, uint32_t count)
		:m_Domain(domain), m_Type(type), m_Name(name), m_Count(count), 
		m_Size(SizeOfUniformType(type) * count)
	{
	}

	OpenGLShaderUniform::OpenGLShaderUniform(ShaderDomain domain, ShaderStruct* uniformStruct, const std::string& name, uint32_t count)
		:m_Domain(domain), m_Struct(uniformStruct), m_Type(OpenGLShaderUniform::Type::Struct),
		m_Name(name), m_Count(count), m_Size(m_Struct->GetSize() * count)
	{
	}

	uint32_t OpenGLShaderUniform::SizeOfUniformType(Type type)
	{
		switch (type)
		{
			case Engine::OpenGLShaderUniform::Type::Int:	return 4;
			case Engine::OpenGLShaderUniform::Type::Float:	return 4;
			case Engine::OpenGLShaderUniform::Type::Vec2:	return 4 * 2;
			case Engine::OpenGLShaderUniform::Type::Vec3:	return 4 * 3;
			case Engine::OpenGLShaderUniform::Type::Vec4:	return 4 * 4;
			case Engine::OpenGLShaderUniform::Type::Mat3:	return 4 * 3 * 3;
			case Engine::OpenGLShaderUniform::Type::Mat4:	return 4 * 4 * 4;
			case Engine::OpenGLShaderUniform::Type::Bool:	return 1;
		}
		return 0;
	}

	OpenGLShaderUniform::Type OpenGLShaderUniform::StringToType(const std::string& type)
	{
		if (type == "int")      return Type::Int;
		if (type == "bool")     return Type::Bool;
		if (type == "float")    return Type::Float;
		if (type == "vec2")     return Type::Vec2;
		if (type == "vec3")     return Type::Vec3;
		if (type == "vec4")     return Type::Vec4;
		if (type == "mat3")     return Type::Mat3;
		if (type == "mat4")     return Type::Mat4;

		return Type::None;
	}

	std::string OpenGLShaderUniform::TypeToString(Type type)
	{
		switch (type)
		{
			case OpenGLShaderUniform::Type::Int:     return "int";
			case OpenGLShaderUniform::Type::Bool:	 return "bool";
			case OpenGLShaderUniform::Type::Float:   return "float";
			case OpenGLShaderUniform::Type::Vec2:    return "vec2";
			case OpenGLShaderUniform::Type::Vec3:    return "vec3";
			case OpenGLShaderUniform::Type::Vec4:    return "vec4";
			case OpenGLShaderUniform::Type::Mat3:    return "mat3";
			case OpenGLShaderUniform::Type::Mat4:    return "mat4";
		}
		return "none";
	}

	void OpenGLShaderUniform::SetOffset(uint32_t offset)
	{
		if (m_Type == OpenGLShaderUniform::Type::Struct)
			m_Struct->SetOffset(offset);
		m_Offset = offset;
	}

	OpenGLShaderUniformBuffer::OpenGLShaderUniformBuffer(const std::string& name, ShaderDomain domain)
		:m_Name(name), m_Size(0), m_Register(0), m_Domain(domain)
	{
	}

	void OpenGLShaderUniformBuffer::PushUniform(OpenGLShaderUniform* uniform)
	{
		uint32_t offset = 0;
		if (m_Uniforms.size())
		{
			OpenGLShaderUniform* previous = (OpenGLShaderUniform*)m_Uniforms.back();
			offset = previous->m_Offset + previous->m_Size;
		}
		uniform->SetOffset(offset);
		m_Size += uniform->GetSize();
		m_Uniforms.push_back(uniform);
	}

	ShaderUniform* OpenGLShaderUniformBuffer::FindUniform(const std::string& name)
	{
		for (ShaderUniform* uniform : m_Uniforms)
		{
			if (uniform->GetName() == name)
				return uniform;
		}
		return nullptr;
	}

	OpenGLShaderResource::OpenGLShaderResource(Type type, const std::string& name, uint32_t count)
		:m_Type(type), m_Name(name), m_Count(count)
	{
	}

	OpenGLShaderResource::Type OpenGLShaderResource::StringToType(const std::string& type)
	{
		if (type == "sampler2D")	return Type::Texture2D;
		if (type == "samplerCube")	return Type::TextureCube;

		return Type::None;
	}

	std::string OpenGLShaderResource::TypeToString(Type type)
	{
		switch (type)
		{
		case Engine::OpenGLShaderResource::Type::Texture2D:
			return "sampler2D";
		case Engine::OpenGLShaderResource::Type::TextureCube:
			return "samplerCube";
		}

		return "None";
	}

}