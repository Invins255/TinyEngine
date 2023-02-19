#include "SceneHierarchyPanel.h"

#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>

#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <assimp/scene.h>

#include "Engine/Core/Application.h"
#include "Engine/Scene/Component.h"
#include "Engine/ImGui/ImGuiUI.h"

namespace Engine
{
	SceneHierarchyPanel::SceneHierarchyPanel(const Ref<Scene>& context):
		m_Context(context)
	{
	}

	void SceneHierarchyPanel::SetContext(const Ref<Scene>& context)
	{
		m_Context = context;
	}

	void SceneHierarchyPanel::OnImGuiRender()
	{
		ImGui::Begin("Scene Hierarchy");
		{
			auto& sceneName = m_Context->GetName();
			char buffer[256];
			memset(buffer, 0, sizeof(buffer));
			strcpy_s(buffer, sizeof(buffer), sceneName.c_str());
			if (ImGui::InputText("Scene Name", buffer, sizeof(buffer)))
			{
				m_Context->SetName(std::string(buffer));
			}
			ImGui::Separator();

			m_Context->m_Registry.each([&](auto entityID)
				{
					Entity entity{ entityID, m_Context.get() };
					DrawEntityNode(entity);
				}
			);
			if (ImGui::IsMouseDown(0) && ImGui::IsWindowHovered())
				m_SelectionContext = {};

			if (ImGui::BeginPopupContextWindow(0, 1, false))
			{
				if (ImGui::MenuItem("Create Empty Entity"))
					m_Context->CreateEntity("Empty Entity");

				ImGui::EndPopup();
			}
		}
		ImGui::End();

		ImGui::Begin("Properties");
		{
			if (m_SelectionContext)
			{
				DrawComponents(m_SelectionContext);
				DrawAddComponentMenu();
			}
		}
		ImGui::End();

		ImGui::Begin("Lighting");
		{
			ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_None;
			if (ImGui::BeginTabBar("MyTabBar", tab_bar_flags))
			{
				if (ImGui::BeginTabItem("Environment"))
				{
					const ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_FramePadding;
					ImVec2 contentRegionAvailable = ImGui::GetContentRegionAvail();

					ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{ 4, 4 });
					float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
					ImGui::Separator();
					if (ImGui::TreeNodeEx("##dummy_id", treeNodeFlags, "Environment"))
					{
						//Environment settings
						if(ImGui::TreeNode("Source"))
						{
							auto path = m_Context->GetEnvironment().SkyboxMap->GetPath();
							
							int width1 = 60, width2 = 250, width3 = 40;
							{
								//Left
								ImGui::PushID(path[1].c_str());
								ImGui::Columns(3);
								ImGui::SetColumnWidth(0, width1);
								ImGui::SetColumnWidth(1, width2);
								ImGui::SetColumnWidth(2, width3);
								ImGui::Text("Left");
								ImGui::NextColumn();
								ImGui::PushItemWidth(-1);
								ImGui::InputText("##left", (char*)path[1].c_str(), 256, ImGuiInputTextFlags_ReadOnly);
								ImGui::PopItemWidth();
								ImGui::NextColumn();
								if (ImGui::Button("...##openmesh"))
								{
									//Get face path
								}
								ImGui::PopID();
								ImGui::NextColumn();
								//Right
								ImGui::PushID(path[0].c_str());
								ImGui::Columns(3);
								ImGui::SetColumnWidth(0, width1);
								ImGui::SetColumnWidth(1, width2);
								ImGui::SetColumnWidth(2, width3);
								ImGui::Text("Right");
								ImGui::NextColumn();
								ImGui::PushItemWidth(-1);
								ImGui::InputText("##right", (char*)path[0].c_str(), 256, ImGuiInputTextFlags_ReadOnly);
								ImGui::PopItemWidth();
								ImGui::NextColumn();
								if (ImGui::Button("...##openmesh"))
								{
									//Get face path
								}
								ImGui::PopID();
								ImGui::NextColumn();
								//Top
								ImGui::PushID(path[2].c_str());
								ImGui::Columns(3);
								ImGui::SetColumnWidth(0, width1);
								ImGui::SetColumnWidth(1, width2);
								ImGui::SetColumnWidth(2, width3);
								ImGui::Text("Top");
								ImGui::NextColumn();
								ImGui::PushItemWidth(-1);
								ImGui::InputText("##top", (char*)path[2].c_str(), 256, ImGuiInputTextFlags_ReadOnly);
								ImGui::PopItemWidth();
								ImGui::NextColumn();
								if (ImGui::Button("...##openmesh"))
								{
									//Get face path
								}
								ImGui::PopID();
								ImGui::NextColumn();
								//Bottom
								ImGui::PushID(path[3].c_str());
								ImGui::Columns(3);
								ImGui::SetColumnWidth(0, width1);
								ImGui::SetColumnWidth(1, width2);
								ImGui::SetColumnWidth(2, width3);
								ImGui::Text("Bottom");
								ImGui::NextColumn();
								ImGui::PushItemWidth(-1);
								ImGui::InputText("##bottom", (char*)path[3].c_str(), 256, ImGuiInputTextFlags_ReadOnly);
								ImGui::PopItemWidth();
								ImGui::NextColumn();
								if (ImGui::Button("...##openmesh"))
								{
									//Get face path
								}
								ImGui::PopID();
								ImGui::NextColumn();
								//Front
								ImGui::PushID(path[4].c_str());
								ImGui::Columns(3);
								ImGui::SetColumnWidth(0, width1);
								ImGui::SetColumnWidth(1, width2);
								ImGui::SetColumnWidth(2, width3);
								ImGui::Text("Front");
								ImGui::NextColumn();
								ImGui::PushItemWidth(-1);
								ImGui::InputText("##front", (char*)path[4].c_str(), 256, ImGuiInputTextFlags_ReadOnly);
								ImGui::PopItemWidth();
								ImGui::NextColumn();
								if (ImGui::Button("...##openmesh"))
								{
									//Get face path
								}
								ImGui::PopID();
								ImGui::NextColumn();
								//Back
								ImGui::PushID(path[5].c_str());
								ImGui::Columns(3);
								ImGui::SetColumnWidth(0, width1);
								ImGui::SetColumnWidth(1, width2);
								ImGui::SetColumnWidth(2, width3);
								ImGui::Text("Back");
								ImGui::NextColumn();
								ImGui::PushItemWidth(-1);
								ImGui::InputText("##back", (char*)path[5].c_str(), 256, ImGuiInputTextFlags_ReadOnly);
								ImGui::PopItemWidth();
								ImGui::NextColumn();
								if (ImGui::Button("...##openmesh"))
								{
									//Get face path
								}
								ImGui::PopID();
								ImGui::NextColumn();
							}

							ImGui::TreePop();
						}

						ImGui::TreePop();
					}
					ImGui::PopStyleVar();

					ImGui::EndTabItem();
				}
				ImGui::EndTabBar();
			}
		}
		ImGui::End();
	}

	void SceneHierarchyPanel::DrawEntityNode(Entity entity)
	{
		auto& tag = entity.GetComponent<TagComponent>().Tag;

		ImGuiTreeNodeFlags flags = (m_SelectionContext == entity ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow;
		bool opened = ImGui::TreeNodeEx((void*)(uint64_t)(uint32_t)entity, flags, tag.c_str());
		if (ImGui::IsItemClicked())
		{
			m_SelectionContext = entity;
		}

		bool entityDeleted = false;
		if (ImGui::BeginPopupContextItem())
		{
			if (ImGui::MenuItem("Delete Entity"))
				entityDeleted = true;

			ImGui::EndPopup();
		}

		if (opened)
		{
			ImGui::TreePop();
		}

		if (entityDeleted)
		{
			m_Context->DestroyEntity(entity);
			if (m_SelectionContext == entity)
				m_SelectionContext = {};
		}
	}

	void SceneHierarchyPanel::DrawComponents(Entity entity)
	{
		ImGui::AlignTextToFramePadding();
		ImVec2 contentRegionAvailable = ImGui::GetContentRegionAvail();

		auto id = entity.GetComponent<IDComponent>().ID;
		ImGui::Text("UID: ");
		ImGui::SameLine();
		ImGui::TextDisabled("%llx", id);
		ImGui::Separator();

		if (entity.HasComponent<TagComponent>())
		{
			auto& tag = entity.GetComponent<TagComponent>().Tag;
			char buffer[256];
			memset(buffer, 0, sizeof(buffer));
			strcpy_s(buffer, sizeof(buffer), tag.c_str());
			ImGui::PushItemWidth(contentRegionAvailable.x * 0.5f);
			if (ImGui::InputText("##Tag", buffer, sizeof(buffer)))
			{
				tag = std::string(buffer);
			}
			ImGui::PopItemWidth();
		}

		//Draw component
		DrawComponent<TransformComponent>("Transform", entity, [](TransformComponent& component)
			{
				UI::PropertyVector3("Position", component.Translation);
				glm::vec3 rotation = glm::degrees(component.Rotation);
				UI::PropertyVector3("Rotation", rotation);
				component.Rotation = glm::radians(rotation);
				UI::PropertyVector3("Scale", component.Scale, 1.0f);
			});
		DrawComponent<MeshComponent>("Mesh", entity, [](MeshComponent& mc)
			{
				ImGui::Columns(3);
				ImGui::SetColumnWidth(0, 80);
				ImGui::SetColumnWidth(1, 240);
				ImGui::SetColumnWidth(2, 40);
				ImGui::Text("Path");
				ImGui::NextColumn();
				ImGui::PushItemWidth(-1);
				if (mc.Mesh)
					ImGui::InputText("##meshfilepath", (char*)mc.Mesh->GetFilePath().c_str(), 256, ImGuiInputTextFlags_ReadOnly);
				else
					ImGui::InputText("##meshfilepath", (char*)"Null", 256, ImGuiInputTextFlags_ReadOnly);
				ImGui::PopItemWidth();		
				ImGui::NextColumn();
				if (ImGui::Button("...##openmesh"))
				{
					std::string file = Application::Get().OpenFile();
					if (!file.empty())
						mc.Mesh = CreateRef<Mesh>(file);
				}
				ImGui::Columns(1);
			});
		DrawComponent<CameraComponent>("Camera", entity, [](CameraComponent& cc)
			{
				// Projection Type
				const char* projTypeStrings[] = { "Perspective", "Orthographic" };
				const char* currentProj = projTypeStrings[(int)cc.Camera.GetProjectionType()];
				if (ImGui::BeginCombo("Projection", currentProj))
				{
					for (int type = 0; type < 2; type++)
					{
						bool is_selected = (currentProj == projTypeStrings[type]);
						if (ImGui::Selectable(projTypeStrings[type], is_selected))
						{
							currentProj = projTypeStrings[type];
							cc.Camera.SetProjectionType((SceneCamera::ProjectionType)type);
						}
						if (is_selected)
							ImGui::SetItemDefaultFocus();
					}
					ImGui::EndCombo();
				}

				UI::BeginPropertyGrid();
				// Perspective parameters
				if (cc.Camera.GetProjectionType() == SceneCamera::ProjectionType::Perspective)
				{
					float FOV = cc.Camera.GetDegPerspectiveFOV();
					if (UI::Property("FOV", FOV))
						cc.Camera.SetDegPerspectiveFOV(FOV);

					float nearClip = cc.Camera.GetPerspectiveNearClip();
					if (UI::Property("Near Clip", nearClip))
						cc.Camera.SetPerspectiveNearClip(nearClip);
					
					float farClip = cc.Camera.GetPerspectiveFarClip();
					if (UI::Property("Far Clip", farClip))
						cc.Camera.SetPerspectiveFarClip(farClip);
				}

				// Orthographic parameters
				else if (cc.Camera.GetProjectionType() == SceneCamera::ProjectionType::Orthographic)
				{
					float orthoSize = cc.Camera.GetOrthographicSize();
					if (UI::Property("Size", orthoSize))
						cc.Camera.SetOrthographicSize(orthoSize);

					float nearClip = cc.Camera.GetOrthographicNearClip();
					if (UI::Property("Near Clip", nearClip))
						cc.Camera.SetOrthographicNearClip(nearClip);
					
					float farClip = cc.Camera.GetOrthographicFarClip();
					if (UI::Property("Far Clip", farClip))
						cc.Camera.SetOrthographicFarClip(farClip);
				}

				UI::EndPropertyGrid();
			});
		DrawComponent<DirectionalLightComponent>("Directional Light", entity, [](DirectionalLightComponent& dlc)
			{
				UI::BeginPropertyGrid();
				UI::PropertyColor("Radiance", dlc.Radiance);
				UI::Property("Intensity", dlc.Intensity);
				UI::Property("Cast Shadows", dlc.CastShadows);
				UI::EndPropertyGrid();
			});
	}

	void SceneHierarchyPanel::DrawAddComponentMenu()
	{
		if (ImGui::Button("Add Component"))
			ImGui::OpenPopup("AddComponentPanel");

		if (ImGui::BeginPopup("AddComponentPanel"))
		{
			DrawAddComponentButton<CameraComponent>("Camera");
			DrawAddComponentButton<MeshComponent>("Mesh");
			DrawAddComponentButton<DirectionalLightComponent>("DirectionalLight");		
			ImGui::EndPopup();
		}
	}

	template<typename T, typename UIFunction>
	void SceneHierarchyPanel::DrawComponent(const std::string& name, Entity entity, UIFunction uiFunction)
	{
		const ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_FramePadding;
		if (entity.HasComponent<T>())
		{
			ImGui::PushID((void*)typeid(T).hash_code());
			auto& component = entity.GetComponent<T>();
			ImVec2 contentRegionAvailable = ImGui::GetContentRegionAvail();

			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{ 4, 4 });
			float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
			ImGui::Separator();
			bool open = ImGui::TreeNodeEx("##dummy_id", treeNodeFlags, name.c_str());
			ImGui::PopStyleVar();
			ImGui::SameLine(contentRegionAvailable.x - lineHeight * 0.5f);
			if (ImGui::Button("+", ImVec2{ lineHeight, lineHeight }))
			{
				ImGui::OpenPopup("ComponentSettings");
			}

			bool removeComponent = false;
			if (ImGui::BeginPopup("ComponentSettings"))
			{
				if (ImGui::MenuItem("Remove component"))
					removeComponent = true;

				ImGui::EndPopup();
			}

			if (open)
			{
				uiFunction(component);
				ImGui::TreePop();
			}

			if (removeComponent)
				entity.RemoveComponent<T>();

			ImGui::PopID();
		}
	}

	template<typename T>
	void SceneHierarchyPanel::DrawAddComponentButton(const std::string& name)
	{
		if (!m_SelectionContext.HasComponent<T>())	
		{
			if (ImGui::Button(name.c_str()))
			{
				m_SelectionContext.AddComponent<T>();
				ImGui::CloseCurrentPopup();
			}
		}
	}
}