//
// Created by rui ding on 2026/6/1.
//

#pragma once

#include "x/core/base.h"

#include "x/animation/animation_clip.h"
#include "x/animation/skeleton.h"

#include <glm/glm.hpp>

#include <vector>

class Animator {
public:
    Animator() = default;

    void SetSkeleton(const Ref<Skeleton>& skeleton) { m_skeleton = skeleton; }
    void PlayClip(const Ref<AnimationClip>& clip, bool loop = true);

    void Update(float deltaTime);

    const std::vector<glm::mat4>& GetBoneMatrices() const { return m_boneMatrices; }
    bool HasAnimation() const { return m_currentClip != nullptr; }

private:
    Ref<Skeleton> m_skeleton;
    Ref<AnimationClip> m_currentClip;

    float m_currentTime = 0.0f;
    float m_speed = 1.0f;
    bool m_looping = false;
    bool m_playing = false;

    std::vector<glm::mat4> m_boneMatrices;
};
