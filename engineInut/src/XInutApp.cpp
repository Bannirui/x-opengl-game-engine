//
// Created by dingrui on 3/3/26.
//

#include <x_engine.h>
#include <x/core/entry_point.h>

#include "editor_layer.h"

class XInut : public XApplication
{
public:
    XInut() : XApplication("XInut") { PushLayer(new EditorLayer()); }

    ~XInut() override {}

};

XApplication* CreateApplication()
{
    return new XInut();
}
