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
out float o_Color;
uniform sampler2D u_SSAOInput;
uniform vec2 u_TexelSize;
void main() {
    float result = 0.0;
    for (int x = -2; x <= 2; ++x)
        for (int y = -2; y <= 2; ++y)
            result += texture(u_SSAOInput, v_TexCoord + vec2(x, y) * u_TexelSize).r;
    o_Color = result / 25.0;
}
