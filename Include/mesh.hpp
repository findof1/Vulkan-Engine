#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include "vertex.h"
#include "textureManager.hpp"
#include "noImage.hpp"
#include <memory>

#ifdef BUILD_ENGINE_DLL

#ifndef ENGINE_API
#define ENGINE_API __declspec(dllexport)
#endif

#else

#ifndef ENGINE_API
#define ENGINE_API __declspec(dllimport)
#endif

#endif

struct ENGINE_API MaterialData
{
  alignas(16) glm::vec3 albedoColor = glm::vec3(1.0f);
  float metallic = 0.0f;

  float roughness = 1.0f;
  float ao = 1.0f;
  float opacity = 1.0f;
  float emissiveStrength = 0.0f;

  int hasAlbedoMap = 0;
  int hasNormalMap = 0;
  int hasHeightMap = 0;
  int hasRoughnessMap = 0;
  int hasMetallicMap = 0;
  int hasAOMap = 0;
  int hasEmissiveMap = 0;

  int isParticle = 0;
  int isSkybox = 0;
};

class TextureManager;
class Renderer;
class ENGINE_API Mesh
{
public:
  std::vector<Vertex> vertices;
  std::vector<uint32_t> indices;
  int id;

  std::string texPath;

  MaterialData material;
  bool ownsTextureManager = true;
  std::shared_ptr<TextureManager> textureManager;

  Mesh(Renderer &renderer, int *nextRenderingId, MaterialData material, const std::vector<Vertex> &vertices, const std::vector<uint32_t> &indices);
  Mesh(Renderer &renderer, std::shared_ptr<TextureManager> texture, int *nextRenderingId, MaterialData newMaterial, const std::vector<Vertex> &vertices, const std::vector<uint32_t> &indices);
  void initGraphics(Renderer &renderer, std::string texturePath, std::string normalPath = NO_IMAGE, std::string heightPath = NO_IMAGE, std::string roughnessPath = NO_IMAGE, std::string metallicPath = NO_IMAGE, std::string aoPath = NO_IMAGE, std::string emissivePath = NO_IMAGE);
  void initGraphics(Renderer &renderer);
  void draw(Renderer *renderer, int currentFrame, glm::mat4 transformation, glm::mat4 view, glm::mat4 projectionMatrix, VkCommandBuffer commandBuffer, int colorStageID);
  void cleanup(VkDevice device, Renderer &renderer);
};