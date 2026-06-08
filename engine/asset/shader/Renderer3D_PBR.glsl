// 基于物理的渲染 Physically Based Rendering
//
// PBR 核心思想
//   用物理光学原理模拟光与材质表面的交互 让材质在任何光照环境下都真实一致
//   实现的是Metallic-Roughness工作流 金属度-粗糙度工作流
//   相比于Specular-Glossiness工作流更物理 参数更少
//
// 渲染方程架构
//   Lo=直接光照(Cook-Torrance BRDF + Lambert漫反射) * 阴影
//      + 间接光照(IBL=漫反射辐照度 + 镜面反射预过滤)
//   最终 = Lo → HDR Tone Mapping → Gamma校正
//
// 参考
//   - Epic Games, "Real Shading in Unreal Engine 4" (2013)
//   - Karis, "Real Shading in Unreal Engine 4" SIGGRAPH 2013
//   - Filament PBR Documentation
//   - LearnOpenGL PBR chapters

#type vertex
#version 450 core

// ---- 顶点输入 (per-vertex attributes) ----
layout(location = 0) in vec3 a_Position;   // 模型空间顶点位置
layout(location = 1) in vec3 a_Normal;     // 模型空间法线
layout(location = 2) in vec2 a_TexCoord;   // 纹理坐标 UV

// ---- UBO: Camera (binding=0) ----
// 相机矩阵 每帧更新一次
layout(std140, binding = 0) uniform Camera {
    mat4 u_ViewProjection; // 剪裁空间坐标=投影矩阵P*观察矩阵V*模型矩阵M*本地坐标 这个地方就是P*V 把世界坐标转换到剪裁空间
};

// ---- UBO: Model (binding=1) ----
// 模型矩阵 每个物体不同
layout(std140, binding = 1) uniform Model {
    mat4 u_Model; // 模型矩阵M 负责把本地坐标转换成世界坐标
};

// ---- 实体ID (用于鼠标拾取/选中) ----
uniform int u_EntityID;

// ---- 顶点着色器输出 (传给片段着色器) ----
layout(location = 0) out vec3 v_WorldPos; // 世界空间片段位置
layout(location = 1) out vec3 v_Normal; // 世界空间法线 (变换后)
layout(location = 2) out vec2 v_TexCoord; // 纹理坐标 (透传)
layout(location = 3) flat out int v_EntityID; // 实体ID (flat 不插值 整个三角形相同)

void main() {
    vec4 worldPos = u_Model * vec4(a_Position, 1.0);
    v_WorldPos = worldPos.xyz;
    v_Normal = mat3(transpose(inverse(u_Model))) * a_Normal;
    v_TexCoord = a_TexCoord;
    v_EntityID = u_EntityID;
    gl_Position = u_ViewProjection * worldPos;
}

// =============================================================================
// 片段着色器 — PBR 核心计算
// =============================================================================
#type fragment
#version 450 core

// ---- 顶点着色器传来的插值数据 ----
layout(location = 0) in vec3 v_WorldPos;
layout(location = 1) in vec3 v_Normal;
layout(location = 2) in vec2 v_TexCoord;
layout(location = 3) flat in int v_EntityID;

// ---- UBO: Camera (binding=0, 与vertex共享) ----
layout(std140, binding = 0) uniform Camera {
    mat4 u_ViewProjection;
};

// ---- UBO: LightBlock (binding=2) ----
// 多光源数组 每帧按Material::lightGroupId切换对应的光源组
#define MAX_LIGHTS 8

#define LIGHT_TYPE_DIRECTIONAL 0
#define LIGHT_TYPE_POINT 1
#define LIGHT_TYPE_SPOT 2

struct GPULight {
    vec4 ColorAndIntensity;    // xyz = 颜色, w = 强度
    vec4 PositionAndRange;     // xyz = 平行光方向(指向光源)/点光世界位置, w = 范围
    int Type;                  // 0=平行光, 1=点光
    float SpotInnerCone;       // 聚光灯内锥角cos (未来扩展)
    float SpotOuterCone;       // 聚光灯外锥角cos (未来扩展)
    float _pad;                // std140 对齐填充
};

layout(std140, binding = 2) uniform LightBlock {
    vec3 u_Ambient;             // 全局环境光 (与IBL相乘)
    int u_LightCount;           // 当前光源组活跃灯光数
    GPULight u_Lights[MAX_LIGHTS];
};

