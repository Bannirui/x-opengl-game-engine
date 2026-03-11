//
// Created by dingrui on 2/28/26.
//

#include "x/renderer/renderer_2D.h"

#include <glm/ext/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>

#include "x/core/base.h"
#include "x/renderer/buffer.h"
#include "x/renderer/camera.h"
#include "x/renderer/editor_camera.h"
#include "x/renderer/render_command.h"
#include "x/renderer/shader.h"
#include "x/renderer/texture.h"
#include "x/renderer/uniform_buffer.h"
#include "x/renderer/vertex_array.h"
#include "x/scene/component.h"

// VBO 正方形顶点
struct QuadVertex
{
    glm::vec3 position;  // pos坐标
    glm::vec4 color;     // rgba颜色
    glm::vec2 texCoord;  // 纹理uv
    float     texIndex;  // 用哪个texture纹理采样
    float     tilingFactor;
    int       entityID;
};

struct CircleVertex
{
    glm::vec3 WorldPosition;
    glm::vec3 LocalPosition;
    glm::vec4 Color;
    float     Thickness;
    float     Fade;
    int       EntityID;
};

struct LineVertex
{
    glm::vec3 Position;
    glm::vec4 Color;

    int EntityID;
};

struct Renderer2DData
{
    static const uint32_t MaxQuads        = 20000;
    static const uint32_t MaxVertices     = MaxQuads * 4;
    static const uint32_t maxIndices      = MaxQuads * 6;
    static const uint32_t MaxTextureSlots = 16;  // OpenGL3.3最多只支持16个槽

    X::Ref<VertexArray>  QuadVertexArray;   // EAO
    X::Ref<VertexBuffer> QuadVertexBuffer;  // VBO
    X::Ref<Shader>       QuadShader;
    X::Ref<Texture2D>    WhiteTexture;

    X::Ref<VertexArray>  CircleVertexArray;   // EAO
    X::Ref<VertexBuffer> CircleVertexBuffer;  // VBO
    X::Ref<Shader>       CircleShader;

    X::Ref<VertexArray>  LineVertexArray;
    X::Ref<VertexBuffer> LineVertexBuffer;
    X::Ref<Shader>       LineShader;

    uint32_t    QuadIndexCount{0};
    QuadVertex* QuadVertexBufferBase{nullptr};
    QuadVertex* QuadVertexBufferPtr{nullptr};

    uint32_t      CircleIndexCount{0};
    CircleVertex* CircleVertexBufferBase{nullptr};
    CircleVertex* CircleVertexBufferPtr{nullptr};

    uint32_t    LineIndexCount{0};
    LineVertex* LineVertexBufferBase{nullptr};
    LineVertex* LineVertexBufferPtr{nullptr};
    float       LineWidth{2.0f};
    uint32_t    LineVertexCount{0};

    std::array<X::Ref<Texture2D>, MaxTextureSlots> TextureSlots;          // 纹理采样对象
    uint32_t                                       TextureSlotIndex = 1;  // 0 = white texture

    glm::vec4 QuadVertexPositions[4];

    Renderer2D::Statistics Stats;

    struct CameraData
    {
        glm::mat4 ViewProjection;
    };

    CameraData            CameraBuffer;
    X::Ref<UniformBuffer> CameraUniformBuffer;
};

static Renderer2DData s_data;

