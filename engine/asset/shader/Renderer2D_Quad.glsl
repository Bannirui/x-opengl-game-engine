#type vertex
#version 450 core

// vertex的入参 顶点属性vertex attribute OpenGL规范最少16个 实际多少由每个平台硬件决定
// 入参attribute和uniform的区别在于 每个顶点的attribute都不一样 uniform是整个shader公用的
layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec4 a_Color;
layout(location = 2) in vec2 a_TexCoord;
// 引擎贴图缓冲区有一批贴图 用哪个 0号位上是引擎自带的
layout(location = 3) in int a_TexIndex;
layout(location = 4) in float a_TilingFactor;
// 实体id 用于鼠标拾取
layout(location = 5) in int a_EntityID;

struct VertexOutput
{
    vec4 Color;
    vec2 TexCoord;
    float TilingFactor;
};
layout(location = 0) out VertexOutput Output;
layout(location = 3) out flat int v_TexIndex;
layout(location = 4) out flat int v_EntityID;

// UBO的用法 拿到共享的UBO的常量缓存区Camera 所有绑到slot=0的shader着色器都可以拿到这个变量
layout(std140, binding = 0) uniform Camera
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
#version 450 core

struct VertexOutput
{
    vec4 Color;
    vec2 TexCoord;
    float TilingFactor;
};
layout(location = 0) in VertexOutput Input;
layout(location = 3) in flat int v_TexIndex;
layout(location = 4) in flat int v_EntityID;

layout(location = 0) out vec4 o_Color;
layout(location = 1) out int o_EntityID;

// 用glUniform1iv把sampler数据传了进来
// 简单情况下在渲染之前调用一次shader的bind OpenGL就会自动地把2D插槽上的texture发送给frag着色器 所以只要uniform sampler2D xxx就拿到了
// 默认情况是纹理单元0号的那个纹理对象
// 现在引擎内部的做法是在初始化texture的时候定义了一个16个大小的texutre缓冲区 在渲染前会变量都bind到插槽上
// 然后手动传uniform数组到shader方式把纹理对象和纹理单元的映射关系传过来 相当于segment会收到16个采样器
// 因此vertex attribute会传进来texture index说明用哪个采样器 两个组合起来 segment就知道用哪个采样器了
uniform sampler2D u_Textures[16];

void main()
{
    // glsl不支持在sampler数组用变量作为下标 texture(u_Textures[int(v_TexIndex)] 用宏展开switch...case
#define TEXTURE_CASE(n) case n: texColor *= texture(u_Textures[n], Input.TexCoord * Input.TilingFactor); break;

    vec4 texColor = Input.Color;
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