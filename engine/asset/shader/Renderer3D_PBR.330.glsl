#type vertex
#version 330 core
#extension GL_ARB_explicit_attrib_location : require
#extension GL_ARB_shading_language_420pack : require

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec2 a_TexCoord;

layout(std140, binding = 0) uniform Camera {
    mat4 u_ViewProjection;
};

layout(std140, binding = 1) uniform Model {
    mat4 u_Model;
};

out vec3 v_WorldPos;
out vec3 v_Normal;
out vec2 v_TexCoord;
flat out int v_EntityID;

uniform int u_EntityID;

void main() {
    vec4 worldPos = u_Model * vec4(a_Position, 1.0);
    v_WorldPos = worldPos.xyz;
    v_Normal = mat3(transpose(inverse(u_Model))) * a_Normal;
    v_TexCoord = a_TexCoord;
    v_EntityID = u_EntityID;
    gl_Position = u_ViewProjection * worldPos;
}

#type fragment
#version 330 core
#extension GL_ARB_explicit_attrib_location : require
#extension GL_ARB_shading_language_420pack : require

in vec3 v_WorldPos;
in vec3 v_Normal;
in vec2 v_TexCoord;
flat in int v_EntityID;

layout(location = 0) out vec4 o_Color;
layout(location = 1) out int o_EntityID;

layout(std140, binding = 2) uniform Light {
    vec3 u_LightDirection;
    vec3 u_LightAmbient;
    vec3 u_LightDiffuse;
    vec3 u_LightSpecular;
    vec3 u_PointLightPosition;
    float u_PointLightRange;
    vec3 u_PointLightColor;
    float u_PointLightIntensity;
};

layout(std140, binding = 3) uniform PBRSettings {
    vec3 u_CameraPosition;
    float u_Exposure;
};

uniform sampler2D u_AlbedoMap;
uniform sampler2D u_NormalMap;
uniform sampler2D u_MetallicMap;
uniform sampler2D u_RoughnessMap;
uniform sampler2D u_AOMap;

uniform samplerCube u_IrradianceMap;
uniform samplerCube u_PrefilterMap;
uniform sampler2D u_BRDFLUT;
uniform sampler2D u_ShadowMap0;
uniform sampler2D u_ShadowMap1;
uniform sampler2D u_ShadowMap2;
uniform sampler2D u_ShadowMap3;

layout(std140, binding = 5) uniform CSMData {
    mat4 u_LightViewProjections[4];
    vec4 u_CascadeSplits;
};

uniform vec3 u_Albedo;
uniform float u_Metallic;
uniform float u_Roughness;
uniform float u_AO;
uniform vec3 u_Emissive;
uniform float u_UseAlbedoMap;
uniform float u_UseNormalMap;
uniform float u_UseMetallicMap;
uniform float u_UseRoughnessMap;
uniform float u_UseAOMap;

const float PI = 3.14159265359;

vec3 FresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

