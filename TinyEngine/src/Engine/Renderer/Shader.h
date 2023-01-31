#pragma once

#include <string>
#include <unordered_map>
#include <glm/glm.hpp>

#include "Engine/Core/Core.h"
#include "Engine/Core/Buffer.h"
#include "Engine/Renderer/ShaderUniform.h"

namespace Engine
{
	struct ShaderUniform
	{

	};

	struct ShaderUniformCollection
	{

	};

	enum class UniformType
	{
		None = 0,
		Int32, Uint32,
		Float, Float2, Float3, Float4,
		Matrix3x3, Matrix4x4,
	};

	struct UniformDecl
	{
		UniformType Type;
		std::ptrdiff_t Offset;
		std::string Name;
	};

	struct UniformBuffer
	{
		uint8_t* Buffer;
		std::vector<UniformDecl> Uniforms;
	};

	struct UniformBufferBase
	{
		virtual const uint8_t* GetBuffer() const = 0;
		virtual const UniformDecl* GetUniforms() const = 0;
		virtual uint32_t GetUniformCount() const = 0;
	};

	template<uint32_t N, uint32_t U>
	struct UniformBufferDeclaration : public UniformBufferBase
	{
		uint8_t Buffer[N];
		UniformDecl Uniforms[U];
		std::ptrdiff_t Cursor = 0;
		int Index = 0;

		virtual const uint8_t* GetBuffer() const override { return Buffer; };
		virtual const UniformDecl* GetUniforms() const override { return Uniforms; };
		virtual uint32_t GetUniformCount() const override { return U; };

		//Push uniform value-----------------------------------------------------------------
		template<typename T>
		void Push(const std::string& name, const T& data) {}
		
		template<>
		void Push(const std::string& name, const float& data)
		{
			Uniforms[Index++] = { UniformType::Float, Cursor, name };
			memcpy(Buffer + Cursor, &data, sizeof(float));
			Cursor += sizeof(float);
		}

		template<>
		void Push(const std::string& name, const glm::vec2& data)
		{
			Uniforms[Index++] = { UniformType::Float2, Cursor, name };
			memcpy(Buffer + Cursor, glm::value_ptr(data), sizeof(glm::vec2));
			Cursor += sizeof(glm::vec2);
		}

		template<>
		void Push(const std::string& name, const glm::vec3& data)
		{
			Uniforms[Index++] = { UniformType::Float3, Cursor, name };
			memcpy(Buffer + Cursor, glm::value_ptr(data), sizeof(glm::vec3));
			Cursor += sizeof(glm::vec3);
		}

		template<>
		void Push(const std::string& name, const glm::vec4& data)
		{
			Uniforms[Index++] = { UniformType::Float4, Cursor, name };
			memcpy(Buffer + Cursor, glm::value_ptr(data), sizeof(glm::vec4));
			Cursor += sizeof(glm::vec4);
		}

		template<>
		void Push(const std::string& name, const glm::mat3& data)
		{
			Uniforms[Index++] = { UniformType::Matrix3x3, Cursor, name };
			memcpy(Buffer + Cursor, glm::value_ptr(data), sizeof(glm::mat3));
			Cursor += sizeof(glm::mat3);
		}

		template<>
		void Push(const std::string& name, const glm::mat4& data)
		{
			Uniforms[Index++] = { UniformType::Matrix4x4, Cursor, name };
			memcpy(Buffer + Cursor, glm::value_ptr(data), sizeof(glm::mat4));
			Cursor += sizeof(glm::mat4);
		}
		//-----------------------------------------------------------------------------------
	};

	class Shader
	{
	public:
		virtual ~Shader() = default;

		virtual uint32_t GetRendererID() const = 0;
		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;

		virtual void SetInt(const std::string& name, int value) = 0;
		virtual void SetIntArray(const std::string& name, int value[], uint32_t count) = 0;
		virtual void SetFloat(const std::string& name, float value) = 0;
		virtual void SetFloat2(const std::string& name, const glm::vec2& value) = 0;
		virtual void SetFloat3(const std::string& name, const glm::vec3& value) = 0;
		virtual void SetFloat4(const std::string& name, const glm::vec4& value) = 0;
		virtual void SetMat3(const std::string& name, const glm::mat3& value) = 0;
		virtual void SetMat4(const std::string& name, const glm::mat4& value) = 0;

		virtual std::string GetName() = 0;

		static Ref<Shader> Create(const std::string& name, const std::string& vertexSrc, const std::string& fragmentSrc);
		static Ref<Shader> Create(const std::string& filepath);
	};

	class ShaderLibrary
	{
	public:
		void Add(const Ref<Shader>& shader);
		void Add(const std::string& name, const Ref<Shader>& shader);
		Ref<Shader> Load(const std::string& filepath);
		Ref<Shader> Load(const std::string& name, const std::string& filepath);
		Ref<Shader> Get(const std::string& name);
		bool Exists(const std::string& name) const;
	private:
		std::unordered_map<std::string, Ref<Shader>> m_Shaders;
	};
}