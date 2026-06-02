#type vertex
#version 330 core
#extension GL_ARB_explicit_attrib_location : require
#extension GL_ARB_shading_language_420pack : require

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec2 a_TexCoord;

layout(std140, binding = 0) uniform Camera {
    mat4 u_ViewProjection;
};

layout(std140, binding = 1) uniform Model {
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
#version 330 core
#extension GL_ARB_explicit_attrib_location : require
#extension GL_ARB_shading_language_420pack : require

in vec3 v_WorldNormal;
in vec3 v_WorldPosition;
in vec2 v_TexCoord;
flat in int v_EntityID;

layout(location = 0) out vec4 o_Color;
layout(location = 1) out int o_EntityID;

layout(std140, binding = 2) uniform Light {
    vec3 u_LightDirection;
    vec3 u_LightAmbient;
    vec3 u_LightDiffuse;
    vec3 u_LightSpecular;
};

uniform sampler2D u_DiffuseMap;
uniform float u_Shininess;
uniform vec3 u_MaterialDiffuse;
uniform vec3 u_MaterialSpecular;

void main() {
    vec3 normal = normalize(v_WorldNormal);
    vec3 lightDir = normalize(-u_LightDirection);
    vec3 viewDir = normalize(-v_WorldPosition);

    vec3 ambient = u_LightAmbient;

    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = u_LightDiffuse * diff;

    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), u_Shininess);
    vec3 specular = u_LightSpecular * spec * u_MaterialSpecular;

    vec4 texColor = texture(u_DiffuseMap, v_TexCoord);

    vec3 result = (ambient + diffuse) * texColor.rgb * u_MaterialDiffuse + specular;
    o_Color = vec4(result, texColor.a);
    o_EntityID = v_EntityID;
}
