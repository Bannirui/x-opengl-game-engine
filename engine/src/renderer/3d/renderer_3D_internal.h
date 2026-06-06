//
// Created by rui ding on 2026/6/1.
//

#pragma once

#include "x/renderer/3d/renderer_3D.h"
#include "x/renderer/buffer/uniform_buffer.h"
#include "x/renderer/material.h"
#include "x/renderer/mesh.h"
#include "x/renderer/shader.h"
#include "x/renderer/texture.h"
#include "x/renderer/texture_cube.h"

#include <glm/glm.hpp>

#include <vector>

// 封装单次的绘制命令
struct MeshDrawCommand {
    // 风格资源
    X::Ref<Mesh> MeshAsset;
    // 模型矩阵 MVP的M 剪裁空间坐标=投影矩阵*观察矩阵*模型矩阵*本地坐标 模型矩阵负责把本地坐标转换到世界坐标
    glm::mat4 Transform;
    // 用来鼠标拾取
    int32_t EntityID;
};

// 按照材质分组 相同材质的多个绘制指令放在一起
struct MaterialBucket {
    // 要绘制哪个材质
    X::Ref<Material> MaterialAsset;
    // 这个材质的绘制指令
    std::vector<MeshDrawCommand> Commands;
};

/**
 * 光照参数 与GPU的UBO一一对应
 * C++端布局要求
 *   - 1 std140规则是 vec3的基准对齐和对齐偏移都是16字节 等同vec4
 *   - 2 因此每个vec3后面必须手动填充1个float 4字节 来让下一个vec3起始地址对齐到16字节边界
 * 对应的GLSL是
 *   layout(std140, binding = 2) uniform Light {
 *      vec3 u_LightDirection;   // offset 0
 *      vec3 u_LightAmbient;     // offset 16
 *      vec3 u_LightDiffuse;     // offset 32
 *      vec3 u_LightSpecular;    // offset 48
 *   };
 */
struct LightData {
    // 光照方向 指向光源 shader中取u_LightDirection得到光线向量
    glm::vec3 Direction;
    // std140对齐填充
    float Padding0;
    // 环境光颜色 所有片段均匀叠加
    glm::vec3 Ambient;
    // std140对齐填充
    float Padding1;
    // 漫反射颜色 Lambert项乘以该值
    glm::vec3 Diffuse;
    // std140对齐填充
    float Padding2;
    // 镜面反射颜色 Blinn-Phong高光颜色
    glm::vec3 Specular;
    // std140对齐填充
    float Padding3;
};

struct PBRSettingsData {
    glm::vec3 CameraPosition;
    float Exposure;
};

struct Renderer3DData {
    /**
     * 绘制命令缓冲
     * 每帧收集到的绘制命令 是按照材质分组的 因为在一帧里面多个mesh可能都是针对同一种材质在画
     *   - DrawMesh生产 往里面塞
     *   - Flush消费
     */
    std::vector<MaterialBucket> Buckets;

    // 剪裁空间坐标gl_Position=投影矩阵P*观察矩阵V*模型矩阵M*本地坐标
    // 约定的UBO
    // slot0 ViewProjection矩阵 每帧更新 P*V的结果 负责把世界坐标转换剪裁坐标
    X::Ref<UniformBuffer> CameraUBO;
    // slot1 模型矩阵M 负责把本地坐标转换世界
    X::Ref<UniformBuffer> ModelUBO;
    X::Ref<UniformBuffer> LightUBO;  // slot2 光照 每帧更新
    X::Ref<UniformBuffer> PBRUBO;  // slot3 相机位置+曝光度 每帧更新

    // 纹理贴图
    X::Ref<Texture2D> WhiteTexture;  // 1*1白色纹理 没有贴图时作为兜底使用
    X::Ref<TextureCube> EnvironmentMap;  // HDR环境光 天空盒原始图
    X::Ref<TextureCube> IrradianceMap;  // 漫反射辐照度图
    X::Ref<TextureCube> PrefilterMap;  // 预过滤镜面反射图 多级的mipmap
    uint32_t BRDFLUTTexture = 0;  // BRDF积分LUT查找表
    uint32_t DefaultShadowMap = 0;  // 默认阴影贴图

    // 着色器
    X::Ref<Shader> DefaultShader;  // 冯氏
    X::Ref<Shader> PBRShader;
    X::Ref<Shader> SkyboxShader;  // 天空盒着色器

    // CPU端缓冲 UBO数据源 最终都会通过ubo->SetD
    struct CameraData {
        glm::mat4 ViewProjection;  // 每帧从摄像机算出来ViewProjection写到这里
    };

    CameraData CameraBuffer;

    glm::mat4 CurrentViewMatrix;  // 当前帧的View矩阵 供天空盒剥离位移用
    glm::mat4 CurrentProjectionMatrix;  // 当前帧的Projection矩阵 供天空盒用

    LightData LightBuffer;  // 平行光
    PBRSettingsData PBRBuffer;  // 相机+曝光度

    Renderer3D::Statistics Stats;
};
