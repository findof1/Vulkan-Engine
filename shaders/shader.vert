#version 450

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

layout(push_constant) uniform MaterialData {
    vec3 ambientColor;
    vec3 diffuseColor;
    vec3 specularColor;
    vec3 emissionColor;
    float shininess;
    float opacity;
    float refractiveIndex;

    int illuminationModel;
    int isParticle;

    int hasDiffuseMap;
    int hasNormalMap;
    int hasHeightMap;
    int hasSpecularMap;
    int hasShininessMap;
} material;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in vec3 inNormal;
layout(location = 4) in vec3 inPositionB;
layout(location = 5) in vec4 inColorB;

layout(location = 0) out vec2 fragTexCoord;
layout(location = 1) out vec4 diffuseColor;
layout(location = 2) out vec3 fragNormal;
layout(location = 3) out vec3 viewPos;
layout(location = 4) out vec3 fragPos;

void main() {

    if(material.isParticle == 1) {
        gl_Position = ubo.proj * ubo.view * ubo.model * vec4(inPositionB, 1.0);
        gl_PointSize = 7;

        diffuseColor = vec4(material.diffuseColor, 1);

        if((inColorB.x > 0 || inColorB.y > 0 || inColorB.z > 0) && diffuseColor.x == 0 && diffuseColor.y == 0 && diffuseColor.z == 0) {
            diffuseColor = inColorB;
        }
        fragTexCoord = vec2(0.0);
    } else {
        gl_Position = ubo.proj * ubo.view * ubo.model * vec4(inPosition, 1.0);
        diffuseColor = vec4(material.diffuseColor, 1);
        if((inColor.x > 0 || inColor.y > 0 || inColor.z > 0) && diffuseColor.x == 0 && diffuseColor.y == 0 && diffuseColor.z == 0) {
            diffuseColor = vec4(inColor, 1);
        }
        fragTexCoord = inTexCoord;
        if(inNormal.x < 0 && inNormal.y < 0 && inNormal.z < 0) {
            fragNormal = vec3(-1);
        } else {
            fragNormal = mat3(transpose(inverse(ubo.model))) * inNormal;
        }
        viewPos = inverse(ubo.view)[3].xyz;

        vec4 worldPos = ubo.model * vec4(inPosition, 1.0);
        fragPos = worldPos.xyz;
    }
}