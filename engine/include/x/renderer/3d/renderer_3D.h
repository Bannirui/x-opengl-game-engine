//
// Created by rui ding on 2026/6/1.
//

#pragma once

#include "x/core/base.h"

#include <glm/glm.hpp>

#include <cstdint>

class Camera;
class EditorCamera;
class Mesh;
class Material;
class TextureCube;

static constexpr uint32_t MAX_GPU_LIGHTS = 8;

enum class GPULightType : int { Directional = 0, Point = 1, Spot = 2 };

struct GPULight {
    glm::vec4 ColorAndIntensity;  // xyz=颜色, w=强度
    glm::vec4 PositionAndRange;   // xyz=方向(平行光)/位置(点光), w=范围
    int Type;                     // GPULightType (0=平行光, 1=点光)
    float SpotInnerCone;          // 聚光灯内锥角cos, 未来扩展
    float SpotOuterCone;          // 聚光灯外锥角cos, 未来扩展
    float _pad;                   // std140 对齐
};

class Renderer3D {
public:
    static void Init();
    static void Shutdown();
    static void SetViewport(uint32_t width, uint32_t height);

    static void BeginScene(const Camera& camera, const glm::mat4& viewMatrix);
    /**
     * 上传UBO
     *   - Camera
     *   - Light
     *   - PBR
     * @param camera 轨迹球相机
     */
    static void BeginScene(const EditorCamera& camera);

    /**
     * 提交一个绘制网格的命令
     * 不立刻绘制 而是按照Material分组累积到Buckets 在EndScene->Flush()时统一批量提交
     * @param transform 模型矩阵M 把本地坐标转换到世界坐标
     */
    static void DrawMesh(const X::Ref<Mesh>& mesh, const X::Ref<Material>& material, const glm::mat4& transform,
                         int32_t entityID = -1);

    /**
     * 每帧结束时调用 触发Flush提交所有累积的绘制命令
     */
    static void EndScene();
    /**
     * 绘制一个覆盖全屏的大立方体 用深度测试GL_LEQUAL让它始终在最远处
     * 这个天空盒只会画一次
     */
    static void DrawSkybox();

    // PBR environment
    static void SetEnvironmentMap(const X::Ref<TextureCube>& envMap);
    static void SetEnvironmentMaps(const X::Ref<TextureCube>& envMap, const X::Ref<TextureCube>& irradianceMap,
                                   const X::Ref<TextureCube>& prefilterMap, uint32_t brdfLUTTexture);
    static void SetExposure(float exposure);
    static const X::Ref<TextureCube>& GetEnvironmentMap();

    // Light
    static void SetLightAmbient(const glm::vec3& ambient, uint32_t lightGroupId = 0);
    static void SetDirectionalLight(const glm::vec3& direction, const glm::vec3& color, float intensity,
                                    uint32_t lightGroupId = 0, uint32_t lightIndex = 0);
    static void SetPointLight(const glm::vec3& position, const glm::vec3& color, float range, float intensity,
                              uint32_t lightGroupId = 0, uint32_t lightIndex = 0);
    static void SetLightCount(uint32_t count, uint32_t lightGroupId = 0);

    // 便捷方法 操作group的Lights[0]
    static void SetLightDirection(const glm::vec3& direction, uint32_t lightGroupId = 0);
    static void SetLightColor(const glm::vec3& color, uint32_t lightGroupId = 0);
    static void SetPointLightPosition(const glm::vec3& position, uint32_t lightGroupId = 0);
    static void SetPointLightColor(const glm::vec3& color, uint32_t lightGroupId = 0);
    static void SetPointLightRange(float range, uint32_t lightGroupId = 0);
    static void SetPointLightIntensity(float intensity, uint32_t lightGroupId = 0);

    struct Statistics {
        uint32_t DrawCalls = 0;
        uint32_t MeshCount = 0;
    };

    static Statistics GetStats();
    static void ResetStats();

private:
    static void Flush();
};
