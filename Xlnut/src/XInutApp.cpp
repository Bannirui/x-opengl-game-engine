//
// Created by dingrui on 3/3/26.
//

#include "editor_layer.h"

#include <x/core/entry_point.h>

#include <x_engine.h>

class XInut : public XApplication
{
public:
    XInut(const ApplicationSpecification& spec) : XApplication(spec) { PushLayer(new EditorLayer()); }
};

XApplication* CreateApplication(ApplicationCommandLineArgs args)
{
    ApplicationSpecification spec;
    spec.Name            = "Xlnut";
    spec.CommandLineArgs = args;
    return new XInut(spec);
}
