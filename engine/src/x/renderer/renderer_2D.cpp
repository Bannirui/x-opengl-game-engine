//
// Created by dingrui on 2/28/26.
//

#include "x/renderer/renderer_2D.h"

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
#include "x/scene/entity.h"

#include <glm/ext/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>

/**
 * 批次化渲染组 管理一种图形的完整渲染管线
 *   - Quad
 *   - Circle
 *   ...
 * GPU侧VAO+VBO+IndexBuffer
 *   - VAO 绑定VBO和IBO的布局描述
 *   - VBO 显存中的顶点缓冲区 每帧Flush时上传
 *   - IndexBuffer 索引缓冲区 内容固定为重复的0-1-2-2-3-0 Init时生成一次
 *
 * CPU侧Base+Ptr+IndexCount
 *   - Base CPU 端顶点数组 堆内存 预分配MaxVertices个
 *   - Ptr 当前写入位置的游标 每写入一个顶点Ptr++
 *   - IndexCount 当前批次累积了多少个索引 供Flush用
 *
 * Flush时 Base到Ptr之间的[Base, Ptr)就是本轮要上传的顶点数据
 * StartBatch时 Ptr回到Base IndexCount清零
 *
 * 使用方式
 *   - BatchGroup<QuadVertex> quad;
 *   - quad.Init({ {kFloat3, "a_Pos"}, ... });      // 创建 VAO+VBO+IBO，分配 CPU 缓冲区
 *   - quad.Ptr->field = value; quad.Ptr++;          // 写入顶点
 *   - quad.IndexCount += 6;                         // 每个 Quad 占 4 顶点 6 索引
 *   - if (quad.IsFull()) nextBatch();               // 批次满则先 Flush
 */
template <typename VertexType>
struct BatchGroup {
    uint32_t MaxQuads = 20000;

    uint32_t MaxVertices() const { return MaxQuads * 4; }
    uint32_t MaxIndices() const { return MaxQuads * 6; }

    X::Ref<VertexArray>  VAO;
    X::Ref<VertexBuffer> VBO;
    VertexType*          Base{nullptr};
    VertexType*          Ptr{nullptr};
    uint32_t             IndexCount{0};

    void Init(const BufferLayout& layout) {
        VAO = VertexArray::Create();
        VBO = VertexBuffer::Create(MaxVertices() * sizeof(VertexType));
        VBO->SetLayout(layout);
        VAO->AddVertexBuffer(VBO);
        Base = new VertexType[MaxVertices()];

        auto indices = std::make_unique<uint32_t[]>(MaxIndices());
        uint32_t offset = 0;
        for (uint32_t i = 0; i < MaxIndices(); i += 6) {
            indices[i + 0] = offset + 0;
            indices[i + 1] = offset + 1;
            indices[i + 2] = offset + 2;
            indices[i + 3] = offset + 2;
            indices[i + 4] = offset + 3;
            indices[i + 5] = offset + 0;
            offset += 4;
        }
        auto ib = IndexBuffer::Create(indices.get(), MaxIndices());
        VAO->SetIndexBuffer(ib);

        Ptr = Base;
        IndexCount = 0;
    }

    void Shutdown() {
        delete[] Base;
        VAO.reset();
        VBO.reset();
    }

    void StartBatch() {
        Ptr = Base;
        IndexCount = 0;
    }

    bool IsFull() const { return IndexCount >= MaxIndices(); }

    uint32_t GetDataSize() const {
        return static_cast<uint32_t>(reinterpret_cast<uint8_t*>(Ptr) - reinterpret_cast<uint8_t*>(Base));
    }
};

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

struct Renderer2DData {
    static const uint32_t MaxTextureSlots = 16;

    BatchGroup<QuadVertex>   Quad;
    X::Ref<Shader>           QuadShader;
    X::Ref<Texture2D>        WhiteTexture;

    BatchGroup<CircleVertex> Circle;
    X::Ref<Shader>           CircleShader;

