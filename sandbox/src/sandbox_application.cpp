//
// Created by dingrui on 2/23/26.
//

#include "pch.h"
#include <x_engine.h>

class ExampleLayer : public Layer
{
public:
    ExampleLayer() : Layer("X-EXAMPLE") {}

    void OnUpdate() override { X_INFO("ExampleLayer::OnUpdate"); }

    void OnEvent(const Event &e) override { X_INFO("{}", e); }
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
