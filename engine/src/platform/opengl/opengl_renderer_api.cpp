//
// Created by rui ding on 2026/2/27.
//

#include "platform/opengl/opengl_renderer_api.h"

#include "x/renderer/buffer.h"
#include "x/renderer/vertex_array.h"

#include <glad/glad.h>

OpenGLRendererAPI::~OpenGLRendererAPI() {}

void OpenGLRendererAPI::Init() {
    X_PROFILE_FUNCTION();
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LINE_SMOOTH);
}

void OpenGLRendererAPI::SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) {
    glViewport(x, y, width, height);
}

void OpenGLRendererAPI::SetClearColor(const glm::vec4& color) {
    glClearColor(color.r, color.g, color.b, color.a);
}

void OpenGLRendererAPI::Clear() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void OpenGLRendererAPI::DrawIndexed(const X::Ref<VertexArray>& vertexArray, uint32_t indexCount) {
    vertexArray->Bind();
    uint32_t count = indexCount ? indexCount : vertexArray->GetIndexBuffer()->GetCount();
    glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, nullptr);
}

/**
 * 画线段
 * @param vertexArray VAO 用DrawArrays并用不到VAO
 * @param vertexCount 用VBO里面多少个顶点绘制
 */
void OpenGLRendererAPI::DrawLines(const X::Ref<VertexArray>& vertexArray, uint32_t vertexCount) {
    vertexArray->Bind();
    /**
     * 真正让GPU开始渲染
     * - 每两个点组成1个线段
     * - 从VBO的0号点开始
     * - 用多少个顶点
     * 比如vertexCount=2就是画一条线 vertexCount=4就是画两条线
     */
    glDrawArrays(GL_LINES, 0, vertexCount);
}

void OpenGLRendererAPI::SetLineWidth(float width) {
    glLineWidth(width);
}
