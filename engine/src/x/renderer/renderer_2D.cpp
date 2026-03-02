//
// Created by dingrui on 2/28/26.
//

#include "x/renderer/renderer_2D.h"

#include <glm/ext/matrix_transform.hpp>

#include "x/renderer/buffer.h"
#include "x/renderer/render_command.h"
#include "x/renderer/texture.h"
#include "x/renderer/shader.h"
#include "x/renderer/vertex_array.h"
#include "x/core/core.h"

struct Renderer2DStorage
{
    X::Ref<VertexArray> quadVertexArray;
    X::Ref<Shader>      textureShader;
    X::Ref<Texture2D>   whiteTexture;
};

static Renderer2DStorage *s_data;

void Renderer2D::Init()
{
    X_PROFILE_FUNCTION();
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

    s_data                  = new Renderer2DStorage();
    s_data->quadVertexArray = VertexArray::Create();

    X::Ref<VertexBuffer> squareVB = VertexBuffer::Create(squareVertices, sizeof(squareVertices));
    squareVB->SetLayout({
        {ShaderDataType::kFloat3, "a_Position"},
        {ShaderDataType::kFloat2, "a_TexCoord"},
    });
    s_data->quadVertexArray->AddVertexBuffer(squareVB);

    X::Ref<IndexBuffer> squareIB = IndexBuffer::Create(squareIndices, sizeof(squareIndices) / sizeof(uint32_t));
    s_data->quadVertexArray->SetIndexBuffer(squareIB);

    s_data->textureShader = Shader::Create("asset/shader/Texture.glsl");
    s_data->textureShader->Bind();
    s_data->textureShader->SetInt("u_Texture", 0);

    s_data->whiteTexture      = Texture2D::Create(1, 1);
    uint32_t whiteTextureData = 0xffffffff;
    s_data->whiteTexture->SetData(&whiteTextureData, sizeof(uint32_t));
}

void Renderer2D::Shutdown()
{
    X_PROFILE_FUNCTION();
    delete s_data;
}

void Renderer2D::BeginScene(const OrthographicCamera &camera)
{
    X_PROFILE_FUNCTION();
    s_data->textureShader->Bind();
    s_data->textureShader->SetMat4("u_ViewProjection", camera.get_viewProjectionMatrix());
}

void Renderer2D::EndScene()
{
    X_PROFILE_FUNCTION();
}

void Renderer2D::DrawQuad(const glm::vec2 &position, const glm::vec2 &size, const glm::vec4 &color)
{
    DrawQuad({position.x, position.y, 0.0f}, size, color);
}

void Renderer2D::DrawQuad(const glm::vec3 &position, const glm::vec2 &size, const glm::vec4 &color)
{
    X_PROFILE_FUNCTION();
    s_data->textureShader->Bind();
    s_data->textureShader->SetFloat4("u_Color", color);
    s_data->textureShader->SetFloat("u_TilingFactor", 1.0f);
    glm::mat4 transform =
        glm::translate(glm::mat4(1.0f), position) * glm::scale(glm::mat4(1.0f), {size.x, size.y, 1.0f});
    s_data->textureShader->SetMat4("u_Transform", transform);

    s_data->quadVertexArray->Bind();
    RenderCommand::DrawIndexed(s_data->quadVertexArray);
}

void Renderer2D::DrawQuad(const glm::vec2 &position, const glm::vec2 &size, const X::Ref<Texture2D> &texture,
                          float tilingFactor, const glm::vec4 &tintColor)
{
    DrawQuad({position.x, position.y, 0.0f}, size, texture, tilingFactor, tintColor);
}

void Renderer2D::DrawQuad(const glm::vec3 &position, const glm::vec2 &size, const X::Ref<Texture2D> &texture,
                          float tilingFactor, const glm::vec4 &tintColor)
{
    X_PROFILE_FUNCTION();
    texture->Bind();
    s_data->textureShader->SetFloat4("u_Color", tintColor);
    s_data->textureShader->SetFloat("u_TilingFactor", tilingFactor);
    glm::mat4 transform =
        glm::translate(glm::mat4(1.0f), position) * glm::scale(glm::mat4(1.0f), {size.x, size.y, 1.0f});
    s_data->textureShader->SetMat4("u_Transform", transform);

    s_data->quadVertexArray->Bind();
    RenderCommand::DrawIndexed(s_data->quadVertexArray);
}

void Renderer2D::DrawRotatedQuad(const glm::vec2 &position, const glm::vec2 &size, float rotation,
                                 const glm::vec4 &color)
{
    DrawRotatedQuad({position.x, position.y, 0.0f}, size, rotation, color);
}

void Renderer2D::DrawRotatedQuad(const glm::vec3 &position, const glm::vec2 &size, float rotation,
                                 const glm::vec4 &color)
{
    X_PROFILE_FUNCTION();

    s_data->textureShader->SetFloat4("u_Color", color);
    s_data->textureShader->SetFloat("u_TilingFactor", 1.0f);
    s_data->whiteTexture->Bind();

    glm::mat4 transform = glm::translate(glm::mat4(1.0f), position) *
                          glm::rotate(glm::mat4(1.0f), rotation, {0.0f, 0.0f, 1.0f}) *
                          glm::scale(glm::mat4(1.0f), {size.x, size.y, 1.0f});
    s_data->textureShader->SetMat4("u_Transform", transform);
    s_data->quadVertexArray->Bind();
    RenderCommand::DrawIndexed(s_data->quadVertexArray);
}

void Renderer2D::DrawRotatedQuad(const glm::vec2 &position, const glm::vec2 &size, float rotation,
                                 const X::Ref<Texture2D> &texture, float tilingFactor, const glm::vec4 &tintColor)
{
    DrawRotatedQuad({position.x, position.y, 0.0f}, size, rotation, texture, tilingFactor, tintColor);
}

void Renderer2D::DrawRotatedQuad(const glm::vec3 &position, const glm::vec2 &size, float rotation,
                                 const X::Ref<Texture2D> &texture, float tilingFactor, const glm::vec4 &tintColor)
{
    X_PROFILE_FUNCTION();

    s_data->textureShader->SetFloat4("u_Color", tintColor);
    s_data->textureShader->SetFloat("u_TilingFactor", tilingFactor);
    texture->Bind();

    glm::mat4 transform = glm::translate(glm::mat4(1.0f), position) *
                          glm::rotate(glm::mat4(1.0f), rotation, {0.0f, 0.0f, 1.0f}) *
                          glm::scale(glm::mat4(1.0f), {size.x, size.y, 1.0f});
    s_data->textureShader->SetMat4("u_Transform", transform);

    s_data->quadVertexArray->Bind();
    RenderCommand::DrawIndexed(s_data->quadVertexArray);
}
