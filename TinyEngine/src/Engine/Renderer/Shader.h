#pragma once

#include <string>
#include <unordered_map>
#include <glm/glm.hpp>

#include "Engine/Core/Core.h"
#include "Engine/Core/Ref.h"
#include "Engine/Core/Buffer.h"
#include "Engine/Renderer/ShaderUniform.h"

namespace Engine
{
#define SHADER_DEBUG 1
#if SHADER_DEBUG
#define SHADER_TRACE(...) ENGINE_TRACE(__VA_ARGS__)
#define SHADER_WARN(...) ENGINE_WARN(__VA_ARGS__)
#else
#define SHADER_WARN(...)
#endif

	enum class UniformType
	{
		None = 0,
		Int32, Uint32,
		Float, Float2, Float3, Float4,
		Matrix3x3, Matrix4x4,
	};

	struct Uniform
	{
		UniformType Type;
		std::ptrdiff_t Offset;
		std::string Name;
	};

	struct UniformBufferBase
	{
		virtual const uint8_t* GetBuffer() const = 0;
		virtual const Uniform* GetUniforms() const = 0;
		virtual uint32_t GetUniformCount() const = 0;
	};

	template<uint32_t N, uint32_t U>
	struct UniformBuffer : public UniformBufferBase
	{
		uint8_t Buffer[N];
		Uniform Uniforms[U];
		std::ptrdiff_t Cursor = 0;
		int Index = 0;

		virtual const uint8_t* GetBuffer() const override { return Buffer; };
		virtual const Uniform* GetUniforms() const override { return Uniforms; };
		virtual uint32_t GetUniformCount() const override { return U; };

		//Push Uniform
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
	};

	class Shader
	{
	public:
		static std::vector<Ref<Shader>> s_AllShaders;
	
	public:
		static Ref<Shader> Create(const std::string& filepath);

	public:
		using ShaderReloadedCallback = std::function<void()>;

		virtual ~Shader() = default;

		virtual void Reload() = 0;

		virtual uint32_t GetRendererID() const = 0;
		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;
		virtual void UploadUniformBuffer(const UniformBufferBase& uniformBuffer) = 0;

		virtual std::string GetName() = 0;

		virtual void SetVSMaterialUniformBuffer(Buffer buffer) = 0;
		virtual void SetPSMaterialUniformBuffer(Buffer buffer) = 0;
		virtual bool HasVSMaterialUniformBuffer() const = 0;
		virtual bool HasPSMaterialUniformBuffer() const = 0;
		virtual const ShaderUniformBuffer& GetVSMaterialUniformBuffer() const = 0;
		virtual const ShaderUniformBuffer& GetPSMaterialUniformBuffer() const = 0;
		virtual const ShaderUniformList& GetVSRendererUniforms() const = 0;
		virtual const ShaderUniformList& GetPSRendererUniforms() const = 0;

		//Bind shader before setting value
		virtual void Set(const std::string& name, int value) = 0;
		virtual void Set(const std::string& name, int value[], uint32_t count) = 0;
		virtual void Set(const std::string& name, float value) = 0;
		virtual void Set(const std::string& name, const glm::vec2& value) = 0;
		virtual void Set(const std::string& name, const glm::vec3& value) = 0;
		virtual void Set(const std::string& name, const glm::vec4& value) = 0;
		virtual void Set(const std::string& name, const glm::mat3& matrix) = 0;
		virtual void Set(const std::string& name, const glm::mat4& matrix) = 0;

		virtual const ShaderResourceList& GetResources() const = 0;

		virtual void AddShaderReloadedCallback(const ShaderReloadedCallback& callback) = 0;
	};

	class ShaderLibrary
	{
	public:
		void Add(const Ref<Shader>& shader);
		Ref<Shader> Load(const std::string& filepath);
		Ref<Shader> Get(const std::string& name);
		bool Exists(const std::string& name) const;

		std::unordered_map<std::string, Ref<Shader>>& GetShaders() { return m_Shaders; }
		const std::unordered_map<std::string, Ref<Shader>>& GetShaders() const { return m_Shaders; }
	private:
		std::unordered_map<std::string, Ref<Shader>> m_Shaders;
	};
}