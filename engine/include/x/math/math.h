//
// Created by rui ding on 2026/3/5.
//

#pragma once

#include <glm/glm.hpp>

namespace MATH
{
    // transform->translation, rotation, scale
    bool DecomposeTransform(const glm::mat4& transform, glm::vec3& translation, glm::vec3& rotation, glm::vec3& scale);
}  // namespace MATH