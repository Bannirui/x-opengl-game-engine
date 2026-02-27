//
// Created by rui ding on 2026/2/27.
//

#include "x/renderer/render_command.h"

#include "platform/opengl/open_gl_renderer_api.h"

RendererAPI *RenderCommand::s_RendererAPI = new OpenGLRendererAPI;
