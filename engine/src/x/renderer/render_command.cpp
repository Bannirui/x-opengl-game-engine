//
// Created by rui ding on 2026/2/27.
//

#include "x/renderer/render_command.h"

#include "platform/opengl/open_gl_renderer_api.h"

X::Scope<RendererAPI> RenderCommand::s_rendererAPI = X::CreateScope<OpenGLRendererAPI>();
