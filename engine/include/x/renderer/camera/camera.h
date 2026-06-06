//
// Created by dingrui on 3/3/26.
//

#pragma once

#include <glm/glm.hpp>

/**
 * 抽象基类 仅持有projection矩阵
 * view从外部传进来
 * 作为接口复用projection 只负责保存和提供投影矩阵 不涉及view和位置
 */
class Camera {
public:
    /**
     * 默认的投影矩阵是单位阵
     */
    Camera() = default;

    /**
     * @param projection 投影矩阵
     */
    Camera(const glm::mat4& projection) : m_projection(projection) {}

    virtual ~Camera() = default;

    const glm::mat4& get_projection() const {
        return m_projection;
    }

protected:
    // 投影矩阵
    glm::mat4 m_projection{glm::mat4(1.0f)};
};
