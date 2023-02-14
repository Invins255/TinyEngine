#include <TinyEngine.h>

#include "imgui/imgui.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Engine/Core/EntryPoint.h"
#include "Engine/Platforms/OpenGL/OpenGLShader.h"

#include "Sandbox2D.h"

class ExampleLayer : public Engine::Layer
{
public:
	ExampleLayer() :
		Layer("Example"), m_CameraController(1280.0f / 720.0f, false), m_Position(0.0f)
	{

		m_VertexArray = Engine::VertexArray::Create();

		float vertices[] = {
			-0.5f, -0.5f, 0.0f,	0.0f, 0.0f,
			 0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
			 0.5f,  0.5f, 0.0f, 1.0f, 1.0f,
			-0.5f,  0.5f, 0.0f, 0.0f, 1.0f
		};

		Engine::Ref<Engine::VertexBuffer> vertexBuffer;
		vertexBuffer = Engine::VertexBuffer::Create(vertices, sizeof(vertices));
		Engine::VertexBufferLayout layout = {
			{Engine::ShaderDataType::Float3, "a_Position"},
			{Engine::ShaderDataType::Float2, "a_TexCoord"}
		};
		vertexBuffer->SetLayout(layout);
		m_VertexArray->AddVertexBuffer(vertexBuffer);

		unsigned int indices[] = {
			0, 1, 2,
			2, 3, 0
		};
		Engine::Ref<Engine::IndexBuffer> indexBuffer;
		indexBuffer = Engine::IndexBuffer::Create(indices, sizeof(indices) / sizeof(uint32_t));
		m_VertexArray->SetIndexBuffer(indexBuffer);

		std::string vertexSrc = R"(
			#version 330 core

			layout(location = 0) in vec3 a_Position;			
			layout(location = 1) in vec2 a_TexCoord;

			uniform mat4 u_Transform;
			uniform mat4 u_ViewProjection;

			out vec2 v_TexCoord;		

			void main()
			{
				v_TexCoord = a_TexCoord;
				gl_Position = u_ViewProjection * u_Transform * vec4(a_Position, 1.0);
			}
		)";
		std::string fragmentSrc = R"(
			#version 330 core

			layout(location = 0) out vec4 color;
			
			in vec2 v_TexCoord;
			
			uniform sampler2D u_Texture;	

			void main()
			{
				color = texture(u_Texture, v_TexCoord);
			}
		)";
		auto shader = m_ShaderLibrary.Load("assets/Shaders/Texture.glsl");

		m_Texture = Engine::Texture2D::Create("assets/Textures/Checkerboard.png");

		std::dynamic_pointer_cast<Engine::OpenGLShader>(shader)->Bind();
		std::dynamic_pointer_cast<Engine::OpenGLShader>(shader)->UploadUniformInt("u_Texture", 0);

	}

	~ExampleLayer()
	{
	}

	void OnUpdate(Engine::Timestep ts) override
	{
		//Update
		m_CameraController.OnUpdate(ts);

		if (Engine::Input::IsKeyPressed(ENGINE_KEY_J))
			m_Position.x -= m_MoveSpeed * ts;
		else if (Engine::Input::IsKeyPressed(ENGINE_KEY_L))
			m_Position.x += m_MoveSpeed * ts;
		if (Engine::Input::IsKeyPressed(ENGINE_KEY_K))
			m_Position.y -= m_MoveSpeed * ts;
		else if (Engine::Input::IsKeyPressed(ENGINE_KEY_I))
			m_Position.y += m_MoveSpeed * ts;

		//Render
		Engine::RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1.0f });
		Engine::RenderCommand::Clear();

		Engine::Renderer::BeginScene(m_CameraController.GetCamera());

		glm::mat4 transform = glm::translate(glm::mat4(1.0f), m_Position);	

		auto shader = m_ShaderLibrary.Get("Texture");

		m_Texture->Bind();
		Engine::Renderer::Submit(m_VertexArray, shader, transform);

		Engine::Renderer::EndScene();
	}

	void OnEvent(Engine::Event& e) override
	{
		m_CameraController.OnEvent(e);
	}

	void OnImGuiRender() override
	{
		ImGui::Begin("Settings");
		ImGui::ColorEdit3("Color", glm::value_ptr(m_Color));
		ImGui::End();
	}


private:
	Engine::ShaderLibrary m_ShaderLibrary;
	Engine::Ref<Engine::VertexArray> m_VertexArray;

	Engine::OrthographicCameraController m_CameraController;

	glm::vec3 m_Position;
	float m_MoveSpeed = 1.0f;

	glm::vec3 m_Color = { 0.2f, 0.3f, 0.8f };
	Engine::Ref<Engine::Texture2D> m_Texture;
};

class Sandbox : public Engine::Application 
{
public:
	Sandbox()
	{
		//PushLayer(new ExampleLayer());
		PushLayer(new Sandbox2D());
	}

	~Sandbox() 
	{

	}
};

Engine::Application* Engine::CreateApplication() 
{
	return new Sandbox();
}
