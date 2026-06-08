#type vertex
#version 410 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec2 a_TexCoord;
layout(location = 3) in ivec4 a_BoneIndices;
layout(location = 4) in vec4 a_BoneWeights;

layout(std140) uniform Camera { mat4 u_ViewProjection; };
layout(std140) uniform Model { mat4 u_Model; };

const int MAX_BONES = 64;
uniform mat4 u_BoneMatrices[MAX_BONES];

out vec3 v_WorldPos;
out vec3 v_Normal;
out vec2 v_TexCoord;
flat out int v_EntityID;

uniform int u_EntityID;

void main() {
    mat4 boneTransform = u_BoneMatrices[a_BoneIndices[0]] * a_BoneWeights[0] +
                         u_BoneMatrices[a_BoneIndices[1]] * a_BoneWeights[1] +
                         u_BoneMatrices[a_BoneIndices[2]] * a_BoneWeights[2] +
                         u_BoneMatrices[a_BoneIndices[3]] * a_BoneWeights[3];
    vec4 localPos = boneTransform * vec4(a_Position, 1.0);
    vec4 worldPos = u_Model * localPos;
    v_WorldPos = worldPos.xyz;
    v_Normal = mat3(transpose(inverse(u_Model * boneTransform))) * a_Normal;
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

#define MAX_LIGHTS 8
#define LIGHT_TYPE_DIRECTIONAL 0
#define LIGHT_TYPE_POINT 1
#define LIGHT_TYPE_SPOT 2

struct GPULight {
    vec4 ColorAndIntensity;
    vec4 PositionAndRange;
    int Type;
    float SpotInnerCone;
    float SpotOuterCone;
    float _pad;
};

layout(std140) uniform LightBlock {
    vec3 u_Ambient;
    int u_LightCount;
    GPULight u_Lights[MAX_LIGHTS];
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
uniform vec3 u_Albedo; uniform float u_Metallic, u_Roughness, u_AO;

const float PI = 3.14159265359;

vec3 FresnelSchlick(float c, vec3 F0) { return F0+(1.0-F0)*pow(clamp(1.0-c,0.0,1.0),5.0); }
vec3 FresnelSchlickRoughness(float c, vec3 F0, float r) { return F0+(max(vec3(1.0-r),F0)-F0)*pow(clamp(1.0-c,0.0,1.0),5.0); }
float D_GGX(vec3 N, vec3 H, float a) { float a2=a*a; float NH=max(dot(N,H),0.0); float d=NH*NH*(a2-1.0)+1.0; return a2/(PI*d*d); }
float G_Schlick(float NdotV,float r){ float k=(r+1.0)*(r+1.0)/8.0; return NdotV/(NdotV*(1.0-k)+k); }
float G_Smith(vec3 N,vec3 V,vec3 L,float r){ return G_Schlick(max(dot(N,V),0.0),r)*G_Schlick(max(dot(N,L),0.0),r); }

float CSMShadow(vec3 wp, float NdotL){
    float vz=-(u_ViewProjection*vec4(wp,1.0)).z; int c=0;
    if(vz>u_CascadeSplits[0])c=1; if(vz>u_CascadeSplits[1])c=2; if(vz>u_CascadeSplits[2])c=3;
    vec4 fls=u_LightViewProjections[c]*vec4(wp,1.0); vec3 p=fls.xyz/fls.w; p=p*0.5+0.5; if(p.z>1.0)return 0.0;
    float b=max(0.0005*(1.0-NdotL),0.00005); float s=0.0;
    vec2 ts=1.0/textureSize(u_ShadowMap0,0);
    for(int x=-1;x<=1;++x)for(int y=-1;y<=1;++y){
        float d;
        if(c==0)d=texture(u_ShadowMap0,p.xy+vec2(x,y)*ts).r;
        else if(c==1)d=texture(u_ShadowMap1,p.xy+vec2(x,y)*ts).r;
        else if(c==2)d=texture(u_ShadowMap2,p.xy+vec2(x,y)*ts).r;
        else d=texture(u_ShadowMap3,p.xy+vec2(x,y)*ts).r;
        s+=p.z-b>d?1.0:0.0;
    }
    return s/9.0;
}

void main(){
    vec3 N=normalize(v_Normal); vec3 V=normalize(u_CameraPosition-v_WorldPos);
    vec3 alb=texture(u_AlbedoMap,v_TexCoord).rgb*u_Albedo;
    float met=texture(u_MetallicMap,v_TexCoord).r*u_Metallic;
    float rou=texture(u_RoughnessMap,v_TexCoord).r*u_Roughness;
    float aoc=texture(u_AOMap,v_TexCoord).r*u_AO;
    vec3 F0=mix(vec3(0.04),alb,met);

    vec3 Lo = vec3(0.0);
    bool shadowCalc = false;
    float sh = 0.0;

    for (int i = 0; i < u_LightCount && i < MAX_LIGHTS; i++) {
        vec3 lightColor = u_Lights[i].ColorAndIntensity.rgb;
        float intensity = u_Lights[i].ColorAndIntensity.w;
        vec3 lightPosOrDir = u_Lights[i].PositionAndRange.xyz;
        float range = u_Lights[i].PositionAndRange.w;
        bool isPt = u_Lights[i].Type == LIGHT_TYPE_POINT;

        vec3 L, rad; float NdotL;

        if (isPt) {
            L = normalize(lightPosOrDir - v_WorldPos);
            NdotL = max(dot(N, L), 0.0);
            float dist = length(lightPosOrDir - v_WorldPos);
            float att = 1.0 / (1.0 + dist * dist / max(range * range, 0.0001));
            rad = lightColor * intensity * att;
        } else {
            L = normalize(-lightPosOrDir);
            NdotL = max(dot(N, L), 0.0);
            rad = lightColor * intensity;
            if (!shadowCalc) { sh = CSMShadow(v_WorldPos, NdotL); shadowCalc = true; }
        }

        vec3 H = normalize(V + L);
        float NDF = D_GGX(N, H, rou);
        float G = G_Smith(N, V, L, rou);
        vec3 F = FresnelSchlick(max(dot(H, V), 0.0), F0);
        vec3 kS = F; vec3 kD = (1.0 - kS) * (1.0 - met);
        vec3 spec = (NDF * G * F) / max(4.0 * max(dot(N, V), 0.0) * NdotL, 0.0001);

        vec3 Li = (kD * alb / PI + spec) * rad * NdotL;
        if (!isPt && shadowCalc) Li *= (1.0 - sh);
        Lo += Li;
    }

    vec3 kSr=FresnelSchlickRoughness(max(dot(N,V),0.0),F0,rou); vec3 kDr=(1.0-kSr)*(1.0-met);
    vec3 diff=texture(u_IrradianceMap,N).rgb*alb;
    vec3 pre=textureLod(u_PrefilterMap,reflect(-V,N),rou*4.0).rgb;
    vec2 brdf=texture(u_BRDFLUT,vec2(max(dot(N,V),0.0),rou)).rg;
    vec3 amb=(kDr*diff+pre*(kSr*brdf.x+brdf.y))*aoc;
    vec3 col=amb+Lo; col=vec3(1.0)-exp(-col*u_Exposure); col=pow(col,vec3(1.0/2.2));
    o_Color=vec4(col,1.0); o_EntityID=v_EntityID;
}
