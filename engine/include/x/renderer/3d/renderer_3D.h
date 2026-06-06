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
    static void SetLightDirection(const glm::vec3& direction);
    static void SetLightColor(const glm::vec3& color);

    struct Statistics {
        uint32_t DrawCalls = 0;
        uint32_t MeshCount = 0;
    };

    static Statistics GetStats();
    static void ResetStats();

private:
    static void Flush();
};
