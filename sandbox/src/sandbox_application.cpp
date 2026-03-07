//
// Created by dingrui on 2/23/26.
//

#include <x/core/entry_point.h>
#include <x_engine.h>

#include "sandbox_2D.h"

class Sandbox : public XApplication
{
public:
    Sandbox(ApplicationCommandLineArgs args)
    {
        // PushLayer(new ExampleLayer());
        PushLayer(new Sandbox2D());
    }

    ~Sandbox() override {}
};

XApplication* CreateApplication(ApplicationCommandLineArgs args)
{
    return new Sandbox(args);
}
