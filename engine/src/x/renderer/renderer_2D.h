//
// Created by dingrui on 2/28/26.
//

#pragma once

#include "x/core/base.h"
#include "x/renderer/orthographic_camera.h"

struct SpriteRendererComponent;
class EditorCamera;
class Camera;
class Texture2D;

class Renderer2D
{
public:
    static void Init();
    static void Shutdown();

    static void BeginScene(const OrthographicCamera& camera);
    static void BeginScene(const Camera& camera, const glm::mat4& transform);
    static void BeginScene(const EditorCamera& camera);
    static void EndScene();

    static void Flush();

    static void DrawQuad(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color);
    static void DrawQuad(const glm::vec3& position, const glm::vec2& size, const glm::vec4& color);

    static void DrawQuad(const glm::vec2& position, const glm::vec2& size, const X::Ref<Texture2D>& texture,
                         float tilingFactor = 1.0f, const glm::vec4& tintColor = glm::vec4(1.0f));
    static void DrawQuad(const glm::vec3& position, const glm::vec2& size, const X::Ref<Texture2D>& texture,
                         float tilingFactor = 1.0f, const glm::vec4& tintColor = glm::vec4(1.0f));

    static void DrawQuad(const glm::mat4& transform, const glm::vec4& color, int entityID = -1);
    static void DrawQuad(const glm::mat4& transform, const X::Ref<Texture2D>& texture, float tilingFactor = 1.0f,
                         const glm::vec4& tintColor = glm::vec4(1.0f), int entityID = -1);

    static void DrawRotatedQuad(const glm::vec2& position, const glm::vec2& size, float rotation,
                                const glm::vec4& color);
    static void DrawRotatedQuad(const glm::vec3& position, const glm::vec2& size, float rotation,
                                const glm::vec4& color);

    static void DrawRotatedQuad(const glm::vec2& position, const glm::vec2& size, float rotation,
                                const X::Ref<Texture2D>& texture, float tilingFactor = 1.0f,
                                const glm::vec4& tintColor = glm::vec4(1.0f));
    static void DrawRotatedQuad(const glm::vec3& position, const glm::vec2& size, float rotation,
                                const X::Ref<Texture2D>& texture, float tilingFactor = 1.0f,
                                const glm::vec4& tintColor = glm::vec4(1.0f));

    static void DrawSprite(const glm::mat4& transform, SpriteRendererComponent& src, int entityID);

    // Stats
    struct Statistics
    {
        uint32_t drawCalls = 0;
        uint32_t quadCount = 0;

        uint32_t GetTotalVertexCount() const { return quadCount * 4; }

        uint32_t GetTotalIndexCount() const { return quadCount * 6; }
    };

    static void       ResetStats();
    static Statistics GetStats();

private:
    static void startBatch();
    static void nextBatch();
};
