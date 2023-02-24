#include "EditorLayer.h"

#include <imgui/imgui.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp> 

#include <chrono>
#include <filesystem>
#include <functional>

#include "Engine/Renderer/SceneRenderer.h"
#include "Engine/Scene/SceneSerializer.h"
#include "Engine/ImGui/ImGuizmo.h"
#include "Engine/Renderer/MeshFactory.h"
#include "Engine/Renderer/Texture.h"
#include "Engine/Scene/Environment.h"
#include "Engine/Core/Math/Ray.h"

namespace Engine
{
    EditorLayer::EditorLayer():
	    Layer("EditorLayer"), m_EditorCamera(glm::perspectiveFov(glm::radians(45.0f), 1280.0f, 720.0f, 0.1f, 1000.0f))
    {
        m_SceneHierarchyPanel.SetSelectionChangedCallback(std::bind(&EditorLayer::SelectEntity, this, std::placeholders::_1));
        m_SceneHierarchyPanel.SetEntityDeletedCallback(std::bind(&EditorLayer::OnEntityDeleted, this, std::placeholders::_1));
    }

    void EditorLayer::OnAttach()
    {
        NewScene();

        //TEMP
        /*
        auto skyboxTexture = TextureCube::Create(
            "assets\\textures\\skybox\\CornellBox\\right.jpg",
            "assets\\textures\\skybox\\CornellBox\\left.jpg",
            "assets\\textures\\skybox\\CornellBox\\top.jpg",
            "assets\\textures\\skybox\\CornellBox\\bottom.jpg",
            "assets\\textures\\skybox\\CornellBox\\front.jpg",
            "assets\\textures\\skybox\\CornellBox\\back.jpg"
        );
        m_EditorScene->SetSkybox(skyboxTexture);
        */
        
        auto environment = Environment::Create("assets\\environment\\Room.hdr");
        m_EditorScene->SetEnvironment(environment);
        /*
        {
            auto mesh = CreateRef<Mesh>("assets\\models\\Can\\SodaCan.fbx");
            auto& meshEntity = m_EditorScene->CreateEntity("SodaCan 1");
            meshEntity.AddComponent<MeshComponent>();
            meshEntity.GetComponent<MeshComponent>().Mesh = mesh;
            meshEntity.GetComponent<TransformComponent>().Translation = glm::vec3(5.0f, 1.0f, 0.0f);
        }
        {
            auto mesh = CreateRef<Mesh>("assets\\models\\Can\\SodaCan.fbx");
            auto& meshEntity = m_EditorScene->CreateEntity("SodaCan 2");
            meshEntity.AddComponent<MeshComponent>();
            meshEntity.GetComponent<MeshComponent>().Mesh = mesh;
            meshEntity.GetComponent<TransformComponent>().Translation = glm::vec3(-5.0f, 1.0f, 0.0f);
        }
        */
        {
            //auto mesh = CreateRef<Mesh>("assets\\models\\Sphere\\Sphere.fbx");
            auto mesh = MeshFactory::CreateSphere(5.0f);
            auto& meshEntity = m_EditorScene->CreateEntity("Sphere");
            meshEntity.AddComponent<MeshComponent>();
            meshEntity.GetComponent<MeshComponent>().Mesh = mesh;
            meshEntity.GetComponent<TransformComponent>().Translation = glm::vec3(0.0f, 8.0f, 0.0f);
        }
        {
            auto mesh = CreateRef<Mesh>("assets\\models\\Plane\\Plane.fbx");
            auto& meshEntity = m_EditorScene->CreateEntity("Plane");
            meshEntity.AddComponent<MeshComponent>();
            meshEntity.GetComponent<MeshComponent>().Mesh = mesh;
        }
        
        auto& camera = m_EditorScene->CreateEntity("Camera");
        camera.AddComponent<CameraComponent>();
        camera.GetComponent<TransformComponent>().Translation = glm::vec3(0.0f, 20.0f, 30.0f);
        camera.GetComponent<TransformComponent>().Rotation = glm::radians(glm::vec3(-20.0f, 0.0f, 0.0f));

        auto& light = m_EditorScene->CreateEntity("Directional Light");
        light.AddComponent<DirectionalLightComponent>();
        light.GetComponent<TransformComponent>().Rotation = glm::radians(glm::vec3(-180.0f, 0.0f, 0.0f));
    }

    void EditorLayer::OnDetach()
    {
    }

    void EditorLayer::OnUpdate(Timestep ts)
    {      
        //Update editor scene
        if(m_ViewportFocused)
            m_EditorCamera.OnUpdate(ts);

        m_EditorScene->OnRenderEditor(ts, m_EditorCamera, m_EditorCamera.GetViewMatrix());
    }

