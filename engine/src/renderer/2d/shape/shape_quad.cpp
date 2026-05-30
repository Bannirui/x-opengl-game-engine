//
// Created by dingrui on 5/26/26.
//

#include "renderer/2d/renderer_2D_internal.h"
#include "x/renderer/renderer_2D.h"
#include "x/scene/component.h"

#include <glm/ext/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>

/**
 * @param position 控制x跟y移动的向量
 * @param size 控制x跟y的缩放
 * @param color 顶点颜色
 */
void Renderer2D::DrawQuad(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color) {
    DrawQuad({position.x, position.y, 0.0f}, size, color);
}

/**
 * @param position 控制xyz移动的向量
 * @param size 控制x跟y轴的缩放
 * @param color 顶点颜色
 */
void Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec2& size, const glm::vec4& color) {
    X_PROFILE_FUNCTION();
    // 变换矩阵
    glm::mat4 transform =
        glm::translate(glm::mat4(1.0f), position) */*再平移 移动用position控制*/
            glm::scale(glm::mat4(1.0f), {size.x, size.y, 1.0f}); /*先缩放 size控制xy的缩放*/
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
 * @param transform 顶点变换矩阵
 * @param color 顶点颜色
 */
void Renderer2D::DrawQuad(const glm::mat4& transform, const glm::vec4& color, int entityID) {
    X_PROFILE_FUNCTION();
    // uv贴图坐标 从左下角开始逆时针
    constexpr glm::vec2 textureCoords[] = {{0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f}};
    if (s_data.Quad.IsFull()) {
        nextBatch();
    }
    // 内存缓冲区[Base...Ptr)放画矩形要的4个顶点
    for (size_t i = 0; i < 4; i++) {
        // 变换后的顶点 也就是要渲染的顶点位置
        s_data.Quad.Ptr->position = transform * s_data.QuadVertexPositions[i];
        // 顶点颜色
        s_data.Quad.Ptr->color = color;
        // 顶点对应的uv贴图坐标
        s_data.Quad.Ptr->texCoord = textureCoords[i];
        // 默认用引擎贴图缓冲区里面0号位上的贴图
        s_data.Quad.Ptr->texIndex = 0;
        // 就用1个贴图 不要重复贴图
        s_data.Quad.Ptr->tilingFactor = 1.0f;
        s_data.Quad.Ptr->entityID = entityID;
        // 内存里面放好顶点 后移 准备放下一个顶点
        s_data.Quad.Ptr++;
    }
    // 画矩形用的是DrawElements 用顶点索引的方式 每个矩形用4个顶点画两个三角形 用6个索引顶点来表示两个三角形的位置
    s_data.Quad.Count += 6;
    // 矩形绘制方式是4个顶点画2个三角形
    s_data.Stats.PrimitiveCount++;
}

/**
 * @param transform 顶点变换矩阵
 * @param texture 用这个贴图
 * @param tilingFactor 贴图因子 配置wrap策略repeat实现重复n个贴图
 * @param tintColor 亮度
 */
void Renderer2D::DrawQuad(const glm::mat4& transform, const X::Ref<Texture2D>& texture, float tilingFactor,
                          const glm::vec4& tintColor, int entityID) {
    X_PROFILE_FUNCTION();
    // 贴图uv坐标 从左下角逆时针开始绕4个点
    constexpr glm::vec2 textureCoords[] = {{0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f}};
    if (s_data.Quad.IsFull()) {
        nextBatch();
    }
    // 看看要用的贴图是不是已经在缓存区了 最后告诉shader用textureIndex这个对应的纹理单元
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
        // VBO里面的顶点坐标是死的 每次修改变换矩阵 然后用变换举证作用得到动态的顶点向量
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

/**
 * @param position 顶点在xyz的移动向量
 * @param size 控制xy的缩放
 * @param rotation 旋转的角度
 * @param texture 纹理贴图
 * @param tilingFactor 纹理贴图控制因子 配合wrap的repeat策略实现重复n个贴图
 * @param tintColor 顶点颜色
 */
void Renderer2D::DrawRotatedQuad(const glm::vec3& position, const glm::vec2& size, float rotation,
                                 const X::Ref<Texture2D>& texture, float tilingFactor, const glm::vec4& tintColor) {
    X_PROFILE_FUNCTION();
    // 变换矩阵 仅仅绕着z轴旋转角度
    glm::mat4 transform = glm::translate(glm::mat4(1.0f), position) * /*最后移动*/
                          glm::rotate(glm::mat4(1.0f), glm::radians(rotation), {0.0f, 0.0f, 1.0f}) * /*再旋转 绕着z轴旋转rotation角度*/
                          glm::scale(glm::mat4(1.0f), {size.x, size.y, 1.0f}); /*先缩放 对xy缩放*/
    DrawQuad(transform, texture, tilingFactor, tintColor);
}

void Renderer2D::DrawSprite(const glm::mat4& transform, SpriteRendererComponent& src, int entityID) {
    if (src.Texture) {
        DrawQuad(transform, src.Texture, src.TilingFactor, src.Color, entityID);
    } else {
        DrawQuad(transform, src.Color, entityID);
    }
}
