//
// Created by dingrui on 3/8/26.
//

#include "content_browser_panel.h"

#include <imgui.h>

#include <x/renderer/texture.h>

#include "ImGuizmo.h"

extern const std::filesystem::path g_assetPath = "asset";

ContentBrowserPanel::ContentBrowserPanel() : m_currentDirectory(g_assetPath)
{
    m_directoryIcon = Texture2D::Create("asset/icon/DirectoryIcon.png");
    m_fileIcon      = Texture2D::Create("asset/icon/FileIcon.png");
}

void ContentBrowserPanel::OnImGuiRender()
{
    ImGui::Begin("Content Browser");
    if (m_currentDirectory != std::filesystem::path(g_assetPath))
    {
        if (ImGui::Button("<-"))
        {
            m_currentDirectory = m_currentDirectory.parent_path();
        }
    }
    static float padding       = 16.0f;
    static float thumbnailSize = 128.0f;
    float        cellSize      = thumbnailSize + padding;
    float        panelWidth    = ImGui::GetContentRegionAvail().x;
    int          columnCount   = (int)(panelWidth / cellSize);
    if (columnCount < 1)
    {
        columnCount = 1;
    }
    ImGui::Columns(columnCount, 0, false);

    for (auto& directoryEntry : std::filesystem::directory_iterator(m_currentDirectory))
    {
        const auto& path           = directoryEntry.path();
        auto        relativePath   = std::filesystem::relative(path, g_assetPath);
        std::string filenameString = relativePath.filename().string();

        ImGui::PushID(filenameString.c_str());
        X::Ref<Texture2D> icon = directoryEntry.is_directory() ? m_directoryIcon : m_fileIcon;
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
        ImGui::ImageButton(directoryEntry.path().string().c_str(), (ImTextureID)(uint64_t)icon->GetRendererID(),
                           {thumbnailSize, thumbnailSize}, {0, 1}, {1, 0});
        if (ImGui::BeginDragDropSource())
        {
            std::string pathStr = relativePath.string();
            ImGui::SetDragDropPayload("CONTENT_BROWSER_ITEM", pathStr.c_str(), (pathStr.size() + 1) * sizeof(char));
            ImGui::EndDragDropSource();
        }
        ImGui::PopStyleColor();
        if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
        {
            if (directoryEntry.is_directory())
            {
                m_currentDirectory /= path.filename();
            }
        }
        ImGui::TextWrapped("%s", filenameString.c_str());
        ImGui::NextColumn();
        ImGui::PopID();
    }
    ImGui::Columns(1);
    ImGui::SliderFloat("Thumbnail Size", &thumbnailSize, 16, 512);
    ImGui::SliderFloat("Padding", &padding, 0, 32);
    ImGui::End();
}
