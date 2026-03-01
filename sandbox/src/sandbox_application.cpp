//
// Created by dingrui on 2/23/26.
//

#include <x_engine.h>
#include <x/core/entry_point.h>

#include "sandbox_2D.h"

class Sandbox : public XApplication
{
public:
    Sandbox()
    {
        // PushLayer(new ExampleLayer());
        PushLayer(new Sandbox2D());
    }

    ~Sandbox() override {}
};

XApplication *CreateApplication()
{
    return new Sandbox();
}
