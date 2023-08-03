#include "EditorLayer.h"

#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>

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
#include "Engine/Core/Math/Matrix.h"
#include "Engine/Asset/AssetManager.h"
#include "Engine/Script/ScriptEngine.h"

namespace Engine
{
    EditorLayer::EditorLayer():
	    Layer("EditorLayer"), m_EditorCamera(45.0f, 1600.0f, 900.0f, 0.1f, 1000.0f)
    {
        m_SceneHierarchyPanel.SetSelectionChangedCallback(std::bind(&EditorLayer::SelectEntity, this, std::placeholders::_1));
        m_SceneHierarchyPanel.SetEntityDeletedCallback(std::bind(&EditorLayer::OnEntityDeleted, this, std::placeholders::_1));

        //Load editor icons 
        m_SelectIcon = AssetManager::CreateNewAsset<Texture2D>("resources\\icons\\GizmosTools\\View.png");
        m_MoveIcon = AssetManager::CreateNewAsset<Texture2D>("resources\\icons\\GizmosTools\\Move.png");
        m_RotateIcon = AssetManager::CreateNewAsset<Texture2D>("resources\\icons\\GizmosTools\\Rotate.png");
        m_ScaleIcon = AssetManager::CreateNewAsset<Texture2D>("resources\\icons\\GizmosTools\\Scale.png");
    
        m_PlayIcon = AssetManager::CreateNewAsset<Texture2D>("resources\\icons\\ToolBar\\Play.png");
        m_StopIcon = AssetManager::CreateNewAsset<Texture2D>("resources\\icons\\ToolBar\\Stop.png");
        m_PauseIcon = AssetManager::CreateNewAsset<Texture2D>("resources\\icons\\ToolBar\\Pause.png");
    }

    void EditorLayer::OnAttach()
    {
        //Create new scene. TODO: load a default scene file
        NewScene();

        //TEMP
        
        auto environment = Environment::Create("assets\\environment\\InDoor.hdr");
        m_EditorScene->SetEnvironment(environment);
        {
            auto mesh = AssetManager::CreateNewAsset<Mesh>("assets\\models\\Sphere\\Sphere.fbx");
            auto& meshEntity = m_EditorScene->CreateEntity("Sphere");
            meshEntity.AddComponent<MeshComponent>();
            meshEntity.GetComponent<MeshComponent>().Mesh = mesh;
            meshEntity.GetComponent<TransformComponent>().Translation = glm::vec3(0.0f, 10.0f, 0.0f);
        }
        {
            auto mesh = AssetManager::CreateNewAsset<Mesh>("assets\\models\\Plane\\Plane.fbx");
            auto& meshEntity = m_EditorScene->CreateEntity("Plane");
            meshEntity.AddComponent<MeshComponent>();
            meshEntity.GetComponent<MeshComponent>().Mesh = mesh;
            meshEntity.GetComponent<TransformComponent>().Scale = glm::vec3(5.0f, 5.0f, 5.0f);
        }

        auto& light = m_EditorScene->CreateEntity("Directional Light");
        light.AddComponent<DirectionalLightComponent>();
        light.GetComponent<TransformComponent>().Rotation = glm::radians(glm::vec3(-180.0f, 0.0f, 0.0f));

        auto& camera = m_EditorScene->CreateEntity("Camera");
        camera.AddComponent<CameraComponent>();
        camera.GetComponent<TransformComponent>().Translation = glm::vec3(0.0f, 20.0f, 30.0f);
        camera.GetComponent<TransformComponent>().Rotation = glm::radians(glm::vec3(-20.0f, 0.0f, 0.0f));
    }

    void EditorLayer::OnDetach()
    {
    }

    void EditorLayer::OnUpdate(Timestep ts)
    {         
        switch (m_SceneState)
        {
            case Engine::EditorLayer::SceneState::Edit:
            {
                if (m_ViewportFocused)
                    m_EditorCamera.OnUpdate(ts);
                
                //Render editor scene
                m_EditorScene->OnRenderEditor(ts, m_EditorCamera, m_EditorCamera.GetViewMatrix());

                break;
            }
            case Engine::EditorLayer::SceneState::Play:
            {
                //Update runtime scene
                m_RuntimeScene->OnUpdate(ts);
                m_RuntimeScene->OnRenderRuntime(ts);

                break;
            }
            case Engine::EditorLayer::SceneState::Pause:
            {
                break;
            }
        }
    }