// ---- UBO: PBR Settings (binding=3) ----
// 相机位置与HDR曝光参数
layout(std140, binding = 3) uniform PBRSettings {
    vec3 u_CameraPosition; // 世界空间相机位置 (用于计算视线方向V)
    float u_Exposure; // HDR曝光度 (用于Tone Mapping)
};

// ---- UBO: CSM Data (binding=5) ----
// Cascaded Shadow Maps — 级联阴影贴图数据
// 将视锥体分成4个级联 每个级联有独立的阴影贴图 近处高精度远处低精度
layout(std140, binding = 5) uniform CSMData {
    mat4 u_LightViewProjections[4]; // 4个级联的光源ViewProjection矩阵
    vec4 u_CascadeSplits; // 级联分割距离 (3个分割点 w未使用)
};

// ---- 纹理采样器 ----// PBR材质贴图 (由u_UseXxxMap 开关控制)
uniform sampler2D u_AlbedoMap; // 反照率贴图 (基础颜色)
uniform sampler2D u_NormalMap; // 法线贴图 (扰动法线，增加表面细节)
uniform sampler2D u_MetallicMap; // 金属度贴图 (R通道)
uniform sampler2D u_RoughnessMap; // 粗糙度贴图 (R通道)
uniform sampler2D u_AOMap; // 环境遮蔽贴图 (Ambient Occlusion, R通道)

// IBL (Image Based Lighting) 环境贴图
uniform samplerCube u_IrradianceMap; // 漫反射辐照度图 (卷积后的环境Cubemap)
uniform samplerCube u_PrefilterMap; // 预过滤环境图 (不同粗糙度的镜面反射Cubemap mipmap链)
uniform sampler2D u_BRDFLUT; // BRDF积分查找表 (2D纹理 x=NdotV y=roughness)

// Shadow maps (CSM 4级级联)
uniform sampler2D u_ShadowMap0; // 级联0阴影贴图 (最近处)
uniform sampler2D u_ShadowMap1; // 级联1阴影贴图
uniform sampler2D u_ShadowMap2; // 级联2阴影贴图
uniform sampler2D u_ShadowMap3; // 级联3阴影贴图 (最远处)

// ---- 材质参数 (Uniform常量值) ----
uniform vec3 u_Albedo; // 反照率基色 (与贴图相乘)
uniform float u_Metallic; // 金属度 0=非金属 1=完全金属
uniform float u_Roughness; // 粗糙度 0=完美镜面 1=完全漫反射
uniform float u_AO; // 环境遮蔽强度
uniform vec3 u_Emissive; // 自发光颜色 (不受光照影响 直接叠加)
// 贴图使用开关 (1.0=使用贴图 0.0=仅用uniform值)
uniform float u_UseAlbedoMap;
uniform float u_UseNormalMap;
uniform float u_UseMetallicMap;
uniform float u_UseRoughnessMap;
uniform float u_UseAOMap;

// ---- 片段着色器输出 (MRT: Multiple Render Targets) ----
layout(location = 0) out vec4 o_Color; // 输出颜色到color attachment 0
layout(location = 1) out int o_EntityID; // 输出实体ID到color attachment 1 (用于鼠标拾取)

// =============================================================================
// PBR 常量与工具函数
// =============================================================================

const float PI = 3.14159265359;

// ---- Fresnel (菲涅尔效应) ----
// 描述: 观察角度越倾斜 表面反射率越高
// 现象: 垂直看水面能看透水底 斜着看水面像镜子反射天空
//
// F0: 垂直入射时的基础反射率
//   非金属(绝缘体) → F0 ≈ 0.04 (大多数非金属在法线方向的反射率都很低)
//   金属 → F0 = 材质颜色/反照率 (金属的反射率很高且带有颜色)
// 注意: 0.04 是大多数非金属电介质的平均F0值，实际各有不同但差异很小

// Schlick近似 — 标准Fresnel公式的快速多项式近似
// cosTheta: 法线N与视线V(或半向量H)之间的点积
// 公式: F = F0 + (1 - F0) * (1 - cosθ)^5
vec3 FresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

