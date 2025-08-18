#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include "vertex.h"
#include "textureManager.hpp"
#include <glm/gtc/quaternion.hpp>
#include "noImage.hpp"

class TextureManager;
class Renderer;
class ParticleEmitter
{
public:
  std::vector<Particle> particles;
  int id;
  glm::vec3 pos;
  glm::quat rotation;
  bool hide = false;

  TextureManager textureManager;

  ParticleEmitter(Renderer &renderer, int *nextRenderingId, int particleCount, glm::vec3 pos, glm::quat rotation = glm::quatLookAt(glm::normalize(glm::vec3(0, 0, -1)), glm::vec3(0, 1, 0)), std::string texturePath = NO_IMAGE);
  void initGraphics(Renderer &renderer, std::string texturePath);
  void draw(Renderer *renderer, int currentFrame, glm::mat4 view, glm::mat4 projectionMatrix, VkCommandBuffer commandBuffer);
};