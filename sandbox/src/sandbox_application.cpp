//
// Created by dingrui on 2/23/26.
//

#include "sandbox_2D.h"
#include "sandbox_3D.h"
#include "solar_system.h"

#include <x/core/entry_point.h>

#include <engine.h>

class Sandbox : public Application {
public:
    Sandbox(const ApplicationSpecification& spec) : Application(spec) {
        // PushLayer(CreateScope<Sandbox2D>());
        // PushLayer(CreateScope<Sandbox3D>());
        PushLayer(CreateScope<SolarSystem>());
    }

    ~Sandbox() override = default;
};

Application* CreateApplication(ApplicationCommandLineArgs args) {
    ApplicationSpecification spec;
    spec.Name = "Sandbox";
    spec.CommandLineArgs = args;
    return new Sandbox(spec);
}