// Fresnel-Schlick with roughness (用于IBL镜面反射)
// 与普通Schlick的区别: 用roughness修正F0 使粗糙表面的边缘反射减弱
// 原理: 粗糙表面的微面元朝向分布更分散 掠射角的Fresnel效应被微观几何遮蔽削弱
vec3 FresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness) {
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

// ---- Normal Distribution Function (法线分布函数) ----
// 描述: 微面元中有多少的法线方向与半向量H对齐
// 物理含义: 决定了高光斑的大小和形状
// 粗糙度越小 → 微面元朝向越集中 → 高光越锐利
// 粗糙度越大 → 微面元朝向越分散 → 高光越大越模糊
//
// 采用 Trowbridge-Reitz GGX分布 (等价于Walter等人的GTR分布 γ=2)
// GGX相比Beckmann有更长的"尾巴" (高光边缘过渡更自然)
//
// 公式: D = α² / (π * ((N·H)² * (α² - 1) + 1)²)
// 其中 α = roughness² (Disney的粗糙度重映射 让粗糙度感知更线性)
float DistributionGGX(vec3 N, vec3 H, float roughness) {
    float a = roughness * roughness;         // α = roughness² (重映射)
    float a2 = a * a;                        // α²
    float NdotH = max(dot(N, H), 0.0);       // N·H 微面元法线与宏观半向量的对齐程度
    float denom = NdotH * NdotH * (a2 - 1.0) + 1.0;
    return a2 / (PI * denom * denom);        // 除以PI保证半球积分归一化
}

// ---- Geometry Function (几何函数/可见性函数) ----
// 描述: 有多少微面元不被其他微面元遮挡
// 物理含义: 粗糙表面中相邻微面元互相遮蔽(shadowing)和遮挡(masking)
// 粗糙度越大 → G越小 → 越暗 (光线被微面元阻挡)
//
// 采用 Schlick-GGX (Smith联合遮蔽函数)
// Gsub = n·v / ((n·v) * (1 - k) + k)
// 其中 k = (roughness + 1)² / 8  (在IBL中直接用roughness²/2)
//       或在直接光照中 k = (roughness + 1)² / 8  (重映射使粗造度响应更线性)
float GeometrySchlickGGX(float NdotV, float roughness) {
    float r = roughness + 1.0;
    float k = (r * r) / 8.0; // k用于直接光照
    return NdotV / (NdotV * (1.0 - k) + k);
}

// Smith法: G = G_sub(N,V) × G_sub(N,L)
// 同时考虑视线方向的遮蔽(masking)和光源方向的遮挡(shadowing)
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
    return GeometrySchlickGGX(max(dot(N, V), 0.0), roughness) *
           GeometrySchlickGGX(max(dot(N, L), 0.0), roughness);
}

// =============================================================================
// Cascaded Shadow Mapping (CSM) + PCF 柔化阴影
// =============================================================================
//
// CSM原理:
//   将视锥体沿Z轴分成4个级联
//   每个级联覆盖不同的深度范围，各自渲染一张阴影贴图
//   片段着色时根据当前片段的深度选择对应的级联
//   优点: 近处阴影精度高 远处覆盖范围广 避免单一shadow map分辨率不足
//
// PCF (Percentage Closer Filtering):
//   对阴影贴图做3×3采样取平均 使阴影边缘柔化
//   本质上是廉价的软阴影近似
float CascadedShadowCalculation(vec3 worldPos, float NdotL) {
    // 步骤1: 确定当前片段属于哪个级联
    // viewZ = 片段在相机空间中的深度 (ViewProjection矩阵变换后取-z)
    float viewZ = -(u_ViewProjection * vec4(worldPos, 1.0)).z;
    int cascade = 0;
    // u_CascadeSplits[0]~[2] 存储3个分割距离 依次扩大
    for (int i = 0; i < 3; ++i)
        if (viewZ > u_CascadeSplits[i]) cascade = i + 1;

    // 步骤2: 将世界坐标变换到光源的裁剪空间
    vec4 fragPosLS = u_LightViewProjections[cascade] * vec4(worldPos, 1.0);
    // 透视除法 → NDC空间[-1, 1]
    vec3 projCoords = fragPosLS.xyz / fragPosLS.w;
    // 变换到[0, 1]纹理坐标空间
    projCoords = projCoords * 0.5 + 0.5;

    // 步骤3: 超出远平面的片段不受阴影(shadow map覆盖范围外)
    if (projCoords.z > 1.0) return 0.0;

    float currentDepth = projCoords.z;

    // 步骤4: Shadow bias(阴影偏移，防止阴影痤疮/条纹)
    // bias需要随NdotL动态调整:
    //   掠射角(NdotL小) → 需要更大的bias (因为shadow map texel覆盖面倾斜)
    //   直视角度(NdotL大) → 需要更小的bias
    // 最小值0.00005防止完全背光面(NdotL≈0)下bias太小
    float bias = max(0.0005 * (1.0 - NdotL), 0.00005);

    // 步骤5: 3×3 PCF采样柔化阴影边缘
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(u_ShadowMap0, 0);   // 单个texel的UV步长
    for (int x = -1; x <= 1; ++x)
        for (int y = -1; y <= 1; ++y) {
            float pcfDepth;
            // 根据级联选择对应的shadow map
            if (cascade == 0) pcfDepth = texture(u_ShadowMap0, projCoords.xy + vec2(x, y) * texelSize).r;
            else if (cascade == 1) pcfDepth = texture(u_ShadowMap1, projCoords.xy + vec2(x, y) * texelSize).r;
            else if (cascade == 2) pcfDepth = texture(u_ShadowMap2, projCoords.xy + vec2(x, y) * texelSize).r;
            else pcfDepth = texture(u_ShadowMap3, projCoords.xy + vec2(x, y) * texelSize).r;
            // 深度比较: 当前片段的深度 > shadow map中记录的深度 → 被遮挡
            shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
        }

    // 返回[0, 1]阴影值: 0=完全照亮 1=完全阴影
    return shadow / 9.0;
}

