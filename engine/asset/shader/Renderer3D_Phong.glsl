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

// Uniform变量 实体ID 用于鼠标拾取 非opaque须指定location
layout(location = 0) uniform int u_EntityID;

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

// UBO 方向光 binding=2 引擎每帧更新
layout(std140, binding = 2) uniform Light {
    vec3 u_LightDirection;         // 光照方向 (指向光源)
    vec3 u_LightAmbient;           // 环境光分量
    vec3 u_LightDiffuse;           // 漫反射颜色
    vec3 u_LightSpecular;          // 镜面反射颜色
};

// Opaque型 uniform变量 纹理采样器 不需要location
uniform sampler2D u_DiffuseMap;

// Non-opaque型 uniform变量 Phong材质参数 SPIR-V要求显式layout(location)
layout(location = 0) uniform float u_Shininess;           // 高光系数 (shininess exponent)
layout(location = 1) uniform vec3 u_MaterialDiffuse;      // 材质漫反射颜色
layout(location = 2) uniform vec3 u_MaterialSpecular;     // 材质镜面反射颜色

void main() {
    // Phong光照模型
    vec3 normal = normalize(v_WorldNormal);                // 法线归一化
    vec3 lightDir = normalize(-u_LightDirection);          // 光线方向 (指向光源)
    vec3 viewDir = normalize(-v_WorldPosition);            // 视线方向 (指向摄像机)

    // 1 环境光Ambient
    vec3 ambient = u_LightAmbient;

    // 2 漫反射Diffuse
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = u_LightDiffuse * diff;

    // 3 镜面反射Specular
    vec3 halfwayDir = normalize(lightDir + viewDir);       // 半角向量
    float spec = pow(max(dot(normal, halfwayDir), 0.0), u_Shininess);
    vec3 specular = u_LightSpecular * spec * u_MaterialSpecular;

    // 4 纹理采样
    vec4 texColor = texture(u_DiffuseMap, v_TexCoord);

    // 5 合成=环境+漫反射 与纹理/材质颜色混合 再加镜面反射
    vec3 result = (ambient + diffuse) * texColor.rgb * u_MaterialDiffuse + specular;
    o_Color = vec4(result, texColor.a);
    o_EntityID = v_EntityID;
}
