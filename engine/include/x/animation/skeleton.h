//
// Created by rui ding on 2026/6/1.
//

#pragma once

#include "x/core/base.h"

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include <string>
#include <vector>

struct Bone {
    std::string Name;
    int32_t ParentIndex = -1;
    glm::mat4 InverseBindMatrix{1.0f};
    glm::mat4 LocalTransform{1.0f};

    Bone() = default;
};

class Skeleton {
public:
    static constexpr uint32_t MAX_BONES = 64;

    void AddBone(const Bone& bone);
    int32_t FindBone(const std::string& name) const;

    const std::vector<Bone>& GetBones() const { return m_bones; }
    std::vector<Bone>& GetBones() { return m_bones; }
    uint32_t GetBoneCount() const { return static_cast<uint32_t>(m_bones.size()); }

    void ComputeFinalTransforms(std::vector<glm::mat4>& outTransforms) const;

private:
    void computeBoneTransform(uint32_t index, const glm::mat4& parentTransform,
                               std::vector<glm::mat4>& outTransforms) const;

    std::vector<Bone> m_bones;
};
