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

#include <unordered_map>
#include <vector>

// 封装单次的绘制命令
struct MeshDrawCommand {
    // 风格资源
    Ref<Mesh> MeshAsset;
    // 模型矩阵 MVP的M 剪裁空间坐标=投影矩阵*观察矩阵*模型矩阵*本地坐标 模型矩阵负责把本地坐标转换到世界坐标
    glm::mat4 Transform;
    // 用来鼠标拾取
    int32_t EntityID;
};

// 按照材质分组 相同材质的多个绘制指令放在一起
struct MaterialBucket {
    // 要绘制哪个材质
    Ref<Material> MaterialAsset;
    // 这个材质的绘制指令
    std::vector<MeshDrawCommand> Commands;
};

/**
 * GPU端灯光布局std140与GLSL LightBlock一一对应
 *
 * layout(std140, binding = 2) uniform LightBlock {
 *     vec3  u_Ambient;                           // offset 0
 *     int   u_LightCount;                        // offset 12 (packed in vec3 tail)
 *     GPULight u_Lights[MAX_GPU_LIGHTS];         // offset 16, 48 bytes each
 * };
 */
struct LightGroupData {
    // 环境光是全局的
    glm::vec3 Ambient;
    // 当前活跃的灯光有几个 从数组里面取
    int LightCount;
    // 灯光
    GPULight Lights[MAX_GPU_LIGHTS];
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
    Ref<UniformBuffer> CameraUBO;
    // slot1 模型矩阵M 负责把本地坐标转换世界
    Ref<UniformBuffer> ModelUBO;
    // slot2 光照 每帧更新 按light group切换时重新上传
    Ref<UniformBuffer> LightUBO;
    // slot3 相机位置+曝光度 每帧更新
    Ref<UniformBuffer> PBRUBO;

    // 每个light group的CPU端光照参数 只有在真正渲染的时候才会将数据通过UBO传给着色器
    std::unordered_map<uint32_t, LightGroupData> LightGroups;
    Ref<Texture2D> WhiteTexture;  // 1*1白色纹理 没有贴图时作为兜底使用
    Ref<TextureCube> EnvironmentMap;  // HDR环境光 天空盒原始图
    Ref<TextureCube> IrradianceMap;  // 漫反射辐照度图
    Ref<TextureCube> PrefilterMap;  // 预过滤镜面反射图 多级的mipmap
    uint32_t BRDFLUTTexture = 0;  // BRDF积分LUT查找表
    uint32_t DefaultShadowMap = 0;  // 默认阴影贴图

    // 着色器
    Ref<Shader> DefaultShader;  // 冯氏
    Ref<Shader> PBRShader;
    Ref<Shader> SkyboxShader;  // 天空盒着色器

    // CPU端缓冲 UBO数据源 最终都会通过ubo->SetD
    struct CameraData {
        glm::mat4 ViewProjection;  // 每帧从摄像机算出来ViewProjection写到这里
    };

    CameraData CameraBuffer;

    glm::mat4 CurrentViewMatrix;  // 当前帧的View矩阵 供天空盒剥离位移用
    glm::mat4 CurrentProjectionMatrix;  // 当前帧的Projection矩阵 供天空盒用

    PBRSettingsData PBRBuffer;  // 相机+曝光度

    Renderer3D::Statistics Stats;
};
