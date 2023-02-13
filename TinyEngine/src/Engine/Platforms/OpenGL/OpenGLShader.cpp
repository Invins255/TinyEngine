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

	OpenGLShader::OpenGLShader(const std::string& filepath)
		:m_Path(filepath)
	{
		auto lashSlash = filepath.find_last_of("/\\");
		lashSlash = lashSlash == std::string::npos ? 0 : lashSlash + 1;
		auto lastDot = filepath.rfind('.');
		auto count = lastDot == std::string::npos ? filepath.size() - lashSlash : lastDot - lashSlash;
		m_Name = filepath.substr(lashSlash, count);

		std::string source = ReadFile(filepath);
		Load(source);
	}

	OpenGLShader::~OpenGLShader()
	{
		uint32_t rendererID = m_RendererID;
		Renderer::Submit([rendererID]()
			{
				RENDERCOMMAND_TRACE("RenderCommand: Destroy shader. ID: {1}", rendererID);

				glDeleteProgram(rendererID);
			}
		);
	}

	void OpenGLShader::Reload()
	{
		std::string source = ReadFile(m_Path);
		Load(source);
	}

	void OpenGLShader::Bind() const
	{
		Renderer::Submit([this]()
			{
				RENDERCOMMAND_TRACE("RenderCommand: Bind shader({0})", m_RendererID);

				glUseProgram(m_RendererID);
			}
		);
	}

	void OpenGLShader::Unbind() const
	{
		Renderer::Submit([this]()
			{
				RENDERCOMMAND_TRACE("RenderCommand: Unbind shader({0})", m_RendererID);

				glUseProgram(0);
			}
		);
	}

	void OpenGLShader::UploadUniformBuffer(const UniformBufferBase& uniformBuffer)
	{
		for (uint32_t i = 0; i < uniformBuffer.GetUniformCount(); i++)
		{
			const Uniform uniform = uniformBuffer.GetUniforms()[i];
			switch (uniform.Type)
			{
				case UniformType::Float:
				{
					const std::string& name = uniform.Name;
					float value = *(float*)(uniformBuffer.GetBuffer() + uniform.Offset);
					Renderer::Submit([=]()
						{
							UploadUniformFloat(name, value);
						}
					);
				}
				break;
				case UniformType::Float2:
				{
					const std::string& name = uniform.Name;
					glm::vec2& value = *(glm::vec2*)(uniformBuffer.GetBuffer() + uniform.Offset);
					Renderer::Submit([=]()
						{
							UploadUniformFloat2(name, value);
						}
					);
				}
				break;
				case UniformType::Float3:
				{
					const std::string& name = uniform.Name;
					glm::vec3& value = *(glm::vec3*)(uniformBuffer.GetBuffer() + uniform.Offset);
					Renderer::Submit([=]()
						{
							UploadUniformFloat3(name, value);
						}
					);
				}
				break;
				case UniformType::Float4:
				{
					const std::string& name = uniform.Name;
					glm::vec4& value = *(glm::vec4*)(uniformBuffer.GetBuffer() + uniform.Offset);
					Renderer::Submit([=]()
						{
							UploadUniformFloat4(name, value);
						}
					);
				}
				break;
				case UniformType::Matrix3x3:
				{
					const std::string& name = uniform.Name;
					glm::mat3& value = *(glm::mat3*)(uniformBuffer.GetBuffer() + uniform.Offset);
					Renderer::Submit([=]()
						{
							UploadUniformMat3(name, value);
						}
					);
				}
				break;
				case UniformType::Matrix4x4:
				{
					const std::string& name = uniform.Name;
					glm::mat4& value = *(glm::mat4*)(uniformBuffer.GetBuffer() + uniform.Offset);
					Renderer::Submit([=]()
						{
							UploadUniformMat4(name, value);
						}
					);
				}
				break;
			}
		}
	}

	/// <summary>
	/// 设置VSMaterialUniformBuffer并上传变量至Shader 
	/// </summary>
	void OpenGLShader::SetVSMaterialUniformBuffer(Buffer buffer)
	{
		Renderer::Submit([this, buffer]()
			{
				glUseProgram(m_RendererID);
				ResolveAndSetUniforms(m_VSMaterialUniformBuffer, buffer);
			}
		);
	}

	/// <summary>
	/// 设置PSMaterialUniformBuffer并上传变量至Shader 
	/// </summary>
	void OpenGLShader::SetPSMaterialUniformBuffer(Buffer buffer)
	{
		Renderer::Submit([this, buffer]()
			{
				glUseProgram(m_RendererID);
				ResolveAndSetUniforms(m_PSMaterialUniformBuffer, buffer);
			}
		);
	}

	void OpenGLShader::UploadUniformInt(const std::string& name, int value)
	{
		GLint location = GetUniformLocation(name);
		UploadUniformInt(location, value);
	}

	void OpenGLShader::UploadUniformIntArray(const std::string& name, int value[], uint32_t count)
	{
		GLint location = GetUniformLocation(name);
		UploadUniformIntArray(location, value, count);
	}

	void OpenGLShader::UploadUniformFloat(const std::string& name, float value)
	{
		GLint location = GetUniformLocation(name);
		UploadUniformFloat(location, value);
	}

	void OpenGLShader::UploadUniformFloat2(const std::string& name, const glm::vec2& value)
	{
		GLint location = GetUniformLocation(name);
		UploadUniformFloat2(location, value);
	}

	void OpenGLShader::UploadUniformFloat3(const std::string& name, const glm::vec3& value)
	{
		GLint location = GetUniformLocation(name);
		UploadUniformFloat3(location, value);
	}

	void OpenGLShader::UploadUniformFloat4(const std::string& name, const glm::vec4& value)
	{
		GLint location = GetUniformLocation(name);
		UploadUniformFloat4(location, value);
	}

	void OpenGLShader::UploadUniformMat3(const std::string& name, const glm::mat3& matrix)
	{
		GLint location = GetUniformLocation(name);
		UploadUniformMat3(location, matrix);
	}

	void OpenGLShader::UploadUniformMat4(const std::string& name, const glm::mat4& matrix)
	{
		GLint location = GetUniformLocation(name);
		UploadUniformMat4(location, matrix);
	}

	void OpenGLShader::UploadUniformMat4Array(const std::string& name, const glm::mat4& matrix, uint32_t count)
	{
		GLint location = GetUniformLocation(name);
		UploadUniformMat4Array(location, matrix, count);
	}

	void OpenGLShader::UploadUniformInt(uint32_t location, int value)
	{
		glUniform1i(location, value);
	}

	void OpenGLShader::UploadUniformIntArray(uint32_t location, int value[], uint32_t count)
	{
		glUniform1iv(location, count, value);
	}

	void OpenGLShader::UploadUniformFloat(uint32_t location, float value)
	{
		glUniform1f(location, value);
	}

	void OpenGLShader::UploadUniformFloat2(uint32_t location, const glm::vec2& value)
	{
		glUniform2f(location, value.x, value.y);
	}

	void OpenGLShader::UploadUniformFloat3(uint32_t location, const glm::vec3& value)
	{
		glUniform3f(location, value.x, value.y, value.z);
	}

	void OpenGLShader::UploadUniformFloat4(uint32_t location, const glm::vec4& value)
	{
		glUniform4f(location, value.x, value.y, value.z, value.w);
	}

	void OpenGLShader::UploadUniformMat3(uint32_t location, const glm::mat3& matrix)
	{
		glUniformMatrix3fv(location, 1, GL_FALSE, glm::value_ptr(matrix));
	}

	void OpenGLShader::UploadUniformMat4(uint32_t location, const glm::mat4& matrix)
	{
		glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(matrix));
	}

	void OpenGLShader::UploadUniformMat4Array(uint32_t location, const glm::mat4& matrix, uint32_t count)
	{
		glUniformMatrix4fv(location, count, GL_FALSE, glm::value_ptr(matrix));
	}

	void OpenGLShader::UploadUniformStruct(OpenGLShaderUniform* uniform, uint8_t* buffer, uint32_t offset)
	{
		const auto& fields = uniform->GetShaderUniformStruct().GetFields();
		for (size_t k = 0; k < fields.size(); k++)
		{
			OpenGLShaderUniform* field = (OpenGLShaderUniform*)fields[k];
			ResolveAndSetUniformField(*field, buffer, offset);
			offset += field->m_Size;
		}
	}

	void OpenGLShader::Set(const std::string& name, int value)
	{
		Renderer::Submit([=]() {
			UploadUniformInt(name, value);
			});
	}

	void OpenGLShader::Set(const std::string& name, int value[], uint32_t count)
	{
		Renderer::Submit([=]() {
			UploadUniformIntArray(name, value, count);
			});
	}

	void OpenGLShader::Set(const std::string& name, float value)
	{
		Renderer::Submit([=]() {
			UploadUniformFloat(name, value);
			});
	}

	void OpenGLShader::Set(const std::string& name, const glm::vec2& value)
	{
		Renderer::Submit([=]() {
			UploadUniformFloat2(name, value);
			});
	}

	void OpenGLShader::Set(const std::string& name, const glm::vec3& value)
	{
		Renderer::Submit([=]() {
			UploadUniformFloat3(name, value);
			});
	}

	void OpenGLShader::Set(const std::string& name, const glm::vec4& value)
	{
		Renderer::Submit([=]() {
			UploadUniformFloat4(name, value);
			});
	}

	void OpenGLShader::Set(const std::string& name, const glm::mat3& matrix)
	{
		Renderer::Submit([=]() {
			UploadUniformMat3(name, matrix);
			});
	}

	void OpenGLShader::Set(const std::string& name, const glm::mat4& matrix)
	{
		Renderer::Submit([=]() {
			UploadUniformMat4(name, matrix);
			});
	}

	void OpenGLShader::AddShaderReloadedCallback(const ShaderReloadedCallback& callback)
	{
		m_ShaderReloadedCallbacks.push_back(callback);
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

	void OpenGLShader::Load(const std::string& source)
	{
		m_ShaderSource = PreProcess(source);
		Parse();

		Renderer::Submit([=]()
			{
				if (m_RendererID)
					glDeleteProgram(m_RendererID);

				Compile();
				ResolveUniforms();

				//Reload callback
				if (m_Loaded)
				{
					for (auto& callback : m_ShaderReloadedCallbacks)
						callback();
				}

				m_Loaded = true;

				RENDERCOMMAND_TRACE("RenderCommand: Construct shader. Name: [{0}], ID: ({1})", m_Name, m_RendererID);
			}
		);
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

	//--------------------------------------------------------------------------------
	//Parsing helper function
	//--------------------------------------------------------------------------------
	const char* FindToken(const char* str, const std::string& token)
	{
		const char* t = str;
		while (t = strstr(t, token.c_str()))
		{
			bool left = str == t || isspace(t[-1]);
			bool right = !t[token.size()] || isspace(t[token.size()]);
			if (left && right)
				return t;

			t += token.size();
		}
		return nullptr;
	}

	const char* FindToken(const std::string& string, const std::string& token)
	{
		return FindToken(string.c_str(), token);
	}

	/// <summary>
	/// 基于符号对字符串分割
	/// </summary>
	/// <param name="string">源字符串</param>
	/// <param name="delimiters">分割符</param>
	/// <returns>分割后的字符串</returns>
	std::vector<std::string> SplitString(const std::string& string, const std::string& delimiters)
	{
		size_t start = 0;
		size_t end = string.find_first_of(delimiters);

		std::vector<std::string> result;

		while (end <= std::string::npos)
		{
			std::string token = string.substr(start, end - start);
			if (!token.empty())
				result.push_back(token);

			if (end == std::string::npos)
				break;

			start = end + 1;
			end = string.find_first_of(delimiters, start);
		}

		return result;
	}

	std::vector<std::string> SplitString(const std::string& string, const char delimiter)
	{
		return SplitString(string, std::string(1, delimiter));
	}

	/// <summary>
	/// 将字符串切割为单词
	/// </summary>
	std::vector<std::string> Tokenize(const std::string& string)
	{
		return SplitString(string, " \t\n\r");
	}

	std::vector<std::string> GetLines(const std::string& string)
	{
		return SplitString(string, "\n");
	}

	/// <summary>
	/// 根据'}'进行字符串裁切
	/// </summary>
	/// <param name="str">源字符串</param>
	/// <param name="outPosition">裁切后的指针位置</param>
	/// <returns>'}'前的字符串</returns>
	std::string GetBlock(const char* str, const char** outPosition)
	{
		const char* end = strstr(str, "}");
		if (!end)
			return str;

		if (outPosition)
			*outPosition = end;
		uint32_t length = end - str + 1;
		return std::string(str, length);
	}

	/// <summary>
	/// 根据';'进行字符串裁切
	/// </summary>
	/// <param name="str">源字符串</param>
	/// <param name="outPosition">裁切后的指针位置</param>
	/// <returns>'}'前的字符串</returns>
	std::string GetStatement(const char* str, const char** outPosition)
	{
		const char* end = strstr(str, ";");
		if (!end)
			return str;

		if (outPosition)
			*outPosition = end;
		uint32_t length = end - str + 1;
		return std::string(str, length);
	}

	/// <summary>
	/// 检查字符串是否以特定前缀起始
	/// </summary>
	bool StartsWith(const std::string& string, const std::string& start)
	{
		return string.find(start) == 0;
	}
	//--------------------------------------------------------------------------------
	//--------------------------------------------------------------------------------

	void OpenGLShader::Parse()
	{
		const char* token;
		const char* vstr;
		const char* fstr;

		m_Resources.clear();
		m_Structs.clear();
		m_VSMaterialUniformBuffer.reset();
		m_PSMaterialUniformBuffer.reset();

		auto& vertexSource = m_ShaderSource[GL_VERTEX_SHADER];
		auto& fragmentSource = m_ShaderSource[GL_FRAGMENT_SHADER];

		//Vertex Shader
		vstr = vertexSource.c_str();
		while (token = FindToken(vstr, "struct"))
			ParseUniformStruct(GetBlock(token, &vstr), ShaderDomain::Vertex);

		vstr = vertexSource.c_str();
		while (token = FindToken(vstr, "uniform"))
			ParseUniform(GetStatement(token, &vstr), ShaderDomain::Vertex);
		
		// Fragment Shader
		fstr = fragmentSource.c_str();
		while (token = FindToken(fstr, "struct"))
			ParseUniformStruct(GetBlock(token, &fstr), ShaderDomain::Pixel);

		fstr = fragmentSource.c_str();
		while (token = FindToken(fstr, "uniform"))
			ParseUniform(GetStatement(token, &fstr), ShaderDomain::Pixel);
	}

	static bool IsTypeStringResource(const std::string& type)
	{
		if (type == "sampler2D")		return true;
		if (type == "samplerCube")		return true;
		return false;
	}

	/// <summary>
	/// 解析Shader中的uniform变量
	/// </summary>
	/// <param name="statement">uniform声明</param>
	/// <param name="domain">Shader类型</param>
	void OpenGLShader::ParseUniform(const std::string& statement, ShaderDomain domain)
	{
		std::vector<std::string> tokens = Tokenize(statement);
		uint32_t index = 0;

		index++;
		std::string typeString = tokens[index++];
		std::string name = tokens[index++];
		//裁切去除name中的';'
		if (const char* s = strstr(name.c_str(), ";"))
			name = std::string(name.c_str(), s - name.c_str());
		//解析变量名、数组长度
		int32_t count = 1;
		const char* nameStr = name.c_str();
		if (const char* s = strstr(nameStr, "["))
		{
			name = std::string(nameStr, s - nameStr);

			const char* end = strstr(nameStr, "]");
			std::string c(s + 1, end - s);
			count = atoi(c.c_str());
		}
		//基于变量类型进行解析
		if (IsTypeStringResource(typeString))
		{
			//sampler
			ShaderResource* shaderResource = new OpenGLShaderResource(OpenGLShaderResource::StringToType(typeString), name, count);
			m_Resources.push_back(shaderResource);
		}
		else
		{
			//other types
			OpenGLShaderUniform::Type t = OpenGLShaderUniform::StringToType(typeString);
			OpenGLShaderUniform* uniform = nullptr;

			if (t == OpenGLShaderUniform::Type::None)
			{
				//struct
				ShaderStruct* s = FindStruct(typeString);
				ENGINE_ASSERT(s, "Fail to find shader struct!");
				uniform = new OpenGLShaderUniform(domain, s, name, count);
			}
			else
			{
				//basic type
				uniform = new OpenGLShaderUniform(domain, t, name, count);
			}

			if (StartsWith(name, "r_"))
			{
				if (domain == ShaderDomain::Vertex)
					((OpenGLShaderUniformBuffer*)m_VSRendererUniformBuffers.front())->PushUniform(uniform);
				else if (domain == ShaderDomain::Pixel)
					((OpenGLShaderUniformBuffer*)m_PSRendererUniformBuffers.front())->PushUniform(uniform);
			}
			else
			{
				if (domain == ShaderDomain::Vertex)
				{
					if (!m_VSMaterialUniformBuffer)
						m_VSMaterialUniformBuffer = CreateRef<OpenGLShaderUniformBuffer>("VertexMaterialUniformBuffer", domain);
					m_VSMaterialUniformBuffer->PushUniform(uniform);
				}
				else if (domain == ShaderDomain::Pixel)
				{
					if (!m_PSMaterialUniformBuffer)
						m_PSMaterialUniformBuffer = CreateRef<OpenGLShaderUniformBuffer>("FragmentMaterialUniformBuffer", domain);
					m_PSMaterialUniformBuffer->PushUniform(uniform);
				}
			}
		}
	}

	/// <summary>
	/// 解析Shader中的Struct
	/// </summary>
	/// <param name="block">Struct块</param>
	/// <param name="domain">Shader类型</param>
	void OpenGLShader::ParseUniformStruct(const std::string& block, ShaderDomain domain)
	{
		std::vector<std::string> tokens = Tokenize(block);
		uint32_t index = 0;

		index++;
		std::string name = tokens[index++];
		ShaderStruct* uniformStruct = new ShaderStruct(name);
		index++;
		//解析结构体内部变量
		while (index < tokens.size())
		{
			if (tokens[index] == "}")
				break;

			std::string type = tokens[index++];
			std::string name = tokens[index++];
			//裁切去除name中的';'
			if (const char* s = strstr(name.c_str(), ";"))
				name = std::string(name.c_str(), s - name.c_str());
			//解析变量名、数组长度
			uint32_t count = 1;
			const char* namestr = name.c_str();
			if (const char* s = strstr(namestr, "["))
			{
				name = std::string(namestr, s - namestr);

				const char* end = strstr(namestr, "]");
				std::string c(s + 1, end - s);
				count = atoi(c.c_str());
			}
			ShaderUniform* field = new OpenGLShaderUniform(domain, OpenGLShaderUniform::StringToType(type), name, count);
			uniformStruct->AddField(field);
		}
		m_Structs.push_back(uniformStruct);
	}

	ShaderStruct* OpenGLShader::FindStruct(const std::string& name)
	{
		for (ShaderStruct* s : m_Structs)
		{
			if (s->GetName() == name)
				return s;
		}
		return nullptr;
	}

	int32_t OpenGLShader::GetUniformLocation(const std::string& name) const
	{
		int32_t result = glGetUniformLocation(m_RendererID, name.c_str());
		if (result == -1)
			ENGINE_WARN("Shader({0}): Uniform({1}) connot be found or unused", m_Name, name);
		return result;
	}

	void OpenGLShader::ResolveUniforms()
	{
		glUseProgram(m_RendererID);

		//Vertex Shader
		for (uint32_t i = 0; i < m_VSRendererUniformBuffers.size(); i++)
		{
			const ShaderUniformList& uniforms = ((OpenGLShaderUniformBuffer*)m_VSRendererUniformBuffers[i])->GetUniforms();
			for (uint32_t j = 0; j < uniforms.size(); j++)
			{
				OpenGLShaderUniform* uniform = (OpenGLShaderUniform*)uniforms[j];
				if (uniform->GetType() == OpenGLShaderUniform::Type::Struct)
				{
					const ShaderStruct& s = uniform->GetShaderUniformStruct();
					const auto& fields = s.GetFields();
					for (size_t k = 0; k < fields.size(); k++)
					{
						OpenGLShaderUniform* field = (OpenGLShaderUniform*)fields[k];
						field->m_Location = GetUniformLocation(uniform->m_Name + "." + field->m_Name);
					}
				}
				else
				{
					uniform->m_Location = GetUniformLocation(uniform->m_Name);
				}
			}
		}

		//Fragment Shader
		for (uint32_t i = 0; i < m_PSRendererUniformBuffers.size(); i++)
		{
			const ShaderUniformList& uniforms = ((OpenGLShaderUniformBuffer*)m_PSRendererUniformBuffers[i])->GetUniforms();
			for (uint32_t j = 0; j < uniforms.size(); j++)
			{
				OpenGLShaderUniform* uniform = (OpenGLShaderUniform*)uniforms[j];
				if (uniform->GetType() == OpenGLShaderUniform::Type::Struct)
				{
					const ShaderStruct& s = uniform->GetShaderUniformStruct();
					const auto& fields = s.GetFields();
					for (size_t k = 0; k < fields.size(); k++)
					{
						OpenGLShaderUniform* field = (OpenGLShaderUniform*)fields[k];
						field->m_Location = GetUniformLocation(uniform->m_Name + "." + field->m_Name);
					}
				}
				else
				{
					uniform->m_Location = GetUniformLocation(uniform->m_Name);
				}
			}
		}

		//Vertex Shader MaterialUniformBuffer
		{
			const auto& uniformBuffer = m_VSMaterialUniformBuffer;
			if (uniformBuffer)
			{
				const ShaderUniformList& uniforms = uniformBuffer->GetUniforms();
				for (uint32_t j = 0; j < uniforms.size(); j++)
				{
					OpenGLShaderUniform* uniform = (OpenGLShaderUniform*)uniforms[j];
					if (uniform->GetType() == OpenGLShaderUniform::Type::Struct)
					{
						const ShaderStruct& s = uniform->GetShaderUniformStruct();
						const auto& fields = s.GetFields();
						for (size_t k = 0; k < fields.size(); k++)
						{
							OpenGLShaderUniform* field = (OpenGLShaderUniform*)fields[k];
							field->m_Location = GetUniformLocation(uniform->m_Name + "." + field->m_Name);
						}
					}
					else
					{
						uniform->m_Location = GetUniformLocation(uniform->m_Name);
					}
				}
			}
		}

		//Fragment Shader MaterialUniformBuffer
		{
			const auto& uniformBuffer = m_PSMaterialUniformBuffer;
			if (uniformBuffer)
			{
				const ShaderUniformList& uniforms = uniformBuffer->GetUniforms();
				for (uint32_t j = 0; j < uniforms.size(); j++)
				{
					OpenGLShaderUniform* uniform = (OpenGLShaderUniform*)uniforms[j];
					if (uniform->GetType() == OpenGLShaderUniform::Type::Struct)
					{
						const ShaderStruct& s = uniform->GetShaderUniformStruct();
						const auto& fields = s.GetFields();
						for (size_t k = 0; k < fields.size(); k++)
						{
							OpenGLShaderUniform* field = (OpenGLShaderUniform*)fields[k];
							field->m_Location = GetUniformLocation(uniform->m_Name + "." + field->m_Name);
						}
					}
					else
					{
						uniform->m_Location = GetUniformLocation(uniform->m_Name);
					}
				}
			}
		}

		//注册Shader sampler 
		uint32_t sampler = 0;
		for (uint32_t i = 0; i < m_Resources.size(); i++)
		{
			OpenGLShaderResource* resource = (OpenGLShaderResource*)m_Resources[i];

			if (resource->GetCount() == 1)
			{
				resource->m_Register = sampler;
				UploadUniformInt(resource->m_Name, sampler);
				sampler++;
			}
			else if (resource->GetCount() > 1)
			{
				resource->m_Register = sampler;
				uint32_t count = resource->GetCount();
				int* samplers = new int[count];
				for (uint32_t s = 0; s < count; s++)
					samplers[s] = sampler++;
				UploadUniformIntArray(resource->GetName(), samplers, count);
				delete[] samplers;
			}
		}
	}

	void OpenGLShader::ResolveAndSetUniforms(const Ref<OpenGLShaderUniformBuffer>& uniformBuffer, Buffer buffer)
	{
		const ShaderUniformList& uniforms = uniformBuffer->GetUniforms();
		for (uint32_t i = 0; i < uniforms.size(); i++)
		{
			OpenGLShaderUniform* uniform = (OpenGLShaderUniform*)uniforms[i];
			if (uniform->IsArray())
				ResolveAndSetUniformArray(uniform, buffer);
			else
				ResolveAndSetUniform(uniform, buffer);
		}
	}

	void OpenGLShader::ResolveAndSetUniform(OpenGLShaderUniform* uniform, Buffer buffer)
	{
		if (uniform->GetLocation() == -1 && uniform->m_Struct == nullptr)
			return;

		uint32_t offset = uniform->GetOffset();
		switch (uniform->GetType())
		{
		case OpenGLShaderUniform::Type::Bool:
			UploadUniformFloat(uniform->GetLocation(), *(bool*)&buffer.Data[offset]);
			break;
		case OpenGLShaderUniform::Type::Float:
			UploadUniformFloat(uniform->GetLocation(), *(float*)&buffer.Data[offset]);
			break;
		case OpenGLShaderUniform::Type::Int:
			UploadUniformInt(uniform->GetLocation(), *(int*)&buffer.Data[offset]);
			break;
		case OpenGLShaderUniform::Type::Vec2:
			UploadUniformFloat2(uniform->GetLocation(), *(glm::vec2*)&buffer.Data[offset]);
			break;
		case OpenGLShaderUniform::Type::Vec3:
			UploadUniformFloat3(uniform->GetLocation(), *(glm::vec3*)&buffer.Data[offset]);
			break;
		case OpenGLShaderUniform::Type::Vec4:
			UploadUniformFloat4(uniform->GetLocation(), *(glm::vec4*)&buffer.Data[offset]);
			break;
		case OpenGLShaderUniform::Type::Mat3:
			UploadUniformMat3(uniform->GetLocation(), *(glm::mat3*)&buffer.Data[offset]);
			break;
		case OpenGLShaderUniform::Type::Mat4:
			UploadUniformMat4(uniform->GetLocation(), *(glm::mat4*)&buffer.Data[offset]);
			break;
		case OpenGLShaderUniform::Type::Struct:
			UploadUniformStruct(uniform, buffer.Data, offset);
			break;
		default:
			ENGINE_ASSERT(false, "Unknown uniform type!");
		}
	}

	void OpenGLShader::ResolveAndSetUniformArray(OpenGLShaderUniform* uniform, Buffer buffer)
	{
		uint32_t offset = uniform->GetOffset();
		switch (uniform->GetType())
		{
		case OpenGLShaderUniform::Type::Bool:
			UploadUniformFloat(uniform->GetLocation(), *(bool*)&buffer.Data[offset]);
			break;
		case OpenGLShaderUniform::Type::Float:
			UploadUniformFloat(uniform->GetLocation(), *(float*)&buffer.Data[offset]);
			break;
		case OpenGLShaderUniform::Type::Int: 
			UploadUniformInt(uniform->GetLocation(), *(int*)&buffer.Data[offset]);
			//Maybe UploadUniformIntArray
			break;
		case OpenGLShaderUniform::Type::Vec2:
			UploadUniformFloat2(uniform->GetLocation(), *(glm::vec2*)&buffer.Data[offset]);
			break;
		case OpenGLShaderUniform::Type::Vec3:
			UploadUniformFloat3(uniform->GetLocation(), *(glm::vec3*)&buffer.Data[offset]);
			break;
		case OpenGLShaderUniform::Type::Vec4:
			UploadUniformFloat4(uniform->GetLocation(), *(glm::vec4*)&buffer.Data[offset]);
			break;
		case OpenGLShaderUniform::Type::Mat3:
			UploadUniformMat3(uniform->GetLocation(), *(glm::mat3*)&buffer.Data[offset]);
			break;
		case OpenGLShaderUniform::Type::Mat4:
			UploadUniformMat4Array(uniform->GetLocation(), *(glm::mat4*)&buffer.Data[offset], uniform->GetCount());
			break;
		case OpenGLShaderUniform::Type::Struct:
			UploadUniformStruct(uniform, buffer.Data, offset);
			break;
		default:
			ENGINE_ASSERT(false, "Unknown uniform type!");
		}
	}

	void OpenGLShader::ResolveAndSetUniformField(const OpenGLShaderUniform& field, uint8_t* data, int32_t offset)
	{
		switch (field.GetType())
		{
		case OpenGLShaderUniform::Type::Bool:
			UploadUniformFloat(field.GetLocation(), *(bool*)&data[offset]);
			break;
		case OpenGLShaderUniform::Type::Float:
			UploadUniformFloat(field.GetLocation(), *(float*)&data[offset]);
			break;
		case OpenGLShaderUniform::Type::Int:
			UploadUniformInt(field.GetLocation(), *(int*)&data[offset]);
			break;
		case OpenGLShaderUniform::Type::Vec2:
			UploadUniformFloat2(field.GetLocation(), *(glm::vec2*)&data[offset]);
			break;
		case OpenGLShaderUniform::Type::Vec3:
			UploadUniformFloat3(field.GetLocation(), *(glm::vec3*)&data[offset]);
			break;
		case OpenGLShaderUniform::Type::Vec4:
			UploadUniformFloat4(field.GetLocation(), *(glm::vec4*)&data[offset]);
			break;
		case OpenGLShaderUniform::Type::Mat3:
			UploadUniformMat3(field.GetLocation(), *(glm::mat3*)&data[offset]);
			break;
		case OpenGLShaderUniform::Type::Mat4:
			UploadUniformMat4(field.GetLocation(), *(glm::mat4*)&data[offset]);
			break;
		default:
			ENGINE_ASSERT(false, "Unknown uniform type!");
		}
	}

	void OpenGLShader::Compile()
	{
		std::vector<GLuint> shaderRendererIDs;

		//Create shader program
		GLuint program = glCreateProgram();
		for (auto& kv : m_ShaderSource)
		{
			GLenum type = kv.first;
			const std::string& source = kv.second;

			//Create shader handle
			GLuint shaderID = glCreateShader(type);
			//Send shader source to GL
			const GLchar* sourceCStr = (const GLchar*)source.c_str();
			glShaderSource(shaderID, 1, &sourceCStr, 0);
			//Compile shader
			glCompileShader(shaderID);
			//If shader is not compiled, print log and delete shader
			GLint isCompiled = 0;
			glGetShaderiv(shaderID, GL_COMPILE_STATUS, &isCompiled);
			if (isCompiled == GL_FALSE)
			{
				GLint maxLength = 0;
				glGetShaderiv(shaderID, GL_INFO_LOG_LENGTH, &maxLength);

				std::vector<GLchar> infoLog(maxLength);
				glGetShaderInfoLog(shaderID, maxLength, &maxLength, &infoLog[0]);

				glDeleteShader(shaderID);

				ENGINE_ERROR("Shader({0}) compilation failure:\n{1}", m_Path, &infoLog[0]);
				return;
			}

			shaderRendererIDs.push_back(shaderID);
			glAttachShader(program, shaderID);
		}

		//Link program
		glLinkProgram(program);

		//If link failed, print log and delete shader
		GLint isLinked = 0;
		glGetProgramiv(program, GL_LINK_STATUS, &isLinked);
		if (isLinked == GL_FALSE)
		{
			GLint maxLength = 1024;
			//glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);

			std::vector<GLchar> infoLog(maxLength);
			glGetProgramInfoLog(program, maxLength, &maxLength, &infoLog[0]);

			glDeleteProgram(program);
			for (auto id : shaderRendererIDs)
				glDeleteShader(id);

			ENGINE_ERROR("Shader({0}) link failure:\n{1}", m_Path, &infoLog[0]);
			return;
		}

		//detach shader after a successful link
		for (auto id : shaderRendererIDs)
			glDetachShader(program, id);

		m_RendererID = program;
	}


}