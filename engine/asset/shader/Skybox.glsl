#type vertex
#version 450 core

layout(location = 0) in vec3 a_Position;

layout(std140, binding = 0) uniform Camera {
    mat4 u_ViewProjection;
};

out vec3 v_WorldPos;

void main() {
    vec4 pos = u_ViewProjection * vec4(a_Position, 1.0);
    gl_Position = pos.xyww;
    v_WorldPos = a_Position;
}

#type fragment
#version 450 core

in vec3 v_WorldPos;
out vec4 o_Color;

uniform samplerCube u_Skybox;
uniform float u_Exposure;

void main() {
    vec3 color = texture(u_Skybox, v_WorldPos).rgb;
    color = vec3(1.0) - exp(-color * u_Exposure);
    color = pow(color, vec3(1.0 / 2.2));
    o_Color = vec4(color, 1.0);
}
