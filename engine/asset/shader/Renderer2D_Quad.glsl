#type vertex
#version 450 core

// 外面传给vertex的attribute变量
// vertex的入参 顶点属性vertex attribute OpenGL规范最少16个 实际多少由每个平台硬件决定
// 入参attribute和uniform的区别在于 每个顶点的attribute都不一样 uniform是整个shader公用的
layout(location = 0) in vec3 a_Position;
// 顶点颜色
layout(location = 1) in vec4 a_Color;
// uv坐标
layout(location = 2) in vec2 a_TexCoord;
// 引擎贴图缓冲区有一批贴图 用哪个 0号位上是引擎自带的
layout(location = 3) in int a_TexIndex;
// 纹理复制调节 铺几个纹理图片
layout(location = 4) in float a_TilingFactor;
// 实体id 用于鼠标拾取
layout(location = 5) in int a_EntityID;

// vertex输出给fragment的变量
// 顶点颜色
layout(location = 0) out vec4 v_Color;
// uv坐标
layout(location = 1) out vec2 v_TexCoord;
// 铺几个纹理图片
layout(location = 2) flat out float v_TilingFactor;
// 用哪个纹理采样器
layout(location = 3) flat out int v_TexIndex;
// 用于鼠标拾取
layout(location = 4) flat out int v_EntityID;

// UBO的用法 拿到共享的UBO的常量缓存区Camera 所有绑到slot=0的shader着色器都可以拿到这个变量
layout(std140, binding = 0) uniform Camera
{
    mat4 u_ViewProjection;
};

void main()
{
    v_Color = a_Color;
    v_TexCoord = a_TexCoord;
    v_TilingFactor = a_TilingFactor;
    v_TexIndex = a_TexIndex;
    v_EntityID = a_EntityID;
    gl_Position = u_ViewProjection * vec4(a_Position, 1.0);
}

#type fragment
#version 450 core

// vertex传过来的attribute变量
// 顶点颜色
layout(location = 0) in vec4 v_Color;
// uv坐标
layout(location = 1) in vec2 v_TexCoord;
// 铺几个纹理图片
layout(location = 2) flat in float v_TilingFactor;
// 用哪个纹理采样器
layout(location = 3) flat in int v_TexIndex;
// 用于鼠标拾取
layout(location = 4) flat in int v_EntityID;

// fragment输出的变量
layout(location = 0) out vec4 o_Color;
layout(location = 1) out int o_EntityID;

// 外面传到shader的uniform变量
uniform sampler2D u_Textures[16];

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
    if(texColor.a == 0.0) {
        discard;
    }
    o_Color = texColor;
    o_EntityID = v_EntityID;
}
