#type vertex
#version 450 core
layout(location = 0) in vec2 a_Position;
layout(location = 0) out vec2 v_TexCoord;
void main() {
    v_TexCoord = a_Position * 0.5 + 0.5;
    gl_Position = vec4(a_Position, 0.0, 1.0);
}

#type fragment
#version 450 core
layout(location = 0) in vec2 v_TexCoord;
layout(location = 0) out vec4 o_Color;
uniform sampler2D u_Scene;
uniform sampler2D u_Bloom;
layout(location = 0) uniform float u_Exposure;
void main() {
    vec3 scene = texture(u_Scene, v_TexCoord).rgb;
    vec3 bloom = texture(u_Bloom, v_TexCoord).rgb;
    vec3 color = scene + bloom;
    color = vec3(1.0) - exp(-color * u_Exposure);
    color = pow(color, vec3(1.0 / 2.2));
    o_Color = vec4(color, 1.0);
}
