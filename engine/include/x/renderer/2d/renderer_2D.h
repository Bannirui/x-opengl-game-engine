//
// Created by dingrui on 2/28/26.
//

#pragma once

#include "x/core/base.h"
#include "x/renderer/camera/orthographic_camera.h"

struct SpriteRendererComponent;
class EditorCamera;
class Camera;
class Texture2D;

/**
 * 使用批量思想
 * GPU层面 VBO预先申请开辟n个顶点容量的显存 init的时候申请好
 * CPU层面 [Base...Ptr)堆内存表示实际放的顶点数据 flush的时候数据灌给GPU
 */
class Renderer2D {
public:
    static void Init();
    static void Shutdown();

    static void BeginScene(const OrthographicCamera& camera);
    static void BeginScene(const Camera& camera, const glm::mat4& transform);
    static void BeginScene(const EditorCamera& camera);
    // 每个帧可能绘制多个图形 把所有要绘制的图形VBO数据都收集好了再一起提交给GPU
    static void EndScene();

    /**
     * 绘制矩形
     */
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

    /**
     * 绘制圆形
     */
    static void DrawCircle(const glm::mat4& transform, const glm::vec4& color, float thickness = 1.0f,
                           float fade = 0.005f, int entityID = 0);

    static void DrawLine(const glm::vec3& p0, glm::vec3& p1, const glm::vec4& color, int entityID = -1);

    static void DrawRect(const glm::vec3& position, const glm::vec2& size, const glm::vec4& color, int entityID = -1);
    static void DrawRect(const glm::mat4& transform, const glm::vec4& color, int entityID = -1);

    /**
     * 绘制单色图形
     */
    static void DrawSprite(const glm::mat4& transform, SpriteRendererComponent& src, int entityID);

    static float GetLineWidth();
    static void SetLineWidth(float width);

    // Stats
    struct Statistics {
        uint32_t DrawCalls = 0;
        // 有多少个图形是用 4个顶点画2个三角形形成的
        uint32_t PrimitiveCount = 0;

        // 4个顶点画2个三角形这种方式 画1个图形用到4个顶点
        uint32_t GetTotalVertexCount() const {
            return PrimitiveCount * 4;
        }

        // 4个顶点画2个三角形这种方式 画1个图形用到6个索引
        uint32_t GetTotalIndexCount() const {
            return PrimitiveCount * 6;
        }
    };

    static Statistics GetStats();
    static void ResetStats();

private:
    static void Flush();

private:
    static void startBatch();
    static void nextBatch();
};
