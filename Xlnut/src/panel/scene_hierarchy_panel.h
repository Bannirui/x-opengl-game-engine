//
// Created by rui ding on 2026/3/4.
//

#pragma once

#include "x/core/base.h"

#include "x/scene/entity.h"

class Scene;

class SceneHierarchyPanel
{
public:
    SceneHierarchyPanel() = default;
    SceneHierarchyPanel(const X::Ref<Scene>& scene);

    void set_context(const X::Ref<Scene>& scene);
    void OnImGuiRender();

private:
    void drawEntityNode(Entity entity);

private:
    X::Ref<Scene> m_context;
    Entity        m_selectionContext;
};
