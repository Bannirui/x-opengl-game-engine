//
// Created by dingrui on 2/23/26.
//

#include "sandbox_2D.h"

#include <x/core/entry_point.h>

#include <x_engine.h>

class Sandbox : public XApplication
{
public:
    Sandbox(const ApplicationSpecification& spec) : XApplication(spec)
    {
        // PushLayer(new ExampleLayer());
        PushLayer(new Sandbox2D());
    }

    ~Sandbox() override {}
};

XApplication* CreateApplication(ApplicationCommandLineArgs args)
{
    ApplicationSpecification spec;
    spec.Name             = "Sandbox";
    spec.WorkingDirectory = "../Xlnut";
    spec.CommandLineArgs  = args;
    return new Sandbox(spec);
}
