#type vertex
#version 450 core

layout(location = 0) in vec3 a_Position;

layout(std140, binding = 0) uniform LightSpace {
    mat4 u_LightViewProjection;
};

layout(std140, binding = 1) uniform Model {
    mat4 u_Model;
};

void main() {
    gl_Position = u_LightViewProjection * u_Model * vec4(a_Position, 1.0);
}

#type fragment
#version 450 core
void main() {}
