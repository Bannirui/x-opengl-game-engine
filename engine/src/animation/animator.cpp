//
// Created by rui ding on 2026/6/1.
//

#include "x/animation/animator.h"

void Animator::PlayClip(const Ref<AnimationClip>& clip, bool loop) {
    m_currentClip = clip;
    m_currentTime = 0.0f;
    m_looping = loop;
    m_playing = true;
    m_boneMatrices.resize(Skeleton::MAX_BONES, glm::mat4(1.0f));
}

void Animator::Update(float deltaTime) {
    if (!m_playing || !m_currentClip || !m_skeleton) return;

    m_currentTime += deltaTime * m_speed;

    float duration = m_currentClip->GetDuration();
    if (m_currentTime > duration) {
        if (m_looping) {
            m_currentTime = std::fmod(m_currentTime, duration);
        } else {
            m_currentTime = duration;
            m_playing = false;
        }
    }

    // Reset bone local transforms
    auto& bones = m_skeleton->GetBones();
    for (auto& bone : bones) {
        bone.LocalTransform = glm::mat4(1.0f);
    }

    // Apply animation tracks
    for (auto& track : m_currentClip->GetBoneTracks()) {
        int32_t boneIdx = m_skeleton->FindBone(track.BoneName);
        if (boneIdx >= 0) {
            bones[boneIdx].LocalTransform = track.Interpolate(m_currentTime);
        }
    }

    // Compute final bone matrices
    m_skeleton->ComputeFinalTransforms(m_boneMatrices);
}
