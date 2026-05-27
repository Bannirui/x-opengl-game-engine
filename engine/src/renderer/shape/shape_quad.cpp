//
// Created by dingrui on 5/26/26.
//

#include "renderer/renderer_2D_internal.h"
#include "x/renderer/renderer_2D.h"
#include "x/scene/component.h"

#include <glm/ext/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>

void Renderer2D::DrawQuad(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color) {
    DrawQuad({position.x, position.y, 0.0f}, size, color);
}

void Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec2& size, const glm::vec4& color) {
    X_PROFILE_FUNCTION();
    glm::mat4 transform =
        glm::translate(glm::mat4(1.0f), position) * glm::scale(glm::mat4(1.0f), {size.x, size.y, 1.0f});
    DrawQuad(transform, color);
}

void Renderer2D::DrawQuad(const glm::vec2& position, const glm::vec2& size, const X::Ref<Texture2D>& texture,
                          float tilingFactor, const glm::vec4& tintColor) {
    DrawQuad({position.x, position.y, 0.0f}, size, texture, tilingFactor, tintColor);
}

void Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec2& size, const X::Ref<Texture2D>& texture,
                          float tilingFactor, const glm::vec4& tintColor) {
    X_PROFILE_FUNCTION();
    glm::mat4 transform =
        glm::translate(glm::mat4(1.0f), position) * glm::scale(glm::mat4(1.0f), {size.x, size.y, 1.0f});
    DrawQuad(transform, texture, tilingFactor, tintColor);
}

void Renderer2D::DrawQuad(const glm::mat4& transform, const glm::vec4& color, int entityID) {
    X_PROFILE_FUNCTION();

    const float textureIndex = 0.0f;
    constexpr glm::vec2 textureCoords[] = {{0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f}};
    const float tilingFactor = 1.0f;
    if (s_data.Quad.IsFull()) {
        nextBatch();
    }
    for (size_t i = 0; i < 4; i++) {
        s_data.Quad.Ptr->position = transform * s_data.QuadVertexPositions[i];
        s_data.Quad.Ptr->color = color;
        s_data.Quad.Ptr->texCoord = textureCoords[i];
        s_data.Quad.Ptr->texIndex = textureIndex;
        s_data.Quad.Ptr->tilingFactor = tilingFactor;
        s_data.Quad.Ptr->entityID = entityID;
        s_data.Quad.Ptr++;
    }
    // 画矩形用的是DrawElements 用顶点索引的方式 每个矩形用4个顶点画两个三角形 用6个索引顶点来表示两个三角形的位置
    s_data.Quad.Count += 6;
    // 矩形绘制方式是4个顶点画2个三角形
    s_data.Stats.PrimitiveCount++;
}

void Renderer2D::DrawQuad(const glm::mat4& transform, const X::Ref<Texture2D>& texture, float tilingFactor,
                          const glm::vec4& tintColor, int entityID) {
    X_PROFILE_FUNCTION();

    constexpr glm::vec2 textureCoords[] = {{0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f}};
    if (s_data.Quad.IsFull()) {
        nextBatch();
    }

    float textureIndex = 0.0f;
    for (uint32_t i = 1; i < s_data.TextureSlotIndex; i++) {
        if (*s_data.TextureSlots[i] == *texture) {
            textureIndex = (float)i;
            break;
        }
    }
    if (textureIndex == 0.0f) {
        if (s_data.TextureSlotIndex >= Renderer2DData::MaxTextureSlots) {
            nextBatch();
        }
        textureIndex = (float)s_data.TextureSlotIndex;
        s_data.TextureSlots[s_data.TextureSlotIndex] = texture;
        s_data.TextureSlotIndex++;
    }
    for (size_t i = 0; i < 4; i++) {
        s_data.Quad.Ptr->position = transform * s_data.QuadVertexPositions[i];
        s_data.Quad.Ptr->color = tintColor;
        s_data.Quad.Ptr->texCoord = textureCoords[i];
        s_data.Quad.Ptr->texIndex = textureIndex;
        s_data.Quad.Ptr->tilingFactor = tilingFactor;
        s_data.Quad.Ptr->entityID = entityID;
        s_data.Quad.Ptr++;
    }
    s_data.Quad.Count += 6;
    // 矩形绘制方式是4个顶点画2个三角形
    s_data.Stats.PrimitiveCount++;
}

void Renderer2D::DrawRotatedQuad(const glm::vec2& position, const glm::vec2& size, float rotation,
                                 const glm::vec4& color) {
    DrawRotatedQuad({position.x, position.y, 0.0f}, size, rotation, color);
}

void Renderer2D::DrawRotatedQuad(const glm::vec3& position, const glm::vec2& size, float rotation,
                                 const glm::vec4& color) {
    X_PROFILE_FUNCTION();
    glm::mat4 transform = glm::translate(glm::mat4(1.0f), position) *
                          glm::rotate(glm::mat4(1.0f), glm::radians(rotation), {0.0f, 0.0f, 1.0f}) *
                          glm::scale(glm::mat4(1.0f), {size.x, size.y, 1.0f});
    DrawQuad(transform, color);
}

void Renderer2D::DrawRotatedQuad(const glm::vec2& position, const glm::vec2& size, float rotation,
                                 const X::Ref<Texture2D>& texture, float tilingFactor, const glm::vec4& tintColor) {
    DrawRotatedQuad({position.x, position.y, 0.0f}, size, rotation, texture, tilingFactor, tintColor);
}

void Renderer2D::DrawRotatedQuad(const glm::vec3& position, const glm::vec2& size, float rotation,
                                 const X::Ref<Texture2D>& texture, float tilingFactor, const glm::vec4& tintColor) {
    X_PROFILE_FUNCTION();
    glm::mat4 transform = glm::translate(glm::mat4(1.0f), position) *
                          glm::rotate(glm::mat4(1.0f), glm::radians(rotation), {0.0f, 0.0f, 1.0f}) *
                          glm::scale(glm::mat4(1.0f), {size.x, size.y, 1.0f});
    DrawQuad(transform, texture, tilingFactor, tintColor);
}

void Renderer2D::DrawSprite(const glm::mat4& transform, SpriteRendererComponent& src, int entityID) {
    if (src.Texture) {
        DrawQuad(transform, src.Texture, src.TilingFactor, src.Color, entityID);
    } else {
        DrawQuad(transform, src.Color, entityID);
    }
}
