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

// 正方形顶点 pos坐标 rgba颜色 纹理uv
struct QuadVertex
{
    glm::vec3 position;
    glm::vec4 color;
    glm::vec2 texCoord;
};

struct Renderer2DData
{
    const uint32_t maxQuads    = 10000;
    const uint32_t maxVertices = maxQuads * 4;
    const uint32_t maxIndices  = maxQuads * 6;

    X::Ref<VertexArray>  quadVertexArray;   // EAO
    X::Ref<VertexBuffer> quadVertexBuffer;  // VBO
    X::Ref<Shader>       textureShader;
    X::Ref<Texture2D>    whiteTexture;

    uint32_t    quadIndexCount       = 0;
    QuadVertex *quadVertexBufferBase = nullptr;
    QuadVertex *quadVertexBufferPtr  = nullptr;
};

static Renderer2DData s_data;

void Renderer2D::Init()
{
    X_PROFILE_FUNCTION();
    s_data.quadVertexArray  = VertexArray::Create();
    s_data.quadVertexBuffer = VertexBuffer::Create(s_data.maxVertices * sizeof(QuadVertex));
    s_data.quadVertexBuffer->SetLayout({{ShaderDataType::kFloat3, "a_Position"},
                                        {ShaderDataType::kFloat4, "a_Color"},
                                        {ShaderDataType::kFloat2, "a_TexCoord"}});
    s_data.quadVertexArray->AddVertexBuffer(s_data.quadVertexBuffer);

    s_data.quadVertexBufferBase = new QuadVertex[s_data.maxVertices];

    uint32_t *quadIndices = new uint32_t[s_data.maxIndices];

    uint32_t offset = 0;
    for (uint32_t i = 0; i < s_data.maxIndices; i += 6)
    {
        quadIndices[i + 0] = offset + 0;
        quadIndices[i + 1] = offset + 1;
        quadIndices[i + 2] = offset + 2;
        quadIndices[i + 3] = offset + 2;
        quadIndices[i + 4] = offset + 3;
        quadIndices[i + 5] = offset + 0;
        offset += 4;
    }
    X::Ref<IndexBuffer> quadIB = IndexBuffer::Create(quadIndices, s_data.maxIndices);
    s_data.quadVertexArray->SetIndexBuffer(quadIB);
    delete[] quadIndices;

    s_data.whiteTexture       = Texture2D::Create(1, 1);
    uint32_t whiteTextureData = 0xffffffff;
    s_data.whiteTexture->SetData(&whiteTextureData, sizeof(uint32_t));

    s_data.textureShader = Shader::Create("asset/shader/Texture.glsl");
    s_data.textureShader->Bind();
    s_data.textureShader->SetInt("u_Texture", 0);
}

void Renderer2D::Shutdown()
{
    X_PROFILE_FUNCTION();
}

void Renderer2D::BeginScene(const OrthographicCamera &camera)
{
    X_PROFILE_FUNCTION();
    s_data.textureShader->Bind();
    s_data.textureShader->SetMat4("u_ViewProjection", camera.get_viewProjectionMatrix());
    s_data.quadIndexCount      = 0;
    s_data.quadVertexBufferPtr = s_data.quadVertexBufferBase;
}

void Renderer2D::EndScene()
{
    X_PROFILE_FUNCTION();
    uint32_t dataSize = reinterpret_cast<uint8_t *>(s_data.quadVertexBufferPtr) -
                        reinterpret_cast<uint8_t *>(s_data.quadVertexBufferBase);
    s_data.quadVertexBuffer->SetData(s_data.quadVertexBufferBase, dataSize);

    Flush();
}

void Renderer2D::Flush()
{
    RenderCommand::DrawIndexed(s_data.quadVertexArray, s_data.quadIndexCount);
}

void Renderer2D::DrawQuad(const glm::vec2 &position, const glm::vec2 &size, const glm::vec4 &color)
{
    DrawQuad({position.x, position.y, 0.0f}, size, color);
}

void Renderer2D::DrawQuad(const glm::vec3 &position, const glm::vec2 &size, const glm::vec4 &color)
{
    X_PROFILE_FUNCTION();

    s_data.quadVertexBufferPtr->position = position;
    s_data.quadVertexBufferPtr->color    = color;
    s_data.quadVertexBufferPtr->texCoord = {0.0f, 0.0f};
    s_data.quadVertexBufferPtr++;

    s_data.quadVertexBufferPtr->position = {position.x + size.x, position.y, 0.0f};
    s_data.quadVertexBufferPtr->color    = color;
    s_data.quadVertexBufferPtr->texCoord = {1.0f, 0.0f};
    s_data.quadVertexBufferPtr++;

    s_data.quadVertexBufferPtr->position = {position.x + size.x, position.y + size.y, 0.0f};
    s_data.quadVertexBufferPtr->color    = color;
    s_data.quadVertexBufferPtr->texCoord = {1.0f, 1.0f};
    s_data.quadVertexBufferPtr++;

    s_data.quadVertexBufferPtr->position = {position.x, position.y + size.y, 0.0f};
    s_data.quadVertexBufferPtr->color    = color;
    s_data.quadVertexBufferPtr->texCoord = {0.0f, 1.0f};
    s_data.quadVertexBufferPtr++;

    s_data.quadIndexCount += 6;

    s_data.textureShader->Bind();
    s_data.textureShader->SetFloat4("u_Color", color);
    s_data.textureShader->SetFloat("u_TilingFactor", 1.0f);
    glm::mat4 transform =
        glm::translate(glm::mat4(1.0f), position) * glm::scale(glm::mat4(1.0f), {size.x, size.y, 1.0f});
    s_data.textureShader->SetMat4("u_Transform", transform);

    s_data.quadVertexArray->Bind();
    RenderCommand::DrawIndexed(s_data.quadVertexArray);
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
    s_data.textureShader->SetFloat4("u_Color", tintColor);
    s_data.textureShader->SetFloat("u_TilingFactor", tilingFactor);
    glm::mat4 transform =
        glm::translate(glm::mat4(1.0f), position) * glm::scale(glm::mat4(1.0f), {size.x, size.y, 1.0f});
    s_data.textureShader->SetMat4("u_Transform", transform);

    s_data.quadVertexArray->Bind();
    RenderCommand::DrawIndexed(s_data.quadVertexArray);
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

    s_data.textureShader->SetFloat4("u_Color", color);
    s_data.textureShader->SetFloat("u_TilingFactor", 1.0f);
    s_data.whiteTexture->Bind();

    glm::mat4 transform = glm::translate(glm::mat4(1.0f), position) *
                          glm::rotate(glm::mat4(1.0f), rotation, {0.0f, 0.0f, 1.0f}) *
                          glm::scale(glm::mat4(1.0f), {size.x, size.y, 1.0f});
    s_data.textureShader->SetMat4("u_Transform", transform);
    s_data.quadVertexArray->Bind();
    RenderCommand::DrawIndexed(s_data.quadVertexArray);
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

    s_data.textureShader->SetFloat4("u_Color", tintColor);
    s_data.textureShader->SetFloat("u_TilingFactor", tilingFactor);
    texture->Bind();

    glm::mat4 transform = glm::translate(glm::mat4(1.0f), position) *
                          glm::rotate(glm::mat4(1.0f), rotation, {0.0f, 0.0f, 1.0f}) *
                          glm::scale(glm::mat4(1.0f), {size.x, size.y, 1.0f});
    s_data.textureShader->SetMat4("u_Transform", transform);

    s_data.quadVertexArray->Bind();
    RenderCommand::DrawIndexed(s_data.quadVertexArray);
}
