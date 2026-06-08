// =============================================================================
// Renderer3D_Phong.glsl Phong氏光照模型3D渲染Shader
//
// 注意事项 SPIR-V编译要求
//   1 所有in/out变量必须显式指定layout(location = N)
//   2 含int类型的变量在stage间传递必须加flat 禁止插值
//   3 非opaque型uniform(float/vec/int)必须显式指定location
//   4 opaque型uniform(sampler2D/samplerCube)不需要location
//   5 UBO用binding标识 不需要location
// =============================================================================

// —————————————————————— Vertex Shader ——————————————————————
#type vertex
#version 450 core

// 顶点输入 Vertex Buffer传入 location与cpp端VertexBufferLayout对齐
layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec2 a_TexCoord;

// UBO 相机矩阵 binding=0 引擎每帧更新
layout(std140, binding = 0) uniform Camera {
    mat4 u_ViewProjection;
};

// UBO 模型矩阵 binding=1 引擎每物体更新
layout(std140, binding = 1) uniform Model {
    mat4 u_Model;
};

// 顶点输出传递给Fragment Shader
layout(location = 0) out vec3 v_WorldNormal;
layout(location = 1) out vec3 v_WorldPosition;
layout(location = 2) out vec2 v_TexCoord;
// flat 每个图元片段取同一个值
layout(location = 3) flat out int v_EntityID;

// Uniform变量 实体ID 用于鼠标拾取
uniform int u_EntityID;

void main() {
    // 计算世界空间坐标
    vec4 worldPos = u_Model * vec4(a_Position, 1.0);
    v_WorldPosition = worldPos.xyz;
    // 计算世界空间法线 非均匀缩放下用逆转置矩阵
    v_WorldNormal = mat3(transpose(inverse(u_Model))) * a_Normal;
    v_TexCoord = a_TexCoord;
    v_EntityID = u_EntityID;
    // 裁剪空间坐标 OpenGL内置变量
    gl_Position = u_ViewProjection * worldPos;
}

// —————————————————————— Fragment Shader ——————————————————————
#type fragment
#version 450 core

// 顶点输出过来的 片段输入location必须与vertex端一一对应匹配
layout(location = 0) in vec3 v_WorldNormal;
layout(location = 1) in vec3 v_WorldPosition;
layout(location = 2) in vec2 v_TexCoord;
layout(location = 3) flat in int v_EntityID;

// 帧缓冲输出
layout(location = 0) out vec4 o_Color;
layout(location = 1) out int o_EntityID;

// UBO LightBlock binding=2 引擎每帧更新
#define MAX_LIGHTS 8
#define LIGHT_TYPE_DIRECTIONAL 0
#define LIGHT_TYPE_POINT 1
#define LIGHT_TYPE_SPOT 2

struct GPULight {
    vec4 ColorAndIntensity;
    vec4 PositionAndRange;
    int Type;
    float SpotInnerCone;
    float SpotOuterCone;
    float _pad;
};

layout(std140, binding = 2) uniform LightBlock {
    vec3 u_Ambient;
    int u_LightCount;
    GPULight u_Lights[MAX_LIGHTS];
};

// Opaque型 uniform变量 纹理采样器 不需要location
uniform sampler2D u_DiffuseMap;

// Phong材质参数
uniform float u_Shininess;           // 高光系数 (shininess exponent)
uniform vec3 u_MaterialDiffuse;      // 材质漫反射颜色
uniform vec3 u_MaterialSpecular;     // 材质镜面反射颜色

void main() {
    vec3 normal = normalize(v_WorldNormal);
    vec3 viewDir = normalize(-v_WorldPosition);

    vec3 ambient = u_Ambient;
    vec3 diffuseSum = vec3(0.0);
    vec3 specularSum = vec3(0.0);

    for (int i = 0; i < u_LightCount && i < MAX_LIGHTS; i++) {
        vec3 lightColor = u_Lights[i].ColorAndIntensity.rgb;
        float intensity = u_Lights[i].ColorAndIntensity.w;
        vec3 lightPosOrDir = u_Lights[i].PositionAndRange.xyz;
        float range = u_Lights[i].PositionAndRange.w;
        bool isPoint = u_Lights[i].Type == LIGHT_TYPE_POINT;

        vec3 lightDir;
        float attenuation = 1.0;

        if (isPoint) {
            lightDir = normalize(lightPosOrDir - v_WorldPosition);
            float dist = length(lightPosOrDir - v_WorldPosition);
            attenuation = 1.0 / (1.0 + dist * dist / max(range * range, 0.0001));
        } else {
            lightDir = normalize(-lightPosOrDir);
        }

        float diff = max(dot(normal, lightDir), 0.0);
        diffuseSum += lightColor * intensity * diff * attenuation;

        vec3 halfwayDir = normalize(lightDir + viewDir);
        float spec = pow(max(dot(normal, halfwayDir), 0.0), u_Shininess);
        specularSum += lightColor * intensity * spec * attenuation;
    }

    vec4 texColor = texture(u_DiffuseMap, v_TexCoord);
    vec3 result = (ambient + diffuseSum) * texColor.rgb * u_MaterialDiffuse + specularSum * u_MaterialSpecular;
    o_Color = vec4(result, texColor.a);
    o_EntityID = v_EntityID;
}
