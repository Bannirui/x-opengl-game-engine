//
// Created by rui ding on 2026/6/1.
//

#include "x/animation/animation_clip.h"

#include <algorithm>

glm::mat4 BoneTrack::Interpolate(float time) const {
    if (Keyframes.empty()) return glm::mat4(1.0f);
    if (Keyframes.size() == 1) {
        auto& k = Keyframes[0];
        return glm::translate(glm::mat4(1.0f), k.Translation) * glm::mat4_cast(k.Rotation) *
               glm::scale(glm::mat4(1.0f), k.Scale);
    }

    // Clamp time
    float t = std::fmod(time, Keyframes.back().TimeStamp);

    // Find surrounding keyframes
    size_t next = 0;
    for (size_t i = 0; i < Keyframes.size(); ++i) {
        if (Keyframes[i].TimeStamp > t) {
            next = i;
            break;
        }
    }
    size_t prev = (next == 0) ? 0 : next - 1;

    if (next == 0) {
        auto& k = Keyframes[0];
        return glm::translate(glm::mat4(1.0f), k.Translation) * glm::mat4_cast(k.Rotation) *
               glm::scale(glm::mat4(1.0f), k.Scale);
    }

    auto& k0 = Keyframes[prev];
    auto& k1 = Keyframes[next];
    float factor = (t - k0.TimeStamp) / (k1.TimeStamp - k0.TimeStamp);
    factor = glm::clamp(factor, 0.0f, 1.0f);

    glm::vec3 trans = glm::mix(k0.Translation, k1.Translation, factor);
    glm::quat rot = glm::slerp(k0.Rotation, k1.Rotation, factor);
    glm::vec3 scale = glm::mix(k0.Scale, k1.Scale, factor);

    return glm::translate(glm::mat4(1.0f), trans) * glm::mat4_cast(rot) * glm::scale(glm::mat4(1.0f), scale);
}

void AnimationClip::AddBoneTrack(const BoneTrack& track) {
    m_boneTracks.push_back(track);
}
