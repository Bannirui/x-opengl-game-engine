//
// Created by dingrui on 3/8/26.
//

#include "content_browser_panel.h"

#include <imgui.h>

#include <x/renderer/texture.h>

static const std::filesystem::path s_assetPath = "asset";

ContentBrowserPanel::ContentBrowserPanel() : m_currentDirectory(s_assetPath)
{
    m_directoryIcon = Texture2D::Create("asset/icon/DirectoryIcon.png");
    m_fileIcon      = Texture2D::Create("asset/icon/FileIcon.png");
}

void ContentBrowserPanel::OnImGuiRender()
{
    ImGui::Begin("Content Browser");
    if (m_currentDirectory != std::filesystem::path(s_assetPath))
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
        auto        relativePath   = std::filesystem::relative(path, s_assetPath);
        std::string filenameString = relativePath.filename().string();

        X::Ref<Texture2D> icon = directoryEntry.is_directory() ? m_directoryIcon : m_fileIcon;
        ImGui::ImageButton((ImTextureID)icon->GetRendererID(), {thumbnailSize, thumbnailSize}, {0, 1}, {1, 0});
        if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
        {
            if (directoryEntry.is_directory())
            {
                m_currentDirectory /= path.filename();
            }
        }
        ImGui::TextWrapped(filenameString.c_str());
        ImGui::NextColumn();
    }
    ImGui::Columns(1);
    ImGui::SliderFloat("Thumbnail Size", &thumbnailSize, 16, 512);
    ImGui::SliderFloat("Padding", &padding, 0, 32);
    ImGui::End();
}
