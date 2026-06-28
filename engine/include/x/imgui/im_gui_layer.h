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

class ImGuiLayer : public Layer {
public:
    ImGuiLayer();
    ~ImGuiLayer() override;

    void OnAttach() override;
    void OnDetach() override;

    void OnUpdate(Timestep ts) override;
    void OnImguiRender() override;
    void OnEvent(Event& e) override;

    static void Begin();
    static void End();

    void BlockEvents(bool block) {
        m_blockEvents = block;
    }

private:
    bool m_blockEvents{true};
};
