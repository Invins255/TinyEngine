#include "MaterialEditorPanel.h"

#include <imgui/imgui.h>

#include "Engine/Scene/Component.h"
#include "Engine/Core/Application.h"

namespace Engine
{
	MaterialEditorPanel::MaterialEditorPanel()
	{
		m_CheckerboardTex = Texture2D::Create("resources\\textures\\Checkerboard.tga");
	}

	void MaterialEditorPanel::OnImGuiRender()
	{
		ImGui::Begin("Materials");
		{
			if (m_SelectedEntity)
			{
				if (m_SelectedEntity.HasComponent<MeshComponent>())	
				{
					auto mesh = m_SelectedEntity.GetComponent<MeshComponent>().Mesh;
					if (mesh)
					{
						auto& materials = mesh->GetMaterials();
						uint32_t selectedMaterialIndex = 0;
						for (uint32_t i = 0; i < materials.size(); i++)
						{
							auto& materialInstance = materials[i];

							ImGuiTreeNodeFlags node_flags = (selectedMaterialIndex == i ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_Leaf;
							bool opened = ImGui::TreeNodeEx((void*)(&materialInstance), node_flags, materialInstance->GetName().c_str());
							if (ImGui::IsItemClicked())
							{
								selectedMaterialIndex = i;
							}
							if (opened)
								ImGui::TreePop();
						}

						// Selected material
						if (selectedMaterialIndex < materials.size())
						{
							auto& materialInstance = materials[selectedMaterialIndex];
							ImGui::Text("Shader: %s", materialInstance->GetShader()->GetName().c_str());

							ImGui::Separator();

							ImVec2 textureSize(64, 64);
							float sizeMutiplier = 4.0f;

							//Albedo
							if (ImGui::CollapsingHeader("Albedo", nullptr, ImGuiTreeNodeFlags_DefaultOpen))
							{
								ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10, 10));
								auto& albedoColor = materialInstance->Get<glm::vec3>("u_AlbedoColor");
								bool useAlbedoMap = materialInstance->Get<float>("u_AlbedoTexToggle");
								auto albedoMap = materialInstance->TryGetResource<Texture2D>("u_AlbedoTexture");

								ImGui::Image(albedoMap ? (void*)albedoMap->GetRendererID() : (void*)m_CheckerboardTex->GetRendererID(), textureSize, { 0, 1 }, { 1, 0 });
								ImGui::PopStyleVar();

								if (ImGui::IsItemHovered())
								{
									if (albedoMap)
									{
										ImGui::BeginTooltip();
										ImGui::Image((void*)albedoMap->GetRendererID(), { textureSize.x * sizeMutiplier, textureSize.y * sizeMutiplier }, { 0, 1 }, { 1, 0 });
										ImGui::EndTooltip();
									}
									if (ImGui::IsItemClicked())
									{
										std::string filename = Application::Get().OpenFile("");
										if (filename != "")
										{
											albedoMap = Texture2D::Create(filename, true/*m_AlbedoInput.SRGB*/);
											materialInstance->Set("u_AlbedoTexture", albedoMap);
										}
									}
								}
								ImGui::SameLine();
								ImGui::BeginGroup();
								if (ImGui::Checkbox("Use##AlbedoMap", &useAlbedoMap))
									materialInstance->Set<float>("u_AlbedoTexToggle", useAlbedoMap ? 1.0f : 0.0f);
								ImGui::EndGroup();
								ImGui::SameLine();
								ImGui::ColorEdit3("Color##Albedo", glm::value_ptr(albedoColor), ImGuiColorEditFlags_NoInputs);
							}

							//Normals
							if (ImGui::CollapsingHeader("Normals", nullptr, ImGuiTreeNodeFlags_DefaultOpen))
							{
								ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10, 10));
								bool useNormalMap = materialInstance->Get<float>("u_NormalTexToggle");
								auto normalMap = materialInstance->TryGetResource<Texture2D>("u_NormalTexture");
								ImGui::Image(normalMap ? (void*)normalMap->GetRendererID() : (void*)m_CheckerboardTex->GetRendererID(), textureSize, { 0, 1 }, { 1, 0 });
								ImGui::PopStyleVar();

								if (ImGui::IsItemHovered())
								{
									if (normalMap)
									{
										ImGui::BeginTooltip();;
										ImGui::Image((void*)normalMap->GetRendererID(), { textureSize.x * sizeMutiplier, textureSize.y * sizeMutiplier }, { 0, 1 }, { 1, 0 });
										ImGui::EndTooltip();
									}
									if (ImGui::IsItemClicked())
									{
										std::string filename = Application::Get().OpenFile("");
										if (filename != "")
										{
											normalMap = Texture2D::Create(filename);
											materialInstance->Set("u_NormalTexture", normalMap);
										}
									}
								}
								ImGui::SameLine();
								if (ImGui::Checkbox("Use##NormalMap", &useNormalMap))
									materialInstance->Set<float>("u_NormalTexToggle", useNormalMap ? 1.0f : 0.0f);
							}

							// Metalness
							if (ImGui::CollapsingHeader("Metalness", nullptr, ImGuiTreeNodeFlags_DefaultOpen))
							{
								ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10, 10));
								float& metalnessValue = materialInstance->Get<float>("u_Metalness");
								bool useMetalnessMap = materialInstance->Get<float>("u_MetalnessTexToggle");
								Ref<Texture2D> metalnessMap = materialInstance->TryGetResource<Texture2D>("u_MetalnessTexture");
								ImGui::Image(metalnessMap ? (void*)metalnessMap->GetRendererID() : (void*)m_CheckerboardTex->GetRendererID(), textureSize, { 0, 1 }, { 1, 0 });
								ImGui::PopStyleVar();
								if (ImGui::IsItemHovered())
								{
									if (metalnessMap)
									{
										ImGui::BeginTooltip();
										ImGui::Image((void*)metalnessMap->GetRendererID(), { textureSize.x * sizeMutiplier, textureSize.y * sizeMutiplier }, { 0, 1 }, { 1, 0 });
										ImGui::EndTooltip();
									}
									if (ImGui::IsItemClicked())
									{
										std::string filename = Application::Get().OpenFile("");
										if (filename != "")
										{
											metalnessMap = Texture2D::Create(filename);
											materialInstance->Set("u_MetalnessTexture", metalnessMap);
										}
									}
								}
								ImGui::SameLine();
								if (ImGui::Checkbox("Use##MetalnessMap", &useMetalnessMap))
									materialInstance->Set<float>("u_MetalnessTexToggle", useMetalnessMap ? 1.0f : 0.0f);
								ImGui::SameLine();
								ImGui::SliderFloat("Value##MetalnessInput", &metalnessValue, 0.0f, 1.0f);
							}

							// Roughness
							if (ImGui::CollapsingHeader("Roughness", nullptr, ImGuiTreeNodeFlags_DefaultOpen))
							{
								ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10, 10));
								float& roughnessValue = materialInstance->Get<float>("u_Roughness");
								bool useRoughnessMap = materialInstance->Get<float>("u_RoughnessTexToggle");
								Ref<Texture2D> roughnessMap = materialInstance->TryGetResource<Texture2D>("u_RoughnessTexture");
								ImGui::Image(roughnessMap ? (void*)roughnessMap->GetRendererID() : (void*)m_CheckerboardTex->GetRendererID(), textureSize, { 0, 1 }, { 1, 0 });
								ImGui::PopStyleVar();
								if (ImGui::IsItemHovered())
								{
									if (roughnessMap)
									{
										ImGui::BeginTooltip();
										ImGui::Image((void*)roughnessMap->GetRendererID(), { textureSize.x * sizeMutiplier, textureSize.y * sizeMutiplier }, { 0, 1 }, { 1, 0 });
										ImGui::EndTooltip();
									}
									if (ImGui::IsItemClicked())
									{
										std::string filename = Application::Get().OpenFile("");
										if (filename != "")
										{
											roughnessMap = Texture2D::Create(filename);
											materialInstance->Set("u_RoughnessTexture", roughnessMap);
										}
									}
								}
								ImGui::SameLine();
								if (ImGui::Checkbox("Use##RoughnessMap", &useRoughnessMap))
									materialInstance->Set<float>("u_RoughnessTexToggle", useRoughnessMap ? 1.0f : 0.0f);
								ImGui::SameLine();
								ImGui::SliderFloat("Value##RoughnessInput", &roughnessValue, 0.0f, 1.0f);
							}
						}
					}
				}
			}
		}
		ImGui::End();
	}
}