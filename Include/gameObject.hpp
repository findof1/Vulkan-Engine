#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include "vertex.h"
#include "textureManager.hpp"
#include "mesh.hpp"

class Renderer;
class TextureManager;

class GameObject
{
public:
  glm::vec3 pos;
  glm::vec3 rotationZYX; // degrees
  glm::vec3 scale;
  bool hide = false;

  std::vector<Mesh> meshes;

  GameObject(Renderer &renderer, const glm::vec3 &pos, const glm::vec3 &scale, const glm::vec3 &rotationZYX);
  ~GameObject() {}

  void draw(Renderer *renderer, int currentFrame, glm::mat4 view, glm::mat4 projectionMatrix, VkCommandBuffer commandBuffer);
  void loadModel(Renderer &renderer, int *nextRenderingId, const std::string objPath, const std::string mtlPath);
  void addMesh(Renderer &renderer, int *nextRenderingId, MaterialData &material, std::string texturePath, const std::vector<Vertex> &vertices, const std::vector<uint32_t> &indices);

  void setScale(const glm::vec3 &newScale);
  void setPosition(const glm::vec3 &newPosition);

  void cleanupMeshes(VkDevice device, Renderer &renderer);

private:
  std::vector<Vertex> vertices;
  std::vector<uint32_t> indices;

  bool initializedPhysics = false;
};