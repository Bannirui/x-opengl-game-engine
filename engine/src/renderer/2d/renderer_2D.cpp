//
// Created by dingrui on 2/28/26.
//

#include "x/renderer/renderer_2D.h"

#include "renderer_2D_internal.h"
#include "x/renderer/camera.h"
#include "x/renderer/editor_camera.h"
#include "x/renderer/render_command.h"
#include "x/renderer/shader.h"
#include "x/renderer/texture.h"
#include "x/renderer/uniform_buffer.h"

Renderer2DData s_data;

void Renderer2D::Init() {
    X_PROFILE_FUNCTION();
    s_data.Quad.Init({
        {ShaderDataType::kFloat3, "a_Position"},     // 对应shader的location=0 glsl里面的变量是a_Position
        {ShaderDataType::kFloat4, "a_Color"},        // 对应shader的location=1 glsl里面的变量是a_Color
        {ShaderDataType::kFloat2, "a_TexCoord"},     // 对应shader的location=2 glsl里面的变量是a_TexCoord
        {ShaderDataType::kInt, "a_TexIndex"},        // 对应shader的location=3 glsl里面的变量是a_TexIndex 用哪个贴图
        {ShaderDataType::kFloat, "a_TilingFactor"},  // 对应shader的location=4 glsl里面的变量是a_TilingFactor
        {ShaderDataType::kInt, "a_EntityID"},        // 对应shader的location=5 glsl里面的变量是a_EntityID
    });

    s_data.Circle.Init({
        {ShaderDataType::kFloat3, "a_WorldPosition"},
        {ShaderDataType::kFloat3, "a_LocalPosition"},
        {ShaderDataType::kFloat4, "a_Color"},
        {ShaderDataType::kFloat, "a_Thickness"},
        {ShaderDataType::kFloat, "a_Fade"},
        {ShaderDataType::kInt, "a_EntityID"},
    });

    s_data.Line.Init({
        {ShaderDataType::kFloat3, "a_Position"},
        {ShaderDataType::kFloat4, "a_Color"},
        {ShaderDataType::kInt, "a_EntityID"},
    });

    s_data.WhiteTexture = Texture2D::Create(1, 1);
    uint32_t whiteTextureData = 0xffffffff;
    s_data.WhiteTexture->SetData(&whiteTextureData, sizeof(uint32_t));

    s_data.QuadShader = Shader::Create("asset/shader/Renderer2D_Quad.glsl");
    s_data.QuadShader->Bind();
    {
        // 用uniform变量形式告诉shader采样器和纹理单元号的映射关系
        // 因为vertex attribute只会告诉shader纹理单元编号 纹理单元跟采样器一一映射 shader就知道用哪个贴图采样器了
        int samplers[Renderer2DData::MaxTextureSlots];
        for (int i = 0; i < Renderer2DData::MaxTextureSlots; i++) {
            samplers[i] = i;
        }
        s_data.QuadShader->SetIntArray("u_Textures", samplers, Renderer2DData::MaxTextureSlots);
    }
    s_data.CircleShader = Shader::Create("asset/shader/Renderer2D_Circle.glsl");
    s_data.LineShader = Shader::Create("asset/shader/Renderer2D_Line.glsl");
    // 贴图缓存区放引擎默认的贴图
    s_data.TextureSlots[0] = s_data.WhiteTexture;

    s_data.QuadVertexPositions[0] = {-0.5f, -0.5f, 0.0f, 1.0f};
    s_data.QuadVertexPositions[1] = {0.5f, -0.5f, 0.0f, 1.0f};
    s_data.QuadVertexPositions[2] = {0.5f, 0.5f, 0.0f, 1.0f};
    s_data.QuadVertexPositions[3] = {-0.5f, 0.5f, 0.0f, 1.0f};
    // 在GPU显存上创建一个UBO binding=0
    s_data.CameraUniformBuffer = UniformBuffer::Create(sizeof(Renderer2DData::CameraData), 0);
}