void Renderer2D::Init()
{
    X_PROFILE_FUNCTION();
    // --- Quad start ---
    s_data.QuadVertexArray  = VertexArray::Create();
    s_data.QuadVertexBuffer = VertexBuffer::Create(s_data.MaxVertices * sizeof(QuadVertex));
    s_data.QuadVertexBuffer->SetLayout({
        {ShaderDataType::kFloat3, "a_Position"},
        {ShaderDataType::kFloat4, "a_Color"},
        {ShaderDataType::kFloat2, "a_TexCoord"},
        {ShaderDataType::kFloat, "a_TexIndex"},
        {ShaderDataType::kFloat, "a_TilingFactor"},
        {ShaderDataType::kInt, "a_EntityID"},
    });
    s_data.QuadVertexArray->AddVertexBuffer(s_data.QuadVertexBuffer);

    s_data.QuadVertexBufferBase = new QuadVertex[s_data.MaxVertices];

    uint32_t* quadIndices = new uint32_t[s_data.maxIndices];

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
    s_data.QuadVertexArray->SetIndexBuffer(quadIB);
    delete[] quadIndices;
    // --- Quad end ---

    // --- Circle start ---
    s_data.CircleVertexArray  = VertexArray::Create();
    s_data.CircleVertexBuffer = VertexBuffer::Create(s_data.MaxVertices * sizeof(CircleVertex));
    s_data.CircleVertexBuffer->SetLayout({{ShaderDataType::kFloat3, "a_WorldPosition"},
                                          {ShaderDataType::kFloat3, "a_LocalPosition"},
                                          {ShaderDataType::kFloat4, "a_Color"},
                                          {ShaderDataType::kFloat, "a_Thickness"},
                                          {ShaderDataType::kFloat, "a_Fade"},
                                          {ShaderDataType::kInt, "a_EntityID"}});
    s_data.CircleVertexArray->AddVertexBuffer(s_data.CircleVertexBuffer);
    s_data.CircleVertexArray->SetIndexBuffer(quadIB);  // Use quad IB
    s_data.CircleVertexBufferBase = new CircleVertex[s_data.MaxVertices];
    // --- Circle end ---

    // --- Line start ---
    s_data.LineVertexArray  = VertexArray::Create();
    s_data.LineVertexBuffer = VertexBuffer::Create(s_data.MaxVertices * sizeof(LineVertex));
    s_data.LineVertexBuffer->SetLayout({{ShaderDataType::kFloat3, "a_Position"},
                                        {ShaderDataType::kFloat4, "a_Color"},
                                        {ShaderDataType::kInt, "a_EntityID"}});
    s_data.LineVertexArray->AddVertexBuffer(s_data.LineVertexBuffer);
    s_data.LineVertexBufferBase = new LineVertex[s_data.MaxVertices];
    // --- Line end ---

    s_data.WhiteTexture       = Texture2D::Create(1, 1);
    uint32_t whiteTextureData = 0xffffffff;
    s_data.WhiteTexture->SetData(&whiteTextureData, sizeof(uint32_t));

    // int32_t samplers[s_data.MaxTextureSlots];
    // for (uint32_t i = 0; i < s_data.MaxTextureSlots; i++)
    // {
    //     samplers[i] = i;
    // }

    s_data.QuadShader   = Shader::Create("asset/shader/Renderer2D_Quad.glsl");
    s_data.CircleShader = Shader::Create("asset/shader/Renderer2D_Circle.glsl");
    s_data.LineShader   = Shader::Create("asset/shader/Renderer2D_Line.glsl");

    // Set first texture slot to 0
    s_data.TextureSlots[0] = s_data.WhiteTexture;

    s_data.QuadVertexPositions[0] = {-0.5f, -0.5f, 0.0f, 1.0f};
    s_data.QuadVertexPositions[1] = {0.5f, -0.5f, 0.0f, 1.0f};
    s_data.QuadVertexPositions[2] = {0.5f, 0.5f, 0.0f, 1.0f};
    s_data.QuadVertexPositions[3] = {-0.5f, 0.5f, 0.0f, 1.0f};

    s_data.CameraUniformBuffer = UniformBuffer::Create(sizeof(Renderer2DData::CameraData), 0);
}

void Renderer2D::Shutdown()
{
    X_PROFILE_FUNCTION();
    delete[] s_data.QuadVertexBufferBase;
}

void Renderer2D::BeginScene(const OrthographicCamera& camera)
{
    X_PROFILE_FUNCTION();
    s_data.CameraBuffer.ViewProjection = camera.get_viewProjectionMatrix();
    s_data.CameraUniformBuffer->SetData(&s_data.CameraBuffer, sizeof(Renderer2DData::CameraData));
    startBatch();
}

void Renderer2D::BeginScene(const Camera& camera, const glm::mat4& transform)
{
    X_PROFILE_FUNCTION();

    s_data.CameraBuffer.ViewProjection = camera.get_projection() * glm::inverse(transform);
    s_data.CameraUniformBuffer->SetData(&s_data.CameraBuffer, sizeof(Renderer2DData::CameraData));
    startBatch();
}

