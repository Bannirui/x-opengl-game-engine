#type vertex
#version 330 core

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
    int TexIndex;
    float TilingFactor;
};
layout(location = 0) out VertexOutput Output;
layout(location = 4) out flat int v_EntityID;

layout(std140, binding = 0) uniform Camera
{
    mat4 u_ViewProjection;
};

void main()
{
    Output.Color = a_Color;
    Output.TexCoord = a_TexCoord;
    Output.TexIndex = a_TexIndex;
    Output.TilingFactor = a_TilingFactor;
    v_EntityID = a_EntityID;
    gl_Position = u_ViewProjection * vec4(a_Position, 1.0);
}

#type fragment
#version 330 core

struct VertexOutput
{
    vec4 Color;
    vec2 TexCoord;
    int TexIndex;
    float TilingFactor;
};
layout(location = 0) in VertexOutput Input;
layout(location = 4) in flat int v_EntityID;

layout(location = 0) out vec4 color;
layout(location = 1) out int color2;

layout(binding = 0) uniform sampler2D u_Textures[16];

void main()
{
#define TEXTURE_CASE(n) case n: texColor *= texture(u_Textures[n], Input.TexCoord * Input.TilingFactor); break;

    vec4 texColor = Input.Color;
    switch (Input.TexIndex) {
        TEXTURE_CASE(0)  TEXTURE_CASE(1)  TEXTURE_CASE(2)  TEXTURE_CASE(3)
        TEXTURE_CASE(4)  TEXTURE_CASE(5)  TEXTURE_CASE(6)  TEXTURE_CASE(7)
        TEXTURE_CASE(8)  TEXTURE_CASE(9)  TEXTURE_CASE(10) TEXTURE_CASE(11)
        TEXTURE_CASE(12) TEXTURE_CASE(13) TEXTURE_CASE(14) TEXTURE_CASE(15)
    }

#undef TEXTURE_CASE
    color = texColor;
    color2 = v_EntityID;
}