#include "EditorLayer.h"

#include <imgui/imgui.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <chrono>
#include <filesystem>

#include "Engine/Renderer/SceneRenderer.h"
#include "Engine/Scene/SceneSerializer.h"
#include "Engine/Renderer/MeshFactory.h"

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

        NewScene();

        //TEMP
        {
            //auto mesh = CreateRef<Mesh>("assets/models/Sphere/Sphere.fbx");
            auto mesh = MeshFactory::CreateBox(glm::vec3(2.0f, 2.0f, 2.0f));
            auto& meshEntity = m_EditorScene->CreateEntity("Cube 1");
            meshEntity.AddComponent<MeshComponent>();
            meshEntity.GetComponent<MeshComponent>().Mesh = mesh;
            meshEntity.GetComponent<TransformComponent>().Translation = glm::vec3(-3.0f, 0.0f, 1.0f);
        }     
        {
            //auto mesh = CreateRef<Mesh>("assets/models/Sphere/Sphere.fbx");
            auto mesh = MeshFactory::CreateBox(glm::vec3(2.0f, 2.0f, 2.0f));
            auto& meshEntity = m_EditorScene->CreateEntity("Cube 2");
            meshEntity.AddComponent<MeshComponent>();
            meshEntity.GetComponent<MeshComponent>().Mesh = mesh;
            meshEntity.GetComponent<TransformComponent>().Translation = glm::vec3(-3.0f, 6.0f, 1.0f);
        }
        /**/
        {
            auto mesh = CreateRef<Mesh>("assets/models/Sphere/Sphere.fbx");
            auto& meshEntity = m_EditorScene->CreateEntity("Sphere 1");
            meshEntity.AddComponent<MeshComponent>();
            meshEntity.GetComponent<MeshComponent>().Mesh = mesh;
            meshEntity.GetComponent<TransformComponent>().Translation = glm::vec3(1.0f, 3.0f, -1.0f);
        }
        {
            auto mesh = CreateRef<Mesh>("assets/models/Sphere/Sphere.fbx");
            auto& meshEntity = m_EditorScene->CreateEntity("Sphere 2");
            meshEntity.AddComponent<MeshComponent>();
            meshEntity.GetComponent<MeshComponent>().Mesh = mesh;
            meshEntity.GetComponent<TransformComponent>().Translation = glm::vec3(2.0f, -3.0f, -1.0f);
        }
        
        auto& camera = m_EditorScene->CreateEntity("Camera Entity");
        camera.AddComponent<CameraComponent>();
        camera.GetComponent<TransformComponent>().Translation = glm::vec3(0.0f, 0.0f, 25.0f);

        auto& light = m_EditorScene->CreateEntity("Light Entity");
        light.AddComponent<DirectionalLightComponent>();
        light.GetComponent<TransformComponent>().Translation = glm::vec3(1.0f, 1.0f, 1.0f);
    }

    void EditorLayer::OnDetach()
    {
    }

    void EditorLayer::OnUpdate(Timestep ts)
    {    
        //Resize
        auto spec = SceneRenderer::GetFinalFrameBuffer()->GetSpecification();
        if (m_ViewportSize.x > 0.0f && m_ViewportSize.y > 0.0f && (spec.Width != m_ViewportSize.x || spec.Height != m_ViewportSize.y))
        {
            SceneRenderer::SetViewportSize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
            m_EditorScene->OnViewportResize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
            ENGINE_INFO("Viewport window size: ({0}, {1})", (uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
        }
               
        //Update scene
        m_EditorScene->OnUpdate(ts);
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

            //Begin Menu Bar
            if (ImGui::BeginMenuBar())
            {
                if (ImGui::BeginMenu("File"))
                {
                    if (ImGui::MenuItem("New Scene", "Ctrl+N"))
                        NewScene();
                    if (ImGui::MenuItem("Open Scene", "Ctrl+O"))
                        OpenScene();
                    ImGui::Separator();
                    if (ImGui::MenuItem("Save Scene", "Ctrl+S"))
                        SaveScene();
                    if (ImGui::MenuItem("Save Scene As", "Ctrl+Shift+S"))
                        SaveSceneAs();
                    ImGui::Separator();
                    if (ImGui::MenuItem("Exit")) Engine::Application::Get().Close();
                    ImGui::EndMenu();
                }

                ImGui::EndMenuBar();
            }

            //Settings----------------------------------------------------------------------
            ImGui::Begin("Stats");
            {

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

                uint32_t textureID = SceneRenderer::GetFinalColorBufferRendererID();
                ImGui::Image((void*)textureID, ImVec2(m_ViewportSize.x, m_ViewportSize.y), { 0, 1 }, { 1, 0 });
            }
            ImGui::End();
            ImGui::PopStyleVar();         

            //Scene hierarchy---------------------------------------------------------------
            m_SceneHierarchyPanel.OnImGuiRender();
        }
        ImGui::End();
    }

    void EditorLayer::NewScene()
    {
        m_EditorScene = CreateRef<Scene>("Empty Scene");
        m_SceneHierarchyPanel.SetContext(m_EditorScene);
        m_SceneFilePath = "";
    }

    void EditorLayer::OpenScene()
    {
        auto& app = Application::Get();
        std::string filepath = app.OpenFile("(*.scene)\0*.scene\0");
        if (!filepath.empty())
            OpenScene(filepath);
    }

    void EditorLayer::OpenScene(const std::string& filepath)
    {
        auto newScene = CreateRef<Scene>("New Scene");
        SceneSerializer serializer(newScene);
        serializer.Deserialize(filepath);
        m_EditorScene = newScene;
        m_SceneHierarchyPanel.SetContext(m_EditorScene);
    }

    void EditorLayer::SaveScene()
    {
        if (!m_SceneFilePath.empty())
        {
            SceneSerializer serializer(m_EditorScene);
            serializer.Serialize(m_SceneFilePath);
        }
        else
        {
            SaveSceneAs();
        }
    }

    void EditorLayer::SaveSceneAs()
    {
        auto& app = Application::Get();
        std::string filepath = app.SaveFile("(*.scene)\0*.scene\0");
        if (!filepath.empty())
        {
            SceneSerializer serializer(m_EditorScene);
            serializer.Serialize(filepath);

            m_SceneFilePath = filepath;
        }
    }
}