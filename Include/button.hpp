#pragma once

#include "UI.hpp"
#include "renderer.hpp"
#include "text.hpp"
#include <functional>

class Button : public UI
{
public:
  std::string label;
  glm::vec3 normalColor = glm::vec3(0.0f, 0.4f, 0.0);
  glm::vec3 hoverColor = glm::vec3(0.0f, 0.7f, 0.0);
  glm::vec3 pressedColor = glm::vec3(0.0, 0.8f, 0.0);

  bool isHovered = false;
  bool isPressed = false;
  bool clicked = false;
  int hasTexture = 0;

  Text *buttonText;
  glm::vec3 scale = glm::vec3(1);

  std::function<void(void)> callback = []() {};

  Button(Renderer &renderer, int *nextRenderingId, const std::string &label, glm::vec3 position, std::array<glm::vec2, 2> verticesOffsets, std::string texture = "models/couch/diffuse.png");
  void initGraphics(Renderer &renderer) override;
  void initGraphics(Renderer &renderer, std::string texture);
  void draw(Renderer *renderer, int currentFrame, glm::mat4 transformation, glm::mat4 view, glm::mat4 projectionMatrix, VkCommandBuffer commandBuffer) override;
  void updateState(float mouseX, float mouseY, bool mousePressed, ImVec2 sceneMin, ImVec2 sceneMax, float screenW, float screenH);
  void cleanup(VkDevice device, Renderer &renderer) override;

private:
  void initButtonVertices();
  std::array<glm::vec2, 2> verticesOffsets;
  glm::vec3 currentColor;
  std::vector<Vertex> vertices;
};