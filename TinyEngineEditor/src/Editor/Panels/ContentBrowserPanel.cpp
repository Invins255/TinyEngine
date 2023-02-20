#include "ContentBrowserPanel.h"

#include <imgui/imgui.h>

namespace Engine
{
    const std::filesystem::path ContentBrowserPanel::m_DefaultAssetPath = "assets";

    ContentBrowserPanel::ContentBrowserPanel()
        :m_CurrentDirectory(m_DefaultAssetPath)
    {
        m_DirectoryIcon = Texture2D::Create("resources\\icons\\ContentBrowser\\Directory.png");
        m_FileIcon = Texture2D::Create("resources\\icons\\ContentBrowser\\File.png");
    }

    void ContentBrowserPanel::OnImGuiRender()
    {
        ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoScrollbar;
        ImGui::Begin("Content Browser",(bool*)0, windowFlags);

        ImGuiTableFlags flags =
            ImGuiTableFlags_SizingFixedFit
            | ImGuiTableFlags_Borders
            | ImGuiTableFlags_Resizable
            | ImGuiTableFlags_Reorderable
            | ImGuiTableFlags_Hideable
            | ImGuiTableFlags_NoHostExtendX;

        if(ImGui::BeginTable("table", 2, flags))
        {
            ImGui::TableSetupColumn("column0", ImGuiTableColumnFlags_WidthFixed, 200.0f);
            ImGui::TableSetupColumn("column1", ImGuiTableColumnFlags_WidthStretch);

            ImGui::TableNextRow();
            //Column0
            ImGui::TableSetColumnIndex(0);
            ImGuiWindowFlags window_flags = ImGuiWindowFlags_None;
            if (ImGui::BeginChild("ChildL"))
            {
                if (ImGui::TreeNode(m_DefaultAssetPath.string().c_str()))
                {
                    TraverseDirectoryTree(m_DefaultAssetPath);
                    ImGui::TreePop();
                }
                ImGui::EndChild();
            }

            //Column1
            ImGui::TableSetColumnIndex(1);
            if (ImGui::BeginChild("ChildR",ImVec2(0, 0), false, ImGuiWindowFlags_MenuBar))
            {
                if (ImGui::BeginMenuBar())
                {
                    ImGui::Text(m_CurrentDirectory.string().c_str());
                    ImGui::EndMenuBar();
                }

                static float padding = 25.0f;
                static float thumbnailSize = 64.0f;
                float cellSize = thumbnailSize + padding;
                float panelWidth = ImGui::GetContentRegionAvail().x;
                int columnCount = (int)(panelWidth / cellSize);
                if (columnCount < 1)
                    columnCount = 1;
                ImGui::Columns(columnCount, 0, false);

                for (auto& directoryEntry : std::filesystem::directory_iterator(m_CurrentDirectory))
                {
                    const auto& path = directoryEntry.path();
                    auto relativePath = std::filesystem::relative(path, m_DefaultAssetPath);
                    std::string fileStringStr = relativePath.filename().string();
                    std::string stemStr = relativePath.stem().string();

                    Ref<Texture2D>& icon = directoryEntry.is_directory() ? m_DirectoryIcon : m_FileIcon;
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
                    ImGui::ImageButton((ImTextureID)icon->GetRendererID(), { thumbnailSize, thumbnailSize }, { 0, 1 }, { 1, 0 });
                    ImGui::PopStyleColor();

                    if (ImGui::IsItemHovered())
                    {
                        ImGui::SetTooltip(fileStringStr.c_str());
                    }

                    ImGui::TextWrapped(stemStr.c_str());

                    ImGui::NextColumn();
                }
                ImGui::Columns(1);

                ImGui::EndChild();
            }
            ImGui::EndTable();
        }
        ImGui::End();
    }

    void ContentBrowserPanel::TraverseDirectoryTree(std::filesystem::path rootPath)
    {
        m_CurrentDirectory = rootPath;
        for (auto& directory : std::filesystem::directory_iterator(rootPath))
        {
            const auto& path = directory.path();
            auto relativePath = std::filesystem::relative(path, m_DefaultAssetPath);
            std::string filenameString = relativePath.filename().string();

            if (directory.is_directory())
            {
                if (ImGui::TreeNode(filenameString.c_str()))
                {
                    TraverseDirectoryTree(directory);
                    ImGui::TreePop();
                }
            }
            else
            {
                if (ImGui::TreeNodeEx(filenameString.c_str(), ImGuiTreeNodeFlags_Leaf))
                {
                    ImGui::TreePop();
                }
            }
        }
    }
}