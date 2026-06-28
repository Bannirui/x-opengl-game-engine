//
// Created by rui ding on 2026/6/1.
//

#include "x/renderer/model.h"

#include "x/renderer/shader.h"
#include "x/renderer/texture.h"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <sstream>
#include <unordered_map>

// ---- File util (internal) ----

static std::string readFile(const std::string& filepath) {
    std::ifstream in(filepath, std::ios::in | std::ios::binary);
    if (!in) {
        X_CORE_ERROR("Failed to open file: {}", filepath);
        return {};
    }
    in.seekg(0, std::ios::end);
    size_t size = in.tellg();
    in.seekg(0, std::ios::beg);
    std::string content;
    content.resize(size);
    in.read(&content[0], content.size());
    return content;
}

static std::string trim(const std::string& s) {
    size_t start = 0;
    while (start < s.size() && std::isspace(static_cast<unsigned char>(s[start]))) ++start;
    size_t end = s.size();
    while (end > start && std::isspace(static_cast<unsigned char>(s[end - 1]))) --end;
    return s.substr(start, end - start);
}

static bool startsWith(const std::string& s, const std::string& prefix) {
    return s.size() >= prefix.size() && s.compare(0, prefix.size(), prefix) == 0;
}

static std::vector<std::string> split(const std::string& s, char delimiter) {
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(s);
    while (std::getline(tokenStream, token, delimiter)) {
        tokens.push_back(trim(token));
    }
    return tokens;
}

static std::string extractDirectory(const std::string& filepath) {
    size_t pos = filepath.find_last_of("/\\");
    return pos == std::string::npos ? "." : filepath.substr(0, pos);
}

// ---- OBJ parsing helpers ----

struct OBJIndex {
    int position = -1;
    int texcoord = -1;
    int normal = -1;

    bool operator==(const OBJIndex& other) const {
        return position == other.position && texcoord == other.texcoord && normal == other.normal;
    }
};

struct OBJIndexHash {
    size_t operator()(const OBJIndex& idx) const {
        return (size_t)(idx.position * 73856093) ^ (size_t)(idx.texcoord * 19349663) ^ (size_t)(idx.normal * 83492791);
    }
};

static OBJIndex parseFaceVertex(const std::string& token) {
    OBJIndex idx;
    auto parts = split(token, '/');
    if (!parts.empty() && !parts[0].empty()) {
        idx.position = std::stoi(parts[0]);
    }
    if (parts.size() > 1 && !parts[1].empty()) {
        idx.texcoord = std::stoi(parts[1]);
    }
    if (parts.size() > 2 && !parts[2].empty()) {
        idx.normal = std::stoi(parts[2]);
    }
    return idx;
}

// ---- MTL parsing ----

struct MTLMaterial {
    std::string Name;
    glm::vec3 DiffuseColor{1.0f};
    glm::vec3 SpecularColor{0.5f};
    float Shininess = 32.0f;
    std::string DiffuseTexture;
};

static std::unordered_map<std::string, MTLMaterial> parseMTL(const std::string& filepath) {
    X_PROFILE_FUNCTION();

    std::string content = readFile(filepath);
    if (content.empty()) return {};

    std::unordered_map<std::string, MTLMaterial> materials;
    MTLMaterial* current = nullptr;

    std::istringstream stream(content);
    std::string line;
    while (std::getline(stream, line)) {
        line = trim(line);
        if (line.empty() || line[0] == '#') continue;

        if (startsWith(line, "newmtl ")) {
            std::string name = trim(line.substr(7));
            materials[name] = MTLMaterial{};
            materials[name].Name = name;
            current = &materials[name];
        } else if (current && startsWith(line, "Kd ")) {
            auto parts = split(line.substr(3), ' ');
            if (parts.size() >= 3) {
                current->DiffuseColor = {std::stof(parts[0]), std::stof(parts[1]), std::stof(parts[2])};
            }
        } else if (current && startsWith(line, "Ks ")) {
            auto parts = split(line.substr(3), ' ');
            if (parts.size() >= 3) {
                current->SpecularColor = {std::stof(parts[0]), std::stof(parts[1]), std::stof(parts[2])};
            }
        } else if (current && startsWith(line, "Ns ")) {
            auto parts = split(line.substr(3), ' ');
            if (!parts.empty()) {
                float ns = std::stof(parts[0]);
                current->Shininess = glm::clamp(ns * 0.128f, 1.0f, 256.0f);
            }
        } else if (current && startsWith(line, "map_Kd ")) {
            current->DiffuseTexture = trim(line.substr(7));
        }
    }

    return materials;
}

