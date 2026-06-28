//
// Created by dingrui on 3/3/26.
//

#include "editor_layer.h"

#include <x/core/entry_point.h>

#include <engine.h>

class XInut : public Application
{
public:
    XInut(const ApplicationSpecification& spec) : Application(spec) { PushLayer(X::CreateScope<EditorLayer>()); }
};

Application* CreateApplication(ApplicationCommandLineArgs args)
{
    ApplicationSpecification spec;
    spec.Name            = "Xlnut";
    spec.CommandLineArgs = args;
    return new XInut(spec);
}