    void EditorLayer::OnEvent(Engine::Event& e)
    {
        if (m_ViewportFocused)
            m_EditorCamera.OnEvent(e); 

        EventDispatcher dispatcher(e);
        dispatcher.Dispatch<KeyPressedEvent>(ENGINE_BIND_EVENT_FN(EditorLayer::OnKeyPressedEvent));
        dispatcher.Dispatch<MouseButtonPressedEvent>(ENGINE_BIND_EVENT_FN(EditorLayer::OnMouseButtonPressed));
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

                if (ImGui::BeginMenu("Shaders"))
                {
                    auto& shaders = Shader::s_AllShaders;
                    for (auto& shader : shaders)
                    {
                        if (ImGui::TreeNode(shader->GetName().c_str()))
                        {
                            std::string buttonName = "Reload##" + shader->GetName();
                            if (ImGui::Button(buttonName.c_str()))
                                shader->Reload();
                            ImGui::TreePop();
                        }
                    }
                    ImGui::EndMenu();
                }

                ImGui::EndMenuBar();
            }

            //ImGui::ShowDemoWindow();

            //Viewport----------------------------------------------------------------------
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
            ImGui::Begin("Viewport");
            {
                m_ViewportFocused = ImGui::IsWindowFocused();
                m_ViewportHovered = ImGui::IsWindowHovered();
                Application::Get().GetImGuiLayer()->BlockEvents(!m_ViewportFocused || !m_ViewportHovered);
                
                //Resize
                ImVec2 viewportSize = ImGui::GetContentRegionAvail();
                SceneRenderer::SetViewportSize((uint32_t)viewportSize.x, (uint32_t)viewportSize.y);
                m_EditorScene->SetViewportSize((uint32_t)viewportSize.x, (uint32_t)viewportSize.y);
                m_EditorCamera.SetProjection(glm::perspectiveFov(glm::radians(45.0f), viewportSize.x, viewportSize.y, 0.1f, 1000.0f));
                m_EditorCamera.SetViewportSize((uint32_t)viewportSize.x, (uint32_t)viewportSize.y);

                //Render final framebuffer to viewport
                uint32_t textureID = SceneRenderer::GetFinalColorBufferRendererID();
                ImGui::Image((void*)textureID, viewportSize, { 0, 1 }, { 1, 0 });
                
                //Calculate viewport bounds
                ImVec2 viewportOffset = ImGui::GetCursorPos();
                ImVec2 windowSize = ImGui::GetWindowSize();
                ImVec2 minBound = ImGui::GetWindowPos();
                minBound.x += viewportOffset.x;
                minBound.y += viewportOffset.y;
                ImVec2 maxBound = { minBound.x + windowSize.x, minBound.y + windowSize.y };
                m_ViewportBounds[0] = { minBound.x, minBound.y };
                m_ViewportBounds[1] = { maxBound.x, maxBound.y };
            }
            ImGui::End();
            ImGui::PopStyleVar();         

            //Material----------------------------------------------------------------------
            if (!m_SelectionContext.empty())
            {
                Entity selectedEntity = m_SelectionContext.front().Entity;
                m_MaterialEditorPanel.SetSelectedEntity(selectedEntity);
            }
            else
                m_MaterialEditorPanel.SetSelectedEntity({});


