//
// Created by rui ding on 2026/6/7.
//

#include "x/renderer/3d/generator.h"

#include <glm/glm.hpp>

#include <vector>

Ref<Mesh> GeometryGenerator::CreateCube(float size) {
    float h = size * 0.5f;
    // clang-format off
    std::vector<StaticMeshVertex> vertices = {
        {{-h, -h, h}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
        {{h, -h, h}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}},
        {{h, h, h}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
        {{-h, h, h}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
        {{h, -h, -h}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f}},
        {{-h, -h, -h}, {0.0f, 0.0f, -1.0f}, {1.0f, 0.0f}},
        {{-h, h, -h}, {0.0f, 0.0f, -1.0f}, {1.0f, 1.0f}},
        {{h, h, -h}, {0.0f, 0.0f, -1.0f}, {0.0f, 1.0f}},
        {{-h, h, h}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
        {{h, h, h}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
        {{h, h, -h}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f}},
        {{-h, h, -h}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f}},
        {{-h, -h, -h}, {0.0f, -1.0f, 0.0f}, {0.0f, 0.0f}},
        {{h, -h, -h}, {0.0f, -1.0f, 0.0f}, {1.0f, 0.0f}},
        {{h, -h, h}, {0.0f, -1.0f, 0.0f}, {1.0f, 1.0f}},
        {{-h, -h, h}, {0.0f, -1.0f, 0.0f}, {0.0f, 1.0f}},
        {{h, -h, h}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
        {{h, -h, -h}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
        {{h, h, -h}, {1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}},
        {{h, h, h}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}},
        {{-h, -h, -h}, {-1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
        {{-h, -h, h}, {-1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
        {{-h, h, h}, {-1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}},
        {{-h, h, -h}, {-1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}},
    };

    std::vector<uint32_t> indices = {
        0,  1,  2,  2,  3,  0,  //
        4,  5,  6,  6,  7,  4,  //
        8,  9,  10, 10, 11, 8,  //
        12, 13, 14, 14, 15, 12,  //
        16, 17, 18, 18, 19, 16,  //
        20, 21, 22, 22, 23, 20,  //
    };
    // clang-format on
    return Mesh::Create(vertices, indices);
}
