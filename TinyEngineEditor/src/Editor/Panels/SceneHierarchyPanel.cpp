#include "SceneHierarchyPanel.h"

#include <vector>

#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>

#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <assimp/scene.h>

#include "Engine/Physics/Physics.h"
#include "Engine/Physics/PhysicsActor.h"
#include "Engine/Physics/PhysicsLayer.h"
#include "Engine/Physics/PXPhysicsWrappers.h"

#include "Engine/Core/Application.h"
#include "Engine/Scene/Component.h"
#include "Engine/ImGui/ImGuiUI.h"

#include "Engine/Renderer/MeshFactory.h"

#include "Engine/Script/ScriptEngine.h"

namespace Engine
{
	SceneHierarchyPanel::SceneHierarchyPanel(const Ref<Scene>& context):
		m_Context(context)
	{
	}

	void SceneHierarchyPanel::SetContext(const Ref<Scene>& context)
	{
		m_Context = context;
		m_SelectionContext = {};
	}

	void SceneHierarchyPanel::OnImGuiRender()
	{
		ImGui::Begin("Scene Hierarchy");
		{
			ImRect windowRect = { ImGui::GetWindowContentRegionMin(), ImGui::GetWindowContentRegionMax() };

			//Scene name
			auto& sceneName = m_Context->GetName();
			char buffer[256];
			memset(buffer, 0, sizeof(buffer));
			strcpy_s(buffer, sizeof(buffer), sceneName.c_str());
			if (ImGui::InputText("Scene Name", buffer, sizeof(buffer)))
			{
				m_Context->SetName(std::string(buffer));
			}
			ImGui::Separator();
			
			for (auto entityID : m_Context->GetAllEntitiesWith<IDComponent, RelationshipComponent>())
			{
				Entity entity{ entityID, m_Context.get() };
				if (entity.GetParentUUID() == 0)
					DrawEntityNode(entity);
			}

			if (ImGui::IsMouseDown(0) && ImGui::IsWindowHovered())
				m_SelectionContext = {};

			if (ImGui::BeginPopupContextWindow(0, 1, false))
			{
				if (ImGui::BeginMenu("Create"))
				{
					if (ImGui::MenuItem("Empty"))
					{
						auto newEntity = m_Context->CreateEntity("Empty Entity");
						SetSelectedEntity(newEntity);
					}
					ImGui::Separator();
					if (ImGui::MenuItem("Mesh"))
					{
						auto newEntity = m_Context->CreateEntity("Mesh");
						newEntity.AddComponent<MeshComponent>();
						SetSelectedEntity(newEntity);
					}
					ImGui::Separator();
					if (ImGui::MenuItem("Camera"))
					{
						auto newEntity = m_Context->CreateEntity("Camera");
						newEntity.AddComponent<CameraComponent>();
						SetSelectedEntity(newEntity);
					}
					ImGui::Separator();
					if (ImGui::BeginMenu("Light"))
					{
						if (ImGui::MenuItem("Directional"))
						{
							auto newEntity = m_Context->CreateEntity("Directional Light");
							newEntity.AddComponent<DirectionalLightComponent>();
							SetSelectedEntity(newEntity);
						}
						ImGui::EndMenu();
					}
					ImGui::EndMenu();
				}
				ImGui::EndPopup();
			}

			//Drag & Drop			
			if (ImGui::BeginDragDropTargetCustom(windowRect, ImGui::GetCurrentWindow()->ID))
			{
				const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("scene_entity_hierarchy", ImGuiDragDropFlags_AcceptNoDrawDefaultRect);
				if (payload)
				{
					size_t count = payload->DataSize / sizeof(UUID);

					for (size_t i = 0; i < count; i++)
					{
						UUID entityID = *(((UUID*)payload->Data) + i);
						Entity entity = m_Context->GetEntityWithUUID(entityID);
						entity.SetParent({});
					}
				}
				ImGui::EndDragDropTarget();
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
						if (ImGui::TreeNode("Source"))
						{
							auto path = m_Context->GetEnvironment().SkyboxMap->GetPath();

							int width1 = 60, width2 = 250, width3 = 40;
							{
								ImGui::PushID(path.c_str());
								ImGui::Columns(3);
								ImGui::SetColumnWidth(0, width1);
								ImGui::SetColumnWidth(1, width2);
								ImGui::SetColumnWidth(2, width3);
								ImGui::Text("Path");
								ImGui::NextColumn();
								ImGui::PushItemWidth(-1);
								ImGui::InputText("##path", (char*)path.c_str(), 256, ImGuiInputTextFlags_ReadOnly);
								if (ImGui::IsItemHovered())
								{
									ImGui::BeginTooltip();
									ImGui::Text((char*)path.c_str());
									ImGui::EndTooltip();
								}

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

		ImGui::Begin("Properties");
		{
			if (m_SelectionContext)
			{
				DrawComponents(m_SelectionContext);
				DrawAddComponentMenu();
			}
		}
		ImGui::End();
	}

	void SceneHierarchyPanel::SetSelectedEntity(Entity entity)
	{
		m_SelectionContext = entity;
	}

	void SceneHierarchyPanel::DrawEntityNode(Entity entity)
	{
		if (entity.HasComponent<SceneComponent>())
			return;

		std::string tag;
		if (entity.HasComponent<TagComponent>())
			tag = entity.GetComponent<TagComponent>().Tag;

		ImGuiTreeNodeFlags flags = (m_SelectionContext == entity ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow;
		flags |= ImGuiTreeNodeFlags_SpanAvailWidth;
		if(entity.GetChildrenUUID().empty())
			flags |= ImGuiTreeNodeFlags_Leaf;
		
		bool opened = ImGui::TreeNodeEx((void*)(uint64_t)(uint32_t)entity, flags, tag.c_str());
		if (ImGui::IsItemClicked())
		{
			m_SelectionContext = entity;
			if (m_SelectionChangedCallback)
				m_SelectionChangedCallback(m_SelectionContext);
		}

		bool entityDeleted = false;
		if (ImGui::BeginPopupContextItem())
		{
			if (ImGui::MenuItem("Delete Entity"))
				entityDeleted = true;

			ImGui::EndPopup();
		}

		// Drag & Drop
		if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
		{
			UUID entityID = entity.ID();
			if ( m_SelectionContext.ID() == entityID)
			{
				ImGui::Text(entity.Tag().c_str());
				ImGui::SetDragDropPayload("scene_entity_hierarchy", &entityID, 1 * sizeof(UUID));
			}
			ImGui::EndDragDropSource();
		}	
		if (ImGui::BeginDragDropTarget())
		{
			const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("scene_entity_hierarchy", ImGuiDragDropFlags_AcceptNoDrawDefaultRect);
			if (payload)
			{
				size_t count = payload->DataSize / sizeof(UUID);

				for (size_t i = 0; i < count; i++)
				{
					auto& children = entity.GetChildrenUUID();
					UUID droppedEntityID = *(((UUID*)payload->Data) + i);
					Entity droppedEntity = m_Context->GetEntityWithUUID(droppedEntityID);
					droppedEntity.SetParent(entity);
				}
			}
			ImGui::EndDragDropTarget();
		}
		

		if (opened)
		{
			for (auto& child : entity.GetChildrenUUID())
			{
				DrawEntityNode(m_Context->GetEntityWithUUID(child));
			}

			ImGui::TreePop();
		}

		if (entityDeleted)
		{
			m_Context->DestroyEntity(entity);
			if (m_SelectionContext == entity)
				m_SelectionContext = {};

			m_EntityDeletedCallback(entity);
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
				if (ImGui::CollapsingHeader("Emission", nullptr, ImGuiTreeNodeFlags_DefaultOpen))
				{
					UI::BeginPropertyGrid();
					UI::PropertyColor("Radiance", dlc.Radiance);
					UI::Property("Intensity", dlc.Intensity, 0.1f, 0.0f, 100.0f);
					UI::EndPropertyGrid();
				}

				if (ImGui::CollapsingHeader("Shadows", nullptr, ImGuiTreeNodeFlags_DefaultOpen))
				{
					ImGui::Columns(2);
					UI::Property("Cast Shadows", dlc.CastShadows);					
					{
						const char* items[] = { "Hard Shadows", "PCF", "PCSS" };
						static int item_current = static_cast<int>(dlc.shadowsType);
						UI::PropertyDropdown("Shadow Type", items, 3, &item_current);
						dlc.shadowsType = static_cast<ShadowsType>(item_current);
					}

					if(dlc.shadowsType == ShadowsType::PCF || dlc.shadowsType == ShadowsType::PCSS)
					{
						const std::vector<uint32_t> values= { 5, 10, 15, 20, 25, 30};
						const char* items[] = { "5", "10", "15", "20", "25", "30"};
						int index = 0;
						for (int i=0;i< values.size();i++)
							if (values[i] == dlc.SamplingRadius)
								index = i;						
						static int item_current = index;
						UI::PropertyDropdown("Sampling Radius", items, 6, &item_current);
						dlc.SamplingRadius = values[item_current];
					}
					ImGui::Columns(1);
				}
			});
		DrawComponent<RigidBodyComponent>("Rigidbody", entity, [](RigidBodyComponent& rc) 
			{
				// Rigidbody Type
				const char* rbTypeStrings[] = { "Static", "Dynamic" };
				const char* currentType = rbTypeStrings[(int)rc.BodyType];
				if (ImGui::BeginCombo("Type", currentType))
				{
					for (int type = 0; type < 2; type++)
					{
						bool is_selected = (currentType == rbTypeStrings[type]);
						if (ImGui::Selectable(rbTypeStrings[type], is_selected))
						{
							currentType = rbTypeStrings[type];
							rc.BodyType = (RigidBodyComponent::Type)type;
						}
						if (is_selected)
							ImGui::SetItemDefaultFocus();
					}
					ImGui::EndCombo();
				}

				// Layer has been removed, set to Default layer
				if (!PhysicsLayerManager::IsLayerValid(rc.Layer))
					rc.Layer = 0;

				uint32_t currentLayer = rc.Layer;
				const PhysicsLayer& layerInfo = PhysicsLayerManager::GetLayer(currentLayer);
				if (ImGui::BeginCombo("Layer", layerInfo.Name.c_str()))
				{
					for (const auto& layer : PhysicsLayerManager::GetLayers())
					{
						bool is_selected = (currentLayer == layer.LayerID);
						if (ImGui::Selectable(layer.Name.c_str(), is_selected))
						{
							currentLayer = layer.LayerID;
							rc.Layer = layer.LayerID;
						}
						if (is_selected)
							ImGui::SetItemDefaultFocus();
					}
					ImGui::EndCombo();
				}

				if (rc.BodyType == RigidBodyComponent::Type::Dynamic)
				{
					UI::BeginPropertyGrid();
					UI::Property("Mass", rc.Mass);
					UI::Property("Linear Drag", rc.LinearDrag);
					UI::Property("Angular Drag", rc.AngularDrag);
					UI::Property("Disable Gravity", rc.DisableGravity);
					UI::Property("Is Kinematic", rc.IsKinematic);
					UI::EndPropertyGrid();

					if (UI::BeginTreeNode("Constraints", false))
					{
						UI::BeginPropertyGrid();

						UI::BeginCheckboxGroup("Freeze Position");
						UI::PropertyCheckboxGroup("X", rc.LockPositionX);
						UI::PropertyCheckboxGroup("Y", rc.LockPositionY);
						UI::PropertyCheckboxGroup("Z", rc.LockPositionZ);
						UI::EndCheckboxGroup();

						UI::BeginCheckboxGroup("Freeze Rotation");
						UI::PropertyCheckboxGroup("X", rc.LockRotationX);
						UI::PropertyCheckboxGroup("Y", rc.LockRotationY);
						UI::PropertyCheckboxGroup("Z", rc.LockRotationZ);
						UI::EndCheckboxGroup();

						UI::EndPropertyGrid();
						UI::EndTreeNode();
					}
				}
			});
		DrawComponent<PhysicsMaterialComponent>("Physics Material", entity, [](PhysicsMaterialComponent& pmc)
			{
				UI::BeginPropertyGrid();

				UI::Property("Static Friction", pmc.StaticFriction);
				UI::Property("Dynamic Friction", pmc.DynamicFriction);
				UI::Property("Bounciness", pmc.Bounciness);

				UI::EndPropertyGrid();
			});
		DrawComponent<BoxColliderComponent>("Box Collider", entity, [](BoxColliderComponent& bcc)
			{
				UI::BeginPropertyGrid();
				if (UI::Property("Size", bcc.Size))
				{
					bcc.DebugMesh = MeshFactory::CreateBox(bcc.Size);
				}
				UI::Property("Is Trigger", bcc.IsTrigger);
				UI::EndPropertyGrid();
			});
		DrawComponent<SphereColliderComponent>("Sphere Collider", entity, [](SphereColliderComponent& scc)
			{
				UI::BeginPropertyGrid();
				if (UI::Property("Radius", scc.Radius))
				{
					scc.DebugMesh = MeshFactory::CreateSphere(scc.Radius);
				}
				UI::Property("Is Trigger", scc.IsTrigger);
				UI::EndPropertyGrid();
			});
		DrawComponent<CapsuleColliderComponent>("Capsule Collider", entity, [=](CapsuleColliderComponent& ccc)
			{
				UI::BeginPropertyGrid();
				bool changed = false;
				if (UI::Property("Radius", ccc.Radius))
					changed = true;
				if (UI::Property("Height", ccc.Height))
					changed = true;
				UI::Property("Is Trigger", ccc.IsTrigger);
				if (changed)
				{
					ccc.DebugMesh = MeshFactory::CreateCapsule(ccc.Radius, ccc.Height);
				}
				UI::EndPropertyGrid();
			});
		DrawComponent<MeshColliderComponent>("Mesh Collider", entity, [&](MeshColliderComponent& mcc)
			{
				if (mcc.OverrideMesh)
				{
					ImGui::Columns(3);
					ImGui::SetColumnWidth(0, 100);
					ImGui::SetColumnWidth(1, 300);
					ImGui::SetColumnWidth(2, 40);
					ImGui::Text("File Path");
					ImGui::NextColumn();
					ImGui::PushItemWidth(-1);
					if (mcc.CollisionMesh)
						ImGui::InputText("##meshfilepath", (char*)mcc.CollisionMesh->GetFilePath().c_str(), 256, ImGuiInputTextFlags_ReadOnly);
					else
						ImGui::InputText("##meshfilepath", (char*)"Null", 256, ImGuiInputTextFlags_ReadOnly);
					ImGui::PopItemWidth();
					ImGui::NextColumn();
					if (ImGui::Button("...##openmesh"))
					{
						std::string file = Application::Get().OpenFile();
						if (!file.empty())
						{
							mcc.CollisionMesh = CreateRef<Mesh>(file);
							if (mcc.IsConvex)
								PXPhysicsWrappers::CreateConvexMesh(mcc, entity.GetTransformComponent().Scale, true);
							else
								PXPhysicsWrappers::CreateTriangleMesh(mcc, entity.GetTransformComponent().Scale, true);
						}
					}

					ImGui::Columns(1);
				}

				UI::BeginPropertyGrid();
				if (UI::Property("Is Convex", mcc.IsConvex))
				{
					if (mcc.IsConvex)
						PXPhysicsWrappers::CreateConvexMesh(mcc, entity.GetTransformComponent().Scale, true);
					else
						PXPhysicsWrappers::CreateTriangleMesh(mcc, entity.GetTransformComponent().Scale, true);
				}
				UI::Property("Is Trigger", mcc.IsTrigger);
				if (UI::Property("Override Mesh", mcc.OverrideMesh))
				{
					if (!mcc.OverrideMesh && entity.HasComponent<MeshComponent>())
					{
						mcc.CollisionMesh = entity.GetComponent<MeshComponent>().Mesh;

						if (mcc.IsConvex)
							PXPhysicsWrappers::CreateConvexMesh(mcc, entity.GetTransformComponent().Scale, true);
						else
							PXPhysicsWrappers::CreateTriangleMesh(mcc, entity.GetTransformComponent().Scale, true);
					}
				}
				UI::EndPropertyGrid();
			});
		DrawComponent<ScriptComponent>("Script", entity, [=](ScriptComponent& sc) mutable
			{
				UI::BeginPropertyGrid();
				std::string oldName = sc.ModuleName;
				if (UI::Property("Module Name", sc.ModuleName, !ScriptEngine::ModuleExists(sc.ModuleName))) // TODO: no live edit
				{
					// Shutdown old script
					if (ScriptEngine::ModuleExists(oldName))
						ScriptEngine::ShutdownScriptEntity(entity, oldName);

					if (ScriptEngine::ModuleExists(sc.ModuleName))
						ScriptEngine::InitScriptEntity(entity);
				}

				// Public Fields
				if (ScriptEngine::ModuleExists(sc.ModuleName))
				{
					EntityInstanceData& entityInstanceData = ScriptEngine::GetEntityInstanceData(entity.GetSceneUUID(), id);
					auto& moduleFieldMap = entityInstanceData.ModuleFieldMap;
					if (moduleFieldMap.find(sc.ModuleName) != moduleFieldMap.end())
					{
						auto& publicFields = moduleFieldMap.at(sc.ModuleName);
						for (auto& [name, field] : publicFields)
						{
							bool isRuntime = m_Context->m_IsPlaying && field.IsRuntimeAvailable();
							switch (field.Type)
							{
							case FieldType::Int:
							{
								int value = isRuntime ? field.GetRuntimeValue<int>() : field.GetStoredValue<int>();
								if (UI::Property(field.Name.c_str(), value))
								{
									if (isRuntime)
										field.SetRuntimeValue(value);
									else
										field.SetStoredValue(value);
								}
								break;
							}
							case FieldType::Float:
							{
								float value = isRuntime ? field.GetRuntimeValue<float>() : field.GetStoredValue<float>();
								if (UI::Property(field.Name.c_str(), value, 0.2f))
								{
									if (isRuntime)
										field.SetRuntimeValue(value);
									else
										field.SetStoredValue(value);
								}
								break;
							}
							case FieldType::Vec2:
							{
								glm::vec2 value = isRuntime ? field.GetRuntimeValue<glm::vec2>() : field.GetStoredValue<glm::vec2>();
								if (UI::Property(field.Name.c_str(), value, 0.2f))
								{
									if (isRuntime)
										field.SetRuntimeValue(value);
									else
										field.SetStoredValue(value);
								}
								break;
							}
							case FieldType::Vec3:
							{
								glm::vec3 value = isRuntime ? field.GetRuntimeValue<glm::vec3>() : field.GetStoredValue<glm::vec3>();
								if (UI::Property(field.Name.c_str(), value, 0.2f))
								{
									if (isRuntime)
										field.SetRuntimeValue(value);
									else
										field.SetStoredValue(value);
								}
								break;
							}
							case FieldType::Vec4:
							{
								glm::vec4 value = isRuntime ? field.GetRuntimeValue<glm::vec4>() : field.GetStoredValue<glm::vec4>();
								if (UI::Property(field.Name.c_str(), value, 0.2f))
								{
									if (isRuntime)
										field.SetRuntimeValue(value);
									else
										field.SetStoredValue(value);
								}
								break;
							}
							}
						}
					}
				}

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
			DrawAddComponentButton<DirectionalLightComponent>("Directional Light");	
			DrawAddComponentButton<RigidBodyComponent>("RigidBody");
			DrawAddComponentButton<PhysicsMaterialComponent>("Physics Material");
			DrawAddComponentButton<BoxColliderComponent>("Box Collider");
			DrawAddComponentButton<SphereColliderComponent>("Sphere Collider");
			DrawAddComponentButton<CapsuleColliderComponent>("Capsule Collider");
			DrawAddComponentButton<ScriptComponent>("Script");
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
			if (ImGui::Selectable(name.c_str()))
			{
				m_SelectionContext.AddComponent<T>();
				ImGui::CloseCurrentPopup();
			}
		}
	}
}