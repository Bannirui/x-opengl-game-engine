//
// Created by rui ding on 2026/6/7.
//

#pragma once

#include "x/core/base.h"

#include <glm/glm.hpp>

struct SpriteRendererComponent;
class Texture2D;

class Renderer2DDraw {
public:
    Renderer2DDraw() = delete;

    static void DrawQuad(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color);
    static void DrawQuad(const glm::vec3& position, const glm::vec2& size, const glm::vec4& color);

    static void DrawQuad(const glm::vec2& position, const glm::vec2& size, const Ref<Texture2D>& texture,
                         float tilingFactor = 1.0f, const glm::vec4& tintColor = glm::vec4(1.0f));
    static void DrawQuad(const glm::vec3& position, const glm::vec2& size, const Ref<Texture2D>& texture,
                         float tilingFactor = 1.0f, const glm::vec4& tintColor = glm::vec4(1.0f));

    static void DrawQuad(const glm::mat4& transform, const glm::vec4& color, int entityID = -1);
    static void DrawQuad(const glm::mat4& transform, const Ref<Texture2D>& texture, float tilingFactor = 1.0f,
                         const glm::vec4& tintColor = glm::vec4(1.0f), int entityID = -1);

    static void DrawRotatedQuad(const glm::vec2& position, const glm::vec2& size, float rotation,
                                const glm::vec4& color);
    static void DrawRotatedQuad(const glm::vec3& position, const glm::vec2& size, float rotation,
                                const glm::vec4& color);

    static void DrawRotatedQuad(const glm::vec2& position, const glm::vec2& size, float rotation,
                                const Ref<Texture2D>& texture, float tilingFactor = 1.0f,
                                const glm::vec4& tintColor = glm::vec4(1.0f));
    static void DrawRotatedQuad(const glm::vec3& position, const glm::vec2& size, float rotation,
                                const Ref<Texture2D>& texture, float tilingFactor = 1.0f,
                                const glm::vec4& tintColor = glm::vec4(1.0f));

    static void DrawCircle(const glm::mat4& transform, const glm::vec4& color, float thickness = 1.0f,
                           float fade = 0.005f, int entityID = 0);

    static void DrawLine(const glm::vec3& p0, glm::vec3& p1, const glm::vec4& color, int entityID = -1);

    static void DrawRect(const glm::vec3& position, const glm::vec2& size, const glm::vec4& color, int entityID = -1);
    static void DrawRect(const glm::mat4& transform, const glm::vec4& color, int entityID = -1);

    static void DrawSprite(const glm::mat4& transform, SpriteRendererComponent& src, int entityID);

    static float GetLineWidth();
    static void SetLineWidth(float width);
};
