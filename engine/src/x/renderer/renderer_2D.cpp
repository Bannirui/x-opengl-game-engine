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

// VBO 正方形顶点
struct QuadVertex
{
    glm::vec3 position;  // pos坐标
    glm::vec4 color;     // rgba颜色
    glm::vec2 texCoord;  // 纹理uv
    float     texIndex;  // 用哪个texture纹理采样
    float     tilingFactor;
};

struct Renderer2DData
{
    static const uint32_t maxQuads        = 20000;
    static const uint32_t maxVertices     = maxQuads * 4;
    static const uint32_t maxIndices      = maxQuads * 6;
    static const uint32_t maxTextureSlots = 16;

    X::Ref<VertexArray>  quadVertexArray;   // EAO
    X::Ref<VertexBuffer> quadVertexBuffer;  // VBO
    X::Ref<Shader>       textureShader;
    X::Ref<Texture2D>    whiteTexture;

    uint32_t                                       quadIndexCount       = 0;
    QuadVertex                                    *quadVertexBufferBase = nullptr;
    QuadVertex                                    *quadVertexBufferPtr  = nullptr;
    std::array<X::Ref<Texture2D>, maxTextureSlots> textureSlots;          // 纹理采样对象
    uint32_t                                       textureSlotIndex = 1;  // 0 = white texture
    glm::vec4                                      quadVertexPositions[4];
    Renderer2D::Statistics                         stats;
};

static Renderer2DData s_data;

void Renderer2D::Init()
{
    X_PROFILE_FUNCTION();
    s_data.quadVertexArray  = VertexArray::Create();
    s_data.quadVertexBuffer = VertexBuffer::Create(s_data.maxVertices * sizeof(QuadVertex));
    s_data.quadVertexBuffer->SetLayout({
        {ShaderDataType::kFloat3, "a_Position"},
        {ShaderDataType::kFloat4, "a_Color"},
        {ShaderDataType::kFloat2, "a_TexCoord"},
        {ShaderDataType::kFloat, "a_TexIndex"},
        {ShaderDataType::kFloat, "a_TilingFactor"},
    });
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

    int32_t samplers[s_data.maxTextureSlots];
    for (uint32_t i = 0; i < s_data.maxTextureSlots; i++)
    {
        samplers[i] = i;
    }

    s_data.textureShader = Shader::Create("asset/shader/Texture.glsl");
    s_data.textureShader->Bind();
    s_data.textureShader->SetIntArray("u_Textures", samplers, s_data.maxTextureSlots);

    // Set all texture slots to 0
    s_data.textureSlots[0] = s_data.whiteTexture;

    s_data.quadVertexPositions[0] = {-0.5f, -0.5f, 0.0f, 1.0f};
    s_data.quadVertexPositions[1] = {0.5f, -0.5f, 0.0f, 1.0f};
    s_data.quadVertexPositions[2] = {0.5f, 0.5f, 0.0f, 1.0f};
    s_data.quadVertexPositions[3] = {-0.5f, 0.5f, 0.0f, 1.0f};
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
    s_data.textureSlotIndex    = 1;
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
    // Bind textures
    for (uint32_t i = 0; i < s_data.textureSlotIndex; i++)
    {
        s_data.textureSlots[i]->Bind(i);
    }
    RenderCommand::DrawIndexed(s_data.quadVertexArray, s_data.quadIndexCount);
    s_data.stats.drawCalls++;
}

void Renderer2D::DrawQuad(const glm::vec2 &position, const glm::vec2 &size, const glm::vec4 &color)
{
    DrawQuad({position.x, position.y, 0.0f}, size, color);
}

