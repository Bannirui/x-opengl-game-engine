//
// Created by dingrui on 2/23/26.
//

#pragma once

#include "core.h"

class XApplication {
public:
    XApplication();;

    virtual ~XApplication() = default;

    void Run();
};

// To be defined in the CLIENT
XApplication *CreateApplication();
