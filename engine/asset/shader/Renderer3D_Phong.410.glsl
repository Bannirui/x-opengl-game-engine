#type vertex
#version 410 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec2 a_TexCoord;

layout(std140) uniform Camera {
    mat4 u_ViewProjection;
};

layout(std140) uniform Model {
    mat4 u_Model;
};

out vec3 v_WorldNormal;
out vec3 v_WorldPosition;
out vec2 v_TexCoord;
flat out int v_EntityID;

uniform int u_EntityID;

void main() {
    vec4 worldPos = u_Model * vec4(a_Position, 1.0);
    v_WorldPosition = worldPos.xyz;
    v_WorldNormal = mat3(transpose(inverse(u_Model))) * a_Normal;
    v_TexCoord = a_TexCoord;
    v_EntityID = u_EntityID;
    gl_Position = u_ViewProjection * worldPos;
}

#type fragment
#version 410 core

in vec3 v_WorldNormal;
in vec3 v_WorldPosition;
in vec2 v_TexCoord;
flat in int v_EntityID;

layout(location = 0) out vec4 o_Color;
layout(location = 1) out int o_EntityID;

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

layout(std140) uniform LightBlock {
    vec3 u_Ambient;
    int u_LightCount;
    GPULight u_Lights[MAX_LIGHTS];
};

uniform sampler2D u_DiffuseMap;
uniform float u_Shininess;
uniform vec3 u_MaterialDiffuse;
uniform vec3 u_MaterialSpecular;

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