    X::Ref<VertexArray>  LineVertexArray;
    X::Ref<VertexBuffer> LineVertexBuffer;
    X::Ref<Shader>       LineShader;
    LineVertex*          LineVertexBufferBase{nullptr};
    LineVertex*          LineVertexBufferPtr{nullptr};
    uint32_t             LineVertexCount{0};
    float                LineWidth{2.0f};

    std::array<X::Ref<Texture2D>, MaxTextureSlots> TextureSlots;
    uint32_t                                       TextureSlotIndex = 1;

    glm::vec4 QuadVertexPositions[4];

    Renderer2D::Statistics Stats;

    struct CameraData {
        glm::mat4 ViewProjection;
    };
    CameraData            CameraBuffer;
    X::Ref<UniformBuffer> CameraUniformBuffer;
};

static Renderer2DData s_data;

void Renderer2D::Init() {
    X_PROFILE_FUNCTION();
    s_data.Quad.Init({
        {ShaderDataType::kFloat3, "a_Position"},
        {ShaderDataType::kFloat4, "a_Color"},
        {ShaderDataType::kFloat2, "a_TexCoord"},
        {ShaderDataType::kFloat, "a_TexIndex"},
        {ShaderDataType::kFloat, "a_TilingFactor"},
        {ShaderDataType::kInt, "a_EntityID"},
    });

    s_data.Circle.Init({
        {ShaderDataType::kFloat3, "a_WorldPosition"},
        {ShaderDataType::kFloat3, "a_LocalPosition"},
        {ShaderDataType::kFloat4, "a_Color"},
        {ShaderDataType::kFloat, "a_Thickness"},
        {ShaderDataType::kFloat, "a_Fade"},
        {ShaderDataType::kInt, "a_EntityID"},
    });

    s_data.LineVertexArray = VertexArray::Create();
    s_data.LineVertexBuffer = VertexBuffer::Create(s_data.Quad.MaxVertices() * sizeof(LineVertex));
    s_data.LineVertexBuffer->SetLayout({{ShaderDataType::kFloat3, "a_Position"},
                                        {ShaderDataType::kFloat4, "a_Color"},
                                        {ShaderDataType::kInt, "a_EntityID"}});
    s_data.LineVertexArray->AddVertexBuffer(s_data.LineVertexBuffer);
    s_data.LineVertexBufferBase = new LineVertex[s_data.Quad.MaxVertices()];

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

void Renderer2D::Shutdown() {
    X_PROFILE_FUNCTION();
    s_data.Quad.Shutdown();
    s_data.Circle.Shutdown();
    s_data.QuadShader.reset();
    s_data.CircleShader.reset();
    s_data.LineShader.reset();
    s_data.WhiteTexture.reset();
    s_data.CameraUniformBuffer.reset();
    delete[] s_data.LineVertexBufferBase;
    s_data.LineVertexArray.reset();
    s_data.LineVertexBuffer.reset();
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

void Renderer2D::Flush() {
    if (s_data.Quad.IndexCount) {
        s_data.Quad.VBO->SetData(s_data.Quad.Base, s_data.Quad.GetDataSize());
        for (uint32_t i = 0; i < s_data.TextureSlotIndex; i++) {
            s_data.TextureSlots[i]->Bind(i);
        }
        s_data.QuadShader->Bind();
        RenderCommand::DrawIndexed(s_data.Quad.VAO, s_data.Quad.IndexCount);
        s_data.Stats.DrawCalls++;
    }
    if (s_data.Circle.IndexCount) {
        s_data.Circle.VBO->SetData(s_data.Circle.Base, s_data.Circle.GetDataSize());
        s_data.CircleShader->Bind();
        RenderCommand::DrawIndexed(s_data.Circle.VAO, s_data.Circle.IndexCount);
        s_data.Stats.DrawCalls++;
    }
    if (s_data.LineVertexCount) {
        uint32_t dataSize = static_cast<uint32_t>(reinterpret_cast<uint8_t*>(s_data.LineVertexBufferPtr) -
                                                  reinterpret_cast<uint8_t*>(s_data.LineVertexBufferBase));
        s_data.LineVertexBuffer->SetData(s_data.LineVertexBufferBase, dataSize);
        s_data.LineShader->Bind();
        RenderCommand::DrawLines(s_data.LineVertexArray, s_data.LineVertexCount);
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
    if (s_data.Quad.IsFull()) {
        nextBatch();
    }
    for (size_t i = 0; i < quadVertexCount; i++)
    {
        s_data.Quad.Ptr->position     = transform * s_data.QuadVertexPositions[i];
        s_data.Quad.Ptr->color        = color;
        s_data.Quad.Ptr->texCoord     = textureCoords[i];
        s_data.Quad.Ptr->texIndex     = textureIndex;
        s_data.Quad.Ptr->tilingFactor = tilingFactor;
        s_data.Quad.Ptr->entityID     = entityID;
        s_data.Quad.Ptr++;
    }
    s_data.Quad.IndexCount += 6;
    s_data.Stats.QuadCount++;
}

void Renderer2D::DrawQuad(const glm::mat4& transform, const X::Ref<Texture2D>& texture, float tilingFactor,
                          const glm::vec4& tintColor, int entityID)
{
    X_PROFILE_FUNCTION();

    constexpr size_t    quadVertexCount = 4;
    constexpr glm::vec2 textureCoords[] = {{0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f}};
    if (s_data.Quad.IsFull()) {
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
        if (s_data.TextureSlotIndex >= Renderer2DData::MaxTextureSlots) {
            nextBatch();
        }
        textureIndex                                 = (float)s_data.TextureSlotIndex;
        s_data.TextureSlots[s_data.TextureSlotIndex] = texture;
        s_data.TextureSlotIndex++;
    }
    for (size_t i = 0; i < quadVertexCount; i++)
    {
        s_data.Quad.Ptr->position     = transform * s_data.QuadVertexPositions[i];
        s_data.Quad.Ptr->color        = tintColor;
        s_data.Quad.Ptr->texCoord     = textureCoords[i];
        s_data.Quad.Ptr->texIndex     = textureIndex;
        s_data.Quad.Ptr->tilingFactor = tilingFactor;
        s_data.Quad.Ptr->entityID     = entityID;
        s_data.Quad.Ptr++;
    }
    s_data.Quad.IndexCount += 6;
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
        s_data.Circle.Ptr->WorldPosition = transform * s_data.QuadVertexPositions[i];
        s_data.Circle.Ptr->LocalPosition = s_data.QuadVertexPositions[i] * 2.0f;
        s_data.Circle.Ptr->Color         = color;
        s_data.Circle.Ptr->Thickness     = thickness;
        s_data.Circle.Ptr->Fade          = fade;
        s_data.Circle.Ptr->EntityID      = entityID;
        s_data.Circle.Ptr++;
    }
    s_data.Circle.IndexCount += 6;
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
    DrawLine(p0, p1, color, entityID);
    DrawLine(p1, p2, color, entityID);
    DrawLine(p2, p3, color, entityID);
    DrawLine(p3, p0, color, entityID);
}

void Renderer2D::DrawRect(const glm::mat4& transform, const glm::vec4& color, int entityID)
{
    glm::vec3 lineVertices[4];
    for (size_t i = 0; i < 4; ++i)
    {
        lineVertices[i] = transform * s_data.QuadVertexPositions[i];
    }
    DrawLine(lineVertices[0], lineVertices[1], color, entityID);
    DrawLine(lineVertices[1], lineVertices[2], color, entityID);
    DrawLine(lineVertices[2], lineVertices[3], color, entityID);
    DrawLine(lineVertices[3], lineVertices[0], color, entityID);
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

void Renderer2D::startBatch() {
    s_data.Quad.StartBatch();
    s_data.Circle.StartBatch();

    s_data.LineVertexCount      = 0;
    s_data.LineVertexBufferPtr = s_data.LineVertexBufferBase;

    s_data.TextureSlotIndex = 1;
}

void Renderer2D::nextBatch()
{
    Flush();
    startBatch();
}
