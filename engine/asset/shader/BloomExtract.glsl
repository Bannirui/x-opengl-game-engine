#type vertex
#version 450 core
layout(location = 0) in vec2 a_Position;
out vec2 v_TexCoord;
void main() {
    v_TexCoord = a_Position * 0.5 + 0.5;
    gl_Position = vec4(a_Position, 0.0, 1.0);
}

#type fragment
#version 450 core
in vec2 v_TexCoord;
out vec4 o_Color;
uniform sampler2D u_Scene;
uniform float u_Threshold;
void main() {
    vec3 color = texture(u_Scene, v_TexCoord).rgb;
    float brightness = dot(color, vec3(0.2126, 0.7152, 0.0722));
    o_Color = vec4(color * step(u_Threshold, brightness), 1.0);
}
