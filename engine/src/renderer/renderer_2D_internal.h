//
// Created by dingrui on 5/26/26.
// 内部头文件 各shape文件共享
//

#pragma once

#include "renderer/batch_group.h"
#include "x/renderer/renderer_2D.h"
#include "x/renderer/shader.h"
#include "x/renderer/texture.h"
#include "x/renderer/uniform_buffer.h"

#include <glm/glm.hpp>

struct QuadVertex {
    glm::vec3 position;
    glm::vec4 color;
    glm::vec2 texCoord;
    float texIndex;
    float tilingFactor;
    int entityID;
};

struct CircleVertex {
    glm::vec3 WorldPosition;
    glm::vec3 LocalPosition;
    glm::vec4 Color;
    float Thickness;
    float Fade;
    int EntityID;
};

struct LineVertex {
    glm::vec3 Position;
    glm::vec4 Color;
    int EntityID;
};

struct Renderer2DData {
    static const uint32_t MaxTextureSlots = 16;

    // 矩形用glDrawElements的方式画 每个矩形4个顶点 画2个三角形
    BatchGroup<QuadVertex, 4> Quad;
    X::Ref<Shader> QuadShader;
    X::Ref<Texture2D> WhiteTexture;
    // 圆形用glDrawElements的方式画 每个圆形4个顶点 画2个三角形
    BatchGroup<CircleVertex, 4> Circle;
    X::Ref<Shader> CircleShader;
    // 线段用glDrawArrays的方式画 每个线段2个顶点
    BatchGroup<LineVertex, 2, false> Line;
    X::Ref<Shader> LineShader;
    float LineWidth{2.0f};

    std::array<X::Ref<Texture2D>, MaxTextureSlots> TextureSlots;
    uint32_t TextureSlotIndex = 1;

    glm::vec4 QuadVertexPositions[4];

    Renderer2D::Statistics Stats;

    struct CameraData {
        glm::mat4 ViewProjection;
    };

    CameraData CameraBuffer;
    // GPU共享的Camera常量缓冲区 里面存储ViewProjection矩阵 所有绑定到slot 0的shader都能访问
    X::Ref<UniformBuffer> CameraUniformBuffer;
};

extern Renderer2DData s_data;
