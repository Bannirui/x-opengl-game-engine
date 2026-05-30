//
// Created by dingrui on 5/26/26.
// 内部头文件 各shape文件共享
//

#pragma once

#include "renderer/buffer/batch_group.h"
#include "x/renderer/renderer_2D.h"
#include "x/renderer/shader.h"
#include "x/renderer/texture.h"
#include "x/renderer/uniform_buffer.h"

#include <glm/glm.hpp>

// 矩形顶点的attribute
struct QuadVertex {
    // 顶点要平移多少
    glm::vec3 position;
    // 颜色
    glm::vec4 color;
    // 贴图uv坐标
    glm::vec2 texCoord;
    // shader采样器用贴图缓冲区的哪个贴图
    int texIndex;
    // 贴图因子 uv坐标值区间[0...1] 假设因子是n 那么现在uv区间就是[0...n] 当uv超出1后触发OpenGL策略 现在设置的是repeat 就会重复n个贴图
    float tilingFactor;
    int entityID;
};

// 圆形顶点的attribute
struct CircleVertex {
    glm::vec3 WorldPosition;
    glm::vec3 LocalPosition;
    glm::vec4 Color;
    float Thickness;
    float Fade;
    int EntityID;
};

// 线段顶点的attribute
struct LineVertex {
    glm::vec3 Position;
    glm::vec4 Color;
    int EntityID;
};

struct Renderer2DData {
    constexpr static int MaxTextureSlots = 16;

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
    /**
     * 贴图缓冲区
     *   - 不重复添加 要用的贴图都缓存在这
     *   - 0号位是引擎的默认贴图
     *   - [1...i)是客户端添加给引擎的
     * 渲染的时候之前会把缓存区所有贴图都绑定到OpenGL的纹理单元
     * 渲染的时候会通过vertex attribute方式把用哪个贴图 对应的脚标告诉shader 然后shader用采样器处理它就行
     */
    std::array<X::Ref<Texture2D>, MaxTextureSlots> TextureSlots;
    // [0...i)都是贴图 0号位上是引擎给的默认贴图 [1...i)是客户端给的贴图
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