vec3 FresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness) {
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

float DistributionGGX(vec3 N, vec3 H, float roughness) {
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float denom = NdotH * NdotH * (a2 - 1.0) + 1.0;
    return a2 / (PI * denom * denom);
}

float GeometrySchlickGGX(float NdotV, float roughness) {
    float r = roughness + 1.0;
    float k = (r * r) / 8.0;
    return NdotV / (NdotV * (1.0 - k) + k);
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
    return GeometrySchlickGGX(max(dot(N, V), 0.0), roughness) *
           GeometrySchlickGGX(max(dot(N, L), 0.0), roughness);
}

float CSMShadow(vec3 worldPos, float NdotL) {
    float viewZ = -(u_ViewProjection * vec4(worldPos, 1.0)).z;
    int cascade = 0;
    if (viewZ > u_CascadeSplits[0]) cascade = 1;
    else if (viewZ > u_CascadeSplits[1]) cascade = 2;
    else if (viewZ > u_CascadeSplits[2]) cascade = 3;

    vec4 fragPosLS = u_LightViewProjections[cascade] * vec4(worldPos, 1.0);
    vec3 projCoords = fragPosLS.xyz / fragPosLS.w;
    projCoords = projCoords * 0.5 + 0.5;
    if (projCoords.z > 1.0) return 0.0;
    float currentDepth = projCoords.z;
    float bias = max(0.0005 * (1.0 - NdotL), 0.00005);
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(u_ShadowMap0, 0);
    for (int x = -1; x <= 1; ++x)
        for (int y = -1; y <= 1; ++y) {
            float pcfDepth;
            if (cascade == 0) pcfDepth = texture(u_ShadowMap0, projCoords.xy + vec2(x, y) * texelSize).r;
            else if (cascade == 1) pcfDepth = texture(u_ShadowMap1, projCoords.xy + vec2(x, y) * texelSize).r;
            else if (cascade == 2) pcfDepth = texture(u_ShadowMap2, projCoords.xy + vec2(x, y) * texelSize).r;
            else pcfDepth = texture(u_ShadowMap3, projCoords.xy + vec2(x, y) * texelSize).r;
            shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
        }
    return shadow / 9.0;
}

void main() {
    vec3 N = normalize(v_Normal);
    vec3 V = normalize(u_CameraPosition - v_WorldPos);

    vec3 albedo = texture(u_AlbedoMap, v_TexCoord).rgb * u_Albedo;
    float metallic = texture(u_MetallicMap, v_TexCoord).r * u_Metallic;
    float roughness = texture(u_RoughnessMap, v_TexCoord).r * u_Roughness;
    float ao = texture(u_AOMap, v_TexCoord).r * u_AO;

    vec3 F0 = mix(vec3(0.04), albedo, metallic);

    // Directional light
    vec3 L_dir = normalize(-u_LightDirection);
    vec3 H_dir = normalize(V + L_dir);
    vec3 radiance_dir = u_LightDiffuse;

    float NDF_dir = DistributionGGX(N, H_dir, roughness);
    float G_dir = GeometrySmith(N, V, L_dir, roughness);
    vec3 F_dir = FresnelSchlick(max(dot(H_dir, V), 0.0), F0);

    vec3 kS_dir = F_dir;
    vec3 kD_dir = (1.0 - kS_dir) * (1.0 - metallic);
    vec3 numerator_dir = NDF_dir * G_dir * F_dir;
    float denominator_dir = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L_dir), 0.0) + 0.0001;
    vec3 specular_dir = numerator_dir / denominator_dir;

    float NdotL_dir = max(dot(N, L_dir), 0.0);
    float shadow = CSMShadow(v_WorldPos, NdotL_dir);
    vec3 Lo = (kD_dir * albedo / PI + specular_dir) * radiance_dir * NdotL_dir * (1.0 - shadow);

    // Point light
    vec3 L_pt = normalize(u_PointLightPosition - v_WorldPos);
    vec3 H_pt = normalize(V + L_pt);
    float NdotL_pt = max(dot(N, L_pt), 0.0);
    float dist_pt = length(u_PointLightPosition - v_WorldPos);
    float attenuation = 1.0 / (1.0 + dist_pt * dist_pt / max(u_PointLightRange * u_PointLightRange, 0.0001));
    vec3 radiance_pt = u_PointLightColor * u_PointLightIntensity * attenuation;

    float NDF_pt = DistributionGGX(N, H_pt, roughness);
    float G_pt = GeometrySmith(N, V, L_pt, roughness);
    vec3 F_pt = FresnelSchlick(max(dot(H_pt, V), 0.0), F0);

    vec3 kS_pt = F_pt;
    vec3 kD_pt = (1.0 - kS_pt) * (1.0 - metallic);
    vec3 numerator_pt = NDF_pt * G_pt * F_pt;
    float denominator_pt = 4.0 * max(dot(N, V), 0.0) * NdotL_pt + 0.0001;
    vec3 specular_pt = numerator_pt / denominator_pt;

    Lo += (kD_pt * albedo / PI + specular_pt) * radiance_pt * NdotL_pt;

    vec3 kS_ibl = FresnelSchlickRoughness(max(dot(N, V), 0.0), F0, roughness);
    vec3 kD_ibl = (1.0 - kS_ibl) * (1.0 - metallic);

    vec3 irradiance = texture(u_IrradianceMap, N).rgb;
    vec3 diffuse = irradiance * albedo;

    float MAX_REFLECTION_LOD = 4.0;
    vec3 R = reflect(-V, N);
    vec3 prefilteredColor = textureLod(u_PrefilterMap, R, roughness * MAX_REFLECTION_LOD).rgb;
    vec2 envBRDF = texture(u_BRDFLUT, vec2(max(dot(N, V), 0.0), roughness)).rg;
    vec3 specularIBL = prefilteredColor * (kS_ibl * envBRDF.x + envBRDF.y);

    vec3 ambient = (kD_ibl * diffuse + specularIBL) * ao;
    vec3 color = ambient + Lo + albedo * u_Emissive;

    color = vec3(1.0) - exp(-color * u_Exposure);
    color = pow(color, vec3(1.0 / 2.2));

    o_Color = vec4(color, 1.0);
    o_EntityID = v_EntityID;
}
