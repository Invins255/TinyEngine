#include "pch.h"
#include "OpenGLShader.h"
#include "Engine/Renderer/Renderer.h"
#include <glad/glad.h>

namespace Engine 
{
	static GLenum ShaderTypeFromString(const std::string& type)
	{
		if (type == "vertex") 
			return GL_VERTEX_SHADER;
		if (type == "fragment" || type == "pixel")
			return GL_FRAGMENT_SHADER;
		ENGINE_ASSERT(false, "Unknown shader type!");
		return 0;
	}

	OpenGLShader::OpenGLShader(const std::string& name, const std::string& vertexSrc, const std::string& fragmentSrc):
		m_Name(name)
	{
		std::unordered_map<GLenum, std::string> sources;
		sources[GL_VERTEX_SHADER] = vertexSrc;
		sources[GL_FRAGMENT_SHADER] = fragmentSrc;
		Compile(sources);
	}

	OpenGLShader::OpenGLShader(const std::string& filepath)
	{
		std::string source = ReadFile(filepath);
		auto shaderSources = PreProcess(source);
		Compile(shaderSources);

		auto lashSlash = filepath.find_last_of("/\\");
		lashSlash = lashSlash == std::string::npos ? 0 : lashSlash + 1;
		auto lastDot = filepath.rfind('.');
		auto count = lastDot == std::string::npos ? filepath.size() - lashSlash : lastDot - lashSlash;
		m_Name = filepath.substr(lashSlash, count);
	}

	OpenGLShader::~OpenGLShader()
	{
		uint32_t rendererID = m_RendererID;
		Renderer::Submit([rendererID]()
			{
				ENGINE_INFO("RenderCommand: Destroy shader({0})", rendererID);
				glDeleteProgram(rendererID);
			}
		);
	}

	void OpenGLShader::Bind() const
	{
		Renderer::Submit([this]()
			{
				ENGINE_INFO("RenderCommand: Bind shader({0})", m_RendererID);
				glUseProgram(m_RendererID);
			}
		);
	}

	void OpenGLShader::Unbind() const
	{
		Renderer::Submit([this]()
			{
				ENGINE_INFO("RenderCommand: Unbind shader({0})", m_RendererID);
				glUseProgram(0);
			}
		);
	}

	void OpenGLShader::SetInt(const std::string& name, int value)
	{
		Renderer::Submit([=]()
			{
				UploadUniformInt(name, value);
			}
		);
	}

	void OpenGLShader::SetIntArray(const std::string& name, int value[], uint32_t count)
	{
		Renderer::Submit([=]()
			{
				UploadIntArray(name, value, count);
			}
		);
	}

	void OpenGLShader::SetFloat(const std::string& name, float value)
	{
		Renderer::Submit([=]()
			{
				UploadUniformFloat(name, value);
			}
		);
	}

	void OpenGLShader::SetFloat2(const std::string& name, const glm::vec2& value)
	{
		Renderer::Submit([=]()
			{
				UploadUniformFloat2(name, value);
			}
		);
	}

	void OpenGLShader::SetFloat3(const std::string& name, const glm::vec3& value)
	{
		Renderer::Submit([=]()
			{
				UploadUniformFloat3(name, value);
			}
		);
	}

	void OpenGLShader::SetFloat4(const std::string& name, const glm::vec4& value)
	{
		Renderer::Submit([=]()
			{
				UploadUniformFloat4(name, value);
			}
		);
	}

	void OpenGLShader::SetMat3(const std::string& name, const glm::mat3& value)
	{
		Renderer::Submit([=]()
			{
				UploadUniformMat3(name, value);
			}
		);
	}

	void OpenGLShader::SetMat4(const std::string& name, const glm::mat4& value)
	{
		Renderer::Submit([=]()
			{
				UploadUniformMat4(name, value);
			}
		);
	}

	void OpenGLShader::UploadUniformInt(const std::string& name, int value)
	{
		GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		glUniform1i(location, value);
	}

	void OpenGLShader::UploadIntArray(const std::string& name, int value[], uint32_t count)
	{
		GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		glUniform1iv(location, count, value);
	}

	void OpenGLShader::UploadUniformFloat(const std::string& name, float value)
	{
		GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		glUniform1f(location, value);
	}

	void OpenGLShader::UploadUniformFloat2(const std::string& name, const glm::vec2& value)
	{
		GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		glUniform2f(location, value.x, value.y);
	}

	void OpenGLShader::UploadUniformFloat3(const std::string& name, const glm::vec3& value)
	{
		GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		glUniform3f(location, value.x, value.y, value.z);
	}

