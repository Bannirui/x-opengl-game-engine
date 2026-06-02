//
// Created by rui ding on 2026/6/1.
//

#pragma once

#include "x/core/base.h"

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include <string>
#include <vector>

struct BoneKeyframe {
    float TimeStamp;
    glm::vec3 Translation{0.0f};
    glm::quat Rotation{1.0f, 0.0f, 0.0f, 0.0f};
    glm::vec3 Scale{1.0f};
};

struct BoneTrack {
    std::string BoneName;
    std::vector<BoneKeyframe> Keyframes;

    glm::mat4 Interpolate(float time) const;
};

class AnimationClip {
public:
    AnimationClip() = default;

    void SetName(const std::string& name) { m_name = name; }
    const std::string& GetName() const { return m_name; }

    void SetDuration(float duration) { m_duration = duration; }
    float GetDuration() const { return m_duration; }

    void SetTicksPerSecond(float tps) { m_ticksPerSecond = tps; }
    float GetTicksPerSecond() const { return m_ticksPerSecond; }

    void AddBoneTrack(const BoneTrack& track);
    const std::vector<BoneTrack>& GetBoneTracks() const { return m_boneTracks; }

private:
    std::string m_name;
    float m_duration = 0.0f;
    float m_ticksPerSecond = 24.0f;
    std::vector<BoneTrack> m_boneTracks;
};
