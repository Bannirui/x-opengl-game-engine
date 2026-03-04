//
// Created by rui ding on 2026/3/4.
//

#include "scene_hierarchy_panel.h"

#include "x/scene/component.h"

#include <imgui.h>
#include <glm/gtc/type_ptr.hpp>

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
    if (ImGui::IsMouseDown(0) && ImGui::IsWindowHovered())
    {
        m_selectionContext = {};
    }
    ImGui::End();

    ImGui::Begin("Properties");
    if (m_selectionContext)
    {
        drawComponents(m_selectionContext);
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

void SceneHierarchyPanel::drawComponents(Entity entity)
{
    if (entity.HasComponent<TagComponent>())
    {
        auto& tag = entity.GetComponent<TagComponent>().m_tag;
        char  buffer[256];
        memset(buffer, 0, sizeof(buffer));
        std::snprintf(buffer, sizeof(buffer), "%s", tag.c_str());
        if (ImGui::InputText("Tag", buffer, sizeof(buffer)))
        {
            tag = std::string(buffer);
        }
    }
    if (entity.HasComponent<TransformComponent>())
    {
        if (ImGui::TreeNodeEx(reinterpret_cast<void*>(typeid(TransformComponent).hash_code()),
                              ImGuiTreeNodeFlags_DefaultOpen, "Transform"))
        {
            auto& transform = entity.GetComponent<TransformComponent>().m_transform;
            ImGui::DragFloat3("Position", glm::value_ptr(transform[3]), 0.1f);
            ImGui::TreePop();
        }
    }
    if (entity.HasComponent<CameraComponent>())
    {
        if (ImGui::TreeNodeEx((void*)typeid(CameraComponent).hash_code(), ImGuiTreeNodeFlags_DefaultOpen, "Camera"))
        {
            auto& cameraComponent = entity.GetComponent<CameraComponent>();
            auto& camera          = cameraComponent.m_camera;

            ImGui::Checkbox("Primary", &cameraComponent.m_primary);

            const char* projectionTypeStrings[]     = {"Perspective", "Orthographic"};
            const char* currentProjectionTypeString = projectionTypeStrings[(int)camera.get_projectionType()];
            if (ImGui::BeginCombo("Projection", currentProjectionTypeString))
            {
                for (int i = 0; i < 2; i++)
                {
                    bool isSelected = currentProjectionTypeString == projectionTypeStrings[i];
                    if (ImGui::Selectable(projectionTypeStrings[i], isSelected))
                    {
                        currentProjectionTypeString = projectionTypeStrings[i];
                        camera.set_projectionType((SceneCamera::ProjectionType)i);
                    }
                    if (isSelected) ImGui::SetItemDefaultFocus();
                }

                ImGui::EndCombo();
            }

            if (camera.get_projectionType() == SceneCamera::ProjectionType::Perspective)
            {
                float verticalFov = glm::degrees(camera.get_perspectiveFOV());
                if (ImGui::DragFloat("Vertical FOV", &verticalFov))
                {
                    camera.SetPerspectiveFOV(glm::radians(verticalFov));
                }
                float orthoNear = camera.get_perspectiveNearClip();
                if (ImGui::DragFloat("Near", &orthoNear))
                {
                    camera.SetPerspectiveNearClip(orthoNear);
                }
                float orthoFar = camera.get_perspectiveFarClip();
                if (ImGui::DragFloat("Far", &orthoFar))
                {
                    camera.SetPerspectiveFarClip(orthoFar);
                }
            }
            if (camera.get_projectionType() == SceneCamera::ProjectionType::Orthographic)
            {
                float orthoSize = camera.get_orthographicSize();
                if (ImGui::DragFloat("Size", &orthoSize))
                {
                    camera.SetOrthographicSize(orthoSize);
                }
                float orthoNear = camera.get_orthographicNearClip();
                if (ImGui::DragFloat("Near", &orthoNear))
                {
                    camera.SetOrthographicNearClip(orthoNear);
                }
                float orthoFar = camera.get_orthographicFarClip();
                if (ImGui::DragFloat("Far", &orthoFar))
                {
                    camera.SetOrthographicFarClip(orthoFar);
                }
                ImGui::Checkbox("Fixed Aspect Ratio", &cameraComponent.m_fixedAspectRatio);
            }
            ImGui::TreePop();
        }
    }
}
