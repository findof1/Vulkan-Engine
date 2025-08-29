#pragma once

#include <string>
#include <vector>
#include <map>
#include "vertex.h"
#include "textureManager.hpp"
#include "renderer.hpp"

#ifdef BUILD_ENGINE_DLL

#ifndef ENGINE_API
#define ENGINE_API __declspec(dllexport)
#endif

#else

#ifndef ENGINE_API
#define ENGINE_API __declspec(dllimport)
#endif

#endif

class ENGINE_API UI
{
public:
  glm::vec3 position;
  std::vector<Vertex> vertices;
  int id;

  TextureManager textureManager;
  bool hide = false;

  UI(Renderer &renderer, int *nextRenderingId, glm::vec3 position) : position(position), id((*nextRenderingId)++), textureManager(renderer.bufferManager, renderer) {};

  virtual void initGraphics(Renderer &renderer) = 0;

  virtual void draw(Renderer *renderer, int currentFrame, glm::mat4 transformation, glm::mat4 view, glm::mat4 projectionMatrix, VkCommandBuffer commandBuffer) = 0;

  virtual void cleanup(VkDevice device, Renderer &renderer) = 0;
};