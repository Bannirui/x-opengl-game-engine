//
// Created by dingrui on 3/8/26.
//

#pragma once

#include <filesystem>

class ContentBrowserPanel
{
public:
    ContentBrowserPanel();
    void OnImGuiRender();

private:
    std::filesystem::path m_currentDirectory;
};