void Renderer2D::Shutdown() {
    X_PROFILE_FUNCTION();
    s_data.Quad.Shutdown();
    s_data.Circle.Shutdown();
    s_data.Line.Shutdown();
    s_data.QuadShader.reset();
    s_data.CircleShader.reset();
    s_data.LineShader.reset();
    s_data.WhiteTexture.reset();
    s_data.CameraUniformBuffer.reset();
}

void Renderer2D::BeginScene(const OrthographicCamera& camera) {
    X_PROFILE_FUNCTION();
    s_data.CameraBuffer.ViewProjection = camera.get_viewProjectionMatrix();
    s_data.CameraUniformBuffer->SetData(&s_data.CameraBuffer, sizeof(Renderer2DData::CameraData));
    startBatch();
}

void Renderer2D::BeginScene(const Camera& camera, const glm::mat4& transform) {
    X_PROFILE_FUNCTION();
    s_data.CameraBuffer.ViewProjection = camera.get_projection() * glm::inverse(transform);
    s_data.CameraUniformBuffer->SetData(&s_data.CameraBuffer, sizeof(Renderer2DData::CameraData));
    startBatch();
}

void Renderer2D::BeginScene(const EditorCamera& camera) {
    X_PROFILE_FUNCTION();
    s_data.CameraBuffer.ViewProjection = camera.GetViewProjection();
    s_data.CameraUniformBuffer->SetData(&s_data.CameraBuffer, sizeof(Renderer2DData::CameraData));
    startBatch();
}

void Renderer2D::EndScene() {
    X_PROFILE_FUNCTION();
    Flush();
}

/**
 * 每绘制一帧 CPU一次性把要画的所有图形的数据一次性一起告诉GPU
 */
void Renderer2D::Flush() {
    // 告诉GPU绘制有矩形要绘制 用的是DrawElements
    if (s_data.Quad.Count) {
        // 把顶点数据灌给GPU的显存
        s_data.Quad.VBO->SetData(s_data.Quad.Base, s_data.Quad.GetDataSize());
        for (uint32_t i = 0; i < s_data.TextureSlotIndex; i++) {
            // 把贴图缓冲区里面的贴图纹理单元都激活 shader程序真正用哪个会用vertex attribute的方式传进去贴图缓冲区的脚标
            s_data.TextureSlots[i]->Bind(i);
        }
        s_data.QuadShader->Bind();
        // 告诉GPU怎么取这些VBO顶点
        RenderCommand::DrawIndexed(s_data.Quad.VAO, s_data.Quad.Count);
        s_data.Stats.DrawCalls++;
    }
    // 告诉GPU绘制有圆形要绘制 用的是DrawElements
    if (s_data.Circle.Count) {
        s_data.Circle.VBO->SetData(s_data.Circle.Base, s_data.Circle.GetDataSize());
        s_data.CircleShader->Bind();
        RenderCommand::DrawIndexed(s_data.Circle.VAO, s_data.Circle.Count);
        s_data.Stats.DrawCalls++;
    }
    // 告诉GPU绘制有线段要绘制 用的是DrawArrays
    if (s_data.Line.Count) {
        s_data.Line.VBO->SetData(s_data.Line.Base, s_data.Line.GetDataSize());
        s_data.LineShader->Bind();
        RenderCommand::DrawLines(s_data.Line.VAO, s_data.Line.Count);
        s_data.Stats.DrawCalls++;
    }
}

void Renderer2D::ResetStats() {
    s_data.Stats = {};
}

Renderer2D::Statistics Renderer2D::GetStats() {
    return s_data.Stats;
}

void Renderer2D::startBatch() {
    s_data.Quad.StartBatch();
    s_data.Circle.StartBatch();
    s_data.Line.StartBatch();

    s_data.TextureSlotIndex = 1;
}

void Renderer2D::nextBatch() {
    Flush();
    startBatch();
}
