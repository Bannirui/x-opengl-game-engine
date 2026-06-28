//
// Created by dingrui on 3/8/26.
//

#pragma once

#include <x/core/base.h>

#include <filesystem>

class Texture2D;

class ContentBrowserPanel
{
public:
    ContentBrowserPanel();
    void OnImGuiRender();

private:
    std::filesystem::path m_currentDirectory;
    Ref<Texture2D>     m_directoryIcon;
    Ref<Texture2D>     m_fileIcon;
};
