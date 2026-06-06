#type vertex
#version 450 core

// 顶点位置
layout(location = 0) in vec3 a_Position;

// 投影矩阵P*观察矩阵V
uniform mat4 u_ViewProjection;
// 模型矩阵M
uniform mat4 u_Transform;

void main()
{
    gl_Position = u_ViewProjection * u_Transform * vec4(a_Position, 1.0);
}

#type fragment
#version 450 core

uniform vec4 u_Color;

layout (location = 0) out vec4 color;

void main()
{
    color = u_Color;
}