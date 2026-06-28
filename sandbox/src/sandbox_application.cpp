//
// Created by dingrui on 2/23/26.
//

#include "sandbox_2D.h"
#include "sandbox_3D.h"
#include "solar_system.h"

#include <x/core/entry_point.h>

#include <engine.h>

class Sandbox : public XApplication {
public:
    Sandbox(const ApplicationSpecification& spec) : XApplication(spec) {
        // PushLayer(CreateScope<Sandbox2D>());
        // PushLayer(CreateScope<Sandbox3D>());
        PushLayer(CreateScope<SolarSystem>());
    }

    ~Sandbox() override = default;
};

XApplication* CreateApplication(ApplicationCommandLineArgs args) {
    ApplicationSpecification spec;
    spec.Name = "Sandbox";
    spec.CommandLineArgs = args;
    return new Sandbox(spec);
}
