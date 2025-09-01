#pragma once

#include "UI.hpp"
#include "renderer.hpp"
#include "text.hpp"
#include <functional>

#ifdef BUILD_ENGINE_DLL

#ifndef ENGINE_API
#define ENGINE_API __declspec(dllexport)
#endif

#else

#ifndef ENGINE_API
#define ENGINE_API __declspec(dllimport)
#endif

#endif

class ENGINE_API Square : public UI
{
public:
  int hasTexture = 0;

  Square(Renderer &renderer, int *nextRenderingId, glm::vec3 position, std::array<glm::vec2, 2> verticesOffsets, std::array<glm::vec2, 2> uvCoords, glm::vec3 color, std::string texture = "models/couch/diffuse.png");

  void initGraphics(Renderer &renderer) override;
  void initGraphics(Renderer &renderer, std::string texture);
  void draw(Renderer *renderer, int currentFrame, glm::mat4 transformation, glm::mat4 view, glm::mat4 projectionMatrix, VkCommandBuffer commandBuffer) override;
  void cleanup(VkDevice device, Renderer &renderer) override;

private:
  void initVertices();
  std::array<glm::vec2, 2> verticesOffsets;
  std::array<glm::vec2, 2> uvCoords;
  glm::vec3 color;
  std::vector<Vertex> vertices;
};