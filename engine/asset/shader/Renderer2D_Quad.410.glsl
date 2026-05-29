#type vertex
#version 410 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec4 a_Color;
layout(location = 2) in vec2 a_TexCoord;
layout(location = 3) in int a_TexIndex;
layout(location = 4) in float a_TilingFactor;
layout(location = 5) in int a_EntityID;

struct VertexOutput
{
    vec4 Color;
    vec2 TexCoord;
    float TilingFactor;
};
layout(location = 0) out VertexOutput Output;
layout(location = 3) flat out int v_TexIndex;
layout(location = 4) flat out int v_EntityID;

layout(std140) uniform Camera
{
    mat4 u_ViewProjection;
};

void main()
{
    Output.Color = a_Color;
    Output.TexCoord = a_TexCoord;
    Output.TilingFactor = a_TilingFactor;
    v_TexIndex = a_TexIndex;
    v_EntityID = a_EntityID;
    gl_Position = u_ViewProjection * vec4(a_Position, 1.0);
}

#type fragment
#version 410 core

struct VertexOutput
{
    vec4 Color;
    vec2 TexCoord;
    float TilingFactor;
};
layout(location = 0) in VertexOutput Output;
layout(location = 3) flat in int v_TexIndex;
layout(location = 4) flat in int v_EntityID;

layout(location = 0) out vec4 o_Color;
layout(location = 1) out int o_EntityID;

// 用glUniform1iv把sampler数据传了进来
uniform sampler2D u_Textures[16];

void main()
{
#define TEXTURE_CASE(n) case n: texColor *= texture(u_Textures[n], Output.TexCoord * Output.TilingFactor); break;

    vec4 texColor = Output.Color;
    switch (v_TexIndex) {
        TEXTURE_CASE(0)  TEXTURE_CASE(1)  TEXTURE_CASE(2)  TEXTURE_CASE(3)
        TEXTURE_CASE(4)  TEXTURE_CASE(5)  TEXTURE_CASE(6)  TEXTURE_CASE(7)
        TEXTURE_CASE(8)  TEXTURE_CASE(9)  TEXTURE_CASE(10) TEXTURE_CASE(11)
        TEXTURE_CASE(12) TEXTURE_CASE(13) TEXTURE_CASE(14) TEXTURE_CASE(15)
    }

#undef TEXTURE_CASE
    if(texColor.a == 0.0) {
        discard;
    }
    o_Color = texColor;
    o_EntityID = v_EntityID;
}