void Renderer2D::BeginScene(const EditorCamera& camera)
{
    X_PROFILE_FUNCTION();

    s_data.CameraBuffer.ViewProjection = camera.GetViewProjection();
    s_data.CameraUniformBuffer->SetData(&s_data.CameraBuffer, sizeof(Renderer2DData::CameraData));
    startBatch();
}

void Renderer2D::EndScene()
{
    X_PROFILE_FUNCTION();
    Flush();
}

void Renderer2D::Flush()
{
    if (s_data.QuadIndexCount)
    {
        uint32_t dataSize = static_cast<uint32_t>(reinterpret_cast<uint8_t*>(s_data.QuadVertexBufferPtr) -
                                                  reinterpret_cast<uint8_t*>(s_data.QuadVertexBufferBase));
        s_data.QuadVertexBuffer->SetData(s_data.QuadVertexBufferBase, dataSize);
        // Bind textures
        for (uint32_t i = 0; i < s_data.TextureSlotIndex; i++)
        {
            s_data.TextureSlots[i]->Bind(i);
        }
        s_data.QuadShader->Bind();
        RenderCommand::DrawIndexed(s_data.QuadVertexArray, s_data.QuadIndexCount);
        s_data.Stats.DrawCalls++;
    }
    if (s_data.CircleIndexCount)
    {
        uint32_t dataSize = static_cast<uint32_t>(reinterpret_cast<uint8_t*>(s_data.CircleVertexBufferPtr) -
                                                  reinterpret_cast<uint8_t*>(s_data.CircleVertexBufferBase));
        s_data.CircleVertexBuffer->SetData(s_data.CircleVertexBufferBase, dataSize);
        s_data.CircleShader->Bind();
        RenderCommand::DrawIndexed(s_data.CircleVertexArray, s_data.CircleIndexCount);
        s_data.Stats.DrawCalls++;
    }
    if (s_data.LineIndexCount)
    {
        uint32_t dataSize = static_cast<uint32_t>(reinterpret_cast<uint8_t*>(s_data.LineVertexBufferPtr) -
                                                  reinterpret_cast<uint8_t*>(s_data.LineVertexBufferBase));
        s_data.LineVertexBuffer->SetData(s_data.LineVertexBufferBase, dataSize);
        s_data.LineShader->Bind();
        RenderCommand::DrawLines(s_data.LineVertexArray, s_data.LineIndexCount);
        s_data.Stats.DrawCalls++;
    }
}

void Renderer2D::DrawQuad(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color)
{
    DrawQuad({position.x, position.y, 0.0f}, size, color);
}

void Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec2& size, const glm::vec4& color)
{
    X_PROFILE_FUNCTION();
    glm::mat4 transform =
        glm::translate(glm::mat4(1.0f), position) * glm::scale(glm::mat4(1.0f), {size.x, size.y, 1.0f});
    DrawQuad(transform, color);
}

void Renderer2D::DrawQuad(const glm::vec2& position, const glm::vec2& size, const X::Ref<Texture2D>& texture,
                          float tilingFactor, const glm::vec4& tintColor)
{
    DrawQuad({position.x, position.y, 0.0f}, size, texture, tilingFactor, tintColor);
}

void Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec2& size, const X::Ref<Texture2D>& texture,
                          float tilingFactor, const glm::vec4& tintColor)
{
    X_PROFILE_FUNCTION();
    glm::mat4 transform =
        glm::translate(glm::mat4(1.0f), position) * glm::scale(glm::mat4(1.0f), {size.x, size.y, 1.0f});
    DrawQuad(transform, texture, tilingFactor, tintColor);
}

