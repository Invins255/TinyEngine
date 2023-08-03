#include "PhysicsSettingsPanel.h"

#include <string>

#include "Engine/Physics/Physics.h"
#include "Engine/Physics/PhysicsLayer.h"

#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>
#include "Engine/ImGui/ImGuiUI.h"

#include <glm/gtc/type_ptr.hpp>


namespace Engine
{
	static int s_SelectedLayer = -1;
	static char s_NewLayerNameBuffer[50];

	void PhysicsSettingsPanel::OnImGuiRender(bool& show)
	{
		if (!show)
			return;

		ImGui::Begin("Physics", &show);

		ImGui::PushID(0);
		ImGui::Columns(2);
		RenderWorldSettings();
		ImGui::EndColumns();
		ImGui::PopID();

		ImGui::Separator();

		RenderLayerList();
		RenderLayerCollisionMatrix();

		ImGui::End();
	}

	void PhysicsSettingsPanel::RenderWorldSettings()
	{
		//Physics scene settings
		PhysicsSettings& settings = Physics::GetSettings();

		UI::Property("Fixed Timestep (Default: 0.02)", settings.FixedTimestep);
		UI::Property("Gravity (Default: -9.81)", settings.Gravity.y);

		static const char* broadphaseTypeStrings[] = { "Sweep And Prune", "Multi Box Pruning", "Automatic Box Pruning" };
		UI::PropertyDropdown("Broadphase Type", broadphaseTypeStrings, 3, (int*)&settings.BroadphaseAlgorithm);
		if (settings.BroadphaseAlgorithm != BroadphaseType::AutomaticBoxPrune)
		{
			UI::Property("World Bounds (Min)", settings.WorldBoundsMin);
			UI::Property("World Bounds (Max)", settings.WorldBoundsMax);
			UI::PropertySlider("Grid Subdivisions", (int&)settings.WorldBoundsSubdivisions, 1, 10000);
		}

		static const char* frictionTypeStrings[] = { "Patch", "One Directional", "Two Directional" };
		UI::PropertyDropdown("Friction Model", frictionTypeStrings, 3, (int*)&settings.FrictionModel);

		UI::PropertySlider("Solver Iterations", (int&)settings.SolverIterations, 1, 512);
		UI::PropertySlider("Solver Velocity Iterations", (int&)settings.SolverVelocityIterations, 1, 512);
	}

	void PhysicsSettingsPanel::RenderLayerList()
	{
		//Create new layer
		if(ImGui::TreeNode("Layers"))
		{
			//Show layers list
			const auto& layers = PhysicsLayerManager::GetLayers();

			for (int i = 0; i < layers.size(); i++)
			{
				const auto& layer = layers[i];
				if (ImGui::Selectable(layer.Name.c_str(), s_SelectedLayer == i))
				{
					s_SelectedLayer = i;
				}
			}

			if (ImGui::Button("New"))
			{
				ImGui::OpenPopup("NewLayerNamePopup");
			}
			if (ImGui::BeginPopup("NewLayerNamePopup"))
			{
				ImGui::InputText("New Layer Name", s_NewLayerNameBuffer, 50);
				if (ImGui::Button("Add"))
				{
					PhysicsLayerManager::AddLayer(std::string(s_NewLayerNameBuffer));
					memset(s_NewLayerNameBuffer, 0, 50);
					ImGui::CloseCurrentPopup();
				}
				ImGui::EndPopup();
			}

			ImGui::SameLine();

			if (ImGui::Button("Delete"))
			{
				if (s_SelectedLayer != -1)
				{
					const auto& layer = layers[s_SelectedLayer];
					if (layer.Name != "Default")
					{
						PhysicsLayerManager::RemoveLayer(layer.LayerID);
					}
				}
			}

			ImGui::TreePop();
		}
	}

	void PhysicsSettingsPanel::RenderLayerCollisionMatrix()
	{
		if (ImGui::TreeNode("Layer Collision Matrix"))
		{
			auto& layers = PhysicsLayerManager::GetLayers();
			int size = layers.size() + 1;

			if (ImGui::BeginTable("Matrix", size))
			{
				for (int row = 0; row < size; row++)
				{
					ImGui::TableNextRow();
					for (int col = 0; col < size; col++)
					{
						if (row == 0 && col == 0)
							continue;
						if (row >= 1 && col == 0)
						{
							ImGui::TableSetColumnIndex(col);

							int idx = row - 1;
							const auto& layerInfo = layers[idx];
							ImGui::Text(layerInfo.Name.c_str());
						}
						if (row == 0 && col >= 1)
						{
							ImGui::TableSetColumnIndex(col);

							int idx = (layers.size() - 1) - (col - 1);
							const auto& layerInfo = layers[idx];
							ImGui::Text(layerInfo.Name.c_str());
						}
						if (row >= 1 && col >= 1)
						{
							if (col > size - row)
								continue;

							ImGui::TableSetColumnIndex(col);

							int rowIdx = row - 1;
							int colIdx = (layers.size() - 1) - (col - 1);
							const auto& layerInfo = layers[rowIdx];
							const auto& otherLayerInfo = layers[colIdx];

							bool shouldCollide;
							if (layerInfo.CollidesWith == 0 || otherLayerInfo.CollidesWith == 0)
								shouldCollide = false;
							else
								shouldCollide = layerInfo.CollidesWith & otherLayerInfo.BitValue;

							if (ImGui::Checkbox(("##" + layerInfo.Name + "-" + otherLayerInfo.Name).c_str(), &shouldCollide))
							{
								PhysicsLayerManager::SetLayerCollision(layerInfo.LayerID, otherLayerInfo.LayerID, shouldCollide);
							}
						}
					}
				}

				ImGui::EndTable();
			}
			ImGui::TreePop();
		}
	}
}