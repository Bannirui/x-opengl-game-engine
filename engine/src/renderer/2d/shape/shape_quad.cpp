//
// Created by dingrui on 5/26/26.
//

#include "renderer/2d/renderer_2D_internal.h"
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

/**
 * 没有指定贴图 就用引擎里面自带的 贴图缓冲区0号位上的贴图
 */
void Renderer2D::DrawQuad(const glm::mat4& transform, const glm::vec4& color, int entityID) {
    X_PROFILE_FUNCTION();

    constexpr glm::vec2 textureCoords[] = {{0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f}};
    const float tilingFactor = 1.0f;
    if (s_data.Quad.IsFull()) {
        nextBatch();
    }
    // 内存缓冲区[Base...Ptr)放画矩形要的4个顶点
    for (size_t i = 0; i < 4; i++) {
        s_data.Quad.Ptr->position = transform * s_data.QuadVertexPositions[i];
        s_data.Quad.Ptr->color = color;
        s_data.Quad.Ptr->texCoord = textureCoords[i];
        // 默认用引擎贴图缓冲区里面0号位上的贴图
        s_data.Quad.Ptr->texIndex = 0;
        s_data.Quad.Ptr->tilingFactor = tilingFactor;
        s_data.Quad.Ptr->entityID = entityID;
        s_data.Quad.Ptr++;
    }
    // 画矩形用的是DrawElements 用顶点索引的方式 每个矩形用4个顶点画两个三角形 用6个索引顶点来表示两个三角形的位置
    s_data.Quad.Count += 6;
    // 矩形绘制方式是4个顶点画2个三角形
    s_data.Stats.PrimitiveCount++;
}

/**
 *
 * @param transform
 * @param texture 指定的贴图贴图
 * @param tilingFactor
 * @param tintColor
 * @param entityID
 */
void Renderer2D::DrawQuad(const glm::mat4& transform, const X::Ref<Texture2D>& texture, float tilingFactor,
                          const glm::vec4& tintColor, int entityID) {
    X_PROFILE_FUNCTION();

    constexpr glm::vec2 textureCoords[] = {{0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f}};
    if (s_data.Quad.IsFull()) {
        nextBatch();
    }
    // 看看要用的贴图是不是已经在缓存区了
    uint32_t textureIndex = 0;
    for (uint32_t i = 1; i < s_data.TextureSlotIndex; i++) {
        if (*s_data.TextureSlots[i] == *texture) {
            textureIndex = i;
            break;
        }
    }
    // 不是0说明在缓存区已经有了相同的贴图 就不用重复添加到缓冲区了
    if (textureIndex == 0) {
        if (s_data.TextureSlotIndex >= Renderer2DData::MaxTextureSlots) {
            nextBatch();
        }
        // 把贴图放到缓冲区缓存起来
        s_data.TextureSlots[s_data.TextureSlotIndex] = texture;
        textureIndex = s_data.TextureSlotIndex;
        s_data.TextureSlotIndex++;
    }
    // 在CPU侧缓存[Base...Ptr)上放4个顶点
    for (size_t i = 0; i < 4; i++) {
        s_data.Quad.Ptr->position = transform * s_data.QuadVertexPositions[i];
        s_data.Quad.Ptr->color = tintColor;
        s_data.Quad.Ptr->texCoord = textureCoords[i];
        // shader的采样器用贴图缓冲区的哪个贴图 用vertex attribute的方式告诉shader
        s_data.Quad.Ptr->texIndex = static_cast<int>(textureIndex);
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
