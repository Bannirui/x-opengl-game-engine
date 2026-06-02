//
// Created by rui ding on 2026/6/1.
//

#pragma once

#include "x/renderer/material.h"
#include "x/renderer/mesh.h"
#include "x/renderer/renderer_3D.h"
#include "x/renderer/shader.h"
#include "x/renderer/texture.h"
#include "x/renderer/texture_cube.h"
#include "x/renderer/uniform_buffer.h"

#include <glm/glm.hpp>

#include <vector>

struct MeshDrawCommand {
    X::Ref<Mesh> Mesh;
    glm::mat4 Transform;
    int32_t EntityID;
};

struct MaterialBucket {
    X::Ref<Material> Material;
    std::vector<MeshDrawCommand> Commands;
};

struct LightData {
    glm::vec3 Direction;
    float Padding0;
    glm::vec3 Ambient;
    float Padding1;
    glm::vec3 Diffuse;
    float Padding2;
    glm::vec3 Specular;
    float Padding3;
};

struct PBRSettingsData {
    glm::vec3 CameraPosition;
    float Exposure;
};

struct Renderer3DData {
    std::vector<MaterialBucket> Buckets;

    X::Ref<UniformBuffer> CameraUBO;
    X::Ref<UniformBuffer> ModelUBO;
    X::Ref<UniformBuffer> LightUBO;
    X::Ref<UniformBuffer> PBRUBO;

    X::Ref<Texture2D> WhiteTexture;
    X::Ref<TextureCube> EnvironmentMap;
    X::Ref<TextureCube> IrradianceMap;
    X::Ref<TextureCube> PrefilterMap;
    uint32_t BRDFLUTTexture = 0;
    uint32_t DefaultShadowMap = 0;

    X::Ref<Shader> DefaultShader;
    X::Ref<Shader> PBRShader;
    X::Ref<Shader> SkyboxShader;

    struct CameraData {
        glm::mat4 ViewProjection;
    };
    CameraData CameraBuffer;

    LightData LightBuffer;
    PBRSettingsData PBRBuffer;

    Renderer3D::Statistics Stats;
};