	void OpenGLShader::UploadUniformFloat4(const std::string& name, const glm::vec4& value)
	{
		GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		glUniform4f(location, value.x, value.y, value.z, value.w);
	}

	void OpenGLShader::UploadUniformMat3(const std::string& name, const glm::mat3& matrix)
	{
		GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		glUniformMatrix3fv(location, 1, GL_FALSE, glm::value_ptr(matrix));
	}

	void OpenGLShader::UploadUniformMat4(const std::string& name, const glm::mat4& matrix)
	{
		GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(matrix));
	}

	std::string OpenGLShader::ReadFile(const std::string& filepath)
	{
		std::string result;
		std::ifstream in(filepath, std::ios::in, std::ios::binary);
		if (in)
		{
			in.seekg(0, std::ios::end);
			result.resize(in.tellg());
			in.seekg(0, std::ios::beg);
			in.read(&result[0], result.size());
			in.close();
		}
		else
		{
			ENGINE_ERROR("Could not open file '{0}'!", filepath);
		}

		return result;
	}

	std::unordered_map<GLenum, std::string> OpenGLShader::PreProcess(const std::string& source)
	{
		std::unordered_map<GLenum, std::string> shaderSources;

		const char* typeToken = "#type";
		size_t typeTokenLength = strlen(typeToken);
		size_t pos = source.find(typeToken, 0);
		while (pos != std::string::npos)
		{
			size_t eol = source.find_first_of("\r\n", pos);
			ENGINE_ASSERT(eol != std::string::npos, "Syntax error!");
			size_t begin = pos + typeTokenLength + 1;
			std::string type = source.substr(begin, eol - begin);
			ENGINE_ASSERT(ShaderTypeFromString(type), "Invaild shader type!");

			size_t nextLinePos = source.find_first_of("\r\n", eol);
			pos = source.find(typeToken, nextLinePos);
			shaderSources[ShaderTypeFromString(type)] =
				source.substr(nextLinePos, pos - (nextLinePos == std::string::npos ? source.size() - 1 : nextLinePos));
		}

		return shaderSources;
	}

	void OpenGLShader::Compile(std::unordered_map<GLenum, std::string>& shaderSources)
	{
		Renderer::Submit([=]()
			{
				//Create shader program
				GLuint program = glCreateProgram();

				ENGINE_ASSERT(shaderSources.size() <= 2, "OpenGLShader only support 2 shaders for now!");
				std::array<GLenum, 2> glShaderIDs;
				int glShaderIDIndex = 0;

				for (auto& kv : shaderSources)
				{
					GLenum type = kv.first;
					const std::string& source = kv.second;

					//Create shader handle
					GLuint shader = glCreateShader(type);
					//Send shader source to GL
					const GLchar* sourceCStr = (const GLchar*)source.c_str();
					glShaderSource(shader, 1, &sourceCStr, 0);
					//Compile shader
					glCompileShader(shader);
					//If shader is not compiled, print log and delete shader
					GLint isCompiled = 0;
					glGetShaderiv(shader, GL_COMPILE_STATUS, &isCompiled);
					if (isCompiled == GL_FALSE)
					{
						GLint maxLength = 0;
						glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);
						std::vector<GLchar> infoLog(maxLength);
						glGetShaderInfoLog(shader, maxLength, &maxLength, &infoLog[0]);

						glDeleteShader(shader);

						ENGINE_ERROR("Shader compilation failure!");
						ENGINE_ERROR("{0}", infoLog.data());
						return;
					}

					//Attach shader to program
					glAttachShader(program, shader);
					glShaderIDs[glShaderIDIndex++] = shader;
				}

				//Link program
				glLinkProgram(program);

				//If link failed, print log and delete shader
				GLint isLinked = 0;
				glGetProgramiv(program, GL_LINK_STATUS, &isLinked);
				if (isLinked == GL_FALSE)
				{
					GLint maxLength = 0;
					glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);
					std::vector<GLchar> infoLog(maxLength);
					glGetProgramInfoLog(program, maxLength, &maxLength, &infoLog[0]);

					glDeleteProgram(program);
					for (auto id : glShaderIDs)
						glDeleteShader(id);

					ENGINE_ERROR("Shader link failure!");
					ENGINE_ERROR("{0}", infoLog.data());
					return;
				}

				//detach shader after a successful link
				for (auto id : glShaderIDs)
					glDetachShader(program, id);

				m_RendererID = program;

				ENGINE_INFO("RenderCommand: Compile shader({0})", m_RendererID);
			}
		);
	}
}