void Renderer2D::DrawQuad(const glm::vec3 &position, const glm::vec2 &size, const glm::vec4 &color)
{
    X_PROFILE_FUNCTION();
    constexpr size_t    quadVertexCount = 4;
    const float         texIndex        = 0.0f;  // White Texture
    constexpr glm::vec2 textureCoords[] = {{0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f}};
    const float         tilingFactor    = 1.0f;

    if (s_data.quadIndexCount >= Renderer2DData::maxIndices)
    {
        FlushAndReset();
    }

    glm::mat4 transform =
        glm::translate(glm::mat4(1.0f), position) * glm::scale(glm::mat4(1.0f), {size.x, size.y, 1.0f});
    for (size_t i = 0; i < quadVertexCount; ++i)
    {
        s_data.quadVertexBufferPtr->position     = transform * s_data.quadVertexPositions[i];
        s_data.quadVertexBufferPtr->color        = color;
        s_data.quadVertexBufferPtr->texCoord     = textureCoords[i];
        s_data.quadVertexBufferPtr->texIndex     = texIndex;
        s_data.quadVertexBufferPtr->tilingFactor = tilingFactor;
        s_data.quadVertexBufferPtr++;
    }
    s_data.quadIndexCount += 6;
    s_data.stats.quadCount++;
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
    constexpr size_t    quadVertexCount = 4;
    constexpr glm::vec4 color           = {1.0f, 1.0f, 1.0f, 1.0f};
    constexpr glm::vec2 textureCoords[] = {{0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f}};
    if (s_data.quadIndexCount >= Renderer2DData::maxIndices)
    {
        FlushAndReset();
    }

    float textureIndex = 0.0f;
    for (uint32_t i = 1; i < s_data.textureSlotIndex; i++)
    {
        if (*s_data.textureSlots[i].get() == *texture.get())
        {
            textureIndex = (float)i;
            break;
        }
    }

    if (textureIndex == 0.0f)
    {
        textureIndex                                 = (float)s_data.textureSlotIndex;
        s_data.textureSlots[s_data.textureSlotIndex] = texture;
        s_data.textureSlotIndex++;
    }

    glm::mat4 transform =
        glm::translate(glm::mat4(1.0f), position) * glm::scale(glm::mat4(1.0f), {size.x, size.y, 1.0f});
    for (size_t i = 0; i < quadVertexCount; ++i)
    {
        s_data.quadVertexBufferPtr->position     = transform * s_data.quadVertexPositions[i];
        s_data.quadVertexBufferPtr->color        = color;
        s_data.quadVertexBufferPtr->texCoord     = textureCoords[i];
        s_data.quadVertexBufferPtr->texIndex     = textureIndex;
        s_data.quadVertexBufferPtr->tilingFactor = tilingFactor;
        s_data.quadVertexBufferPtr++;
    }
    s_data.quadIndexCount += 6;
    s_data.stats.quadCount++;
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
    constexpr size_t    quadVertexCount = 4;
    const float         textureIndex    = 0.0f;  // White Texture
    constexpr glm::vec2 textureCoords[] = {{0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f}};
    const float         tilingFactor    = 1.0f;
    if (s_data.quadIndexCount >= Renderer2DData::maxIndices)
    {
        FlushAndReset();
    }

    glm::mat4 transform = glm::translate(glm::mat4(1.0f), position) *
                          glm::rotate(glm::mat4(1.0f), glm::radians(rotation), {0.0f, 0.0f, 1.0f}) *
                          glm::scale(glm::mat4(1.0f), {size.x, size.y, 1.0f});
    for (size_t i = 0; i < quadVertexCount; ++i)
    {
        s_data.quadVertexBufferPtr->position     = transform * s_data.quadVertexPositions[i];
        s_data.quadVertexBufferPtr->color        = color;
        s_data.quadVertexBufferPtr->texCoord     = textureCoords[i];
        s_data.quadVertexBufferPtr->texIndex     = textureIndex;
        s_data.quadVertexBufferPtr->tilingFactor = tilingFactor;
        s_data.quadVertexBufferPtr++;
    }
    s_data.quadIndexCount += 6;
    s_data.stats.quadCount++;
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
    constexpr size_t    quadVertexCount = 4;
    constexpr glm::vec4 color           = {1.0f, 1.0f, 1.0f, 1.0f};
    constexpr glm::vec2 textureCoords[] = {{0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f}};
    if (s_data.quadIndexCount >= Renderer2DData::maxIndices)
    {
        FlushAndReset();
    }

    float textureIndex = 0.0f;
    for (uint32_t i = 1; i < s_data.textureSlotIndex; i++)
    {
        if (*s_data.textureSlots[i].get() == *texture.get())
        {
            textureIndex = (float)i;
            break;
        }
    }

    if (textureIndex == 0.0f)
    {
        textureIndex                                 = (float)s_data.textureSlotIndex;
        s_data.textureSlots[s_data.textureSlotIndex] = texture;
        s_data.textureSlotIndex++;
    }

    glm::mat4 transform = glm::translate(glm::mat4(1.0f), position) *
                          glm::rotate(glm::mat4(1.0f), glm::radians(rotation), {0.0f, 0.0f, 1.0f}) *
                          glm::scale(glm::mat4(1.0f), {size.x, size.y, 1.0f});
    for (size_t i = 0; i < quadVertexCount; ++i)
    {
        s_data.quadVertexBufferPtr->position     = transform * s_data.quadVertexPositions[i];
        s_data.quadVertexBufferPtr->color        = color;
        s_data.quadVertexBufferPtr->texCoord     = textureCoords[i];
        s_data.quadVertexBufferPtr->texIndex     = textureIndex;
        s_data.quadVertexBufferPtr->tilingFactor = tilingFactor;
        s_data.quadVertexBufferPtr++;
    }
    s_data.quadIndexCount += 6;
    s_data.stats.quadCount++;
}

void Renderer2D::ResetStats()
{
    memset(&s_data.stats, 0, sizeof(Statistics));
}

Renderer2D::Statistics Renderer2D::GetStats()
{
    return s_data.stats;
}

void Renderer2D::FlushAndReset()
{
    EndScene();
    s_data.quadIndexCount      = 0;
    s_data.quadVertexBufferPtr = s_data.quadVertexBufferBase;
    s_data.textureSlotIndex    = 1;
}
