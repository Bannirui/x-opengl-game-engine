//
// Created by rui ding on 2026/6/1.
//

#pragma once

#include "x/core/base.h"

#include "x/renderer/material.h"
#include "x/renderer/mesh.h"

#include <string>
#include <vector>

class Model {
public:
    struct SubMesh {
        Ref<Mesh> MeshHandle;
        Ref<Material> MaterialHandle;
        std::string Name;
    };

    static Ref<Model> Load(const std::string& filepath, const Ref<Shader>& defaultShader = nullptr);

    const std::vector<SubMesh>& GetSubMeshes() const { return m_subMeshes; }
    const std::string& GetDirectory() const { return m_directory; }

    Model() = default;

public:

    std::vector<SubMesh> m_subMeshes;
    std::string m_directory;
};
