#pragma once

#include "Engine/Renderer/Shader.h"
#include "Engine/Platforms/OpenGL/OpenGLShaderUniform.h"
#include <glm/glm.hpp>
#include <unordered_map>

//TODO: Remove
typedef unsigned int GLenum;

namespace Engine
{
	class OpenGLShader : public Shader
	{
	public:
		OpenGLShader(const std::string& filepath);
		virtual ~OpenGLShader();

		virtual void Reload() override;

		virtual uint32_t GetRendererID() const override { return m_RendererID; }
		virtual void Bind() const override;
		virtual void Unbind() const override;

		virtual void UploadUniformBuffer(const UniformBufferBase& uniformBuffer) override;

		//Temp
		virtual void SetInt(const std::string& name, int value) override;
		virtual void SetIntArray(const std::string& name, int value[], uint32_t count) override;
		virtual void SetFloat(const std::string& name, float value) override;
		virtual void SetFloat2(const std::string& name, const glm::vec2& value) override;
		virtual void SetFloat3(const std::string& name, const glm::vec3& value) override;
		virtual void SetFloat4(const std::string& name, const glm::vec4& value) override;
		virtual void SetMat3(const std::string& name, const glm::mat3& value) override;
		virtual void SetMat4(const std::string& name, const glm::mat4& value) override;

		virtual std::string GetName() override { return m_Name; }

		virtual void SetVSMaterialUniformBuffer(Buffer buffer) override;
		virtual void SetPSMaterialUniformBuffer(Buffer buffer) override;
		virtual bool HasVSMaterialUniformBuffer() const override { return (bool)m_VSMaterialUniformBuffer; }
		virtual bool HasPSMaterialUniformBuffer() const override { return (bool)m_PSMaterialUniformBuffer; }
		virtual const ShaderUniformBuffer& GetVSMaterialUniformBuffer() const override { return *m_VSMaterialUniformBuffer; }
		virtual const ShaderUniformBuffer& GetPSMaterialUniformBuffer() const override { return *m_PSMaterialUniformBuffer; }
		virtual const ShaderUniformList& GetVSRendererUniforms() const override { return m_VSRendererUniformBuffers; }
		virtual const ShaderUniformList& GetPSRendererUniforms() const override { return m_PSRendererUniformBuffers; }

		virtual const ShaderResourceList& GetResources() const override { return m_Resources; }

		virtual void AddShaderReloadedCallback(const ShaderReloadedCallback& callback) override;

	private:
		std::string ReadFile(const std::string& filepath);
		void Load(const std::string& source);
		std::unordered_map<GLenum, std::string> PreProcess(const std::string& source);
		void Parse();
		void ParseUniform(const std::string& statement, ShaderDomain domain);
		void ParseUniformStruct(const std::string& block, ShaderDomain domain);
		ShaderStruct* FindStruct(const std::string& name);		
		int32_t GetUniformLocation(const std::string& name) const;
		void ResolveUniforms();

		void ResolveAndSetUniforms(const Ref<OpenGLShaderUniformBuffer>& uniformBuffer, Buffer buffer);
		void ResolveAndSetUniform(OpenGLShaderUniform* uniform, Buffer buffer);
		void ResolveAndSetUniformArray(OpenGLShaderUniform* uniform, Buffer buffer);
		void ResolveAndSetUniformField(const OpenGLShaderUniform& field, uint8_t* data, int32_t offset);

		void Compile();

		void UploadUniformInt(const std::string& name, int value);
		void UploadUniformIntArray(const std::string& name, int value[], uint32_t count);
		void UploadUniformFloat(const std::string& name, float value);
		void UploadUniformFloat2(const std::string& name, const glm::vec2& value);
		void UploadUniformFloat3(const std::string& name, const glm::vec3& value);
		void UploadUniformFloat4(const std::string& name, const glm::vec4& value);
		void UploadUniformMat3(const std::string& name, const glm::mat3& matrix);
		void UploadUniformMat4(const std::string& name, const glm::mat4& matrix);
		void UploadUniformMat4Array(const std::string& name, const glm::mat4& matrix, uint32_t count);
		void UploadUniformInt(uint32_t location, int value);
		void UploadUniformIntArray(uint32_t location, int value[], uint32_t count);
		void UploadUniformFloat(uint32_t location, float value);
		void UploadUniformFloat2(uint32_t location, const glm::vec2& value);
		void UploadUniformFloat3(uint32_t location, const glm::vec3& value);
		void UploadUniformFloat4(uint32_t location, const glm::vec4& value);
		void UploadUniformMat3(uint32_t location, const glm::mat3& matrix);
		void UploadUniformMat4(uint32_t location, const glm::mat4& matrix);
		void UploadUniformMat4Array(uint32_t location, const glm::mat4& matrix, uint32_t count);
		void UploadUniformStruct(OpenGLShaderUniform* uniform, uint8_t* buffer, uint32_t offset);

	private:
		uint32_t m_RendererID = 0;
		std::string m_Name;
		std::string m_Path;
		bool m_Loaded = false;

		ShaderUniformList m_VSRendererUniformBuffers;
		ShaderUniformList m_PSRendererUniformBuffers;
		Ref<OpenGLShaderUniformBuffer> m_VSMaterialUniformBuffer;
		Ref<OpenGLShaderUniformBuffer> m_PSMaterialUniformBuffer;
		ShaderResourceList m_Resources;
		ShaderStructList m_Structs;
		std::unordered_map<GLenum, std::string> m_ShaderSource;
		std::vector<ShaderReloadedCallback> m_ShaderReloadedCallbacks;

	};
}