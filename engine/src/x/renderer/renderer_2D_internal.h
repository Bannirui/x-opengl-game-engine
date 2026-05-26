//
// Created by dingrui on 5/26/26.
// 内部头文件 各shape文件共享
//

#pragma once

#include "x/renderer/batch_group.h"
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

    BatchGroup<QuadVertex> Quad;
    X::Ref<Shader> QuadShader;
    X::Ref<Texture2D> WhiteTexture;

    BatchGroup<CircleVertex> Circle;
    X::Ref<Shader> CircleShader;

    X::Ref<VertexArray> LineVertexArray;
    X::Ref<VertexBuffer> LineVertexBuffer;
    X::Ref<Shader> LineShader;
    LineVertex* LineVertexBufferBase{nullptr};
    LineVertex* LineVertexBufferPtr{nullptr};
    uint32_t LineVertexCount{0};
    float LineWidth{2.0f};

    std::array<X::Ref<Texture2D>, MaxTextureSlots> TextureSlots;
    uint32_t TextureSlotIndex = 1;

    glm::vec4 QuadVertexPositions[4];

    Renderer2D::Statistics Stats;

    struct CameraData {
        glm::mat4 ViewProjection;
    };

    CameraData CameraBuffer;
    X::Ref<UniformBuffer> CameraUniformBuffer;
};

extern Renderer2DData s_data;
