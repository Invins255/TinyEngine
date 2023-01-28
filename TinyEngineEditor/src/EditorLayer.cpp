#include "EditorLayer.h"

#include <imgui/imgui.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <chrono>

namespace Engine
{
    EditorLayer::EditorLayer():
	    Layer("EditorLayer")
    {
    }

    void EditorLayer::OnAttach()
    {
        Engine::FrameBufferSpecification spec;
        spec.Width = 1280;
        spec.Height = 720;
        m_FrameBuffer = Engine::FrameBuffer::Create(spec);

        m_ActiveScene = CreateRef<Scene>();
        m_SceneHierarchyPanel.SetContext(m_ActiveScene);

        auto& camera = m_ActiveScene->CreateEntity("Camera");
        camera.AddComponent<CameraComponent>();
        auto& square = m_ActiveScene->CreateEntity("Square");
        square.AddComponent<SpriteRendererComponent>();

    }

    void EditorLayer::OnDetach()
    {
    }

    void EditorLayer::OnUpdate(Timestep ts)
    {    
        //Resize
        auto spec = m_FrameBuffer->GetSpecification();
        if (m_ViewportSize.x > 0.0f && m_ViewportSize.y > 0.0f && (spec.Width != m_ViewportSize.x || spec.Height != m_ViewportSize.y))
        {
            m_FrameBuffer->Resize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
            ENGINE_INFO("Viewport window size: ({0}, {1}), Sepc ({2}, {3})", (uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y, (uint32_t)spec.Width, (uint32_t)spec.Height);

            m_ActiveScene->OnViewportResize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
        }
        
        //Render--------------------------------------------------------------------- 		
        m_FrameBuffer->Bind();
        //Clear frameBuffer
        Engine::Renderer2D::ResetStats();

		Engine::RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1.0f });
		Engine::RenderCommand::Clear();	          
        //Update scene
        m_ActiveScene->OnUpdate(ts);

        m_FrameBuffer->Unbind();
        //---------------------------------------------------------------------------	    
    }

    void EditorLayer::OnEvent(Engine::Event& e)
    {

    }

    void EditorLayer::OnImGuiRender()
    {
	    //Dock space-----------------------------------------------------------------
        static bool p_open = true;

        static bool opt_fullscreen_persistant = true;
        static ImGuiDockNodeFlags opt_flags = ImGuiDockNodeFlags_None;
        bool opt_fullscreen = opt_fullscreen_persistant;

        // We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
        // because it would be confusing to have two docking targets within each others.
        ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
        if (opt_fullscreen)
        {
            ImGuiViewport* viewport = ImGui::GetMainViewport();
            ImGui::SetNextWindowPos(viewport->Pos);
            ImGui::SetNextWindowSize(viewport->Size);
            ImGui::SetNextWindowViewport(viewport->ID);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
            window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
            window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
        }

        // When using ImGuiDockNodeFlags_PassthruDockspace, DockSpace() will render our background and handle the pass-thru hole, so we ask Begin() to not render a background.
        //if (opt_flags & ImGuiDockNodeFlags_PassthruDockspace)
        //	window_flags |= ImGuiWindowFlags_NoBackground;

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        
        ImGui::Begin("DockSpace", &p_open, window_flags);
        {
            ImGui::PopStyleVar();

            if (opt_fullscreen)
                ImGui::PopStyleVar(2);

            // Submit the DockSpace
            ImGuiIO& io = ImGui::GetIO();
            if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
            {
                ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
                ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), opt_flags);
            }

            if (ImGui::BeginMenuBar())
            {
                if (ImGui::BeginMenu("File"))
                {
                    if (ImGui::MenuItem("Exit")) Engine::Application::Get().Close();
                    ImGui::EndMenu();
                }

                ImGui::EndMenuBar();
            }

            //Settings----------------------------------------------------------------------
            ImGui::Begin("Stats");
            {
                auto& stats = Engine::Renderer2D::GetStats();
                ImGui::Text("Renderer2D Stats:");
                ImGui::Text("Draw Calls: %d", stats.DrawCalls);
                ImGui::Text("Quads: %d", stats.QuadCounts);
                ImGui::Text("Vertices: %d", stats.GetTotalVertexCount());
                ImGui::Text("Indices: %d", stats.GetTotalIndexCount());
            }
            ImGui::End();

            //Viewport----------------------------------------------------------------------
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
            ImGui::Begin("Viewport");
            {
                m_ViewportFocused = ImGui::IsWindowFocused();
                m_ViewportHovered = ImGui::IsWindowHovered();
                Application::Get().GetImGuiLayer()->BlockEvents(!m_ViewportFocused || !m_ViewportHovered);
                
                ImVec2 viewportSize = ImGui::GetContentRegionAvail();
                m_ViewportSize = glm::vec2(viewportSize.x, viewportSize.y);

                uint32_t textureID = m_FrameBuffer->GetColorAttachmentID();
                ImGui::Image((void*)textureID, ImVec2(m_ViewportSize.x, m_ViewportSize.y), { 0, 1 }, { 1, 0 });
            }
            ImGui::End();
            ImGui::PopStyleVar();
            

            //Scene hierarchy---------------------------------------------------------------
            m_SceneHierarchyPanel.OnImGuiRender();
        }
        ImGui::End();
    }

}