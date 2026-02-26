//
// Created by rui ding on 2026/2/26.
//

#pragma once

#include "x/layer.h"

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
    ~ImGuiLayer() override;

    void OnAttach() override;
    void OnDetach() override;
    void OnUpdate() override;
    void OnEvent(Event &e) override;

private:
    float m_time{0.0f};
};
