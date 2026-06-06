#type vertex
#version 450 core

// attribute0传进来的变量
layout(location = 0) in vec3 a_Position;

// UBO变量
layout(std140, binding = 0) uniform Camera {
    mat4 u_ViewProjection;
};

// 给fragment的attribute0传世界矩阵
layout(location = 0) out vec3 v_WorldPos;

void main() {
    vec4 pos = u_ViewProjection * vec4(a_Position, 1.0);
    gl_Position = pos.xyww;
    v_WorldPos = a_Position;
}

#type fragment
#version 450 core

// vertex shader传过来的世界坐标
layout(location = 0) in vec3 v_WorldPos;

// 传进来的uniform变量 采样器
uniform samplerCube u_Skybox;
// 传进来的曝光度
uniform float u_Exposure;

// 输出
layout(location = 0) out vec4 o_Color;

void main() {
    vec3 color = texture(u_Skybox, v_WorldPos).rgb;
    color = vec3(1.0) - exp(-color * u_Exposure);
    color = pow(color, vec3(1.0 / 2.2));
    o_Color = vec4(color, 1.0);
}