            //Panels------------------------------------------------------------------------ 
            m_SceneHierarchyPanel.OnImGuiRender();
            m_ContentBrowserPanel.OnImGuiRender();
            m_MaterialEditorPanel.OnImGuiRender();

        }
        ImGui::End();
    }

    bool EditorLayer::OnKeyPressedEvent(KeyPressedEvent& e)
    {
        if (Input::IsKeyPressed(ENGINE_KEY_LEFT_CONTROL))
        {
            switch (e.GetKeyCode())
            {
            case ENGINE_KEY_N:
                NewScene();
                break;
            case ENGINE_KEY_O:
                OpenScene();
                break;
            case ENGINE_KEY_S:
                SaveScene();
                break;
            }
        }
        if (Input::IsKeyPressed(ENGINE_KEY_LEFT_SHIFT))
        {
            switch (e.GetKeyCode())
            {
            case ENGINE_KEY_S:
                SaveSceneAs();
                break;
            }
        }

        return false;
    }

    bool EditorLayer::OnMouseButtonPressed(MouseButtonPressedEvent& e)
    {
        //Mouse pick 
        if (e.GetMouseButton() == ENGINE_MOUSE_BUTTON_LEFT && m_ViewportHovered)
        {
            auto [mouseX, mouseY] = GetMouseViewportSpace();
            if (mouseX > -1.0f && mouseX < 1.0f && mouseY > -1.0f && mouseY < 1.0f)
            {
                auto [origin, direction] = CastMouseRay(mouseX, mouseY);

                m_SelectionContext.clear();
                m_EditorScene->SetSelectedEntity({});
                auto meshEntities = m_EditorScene->GetAllEntitiesWith<MeshComponent>();
                for (auto e : meshEntities)
                {
                    Entity entity(e, m_EditorScene.get());
                    auto mesh = entity.GetComponent<MeshComponent>().Mesh;
                    if (!mesh)
                        continue;

                    //查询Ray所碰撞的AABB
                    auto& submeshes = mesh->GetSubmeshes();
                    for (uint32_t i = 0; i < submeshes.size(); i++)
                    {
                        auto& submesh = submeshes[i];
                        Ray ray(
                            glm::inverse(entity.Transform().GetTransform() * submesh.Transform) * glm::vec4(origin, 1.0f),
                            glm::inverse(glm::mat3(entity.Transform().GetTransform()) * glm::mat3(submesh.Transform)) * direction
                        );

                        float t;
                        bool intersects = ray.IntersectsAABB(submesh.BoundingBox, t);
                        if (intersects)
                        {
                            const auto& triangleCache = mesh->GetTriangleCache(i);
                            for (const auto& triangle : triangleCache)
                            {
                                if (ray.IntersectsTriangle(triangle.V0.Position, triangle.V1.Position, triangle.V2.Position, t))
                                {
                                    m_SelectionContext.push_back({ entity, &submesh, t });
                                    break;
                                }
                            }
                        }
                    }
                }

                //根据最小距离排序
                std::sort(m_SelectionContext.begin(), m_SelectionContext.end(), [](auto& a, auto& b) { return a.Distance < b.Distance; });
                if (m_SelectionContext.size())
                    OnEntitySelected(m_SelectionContext[0]);
            }
        }

        return false;
    }


    void EditorLayer::NewScene()
    {
        m_EditorScene = CreateRef<Scene>("Untitled Scene");
        m_SceneHierarchyPanel.SetContext(m_EditorScene);
        m_SceneFilePath = "";

        m_EditorCamera = EditorCamera(glm::perspectiveFov(glm::radians(45.0f), 1280.0f, 720.0f, 0.1f, 1000.0f));

        APP_INFO("Create new scene");
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

        APP_INFO("Open scene '{0}'", filepath);
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

        APP_INFO("Save scene '{0}'", m_Name);
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

    void EditorLayer::SelectEntity(Entity entity)
    {
        SelectedSubmesh selection;
        selection.Entity = entity;
        if (entity.HasComponent<MeshComponent>())
        {
            auto& mc = entity.GetComponent<MeshComponent>();
            if (mc.Mesh)
                selection.Mesh = &mc.Mesh->GetSubmeshes()[0];
        }

        m_SelectionContext.clear();
        m_SelectionContext.push_back(selection);

        m_EditorScene->SetSelectedEntity(entity);
    }

    std::pair<float, float> EditorLayer::GetMouseViewportSpace() const
    {
        auto [mx, my] = ImGui::GetMousePos();
        mx -= m_ViewportBounds[0].x;
        my -= m_ViewportBounds[0].y;

        auto viewportWidth = m_ViewportBounds[1].x - m_ViewportBounds[0].x;
        auto viewportHeight = m_ViewportBounds[1].y - m_ViewportBounds[0].y;

        return { (mx / viewportWidth) * 2.0f - 1.0f, ((-my / viewportHeight) * 2.0f - 1.0f) };
    }

    std::pair<glm::vec3, glm::vec3> EditorLayer::CastMouseRay(float mx, float my)
    {
        glm::vec4 mouseClipPos = { mx, my, -1.0f, 1.0f };

        auto inverseProj = glm::inverse(m_EditorCamera.GetProjection());
        auto inverseView = glm::inverse(glm::mat3(m_EditorCamera.GetViewMatrix()));

        glm::vec4 ray = inverseProj * mouseClipPos;
        glm::vec3 rayPos = m_EditorCamera.GetPosition();
        glm::vec3 rayDir = inverseView * glm::vec3(ray);

        return { rayPos, rayDir };
    }

    void EditorLayer::OnEntitySelected(SelectedSubmesh& selectionContext)
    {
        APP_TRACE("Select entity: '{0}', mesh: '{1}', submesh: '{2}'",
            selectionContext.Entity.GetComponent<TagComponent>().Tag,
            selectionContext.Mesh ? selectionContext.Mesh->MeshName : "",
            selectionContext.Mesh ? selectionContext.Mesh->NodeName : ""
        );
        m_SceneHierarchyPanel.SetSelectedEntity(selectionContext.Entity);
        m_EditorScene->SetSelectedEntity(selectionContext.Entity);
    }

    void EditorLayer::OnEntityDeleted(Entity e)
    {
        if (m_SelectionContext[0].Entity == e)
        {
            m_SelectionContext.clear();
            m_EditorScene->SetSelectedEntity({});
        }
    }
}