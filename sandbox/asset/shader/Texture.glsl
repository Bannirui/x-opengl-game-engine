#type vertex
#version 330 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec4 a_Color;
layout(location = 2) in vec2 a_TexCoord;
layout(location = 3) in int a_TexIndex;
layout(location = 4) in float a_TilingFactor;
layout(location = 5) in int a_EntityID;

layout(location = 0) out vec4 v_Color;
layout(location = 1) out vec2 v_TexCoord;
layout(location = 2) flat out int v_TexIndex;
layout(location = 3) out float v_TilingFactor;
layout(location = 4) flat out int v_EntityID;

layout(std140, binding = 0) uniform Camera
{
    mat4 u_ViewProjection;
};

void main()
{
    v_Color = a_Color;
    v_TexCoord = a_TexCoord;
    v_TexIndex = a_TexIndex;
    v_TilingFactor = a_TilingFactor;
    v_EntityID = a_EntityID;
    gl_Position = u_ViewProjection * vec4(a_Position, 1.0);
}

#type fragment
#version 330 core

layout(location = 0) in vec4 v_Color;
layout(location = 1) in vec2 v_TexCoord;
layout(location = 2) flat in int v_TexIndex;
layout(location = 3) in float v_TilingFactor;
layout(location = 4) flat in int v_EntityID;

layout(location = 0) out vec4 color;
layout(location = 1) out int color2;

layout(binding = 0) uniform sampler2D u_Textures[16];

void main()
{
#define TEXTURE_CASE(n) case n: texColor *= texture(u_Textures[n], v_TexCoord * v_TilingFactor); break;

    vec4 texColor = v_Color;
    switch (v_TexIndex) {
        TEXTURE_CASE(0)  TEXTURE_CASE(1)  TEXTURE_CASE(2)  TEXTURE_CASE(3)
        TEXTURE_CASE(4)  TEXTURE_CASE(5)  TEXTURE_CASE(6)  TEXTURE_CASE(7)
        TEXTURE_CASE(8)  TEXTURE_CASE(9)  TEXTURE_CASE(10) TEXTURE_CASE(11)
        TEXTURE_CASE(12) TEXTURE_CASE(13) TEXTURE_CASE(14) TEXTURE_CASE(15)
    }

#undef TEXTURE_CASE
    color = texColor;
    color2 = v_EntityID;
}
