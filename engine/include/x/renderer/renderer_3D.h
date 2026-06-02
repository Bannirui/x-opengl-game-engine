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
    static void BeginScene(const EditorCamera& camera);

    static void DrawMesh(const X::Ref<Mesh>& mesh, const X::Ref<Material>& material,
                         const glm::mat4& transform, int32_t entityID = -1);

    static void EndScene();
    static void DrawSkybox();

    // PBR environment
    static void SetEnvironmentMap(const X::Ref<TextureCube>& envMap);
    static void SetEnvironmentMaps(const X::Ref<TextureCube>& envMap,
                                    const X::Ref<TextureCube>& irradianceMap,
                                    const X::Ref<TextureCube>& prefilterMap,
                                    uint32_t brdfLUTTexture);
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
