//
// Created by rui ding on 2026/3/4.
//

#include "scene_hierarchy_panel.h"

#include "x/scene/component.h"

#include <imgui.h>

SceneHierarchyPanel::SceneHierarchyPanel(const X::Ref<Scene>& scene)
{
    set_context(scene);
}

void SceneHierarchyPanel::set_context(const X::Ref<Scene>& scene)
{
    m_context = scene;
}

void SceneHierarchyPanel::OnImGuiRender()
{
    ImGui::Begin("Scene Hierarchy");

    for (auto entityID : m_context->get_registry().view<entt::entity>())
    {
        Entity entity{entityID, m_context.get()};
        drawEntityNode(entity);
    }

    ImGui::End();
}

void SceneHierarchyPanel::drawEntityNode(Entity entity)
{
    auto&              tag = entity.GetComponent<TagComponent>().m_tag;
    ImGuiTreeNodeFlags flags =
        ((m_selectionContext == entity) ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow;
    bool opened = ImGui::TreeNodeEx(reinterpret_cast<void*>(static_cast<uint64_t>(static_cast<uint32_t>(entity))),
                                    flags, "%s", tag.c_str());
    if (ImGui::IsItemClicked())
    {
        m_selectionContext = entity;
    }

    if (opened)
    {
        ImGuiTreeNodeFlags flags  = ImGuiTreeNodeFlags_OpenOnArrow;
        bool               opened = ImGui::TreeNodeEx(reinterpret_cast<void*>(9817239), flags, "%s", tag.c_str());
        if (opened) ImGui::TreePop();
        ImGui::TreePop();
    }
}
