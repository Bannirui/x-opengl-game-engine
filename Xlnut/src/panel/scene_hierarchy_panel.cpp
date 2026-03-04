//
// Created by rui ding on 2026/3/4.
//

#include "scene_hierarchy_panel.h"

#include "x/scene/component.h"

#include <imgui.h>
#include <imgui_internal.h>
#include <glm/gtc/type_ptr.hpp>

static void DrawVec3Control(const std::string& label, glm::vec3& values, float resetValue = 0.0f,
                            float columnWidth = 100.0f)
{
    ImGuiIO& io       = ImGui::GetIO();
    auto     boldFont = io.Fonts->Fonts[0];
    ImGui::PushID(label.c_str());
    ImGui::Columns(2);
    ImGui::SetColumnWidth(0, columnWidth);
    ImGui::Text("%s", label.c_str());
    ImGui::NextColumn();
    ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{0, 0});
    float  lineHeight = ImGui::GetFrameHeight();
    ImVec2 buttonSize = {lineHeight + 3.0f, lineHeight};
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{0.8f, 0.1f, 0.15f, 1.0f});
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{0.9f, 0.2f, 0.2f, 1.0f});
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{0.8f, 0.1f, 0.15f, 1.0f});
    ImGui::PushFont(boldFont);
    if (ImGui::Button("X", buttonSize))
    {
        values.x = resetValue;
    }
    ImGui::PopFont();
    ImGui::PopStyleColor(3);
    ImGui::SameLine();
    ImGui::DragFloat("##X", &values.x, 0.1f, 0.0f, 0.0f, "%.2f");
    ImGui::PopItemWidth();
    ImGui::SameLine();
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{0.2f, 0.7f, 0.2f, 1.0f});
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{0.3f, 0.8f, 0.3f, 1.0f});
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{0.2f, 0.7f, 0.2f, 1.0f});

    ImGui::PushFont(boldFont);
    if (ImGui::Button("Y", buttonSize))
    {
        values.y = resetValue;
    }
    ImGui::PopFont();

    ImGui::PopStyleColor(3);
    ImGui::SameLine();
    ImGui::DragFloat("##Y", &values.y, 0.1f, 0.0f, 0.0f, "%.2f");
    ImGui::PopItemWidth();
    ImGui::SameLine();
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{0.1f, 0.25f, 0.8f, 1.0f});
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{0.2f, 0.35f, 0.9f, 1.0f});
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{0.1f, 0.25f, 0.8f, 1.0f});

    ImGui::PushFont(boldFont);
    if (ImGui::Button("Z", buttonSize))
    {
        values.z = resetValue;
    }
    ImGui::PopFont();

    ImGui::PopStyleColor(3);
    ImGui::SameLine();
    ImGui::DragFloat("##Z", &values.z, 0.1f, 0.0f, 0.0f, "%.2f");
    ImGui::PopItemWidth();
    ImGui::PopStyleVar();
    ImGui::Columns(1);
    ImGui::PopID();
}

