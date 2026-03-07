//
// Created by dingrui on 3/3/26.
//

#include <x/core/entry_point.h>
#include <x_engine.h>

#include "editor_layer.h"

class XInut : public XApplication
{
public:
    XInut(ApplicationCommandLineArgs args) : XApplication("XInut", args) { PushLayer(new EditorLayer()); }

    ~XInut() override {}
};

XApplication* CreateApplication(ApplicationCommandLineArgs args)
{
    return new XInut(args);
}
