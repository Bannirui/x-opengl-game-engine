//
// Created by rui ding on 2026/2/27.
//

#include "x/renderer/renderer.h"

#include "x/renderer/2d/renderer_2D.h"
#include "x/renderer/3d/renderer_3D.h"
#include "x/renderer/buffer/vertex_array.h"
#include "x/renderer/camera/orthographic_camera.h"
#include "x/renderer/render_command.h"
#include "x/renderer/shader.h"

Scope<Renderer::SceneData> Renderer::s_sceneData = CreateScope<Renderer::SceneData>();

void Renderer::Init() {
    X_PROFILE_FUNCTION();
    RenderCommand::Init();
    Renderer2D::Init();
    Renderer3D::Init();
}

void Renderer::Shutdown() {
    Renderer3D::Shutdown();
    Renderer2D::Shutdown();
}

void Renderer::OnWindowResize(uint32_t width, uint32_t height) {
    RenderCommand::SetViewport(0, 0, width, height);
}