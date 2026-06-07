//
// Created by rui ding on 2026/6/7.
//

#pragma once

#include "x/core/base.h"
#include "x/renderer/mesh.h"

#include <cstdint>

class GeometryGenerator {
public:
    GeometryGenerator() = delete;

    /**
     * @param size 立方体边长
     * @return 画立方体需要的数据
     */
    static X::Ref<Mesh> CreateCube(float size = 1.0f);

    static X::Ref<Mesh> CreateSphere(float radius = 0.5f, uint32_t sectors = 36, uint32_t stacks = 18);
    static X::Ref<Mesh> CreateRing(float innerRadius, float outerRadius, uint32_t segments = 128);
};