template <typename T, typename UIFunction>
static void DrawComponent(const std::string& name, Entity entity, UIFunction uiFunction)
{
    const ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed |
                                             ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_FramePadding;
    if (entity.HasComponent<T>())
    {
        auto&  component              = entity.GetComponent<T>();
        ImVec2 contentRegionAvailable = ImGui::GetContentRegionAvail();
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{4, 4});
        float lineHeight = ImGui::GetFrameHeight();
        ImGui::Separator();
        bool open = ImGui::TreeNodeEx((void*)typeid(T).hash_code(), treeNodeFlags, "%s", name.c_str());
        ImGui::PopStyleVar();
        ImGui::SameLine(contentRegionAvailable.x - lineHeight * 0.5f);
        if (ImGui::Button("+", ImVec2{lineHeight, lineHeight}))
        {
            ImGui::OpenPopup("ComponentSettings");
        }
        bool removeComponent = false;
        if (ImGui::BeginPopup("ComponentSettings"))
        {
            if (ImGui::MenuItem("Remove component"))
            {
                removeComponent = true;
            }
            ImGui::EndPopup();
        }
        if (open)
        {
            uiFunction(component);
            ImGui::TreePop();
        }
        if (removeComponent)
        {
            entity.RemoveComponent<T>();
        }
    }
}

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
    // Right-click on blank space
    if (ImGui::BeginPopupContextWindow(nullptr, ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverItems))
    {
        if (ImGui::MenuItem("Create Empty Entity"))
        {
            m_context->CreateEntity("Empty Entity");
        }
        ImGui::EndPopup();
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
    flags |= ImGuiTreeNodeFlags_SpanAvailWidth;
    bool opened = ImGui::TreeNodeEx(reinterpret_cast<void*>(static_cast<uint64_t>(static_cast<uint32_t>(entity))),
                                    flags, "%s", tag.c_str());
    if (ImGui::IsItemClicked())
    {
        m_selectionContext = entity;
    }
    bool entityDeleted = false;
    if (ImGui::BeginPopupContextItem())
    {
        if (ImGui::MenuItem("Delete Entity"))
        {
            entityDeleted = true;
        }
        ImGui::EndPopup();
    }
    if (opened)
    {
        ImGuiTreeNodeFlags flags  = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;
        bool               opened = ImGui::TreeNodeEx(reinterpret_cast<void*>(9817239), flags, "%s", tag.c_str());
        if (opened) ImGui::TreePop();
        ImGui::TreePop();
    }
    if (entityDeleted)
    {
        m_context->DestroyEntity(entity);
        if (m_selectionContext == entity)
        {
            m_selectionContext = {};
        }
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
        if (ImGui::InputText("##Tag", buffer, sizeof(buffer)))
        {
            tag = std::string(buffer);
        }
    }

    ImGui::SameLine();
    ImGui::PushItemWidth(-1);

    if (ImGui::Button("Add Component")) ImGui::OpenPopup("AddComponent");

    if (ImGui::BeginPopup("AddComponent"))
    {
        if (ImGui::MenuItem("Camera"))

        {
            m_selectionContext.AddComponent<CameraComponent>();
            ImGui::CloseCurrentPopup();
        }

        if (ImGui::MenuItem("Sprite Renderer"))
        {
            m_selectionContext.AddComponent<SpriteRendererComponent>();
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }

    ImGui::PopItemWidth();

    DrawComponent<TransformComponent>("Transform", entity,
                                      [](auto& component)
                                      {
                                          DrawVec3Control("Translation", component.m_translation);
                                          glm::vec3 rotation = glm::degrees(component.m_rotation);
                                          DrawVec3Control("Rotation", rotation);
                                          component.m_rotation = glm::radians(rotation);
                                          DrawVec3Control("Scale", component.m_scale, 1.0f);
                                      });

    DrawComponent<CameraComponent>(
        "Camera", entity,
        [](auto& component)
        {
            auto& camera = component.m_camera;

            ImGui::Checkbox("Primary", &component.m_primary);

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
                float perspectiveVerticalFov = glm::degrees(camera.get_perspectiveFOV());
                if (ImGui::DragFloat("Vertical FOV", &perspectiveVerticalFov))
                    camera.SetPerspectiveFOV(glm::radians(perspectiveVerticalFov));

                float perspectiveNear = camera.get_perspectiveNearClip();
                if (ImGui::DragFloat("Near", &perspectiveNear)) camera.SetPerspectiveNearClip(perspectiveNear);

                float perspectiveFar = camera.get_perspectiveFarClip();
                if (ImGui::DragFloat("Far", &perspectiveFar)) camera.SetPerspectiveFarClip(perspectiveFar);
            }

            if (camera.get_projectionType() == SceneCamera::ProjectionType::Orthographic)

            {
                float orthoSize = camera.get_orthographicSize();
                if (ImGui::DragFloat("Size", &orthoSize)) camera.SetOrthographicSize(orthoSize);

                float orthoNear = camera.get_orthographicNearClip();
                if (ImGui::DragFloat("Near", &orthoNear)) camera.SetOrthographicNearClip(orthoNear);

                float orthoFar = camera.get_orthographicFarClip();
                if (ImGui::DragFloat("Far", &orthoFar)) camera.SetOrthographicFarClip(orthoFar);

                ImGui::Checkbox("Fixed Aspect Ratio", &component.m_fixedAspectRatio);
            }
        });

    DrawComponent<SpriteRendererComponent>("Sprite Renderer", entity,
                                           [](auto& component)
                                           {
                                               ImGui::ColorEdit4("Color", glm::value_ptr(component.m_color));
                                           });
}
