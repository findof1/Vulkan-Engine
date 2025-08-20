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
} material;

layout(location = 0) out vec4 outColor;

layout(binding = 1) uniform sampler2D albedoMap;
layout(binding = 2) uniform sampler2D normalMap;
layout(binding = 3) uniform sampler2D heightMap;
layout(binding = 4) uniform sampler2D roughnessMap;
layout(binding = 5) uniform sampler2D metallicMap;
layout(binding = 6) uniform sampler2D aoMap;
layout(binding = 7) uniform sampler2D emissiveMap;

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
        vec3 tangentNormal = texture(normalMap, fragTexCoord).xyz * 2.0 - 1.0;
        // TODO: apply TBN matrix to transform tangentNormal → world space
        norm = normalize(tangentNormal);
    }

    vec3 lightDir = normalize(vec3(-0.5, -1.0, -0.3));
    vec3 viewDir = normalize(viewPos - fragPos);
    vec3 lightColor = vec3(1.0);

    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(norm, halfwayDir), 0.0), mix(2.0, 256.0, 1.0 - roughness));
    vec3 specular = spec * mix(vec3(0.04), albedo, metallic);

    // Combine
    vec3 color = (albedo * diffuse + specular) * ao + emissive;

    outColor = vec4(color, material.opacity);
}