void Renderer2D::DrawQuad(const glm::mat4& transform, const glm::vec4& color, int entityID)
{
    X_PROFILE_FUNCTION();

    constexpr size_t    quadVertexCount = 4;
    const float         textureIndex    = 0.0f;  // White Texture
    constexpr glm::vec2 textureCoords[] = {{0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f}};
    const float         tilingFactor    = 1.0f;
    if (s_data.QuadIndexCount >= Renderer2DData::maxIndices)
    {
        nextBatch();
    }
    for (size_t i = 0; i < quadVertexCount; i++)
    {
        s_data.QuadVertexBufferPtr->position     = transform * s_data.QuadVertexPositions[i];
        s_data.QuadVertexBufferPtr->color        = color;
        s_data.QuadVertexBufferPtr->texCoord     = textureCoords[i];
        s_data.QuadVertexBufferPtr->texIndex     = textureIndex;
        s_data.QuadVertexBufferPtr->tilingFactor = tilingFactor;
        s_data.QuadVertexBufferPtr->entityID     = entityID;
        s_data.QuadVertexBufferPtr++;
    }
    s_data.QuadIndexCount += 6;
    s_data.Stats.QuadCount++;
}

void Renderer2D::DrawQuad(const glm::mat4& transform, const X::Ref<Texture2D>& texture, float tilingFactor,
                          const glm::vec4& tintColor, int entityID)
{
    X_PROFILE_FUNCTION();

    constexpr size_t    quadVertexCount = 4;
    constexpr glm::vec2 textureCoords[] = {{0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f}};
    if (s_data.QuadIndexCount >= Renderer2DData::maxIndices)
    {
        nextBatch();
    }

    float textureIndex = 0.0f;
    for (uint32_t i = 1; i < s_data.TextureSlotIndex; i++)
    {
        if (*s_data.TextureSlots[i] == *texture)
        {
            textureIndex = (float)i;
            break;
        }
    }
    if (textureIndex == 0.0f)
    {
        if (s_data.TextureSlotIndex >= Renderer2DData::MaxTextureSlots)
        {
            nextBatch();
        }
        textureIndex                                 = (float)s_data.TextureSlotIndex;
        s_data.TextureSlots[s_data.TextureSlotIndex] = texture;
        s_data.TextureSlotIndex++;
    }
    for (size_t i = 0; i < quadVertexCount; i++)
    {
        s_data.QuadVertexBufferPtr->position     = transform * s_data.QuadVertexPositions[i];
        s_data.QuadVertexBufferPtr->color        = tintColor;
        s_data.QuadVertexBufferPtr->texCoord     = textureCoords[i];
        s_data.QuadVertexBufferPtr->texIndex     = textureIndex;
        s_data.QuadVertexBufferPtr->tilingFactor = tilingFactor;
        s_data.QuadVertexBufferPtr->entityID     = entityID;
        s_data.QuadVertexBufferPtr++;
    }
    s_data.QuadIndexCount += 6;
    s_data.Stats.QuadCount++;
}

void Renderer2D::DrawRotatedQuad(const glm::vec2& position, const glm::vec2& size, float rotation,
                                 const glm::vec4& color)
{
    DrawRotatedQuad({position.x, position.y, 0.0f}, size, rotation, color);
}

void Renderer2D::DrawRotatedQuad(const glm::vec3& position, const glm::vec2& size, float rotation,
                                 const glm::vec4& color)
{
    X_PROFILE_FUNCTION();
    glm::mat4 transform = glm::translate(glm::mat4(1.0f), position) *
                          glm::rotate(glm::mat4(1.0f), glm::radians(rotation), {0.0f, 0.0f, 1.0f}) *
                          glm::scale(glm::mat4(1.0f), {size.x, size.y, 1.0f});
    DrawQuad(transform, color);
}

void Renderer2D::DrawRotatedQuad(const glm::vec2& position, const glm::vec2& size, float rotation,
                                 const X::Ref<Texture2D>& texture, float tilingFactor, const glm::vec4& tintColor)
{
    DrawRotatedQuad({position.x, position.y, 0.0f}, size, rotation, texture, tilingFactor, tintColor);
}

void Renderer2D::DrawRotatedQuad(const glm::vec3& position, const glm::vec2& size, float rotation,
                                 const X::Ref<Texture2D>& texture, float tilingFactor, const glm::vec4& tintColor)
{
    X_PROFILE_FUNCTION();
    glm::mat4 transform = glm::translate(glm::mat4(1.0f), position) *
                          glm::rotate(glm::mat4(1.0f), glm::radians(rotation), {0.0f, 0.0f, 1.0f}) *
                          glm::scale(glm::mat4(1.0f), {size.x, size.y, 1.0f});
    DrawQuad(transform, texture, tilingFactor, tintColor);
}

