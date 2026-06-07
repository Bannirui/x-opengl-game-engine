#type vertex
#version 410 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec2 a_TexCoord;

layout(std140) uniform Camera { mat4 u_ViewProjection; };
layout(std140) uniform Model { mat4 u_Model; };

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
#version 410 core

in vec3 v_WorldPos;
in vec3 v_Normal;
in vec2 v_TexCoord;
flat in int v_EntityID;

layout(location = 0) out vec4 o_Color;
layout(location = 1) out int o_EntityID;

layout(std140) uniform Camera {
    mat4 u_ViewProjection;
};
layout(std140) uniform Light {
    vec3 u_LightDirection; vec3 u_LightAmbient; vec3 u_LightDiffuse; vec3 u_LightSpecular;
    vec3 u_PointLightPosition; float u_PointLightRange; vec3 u_PointLightColor; float u_PointLightIntensity;
};
layout(std140) uniform PBRSettings {
    vec3 u_CameraPosition; float u_Exposure;
};
layout(std140) uniform CSMData {
    mat4 u_LightViewProjections[4]; vec4 u_CascadeSplits;
};

uniform sampler2D u_AlbedoMap, u_MetallicMap, u_RoughnessMap, u_AOMap;
uniform samplerCube u_IrradianceMap, u_PrefilterMap;
uniform sampler2D u_BRDFLUT;
uniform sampler2D u_ShadowMap0, u_ShadowMap1, u_ShadowMap2, u_ShadowMap3;
uniform vec3 u_Albedo; uniform float u_Metallic, u_Roughness, u_AO; uniform vec3 u_Emissive;

const float PI = 3.14159265359;

vec3 FresnelSchlick(float c, vec3 F0) { return F0+(1.0-F0)*pow(clamp(1.0-c,0.0,1.0),5.0); }
vec3 FresnelSchlickRoughness(float c, vec3 F0, float r) { return F0+(max(vec3(1.0-r),F0)-F0)*pow(clamp(1.0-c,0.0,1.0),5.0); }

float DistributionGGX(vec3 N, vec3 H, float roughness) {
    float a = roughness*roughness; float a2=a*a; float NH=max(dot(N,H),0.0); float d=NH*NH*(a2-1.0)+1.0; return a2/(PI*d*d);
}
float GeometrySchlickGGX(float NdotV, float roughness) {
    float r=roughness+1.0; float k=(r*r)/8.0; return NdotV/(NdotV*(1.0-k)+k);
}
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
    return GeometrySchlickGGX(max(dot(N,V),0.0),roughness)*GeometrySchlickGGX(max(dot(N,L),0.0),roughness);
}

float CSMShadow(vec3 wp, float NdotL) {
    float vz = -(u_ViewProjection*vec4(wp,1.0)).z;
    int c = 0;
    if (vz > u_CascadeSplits[0]) c = 1;
    if (vz > u_CascadeSplits[1]) c = 2;
    if (vz > u_CascadeSplits[2]) c = 3;
    vec4 fls = u_LightViewProjections[c] * vec4(wp, 1.0);
    vec3 p = fls.xyz/fls.w; p = p*0.5+0.5;
    if (p.z > 1.0) return 0.0;
    float bias = max(0.0005*(1.0-NdotL), 0.00005);
    float shadow = 0.0;
    vec2 ts = 1.0/textureSize(u_ShadowMap0,0);
    for (int x=-1;x<=1;++x) for (int y=-1;y<=1;++y) {
        float d;
        if (c==0) d=texture(u_ShadowMap0,p.xy+vec2(x,y)*ts).r;
        else if (c==1) d=texture(u_ShadowMap1,p.xy+vec2(x,y)*ts).r;
        else if (c==2) d=texture(u_ShadowMap2,p.xy+vec2(x,y)*ts).r;
        else d=texture(u_ShadowMap3,p.xy+vec2(x,y)*ts).r;
        shadow += p.z-bias>d ? 1.0 : 0.0;
    }
    return shadow/9.0;
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
    vec3 Ld = normalize(-u_LightDirection);
    vec3 Hd = normalize(V+Ld);
    float NdotLd = max(dot(N,Ld),0.0);
    float Nd = DistributionGGX(N,Hd,roughness);
    float Gd = GeometrySmith(N,V,Ld,roughness);
    vec3 Fd = FresnelSchlick(max(dot(Hd,V),0.0),F0);
    vec3 kSd = Fd; vec3 kDd = (1.0-kSd)*(1.0-metallic);
    vec3 sd = (Nd*Gd*Fd)/max(4.0*max(dot(N,V),0.0)*NdotLd,0.0001);
    float shadow = CSMShadow(v_WorldPos, NdotLd);
    vec3 Lo = (kDd*albedo/PI+sd)*u_LightDiffuse*NdotLd*(1.0-shadow);

    // Point light
    vec3 Lp = normalize(u_PointLightPosition - v_WorldPos);
    vec3 Hp = normalize(V+Lp);
    float NdotLp = max(dot(N,Lp),0.0);
    float d = length(u_PointLightPosition - v_WorldPos);
    float att = 1.0/(1.0+d*d/max(u_PointLightRange*u_PointLightRange,0.0001));
    vec3 rp = u_PointLightColor*u_PointLightIntensity*att;
    float Np = DistributionGGX(N,Hp,roughness);
    float Gp = GeometrySmith(N,V,Lp,roughness);
    vec3 Fp = FresnelSchlick(max(dot(Hp,V),0.0),F0);
    vec3 kSp=Fp; vec3 kDp=(1.0-kSp)*(1.0-metallic);
    vec3 sp = (Np*Gp*Fp)/max(4.0*max(dot(N,V),0.0)*NdotLp,0.0001);
    Lo += (kDp*albedo/PI+sp)*rp*NdotLp;

    vec3 kSr = FresnelSchlickRoughness(max(dot(N,V),0.0), F0, roughness);
    vec3 kDr = (1.0-kSr)*(1.0-metallic);
    vec3 irradiance = texture(u_IrradianceMap, N).rgb;
    vec3 pre = textureLod(u_PrefilterMap, reflect(-V,N), roughness*4.0).rgb;
    vec2 brdf = texture(u_BRDFLUT, vec2(max(dot(N,V),0.0), roughness)).rg;
    vec3 amb = (kDr*irradiance*albedo + pre*(kSr*brdf.x+brdf.y)) * ao;
    vec3 color = amb + Lo + albedo*u_Emissive;
    color = vec3(1.0)-exp(-color*u_Exposure);
    color = pow(color, vec3(1.0/2.2));
    o_Color = vec4(color, 1.0);
    o_EntityID = v_EntityID;
}
