//
// Created by rui ding on 2026/6/1.
//

#include "x/animation/skeleton.h"

#include "x/core/assert.h"

void Skeleton::AddBone(const Bone& bone) {
    X_CORE_ASSERT(m_bones.size() < MAX_BONES, "Exceeded max bone count");
    m_bones.push_back(bone);
}

int32_t Skeleton::FindBone(const std::string& name) const {
    for (size_t i = 0; i < m_bones.size(); ++i) {
        if (m_bones[i].Name == name) return static_cast<int32_t>(i);
    }
    return -1;
}

void Skeleton::ComputeFinalTransforms(std::vector<glm::mat4>& outTransforms) const {
    outTransforms.resize(m_bones.size(), glm::mat4(1.0f));
    glm::mat4 identity(1.0f);
    for (size_t i = 0; i < m_bones.size(); ++i) {
        if (m_bones[i].ParentIndex == -1) {
            computeBoneTransform(static_cast<uint32_t>(i), identity, outTransforms);
        }
    }
}

void Skeleton::computeBoneTransform(uint32_t index, const glm::mat4& parentTransform,
                                    std::vector<glm::mat4>& outTransforms) const {
    glm::mat4 globalTransform = parentTransform * m_bones[index].LocalTransform;
    outTransforms[index] = globalTransform * m_bones[index].InverseBindMatrix;

    for (size_t i = 0; i < m_bones.size(); ++i) {
        if (m_bones[i].ParentIndex == static_cast<int32_t>(index)) {
            computeBoneTransform(static_cast<uint32_t>(i), globalTransform, outTransforms);
        }
    }
}
