//
// Created by dingrui on 2/23/26.
//

#pragma once

#include "engine/core.h"

class XApplication {
public:
    XApplication() = default;

    virtual ~XApplication() = default;

    void Run();
};

// To be defined in the CLIENT
XApplication *CreateApplication();
