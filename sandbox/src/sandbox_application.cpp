//
// Created by dingrui on 2/23/26.
//

#include "example_layer.h"
#include "sandbox_2D.h"

#include <x/core/entry_point.h>

#include <engine.h>

class Sandbox : public XApplication {
public:
    Sandbox(const ApplicationSpecification& spec) : XApplication(spec) {
        // PushLayer(X::CreateScope<ExampleLayer>());
        PushLayer(X::CreateScope<Sandbox2D>());
    }

    ~Sandbox() override {}
};

XApplication* CreateApplication(ApplicationCommandLineArgs args) {
    ApplicationSpecification spec;
    spec.Name = "Sandbox";
    spec.CommandLineArgs = args;
    return new Sandbox(spec);
}
