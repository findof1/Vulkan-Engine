#version 450

layout(location = 0) in vec2 fragTexCoord;
layout(location = 1) in vec4 vertexColor;
layout(location = 2) in vec3 fragNormal;
layout(location = 3) in vec3 viewPos;
layout(location = 4) in vec3 fragPos;

layout(push_constant) uniform MaterialData {
    vec3 albedoColor;
    float metallic;

    float roughness;
    float ao;
    float opacity;
    float emissiveStrength;

    int hasAlbedoMap;
    int hasNormalMap;
    int hasHeightMap;
    int hasRoughnessMap;
    int hasMetallicMap;
    int hasAOMap;
    int hasEmissiveMap;

    int isParticle;
    int isSkybox;
} material;

layout(location = 0) out vec4 outColor;

struct Light
{
  vec3 position;
  vec3 color;
  float intensity;
};

const int MAX_LIGHTS = 100;

layout(binding = 1) uniform LightsUBO {
  vec3 cameraPos;
  int lightsCount;
  Light lights[MAX_LIGHTS];
} lightsUBO;

layout(binding = 2) uniform sampler2D albedoMap;
layout(binding = 3) uniform sampler2D normalMap;
layout(binding = 4) uniform sampler2D heightMap;
layout(binding = 5) uniform sampler2D roughnessMap;
layout(binding = 6) uniform sampler2D metallicMap;
layout(binding = 7) uniform sampler2D aoMap;
layout(binding = 8) uniform sampler2D emissiveMap;

const float PI = 3.14159265359;

float DistributionGGX(vec3 N, vec3 H, float roughness) {
    float a = roughness*roughness;
    float a2 = a*a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float num   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return num / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness) {
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float num   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return num / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

vec3 FresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

void main() {
    vec3 albedo = material.albedoColor;
    float metallic = material.metallic;
    float roughness = material.roughness;
    float ao = material.ao;
    vec3 emissive = vec3(0.0);

    if (material.isParticle == 1) {
        vec2 texCoord = gl_PointCoord;
        if (material.hasAlbedoMap == 1) {
            vec4 texColor = texture(albedoMap, texCoord);
            outColor = texColor * vertexColor;
        } else {
            outColor = vertexColor;
        }
        return;
    }

    if (material.isSkybox == 1) {
        if (material.hasAlbedoMap == 1){
            vec3 texColor = texture(albedoMap, fragTexCoord).rgb;
            outColor = vec4(texColor, 1.0); 
            return;
        }
        outColor = vec4(albedo, 1.0);
        return;
    }

    if (material.hasAlbedoMap == 1)
        albedo = texture(albedoMap, fragTexCoord).rgb;

    if (material.hasRoughnessMap == 1)
        roughness = texture(roughnessMap, fragTexCoord).r;

    if (material.hasMetallicMap == 1)
        metallic = texture(metallicMap, fragTexCoord).r;

    if (material.hasAOMap == 1)
        ao = texture(aoMap, fragTexCoord).r;

    if (material.hasEmissiveMap == 1)
        emissive = texture(emissiveMap, fragTexCoord).rgb * material.emissiveStrength;

    vec3 norm = normalize(fragNormal);
    if (material.hasNormalMap == 1) {
        vec3 tangent = normalize(abs(norm.x) < 0.99 ? vec3(1,0,0) : vec3(0,1,0));

        tangent = normalize(tangent - dot(tangent, norm) * norm);

        vec3 bitangent = cross(norm, tangent);

        vec3 tangentNormal = texture(normalMap, fragTexCoord).xyz * 2.0 - 1.0;

        mat3 TBN = mat3(normalize(tangent), normalize(bitangent), normalize(fragNormal));

        norm = normalize(TBN *tangentNormal);
    }

    vec3 N = normalize(norm);
vec3 V = normalize(lightsUBO.cameraPos - fragPos);

vec3 F0 = vec3(0.04); 
F0 = mix(F0, albedo, metallic);

vec3 Lo = vec3(0.0);

for (int i = 0; i < lightsUBO.lightsCount; i++) {
    Light light = lightsUBO.lights[i];

    vec3 L = normalize(light.position - fragPos);
    vec3 H = normalize(V + L);
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (distance * distance); 
    vec3 radiance = light.color * light.intensity * attenuation;

    float NDF = DistributionGGX(N, H, roughness);
    float G   = GeometrySmith(N, V, L, roughness);
    vec3  F   = FresnelSchlick(max(dot(H, V), 0.0), F0);

    vec3 numerator    = NDF * G * F;
    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.001;
    vec3 specular = numerator / denominator;

    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - metallic;  

    float NdotL = max(dot(N, L), 0.0);

    Lo += (kD * albedo / PI + specular) * radiance * NdotL;
}

vec3 dirLightDir = normalize(vec3(-0.5, -1.0, -0.3)); 
    vec3 dirLightColor = vec3(1.0);
    float dirLightIntensity = 1.0;

    {
        vec3 L = normalize(-dirLightDir); 
        vec3 H = normalize(V + L);
        vec3 radiance = dirLightColor * dirLightIntensity;

        float NDF = DistributionGGX(N, H, roughness);
        float G   = GeometrySmith(N, V, L, roughness);
        vec3  F   = FresnelSchlick(max(dot(H, V), 0.0), F0);

        vec3 numerator    = NDF * G * F;
        float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.001;
        vec3 specular = numerator / denominator;

        vec3 kS = F;
        vec3 kD = vec3(1.0) - kS;
        kD *= 1.0 - metallic;  

        float NdotL = max(dot(N, L), 0.0);

        Lo += (kD * albedo / PI + specular) * radiance * NdotL;
    }

 vec3 ambient = vec3(0.03) * albedo * ao;

    vec3 color = ambient + Lo + emissive;

    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0/2.2));

    outColor = vec4(color, material.opacity);
}