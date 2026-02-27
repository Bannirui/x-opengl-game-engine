//
// Created by dingrui on 2/23/26.
//

#include "pch.h"
#include <x_engine.h>

#include <imgui.h>

class ExampleLayer : public Layer
{
public:
    ExampleLayer() : Layer("X-EXAMPLE") {}

    void OnUpdate() override
    {
        if (Input::IsKeyPressed(X_KEY::Tab))
        {
            X_TRACE("Tab is pressed (poll)");
        }
    }

    void OnImguiRender() override
    {
        ImGui::Begin("Test");
        ImGui::Text("Hello World");
        ImGui::End();
    }

    void OnEvent(Event &e) override
    {
        if (e.GetEventType() == EventType::kKeyPressed)
        {
            KeyPressEvent &event = static_cast<KeyPressEvent &>(e);
            if (event.get_keyCode() == X_KEY::Tab)
            {
                X_TRACE("Tab key is pressed (event)!");
            }
            X_TRACE("{}", (char)event.get_keyCode());
        }
    }
};

class Sandbox : public XApplication
{
public:
    Sandbox()
    {
        PushLayer(new ExampleLayer());
    }

    ~Sandbox() override {}
};

XApplication *CreateApplication()
{
    return new Sandbox();
}
