//
// Created by dingrui on 2/28/26.
//

#pragma once

#include "x/core/base.h"
#include "x/renderer/camera/orthographic_camera.h"

class EditorCamera;
class Camera;

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
    static void EndScene();

    static void Flush();

    // Stats
    struct Statistics {
        uint32_t DrawCalls = 0;
        uint32_t PrimitiveCount = 0;

        uint32_t GetTotalVertexCount() const {
            return PrimitiveCount * 4;
        }

        uint32_t GetTotalIndexCount() const {
            return PrimitiveCount * 6;
        }
    };

    static Statistics GetStats();
    static void ResetStats();

    static void startBatch();
    static void nextBatch();
};
