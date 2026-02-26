//
// Created by dingrui on 2/23/26.
//

#include "pch.h"
#include <x_engine.h>

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

    void OnEvent(Event &e) override
    {
        if (e.GetEventType() == EventType::kKeyPressed)
        {
            KeyPressEvent &event = static_cast<KeyPressEvent &>(e);
            if (event.get_keyCode() == X_KEY::Tab)
            {
                X_TRACE("Tab key is pressed (event)!");
            }
            X_TRACE("{0}", (char)event.get_keyCode());
        }
    }
};

class Sandbox : public XApplication
{
public:
    Sandbox()
    {
        PushLayer(new ExampleLayer());
        PushLayer(new ImGuiLayer());
    }

    ~Sandbox() override {}
};

XApplication *CreateApplication()
{
    return new Sandbox();
}
