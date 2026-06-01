#type vertex
#version 450 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec2 a_TexCoord;

layout(std140, binding = 0) uniform Camera {
    mat4 u_ViewProjection;
};

layout(std140, binding = 1) uniform Model {
    mat4 u_Model;
};

struct VertexOutput {
    vec3 WorldNormal;
    vec3 WorldPosition;
    vec2 TexCoord;
    int EntityID;
};

layout(location = 0) out VertexOutput Output;

uniform int u_EntityID;

void main() {
    vec4 worldPos = u_Model * vec4(a_Position, 1.0);
    Output.WorldPosition = worldPos.xyz;
    Output.WorldNormal = mat3(transpose(inverse(u_Model))) * a_Normal;
    Output.TexCoord = a_TexCoord;
    Output.EntityID = u_EntityID;
    gl_Position = u_ViewProjection * worldPos;
}

#type fragment
#version 450 core

struct VertexOutput {
    vec3 WorldNormal;
    vec3 WorldPosition;
    vec2 TexCoord;
    int EntityID;
};

layout(location = 0) in VertexOutput Input;

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
    vec3 normal = normalize(Input.WorldNormal);
    vec3 lightDir = normalize(-u_LightDirection);
    vec3 viewDir = normalize(-Input.WorldPosition);

    vec3 ambient = u_LightAmbient;

    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = u_LightDiffuse * diff;

    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), u_Shininess);
    vec3 specular = u_LightSpecular * spec * u_MaterialSpecular;

    vec4 texColor = texture(u_DiffuseMap, Input.TexCoord);

    vec3 result = (ambient + diffuse) * texColor.rgb * u_MaterialDiffuse + specular;
    o_Color = vec4(result, texColor.a);
    o_EntityID = Input.EntityID;
}
