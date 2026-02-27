//
// Created by rui ding on 2026/2/27.
//

#pragma once

class GraphicsContext
{
public:
    virtual void Init()        = 0;
    virtual void SwapBuffers() = 0;
};