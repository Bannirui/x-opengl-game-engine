//
// Created by rui ding on 2026/2/27.
//

#pragma once

#include "x/renderer/graphics_context.h"

struct GLFWwindow;

class OpenGLContext : public GraphicsContext
{
public:
    OpenGLContext(GLFWwindow *windowHandle);

    void Init() override;
    void SwapBuffers() override;

private:
    GLFWwindow *m_windowHandle;
};
