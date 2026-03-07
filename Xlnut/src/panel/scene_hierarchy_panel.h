//
// Created by rui ding on 2026/3/4.
//

#pragma once

#include <x/core/base.h>

#include <x/scene/entity.h>

class Scene;

class SceneHierarchyPanel
{
public:
    SceneHierarchyPanel() = default;
    SceneHierarchyPanel(const X::Ref<Scene>& scene);

    Entity get_selectedEntity() const { return m_selectionContext; }

    void set_selectedEntity(Entity entity) { m_selectionContext = entity; }

    void set_context(const X::Ref<Scene>& scene);
    void OnImGuiRender();

private:
    void drawEntityNode(Entity entity);
    void drawComponents(Entity entity);

private:
    X::Ref<Scene> m_context;
    Entity        m_selectionContext;
};