// =============================================================================
// main() — PBR主计算
// =============================================================================

void main() {
    // ---- 方向向量 ----
    vec3 N = normalize(v_Normal); // 表面法线 (世界空间)
    vec3 V = normalize(u_CameraPosition - v_WorldPos); // 视线方向 (片段→相机)

    // ---- 采样材质贴图 ----
    // 每个贴图值与uniform常量值相乘 实现"贴图×调参"的灵活性
    vec3 albedo = texture(u_AlbedoMap, v_TexCoord).rgb * u_Albedo;
    float metallic = texture(u_MetallicMap, v_TexCoord).r * u_Metallic;
    float roughness = texture(u_RoughnessMap, v_TexCoord).r * u_Roughness;
    float ao = texture(u_AOMap, v_TexCoord).r * u_AO;

    // ---- 基础反射率 F0 ----
    // 非金属(绝缘体) F0 ≈ 0.04 (sRGB的4%反射率)
    // 金属 F0 = albedo (金属反射率包含颜色)
    // mix(a, b, t): 非金属部分为0.04 金属部分为albedo metallic决定混合权重
    vec3 F0 = mix(vec3(0.04), albedo, metallic);

    // =======================================================================
    // 第1部分: 直接光照 (Direct Lighting)
    // 遍历当前光源组的所有灯光
    // =======================================================================

    vec3 Lo = vec3(0.0);
    bool shadowCalculated = false;
    float shadow = 0.0;

    for (int i = 0; i < u_LightCount && i < MAX_LIGHTS; i++) {
        vec3 lightColor = u_Lights[i].ColorAndIntensity.rgb;
        float intensity = u_Lights[i].ColorAndIntensity.w;
        vec3 lightPosOrDir = u_Lights[i].PositionAndRange.xyz;
        float range = u_Lights[i].PositionAndRange.w;
        bool isPoint = u_Lights[i].Type == LIGHT_TYPE_POINT;

        vec3 L, radiance;
        float NdotL;

        if (isPoint) {
            L = normalize(lightPosOrDir - v_WorldPos);
            NdotL = max(dot(N, L), 0.0);
            float dist = length(lightPosOrDir - v_WorldPos);
            float attenuation = 1.0 / (1.0 + dist * dist / max(range * range, 0.0001));
            radiance = lightColor * intensity * attenuation;
        } else {
            L = normalize(-lightPosOrDir);
            NdotL = max(dot(N, L), 0.0);
            radiance = lightColor * intensity;
            if (!shadowCalculated) {
                shadow = CascadedShadowCalculation(v_WorldPos, NdotL);
                shadowCalculated = true;
            }
        }

        vec3 H = normalize(V + L);
        float NDF = DistributionGGX(N, H, roughness);
        float G = GeometrySmith(N, V, L, roughness);
        vec3 F = FresnelSchlick(max(dot(H, V), 0.0), F0);
        vec3 kS = F;
        vec3 kD = (1.0 - kS) * (1.0 - metallic);
        vec3 numerator = NDF * G * F;
        float denominator = 4.0 * max(dot(N, V), 0.0) * NdotL + 0.0001;
        vec3 specular = numerator / denominator;

        vec3 Li = (kD * albedo / PI + specular) * radiance * NdotL;
        if (!isPoint && shadowCalculated) {
            Li *= (1.0 - shadow);
        }
        Lo += Li;
    }

    // =======================================================================
    // 第2部分: 间接光照 — IBL (Image Based Lighting)
    // 用环境Cubemap近似场景中所有间接光 (来自天空 周围物体的反射光)
    //
    // 渲染方程的间接光照部分:
    //   Lo_indirect = ∫_Ω (kD·albedo/π + kS·f_s)·Li·(n·ωi) dωi
    //
    // 这个积分很昂贵，所以用 Split-Sum Approximation (裂和近似) 加速:
    //   ∫_Ω f·Li·cosθ dω ≈ (1/N) Σ Li  ×  ∫_Ω f·cosθ dω
    //                    = 预过滤环境贴图 × BRDF积分LUT
    // =======================================================================

    // IBL下的Fresnel (考虑粗糙度修正 见FresnelSchlickRoughness注释)
    vec3 kS_ibl = FresnelSchlickRoughness(max(dot(N, V), 0.0), F0, roughness);
    vec3 kD_ibl = (1.0 - kS_ibl) * (1.0 - metallic);

    // --- IBL 漫反射 (Diffuse IBL) ---
    // 从辐照度图采样 辐照度图是环境Cubemap在半球上的卷积结果
    // irradiance = (1/π)·∫_Ω Li·(n·ωi) dωi  (已预计算并存入Cubemap)
    vec3 irradiance = texture(u_IrradianceMap, N).rgb;
    vec3 diffuse = irradiance * albedo; // Lambert漫反射 × 辐照度

    // --- IBL 镜面反射 (Specular IBL) ---
    const float MAX_REFLECTION_LOD = 4.0; // PrefilterMap的mip级别数-1

    // 反射方向: 视线V关于法线N的镜面反射
    vec3 R = reflect(-V, N);

    // 预过滤环境图: 不同粗糙度对应不同mip级别
    // 粗糙表面 = 模糊的反射 → 采样低精度mip级别 (高LOD值)
    // 光滑表面 = 清晰的反射 → 采样高精度mip级别 (低LOD值)
    vec3 prefilteredColor = textureLod(u_PrefilterMap, R, roughness * MAX_REFLECTION_LOD).rgb;

    // BRDF积分LUT (Look-Up Table):
    //   横轴 = NdotV 纵轴 = roughness
    //   R通道 = F0的缩放系数 (scale)
    //   G通道 = F0的偏移系数 (bias)
    // 预先计算了: ∫_Ω f_s·(1-(1-ωo·h)^5)·cosθ dω ... 的积分结果
    vec2 envBRDF = texture(u_BRDFLUT, vec2(max(dot(N, V), 0.0), roughness)).rg;

    // 镜面IBL = 预过滤颜色 × (F0 × scale + bias)
    // 其中 scale=envBRDF.x bias=envBRDF.y 来自LUT
    vec3 specularIBL = prefilteredColor * (kS_ibl * envBRDF.x + envBRDF.y);

    // 间接光照 = (漫反射IBL + 镜面反射IBL) × AO
    // AO (环境遮蔽) 近似模拟几何角落中光线的衰减
    vec3 ambient = (kD_ibl * diffuse + specularIBL) * ao;

    // ---- 合并直接光 + 间接光 ----
    vec3 color = ambient + Lo;

    // ---- 自发光 (Emissive) ----
    color += albedo * u_Emissive;

    // =======================================================================
    // 第3部分: HDR Tone Mapping (色调映射)
    // 将HDR的高动态范围颜色映射到LDR [0, 1]
    //
    // 使用 Reinhard 的指数形式变体:
    //   color = 1 - e^(-color × exposure)
    //
    // 效果: 高亮区域自然饱和而非硬裁切(clamp)
    // exposure参数控制整体亮度
    // =======================================================================
    color = vec3(1.0) - exp(-color * u_Exposure);

    // =======================================================================
    // 第4部分: Gamma Correction (伽马校正)
    //
    // 原因: 显示器有非线性的伽马响应 (典型的gamma=2.2)
    //   显示器输出亮度 = input^(2.2)
    //   所以图像需要先做 input^(1/2.2) 来预校正
    //
    // 不在shader做的话画面会偏暗 (因为显示器gamma压暗了颜色)
    // =======================================================================
    color = pow(color, vec3(1.0 / 2.2));

    // ---- 输出 ----
    o_Color = vec4(color, 1.0);
    o_EntityID = v_EntityID;
}
