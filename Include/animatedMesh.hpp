#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include "vertex.h"
#include "textureManager.hpp"
#include "mesh.hpp"

class TextureManager;
class Renderer;
class AnimatedMesh
{
public:
  std::vector<AnimatedVertex> vertices;
  std::vector<uint32_t> indices;
  int id;

  std::string texPath;

  MaterialData material;
  TextureManager textureManager;

  AnimatedMesh(Renderer &renderer, int *nextRenderingId, MaterialData material, const std::vector<AnimatedVertex> &vertices, const std::vector<uint32_t> &indices);
  void initGraphics(Renderer &renderer, std::string texturePath);
  void draw(Renderer *renderer, int currentFrame, glm::mat4 transformation, glm::mat4 view, glm::mat4 projectionMatrix, std::array<glm::mat4, 100> &finalBoneMatrices, VkCommandBuffer commandBuffer, int colorStageID);
  void cleanup(VkDevice device, Renderer &renderer);
};