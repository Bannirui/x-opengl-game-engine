//
// Created by rui ding on 2026/2/27.
//

#pragma once
#include "x/core/base.h"

class GraphicsContext
{
protected:
    GraphicsContext() = default;

public:
    virtual ~GraphicsContext() = default;

    virtual void Init()        = 0;
    virtual void SwapBuffers() = 0;

    static X::Scope<GraphicsContext> Create(void *window);
};