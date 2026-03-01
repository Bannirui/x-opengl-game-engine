//
// Created by dingrui on 2/28/26.
//

#include "x/renderer/renderer_2D.h"

#include <glm/ext/matrix_transform.hpp>

#include "buffer.h"
#include "render_command.h"
#include "texture.h"
#include "x/renderer/shader.h"
#include "x/renderer/vertex_array.h"
#include "x/core.h"

struct Renderer2DStorage {
    X::Ref<VertexArray> quadVertexArray;
    X::Ref<Shader> flatColorShader;
    X::Ref<Shader> textureShader;
};

static Renderer2DStorage *s_data;

void Renderer2D::Init() {
    // clang-format off
	float squareVertices[] = {
	     // pos(xyz)          // uv(xy)
        -0.5f, -0.5f,  0.0f,  0.0f,  0.0f,
         0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
         0.5f,  0.5f,  0.0f,  1.0f,  1.0f,
        -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
    };
	uint32_t squareIndices[] = {0, 1, 2, 2, 3, 0};
    // clang-format on

    s_data = new Renderer2DStorage();
    s_data->quadVertexArray = VertexArray::Create();

    X::Ref<VertexBuffer> squareVB;
    squareVB.reset(VertexBuffer::Create(squareVertices, sizeof(squareVertices)));
    squareVB->SetLayout({
        {ShaderDataType::kFloat3, "a_Position"},
        {ShaderDataType::kFloat2, "a_TexCoord"},
    });
    s_data->quadVertexArray->AddVertexBuffer(squareVB);

    X::Ref<IndexBuffer> squareIB;
    squareIB.reset(IndexBuffer::Create(squareIndices, sizeof(squareIndices) / sizeof(uint32_t)));
    s_data->quadVertexArray->SetIndexBuffer(squareIB);

    s_data->flatColorShader = Shader::Create("asset/shader/FlatColor.glsl");
    s_data->textureShader = Shader::Create("asset/shader/Texture.glsl");
    s_data->textureShader->Bind();
    s_data->textureShader->SetInt("u_Texture", 0);
}

void Renderer2D::Shutdown() {
    delete s_data;
}

void Renderer2D::BeginScene(const OrthographicCamera &camera) {
    s_data->flatColorShader->Bind();
    s_data->flatColorShader->SetMat4("u_ViewProjection", camera.get_viewProjectionMatrix());
    s_data->textureShader->Bind();
    s_data->flatColorShader->SetMat4("u_ViewProjection", camera.get_viewProjectionMatrix());
}

void Renderer2D::EndScene() {
}

void Renderer2D::DrawQuad(const glm::vec2 &position, const glm::vec2 &size, const glm::vec4 &color) {
    DrawQuad({position.x, position.y, 0.0f}, size, color);
}

void Renderer2D::DrawQuad(const glm::vec3 &position, const glm::vec2 &size, const glm::vec4 &color) {
    s_data->flatColorShader->Bind();
    s_data->flatColorShader->SetFloat4("u_Color", color);

    glm::mat4 transform = glm::translate(glm::mat4(1.0f), position) *
                          glm::scale(glm::mat4(1.0f), {size.x, size.y, 1.0f});
    s_data->flatColorShader->SetMat4("u_Transform", transform);

    s_data->quadVertexArray->Bind();
    RenderCommand::DrawIndexed(s_data->quadVertexArray);
}

void Renderer2D::DrawQuad(const glm::vec2 &position, const glm::vec2 &size, const X::Ref<Texture2D> &texture) {
    DrawQuad({position.x, position.y, 0.0f}, size, texture);
}

void Renderer2D::DrawQuad(const glm::vec3 &position, const glm::vec2 &size, const X::Ref<Texture2D> &texture) {
    s_data->textureShader->Bind();

    glm::mat4 transform = glm::translate(glm::mat4(1.0f), position) *
                          glm::scale(glm::mat4(1.0f), {size.x, size.y, 1.0f});
    s_data->textureShader->SetMat4("u_Transform", transform);

    texture->Bind();
    RenderCommand::DrawIndexed(s_data->quadVertexArray);
}