// ---- Main OBJ + MTL loader ----

Ref<Model> Model::Load(const std::string& filepath, const Ref<Shader>& defaultShader) {
    X_PROFILE_FUNCTION();

    std::string content = readFile(filepath);
    if (content.empty()) {
        X_CORE_ERROR("Failed to read OBJ file: {}", filepath);
        return nullptr;
    }

    std::string directory = extractDirectory(filepath);

    // Storage for parsed data
    std::vector<glm::vec3> positions;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec2> texcoords;

    // Grouping: materialName → list of (indices, ...)
    struct FaceGroup {
        std::string MaterialName;
        std::vector<std::vector<OBJIndex>> Faces;  // each face is 3 or 4 OBJIndex
    };
    std::vector<FaceGroup> groups;
    std::string currentMaterial = "default";

    // Parse OBJ
    {
        std::istringstream stream(content);
        std::string line;
        while (std::getline(stream, line)) {
            line = trim(line);
            if (line.empty() || line[0] == '#') continue;

            if (startsWith(line, "v ")) {
                auto parts = split(line.substr(2), ' ');
                if (parts.size() >= 3) {
                    positions.push_back({std::stof(parts[0]), std::stof(parts[1]), std::stof(parts[2])});
                }
            } else if (startsWith(line, "vn ")) {
                auto parts = split(line.substr(3), ' ');
                if (parts.size() >= 3) {
                    normals.push_back({std::stof(parts[0]), std::stof(parts[1]), std::stof(parts[2])});
                }
            } else if (startsWith(line, "vt ")) {
                auto parts = split(line.substr(3), ' ');
                if (parts.size() >= 2) {
                    texcoords.push_back({std::stof(parts[0]), std::stof(parts[1])});
                }
            } else if (startsWith(line, "usemtl ")) {
                currentMaterial = trim(line.substr(7));
            } else if (startsWith(line, "mtllib ")) {
                // MTL parsing deferred — we just note it
            } else if (startsWith(line, "f ")) {
                auto parts = split(line.substr(2), ' ');
                std::vector<OBJIndex> faceIndices;
                for (const auto& token : parts) {
                    faceIndices.push_back(parseFaceVertex(token));
                }
                if (faceIndices.size() >= 3) {
                    if (groups.empty() || groups.back().MaterialName != currentMaterial) {
                        groups.push_back({currentMaterial, {}});
                    }
                    groups.back().Faces.push_back(std::move(faceIndices));
                }
            }
        }
    }

    // Parse MTL file if available
    std::unordered_map<std::string, MTLMaterial> mtlMaterials;
    std::string mtlPath = directory + "/" + filepath.substr(0, filepath.find_last_of('.')) + ".mtl";
    {
        std::ifstream test(mtlPath);
        if (test.good()) {
            mtlMaterials = parseMTL(mtlPath);
        }
    }

    // Resolve default shader
    Ref<Shader> shader = defaultShader;
    if (!shader) {
        shader = Shader::Create("asset/shader/Renderer3D_Phong.glsl");
    }

    // Build sub-meshes from face groups
    Ref<Model> model = CreateRef<Model>();
    model->m_directory = directory;

    for (auto& group : groups) {
        std::vector<StaticMeshVertex> vertices;
        std::vector<uint32_t> indices;
        std::unordered_map<OBJIndex, uint32_t, OBJIndexHash> indexMap;

        // Triangulate faces
        for (auto& face : group.Faces) {
            if (face.size() == 3) {
                // Triangle — process 3 vertices
                for (int i = 0; i < 3; ++i) {
                    OBJIndex& idx = face[i];
                    auto it = indexMap.find(idx);
                    if (it != indexMap.end()) {
                        indices.push_back(it->second);
                    } else {
                        StaticMeshVertex vert{};
                        if (idx.position > 0 && idx.position <= (int)positions.size()) {
                            vert.Position = positions[idx.position - 1];
                        }
                        if (idx.normal > 0 && idx.normal <= (int)normals.size()) {
                            vert.Normal = normals[idx.normal - 1];
                        }
                        if (idx.texcoord > 0 && idx.texcoord <= (int)texcoords.size()) {
                            vert.TexCoord = texcoords[idx.texcoord - 1];
                        }
                        uint32_t newIndex = static_cast<uint32_t>(vertices.size());
                        indexMap[idx] = newIndex;
                        vertices.push_back(vert);
                        indices.push_back(newIndex);
                    }
                }
            } else if (face.size() == 4) {
                // Quad → triangulate: 0-1-2, 0-2-3
                for (int i : {0, 1, 2, 0, 2, 3}) {
                    OBJIndex& idx = face[i];
                    auto it = indexMap.find(idx);
                    if (it != indexMap.end()) {
                        indices.push_back(it->second);
                    } else {
                        StaticMeshVertex vert{};
                        if (idx.position > 0 && idx.position <= (int)positions.size()) {
                            vert.Position = positions[idx.position - 1];
                        }
                        if (idx.normal > 0 && idx.normal <= (int)normals.size()) {
                            vert.Normal = normals[idx.normal - 1];
                        }
                        if (idx.texcoord > 0 && idx.texcoord <= (int)texcoords.size()) {
                            vert.TexCoord = texcoords[idx.texcoord - 1];
                        }
                        uint32_t newIndex = static_cast<uint32_t>(vertices.size());
                        indexMap[idx] = newIndex;
                        vertices.push_back(vert);
                        indices.push_back(newIndex);
                    }
                }
            }
        }

        if (vertices.empty() || indices.empty()) continue;

        // Generate normals if none provided
        if (normals.empty()) {
            for (size_t i = 0; i < indices.size(); i += 3) {
                auto& v0 = vertices[indices[i]];
                auto& v1 = vertices[indices[i + 1]];
                auto& v2 = vertices[indices[i + 2]];
                glm::vec3 edge1 = v1.Position - v0.Position;
                glm::vec3 edge2 = v2.Position - v0.Position;
                glm::vec3 faceNormal = glm::normalize(glm::cross(edge1, edge2));
                v0.Normal = faceNormal;
                v1.Normal = faceNormal;
                v2.Normal = faceNormal;
            }
        }

        // Create mesh and material for this group
        auto mesh = Mesh::Create(vertices, indices);

        Ref<Material> material;
        MTLMaterial* mtl = nullptr;
        auto mtlIt = mtlMaterials.find(group.MaterialName);
        if (mtlIt != mtlMaterials.end()) {
            mtl = &mtlIt->second;
        }

        material = Material::Create(shader);
        if (mtl) {
            if (!mtl->DiffuseTexture.empty()) {
                std::string texPath = directory + "/" + mtl->DiffuseTexture;
                auto texture = Texture2D::Create(texPath);
                if (texture->IsLoaded()) {
                    material->SetTexture("u_DiffuseMap", texture);
                }
            }
            material->SetFloat3("u_MaterialDiffuse", mtl->DiffuseColor);
            material->SetFloat3("u_MaterialSpecular", mtl->SpecularColor);
            material->SetFloat("u_Shininess", mtl->Shininess);
        } else {
            material->SetFloat3("u_MaterialDiffuse", glm::vec3(0.8f));
            material->SetFloat3("u_MaterialSpecular", glm::vec3(0.5f));
            material->SetFloat("u_Shininess", 32.0f);
        }

        Model::SubMesh subMesh;
        subMesh.MeshHandle = mesh;
        subMesh.MaterialHandle = material;
        subMesh.Name = group.MaterialName;
        model->m_subMeshes.push_back(std::move(subMesh));
    }

    X_CORE_INFO("Loaded model '{}': {} sub-meshes, {} vertices, {} indices",
                 filepath, model->m_subMeshes.size(),
                 [&]() {
                     size_t v = 0;
                     for (auto& s : model->m_subMeshes) v += s.MeshHandle->GetIndexCount();
                     return v;
                 }(),
                 positions.size());

    return model;
}
