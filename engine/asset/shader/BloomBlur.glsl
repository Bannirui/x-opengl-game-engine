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
uniform sampler2D u_Image;
uniform vec2 u_TexelSize;
uniform bool u_Horizontal;
const float weight[5] = float[](0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);
void main() {
    vec3 result = texture(u_Image, v_TexCoord).rgb * weight[0];
    vec2 dir = u_Horizontal ? vec2(u_TexelSize.x, 0.0) : vec2(0.0, u_TexelSize.y);
    for (int i = 1; i < 5; ++i) {
        result += texture(u_Image, v_TexCoord + dir * float(i)).rgb * weight[i];
        result += texture(u_Image, v_TexCoord - dir * float(i)).rgb * weight[i];
    }
    o_Color = vec4(result, 1.0);
}
