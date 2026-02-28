//
// Created by rui ding on 2026/2/26.
//

#pragma once

#include "x/core/layer.h"

class WindowResizeEvent;
class KeyTypedEvent;
class KeyReleaseEvent;
class KeyPressEvent;
class MouseScrolledEvent;
class MouseMovedEvent;
class MouseButtonReleasedEvent;
class MouseButtonPressedEvent;

class ImGuiLayer : public Layer
{
public:
    ImGuiLayer();
    ~ImGuiLayer() override = default;

    void OnAttach() override;
    void OnDetach() override;
    void OnImguiRender() override;

    void Begin();
    void End();
};
