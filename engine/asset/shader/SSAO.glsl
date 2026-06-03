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
layout(location = 0) out float o_SSAO;
uniform sampler2D u_DepthMap;
uniform sampler2D u_NormalMap;
layout(location = 3) uniform vec3 u_Samples[64];
layout(location = 0) uniform mat4 u_Projection;
layout(location = 1) uniform float u_Radius;
layout(location = 2) uniform float u_Bias;
const int KERNEL_SIZE = 64;

void main() {
    float depth = texture(u_DepthMap, v_TexCoord).r;
    vec3 normal = texture(u_NormalMap, v_TexCoord).rgb;
    vec3 randomVec = vec3(1.0);
    vec3 tangent = normalize(randomVec - normal * dot(randomVec, normal));
    vec3 bitangent = cross(normal, tangent);
    mat3 TBN = mat3(tangent, bitangent, normal);
    float occlusion = 0.0;
    for (int i = 0; i < KERNEL_SIZE; ++i) {
        vec3 samplePos = TBN * u_Samples[i];
        samplePos = normal * 0.5 + samplePos * u_Radius;
        vec4 offset = u_Projection * vec4(samplePos, 1.0);
        offset.xyz /= offset.w;
        offset.xy = offset.xy * 0.5 + 0.5;
        float sampleDepth = texture(u_DepthMap, offset.xy).r;
        float rangeCheck = smoothstep(0.0, 1.0, u_Radius / abs(depth - sampleDepth));
        occlusion += (sampleDepth >= samplePos.z + u_Bias ? 1.0 : 0.0) * rangeCheck;
    }
    o_SSAO = 1.0 - (occlusion / float(KERNEL_SIZE));
}