    void EditorLayer::OnEvent(Engine::Event& e)
    {
        if (m_SceneState == SceneState::Edit)
        {
            if (m_ViewportFocused)
                m_EditorCamera.OnEvent(e);

            m_EditorScene->OnEvent(e);
        }
        else if (m_SceneState == SceneState::Play)
        {
            m_RuntimeScene->OnEvent(e);
        }

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

            //ImGui::ShowDemoWindow();

            //Viewport----------------------------------------------------------------------
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
            ImGui::Begin("Viewport");
            {
                m_ViewportFocused = ImGui::IsWindowFocused();
                m_ViewportHovered = ImGui::IsWindowHovered();
                Application::Get().GetImGuiLayer()->BlockEvents(!m_ViewportFocused || !m_ViewportHovered);
                
                //Resize viewport and editor camera
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

                //Gizmos
                if (m_GizmoType != -1 && m_SelectionContext.size())
                {
                    auto& selection = m_SelectionContext[0];

                    float rw = (float)ImGui::GetWindowWidth();
                    float rh = (float)ImGui::GetWindowHeight();

                    ImGuizmo::SetOrthographic(false);
                    ImGuizmo::SetDrawlist();
                    ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, rw, rh);

                    bool snap = Input::IsKeyPressed(ENGINE_KEY_LEFT_CONTROL);

                    auto& entityTransform = selection.Entity.GetTransformComponent();
                    glm::mat4 transform = entityTransform.GetTransform();
                    float snapValue = GetSnapValue();
                    float snapValues[3] = { snapValue, snapValue, snapValue };

                    if (m_SelectionMode == SelectionMode::Entity)
                    {
                        ImGuizmo::Manipulate(glm::value_ptr(m_EditorCamera.GetViewMatrix()),
                            glm::value_ptr(m_EditorCamera.GetProjection()),
                            (ImGuizmo::OPERATION)m_GizmoType,
                            ImGuizmo::LOCAL,
                            glm::value_ptr(transform),
                            nullptr,
                            snap ? snapValues : nullptr);

                        if (ImGuizmo::IsUsing())
                        {
                            glm::vec3 translation, rotation, scale;
                            Math::DecomposeTransform(transform, translation, rotation, scale);

                            glm::vec3 deltaRotation = rotation - entityTransform.Rotation;
                            entityTransform.Translation = translation;
                            entityTransform.Rotation += deltaRotation;
                            entityTransform.Scale = scale;
                        }
                    }
                    else 
                    {
                        glm::mat4 transformBase = transform * selection.Mesh->Transform;
                        ImGuizmo::Manipulate(glm::value_ptr(m_EditorCamera.GetViewMatrix()),
                            glm::value_ptr(m_EditorCamera.GetProjection()),
                            (ImGuizmo::OPERATION)m_GizmoType,
                            ImGuizmo::LOCAL,
                            glm::value_ptr(transformBase),
                            nullptr,
                            snap ? snapValues : nullptr);

                        selection.Mesh->Transform = glm::inverse(transform) * transformBase;
                    }
                }
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

            //UI
            UI_MenuBar();
            UI_ToolBar();
            UI_GizmosToolBar();
            m_ContentBrowserPanel.OnImGuiRender();
            m_MaterialEditorPanel.OnImGuiRender();
            m_SceneHierarchyPanel.OnImGuiRender();
            PhysicsSettingsPanel::OnImGuiRender(m_ShowPhysicsSettings);
            ScriptEngine::OnImGuiRender();
        }
        ImGui::End();
    }

    bool EditorLayer::OnKeyPressedEvent(KeyPressedEvent& e)
    {
        //Gizmos
        //Key Q: None
        //Key W: Translate
        //Key E: Rotate
        //Key R: Scale
        if (GImGui->ActiveId == 0)
        {
            if (m_ViewportFocused && Input::IsKeyPressed(ENGINE_KEY_LEFT_CONTROL) && Input::IsKeyPressed(ENGINE_KEY_LEFT_SHIFT))
            {
                switch (e.GetKeyCode())
                {
                case ENGINE_KEY_Q:
                    m_GizmoType = -1;
                    break;
                case ENGINE_KEY_W:
                    m_GizmoType = ImGuizmo::OPERATION::TRANSLATE;
                    break;
                case ENGINE_KEY_E:
                    m_GizmoType = ImGuizmo::OPERATION::ROTATE;
                    break;
                case ENGINE_KEY_R:
                    m_GizmoType = ImGuizmo::OPERATION::SCALE;
                    break;
                }
            }
        }

        //Scene edit
        //LeftCtrl + N: New
        //LeftCtrl + O: Open
        //LeftCtrl + S: Save
        //LShift + LCtrl + N: Save as
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

            if (Input::IsKeyPressed(ENGINE_KEY_LEFT_SHIFT))
            {
                switch (e.GetKeyCode())
                {
                case ENGINE_KEY_S:
                    SaveSceneAs();
                    break;
                }
            }
        }

        return false;
    }

    bool EditorLayer::OnMouseButtonPressed(MouseButtonPressedEvent& e)
    {
        //Mouse left button: Mouse pick entity or submesh   
        if (e.GetMouseButton() == ENGINE_MOUSE_BUTTON_LEFT && m_ViewportHovered && m_GizmoType == -1 && m_SceneState != SceneState::Play)
        {
            // Mouse Pick: 基于屏幕空间射线碰撞           
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
                            glm::inverse(entity.GetTransformComponent().GetTransform() * submesh.Transform) * glm::vec4(origin, 1.0f),
                            glm::inverse(glm::mat3(entity.GetTransformComponent().GetTransform()) * glm::mat3(submesh.Transform)) * direction
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

                //根据最小距离排序, 选择最近的Entity
                std::sort(m_SelectionContext.begin(), m_SelectionContext.end(), [](auto& a, auto& b) { return a.Distance < b.Distance; });
                if (m_SelectionContext.size())
                    OnEntitySelected(m_SelectionContext[0]);
            }
        }

        return false;
    }

    void EditorLayer::NewScene()
    {
        ClearSelectContext();

        m_EditorScene = CreateRef<Scene>("Untitled Scene", true);
        m_SceneHierarchyPanel.SetContext(m_EditorScene);
        m_SceneFilePath = "";

        m_EditorCamera = EditorCamera(45.0f, 1600.0f, 900.0f, 0.1f, 1000.0f);

        ScriptEngine::SetSceneContext(m_EditorScene);

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
        ClearSelectContext();

        auto newScene = CreateRef<Scene>("Untitled Scene", true);
        SceneSerializer serializer(newScene);
        serializer.Deserialize(filepath);
        m_EditorScene = newScene;
        
        m_SceneHierarchyPanel.SetContext(m_EditorScene);
        ScriptEngine::SetSceneContext(m_EditorScene);

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

        APP_INFO("Save scene '{0}'", m_Name);
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

    void EditorLayer::ClearSelectContext()
    {
        m_SelectionContext.clear();
        
        m_SceneHierarchyPanel.SetSelectedEntity({});
        m_MaterialEditorPanel.SetSelectedEntity({});
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
        if (m_SelectionContext.empty())
            return;

        if (m_SelectionContext[0].Entity == e)
        {
            ClearSelectContext();
            m_EditorScene->SetSelectedEntity({});
        }
    }

    void EditorLayer::OnScenePlay()
    {
        m_SceneState = SceneState::Play;

        //Load runtime scene
        ClearSelectContext();

        if (m_ReloadScriptOnPlay)
            ScriptEngine::ReloadAssembly("assets/scripts/SandBox.dll");

        m_RuntimeScene = CreateRef<Scene>(m_EditorScene->GetName() + "[Running]");
        m_EditorScene->CopeTo(m_RuntimeScene);

        ScriptEngine::SetSceneContext(m_RuntimeScene);        
        m_SceneHierarchyPanel.SetContext(m_RuntimeScene);

        m_RuntimeScene->OnRuntimeStart();

        APP_TRACE("Scene: {0} play", m_EditorScene->GetName());
    }

    void EditorLayer::OnSceneStop()
    {
        m_SceneState = SceneState::Edit;
    
        ScriptEngine::SetSceneContext(m_EditorScene);
        m_SceneHierarchyPanel.SetContext(m_EditorScene);

        ClearSelectContext();

        //Unload runtime scene        
        m_RuntimeScene->OnRuntimeStop();
        m_RuntimeScene = nullptr;

        APP_TRACE("Scene: {0} stop", m_EditorScene->GetName());
    }

    float EditorLayer::GetSnapValue()
    {
        switch (m_GizmoType)
        {
        case  ImGuizmo::OPERATION::TRANSLATE: return 1.0f;
        case  ImGuizmo::OPERATION::ROTATE: return 90.0f;
        case  ImGuizmo::OPERATION::SCALE: return 1.0f;
        }
        return 0.0f;
    }

    void EditorLayer::UI_MenuBar()
    {
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

            if (ImGui::BeginMenu("Edit"))
            {
                ImGui::MenuItem("Physics settings", nullptr, &m_ShowPhysicsSettings);
                
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Shaders"))
            {
                auto& shaders = Renderer::GetShaderLibrary().GetShaders();
                for (auto& shader : shaders)
                {
                    if (ImGui::TreeNode(shader.second->GetName().c_str()))
                    {
                        std::string buttonName = "Reload##" + shader.second->GetName();
                        if (ImGui::Button(buttonName.c_str()))
                            shader.second->Reload();
                        ImGui::TreePop();
                    }
                }
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Script"))
            {
                if (ImGui::MenuItem("Reload C# Assembly"))
                    ScriptEngine::ReloadAssembly("assets/scripts/SandBox.dll");

                ImGui::MenuItem("Reload assembly on play", nullptr, &m_ReloadScriptOnPlay);
                ImGui::EndMenu();
            }

            ImGui::EndMenuBar();
        }
    }

    void EditorLayer::UI_ToolBar()
    {
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 2));
        ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, ImVec2(0, 0));
        auto& colors = ImGui::GetStyle().Colors;
        const auto& buttonHovered = colors[ImGuiCol_ButtonHovered];
        const auto& buttonActive = colors[ImGuiCol_ButtonActive];
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(buttonHovered.x, buttonHovered.y, buttonHovered.z, 0.5f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(buttonActive.x, buttonActive.y, buttonActive.z, 0.5f));

        ImGui::Begin("##Toolbar", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
        float iconSize = ImGui::GetWindowHeight() - 4.0f; 
        auto buttonIcon = m_SceneState == SceneState::Edit ? m_PlayIcon : m_StopIcon;
        ImGui::SetCursorPosX((ImGui::GetWindowContentRegionMax().x * 0.5f) - (iconSize * 0.5f));         
        if (ImGui::ImageButton((ImTextureID)buttonIcon->GetRendererID(), ImVec2(iconSize, iconSize), ImVec2(0, 0), ImVec2(1, 1), 0))
        {
            if (m_SceneState == SceneState::Edit)
            {
                OnScenePlay();
            }
            else if (m_SceneState == SceneState::Play)
            {
                OnSceneStop();
            }
        }
        ImGui::PopStyleVar(2);
        ImGui::PopStyleColor(3);
        ImGui::End();
    }

    void EditorLayer::UI_GizmosToolBar()
    {
        const ImColor c_SelectedGizmoButtonColor = IM_COL32(236, 158, 36, 255);
        const ImColor c_UnselectedGizmoButtonColor = IM_COL32(210, 210, 210, 255);
        ImColor buttonTint;

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
        if(ImGui::Begin("##Gizmos", (bool*)0, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoDocking));
        {
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10, 5));
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("Tools");
            ImGui::PopStyleVar();

            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));

            buttonTint = m_GizmoType == -1 ? c_SelectedGizmoButtonColor : c_UnselectedGizmoButtonColor;
            if (ImGui::ImageButton((ImTextureID)m_SelectIcon->GetRendererID(), { 20, 20 }, { 0, 1 }, { 1, 0 }, -1, ImVec4(0, 0, 0, 0), buttonTint))
                m_GizmoType = -1;
            buttonTint = m_GizmoType == ImGuizmo::OPERATION::TRANSLATE ? c_SelectedGizmoButtonColor : c_UnselectedGizmoButtonColor;
            if (ImGui::ImageButton((ImTextureID)m_MoveIcon->GetRendererID(), { 20, 20 }, { 0, 1 }, { 1, 0 }, -1, ImVec4(0, 0, 0, 0), buttonTint))
                m_GizmoType = ImGuizmo::OPERATION::TRANSLATE;
            buttonTint = m_GizmoType == ImGuizmo::OPERATION::ROTATE ? c_SelectedGizmoButtonColor : c_UnselectedGizmoButtonColor;
            if (ImGui::ImageButton((ImTextureID)m_RotateIcon->GetRendererID(), { 20, 20 }, { 0, 1 }, { 1, 0 }, -1, ImVec4(0, 0, 0, 0), buttonTint))
                m_GizmoType = ImGuizmo::OPERATION::ROTATE;
            buttonTint = m_GizmoType == ImGuizmo::OPERATION::SCALE ? c_SelectedGizmoButtonColor : c_UnselectedGizmoButtonColor;
            if (ImGui::ImageButton((ImTextureID)m_ScaleIcon->GetRendererID(), { 20, 20 }, { 0, 1 }, { 1, 0 }, -1, ImVec4(0, 0, 0, 0), buttonTint))
                m_GizmoType = ImGuizmo::OPERATION::SCALE;

            ImGui::PopStyleColor();

            ImGui::End();
        }
        ImGui::PopStyleVar();
    }

}