void Renderer2D::DrawCircle(const glm::mat4& transform, const glm::vec4& color, float thickness, float fade,
                            int entityID)
{
    X_PROFILE_FUNCTION();
    for (size_t i = 0; i < 4; i++)
    {
        s_data.CircleVertexBufferPtr->WorldPosition = transform * s_data.QuadVertexPositions[i];
        s_data.CircleVertexBufferPtr->LocalPosition = s_data.QuadVertexPositions[i] * 2.0f;
        s_data.CircleVertexBufferPtr->Color         = color;
        s_data.CircleVertexBufferPtr->Thickness     = thickness;
        s_data.CircleVertexBufferPtr->Fade          = fade;
        s_data.CircleVertexBufferPtr->EntityID      = entityID;
        s_data.CircleVertexBufferPtr++;
    }
    s_data.CircleIndexCount += 6;
    s_data.Stats.QuadCount++;
}

void Renderer2D::DrawLine(const glm::vec3& p0, glm::vec3& p1, const glm::vec4& color, int entityID)
{
    s_data.LineVertexBufferPtr->Position = p0;
    s_data.LineVertexBufferPtr->Color    = color;
    s_data.LineVertexBufferPtr->EntityID = entityID;
    s_data.LineVertexBufferPtr++;

    s_data.LineVertexBufferPtr->Position = p1;
    s_data.LineVertexBufferPtr->Color    = color;
    s_data.LineVertexBufferPtr->EntityID = entityID;
    s_data.LineVertexBufferPtr++;

    s_data.LineVertexCount += 2;
}

void Renderer2D::DrawRect(const glm::vec3& position, const glm::vec2& size, const glm::vec4& color, int entityID)
{
    glm::vec3 p0 = glm::vec3(position.x - size.x * 0.5f, position.y - size.y * 0.5f, position.z);
    glm::vec3 p1 = glm::vec3(position.x + size.x * 0.5f, position.y - size.y * 0.5f, position.z);
    glm::vec3 p2 = glm::vec3(position.x + size.x * 0.5f, position.y + size.y * 0.5f, position.z);
    glm::vec3 p3 = glm::vec3(position.x - size.x * 0.5f, position.y + size.y * 0.5f, position.z);
    DrawLine(p0, p1, color);
    DrawLine(p1, p2, color);
    DrawLine(p2, p3, color);
    DrawLine(p3, p0, color);
}

void Renderer2D::DrawRect(const glm::mat4& transform, const glm::vec4& color, int entityID)
{
    glm::vec3 lineVertices[4];
    for (size_t i = 0; i < 4; ++i)
    {
        lineVertices[i] = transform * s_data.QuadVertexPositions[i];
    }
    DrawLine(lineVertices[0], lineVertices[1], color);
    DrawLine(lineVertices[1], lineVertices[2], color);
    DrawLine(lineVertices[2], lineVertices[3], color);
    DrawLine(lineVertices[3], lineVertices[0], color);
}

void Renderer2D::DrawSprite(const glm::mat4& transform, SpriteRendererComponent& src, int entityID)
{
    if (src.Texture)
    {
        DrawQuad(transform, src.Texture, src.TilingFactor, src.Color, entityID);
    }
    else
    {
        DrawQuad(transform, src.Color, entityID);
    }
}

float Renderer2D::GetLineWidth()
{
    return s_data.LineWidth;
}

void Renderer2D::SetLineWidth(float width)
{
    s_data.LineWidth = width;
}

void Renderer2D::ResetStats()
{
    s_data.Stats = {};
}

Renderer2D::Statistics Renderer2D::GetStats()
{
    return s_data.Stats;
}

void Renderer2D::startBatch()
{
    s_data.QuadIndexCount      = 0;
    s_data.QuadVertexBufferPtr = s_data.QuadVertexBufferBase;

    s_data.CircleIndexCount      = 0;
    s_data.CircleVertexBufferPtr = s_data.CircleVertexBufferBase;

    s_data.LineIndexCount      = 0;
    s_data.LineVertexBufferPtr = s_data.LineVertexBufferBase;

    s_data.TextureSlotIndex = 1;
}

void Renderer2D::nextBatch()
{
    Flush();
    startBatch();
}
