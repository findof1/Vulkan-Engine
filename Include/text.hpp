#pragma once

#include <string>
#include <vector>
#include <map>
#include "vertex.h"
#include "textureManager.hpp"
#include "UI.hpp"
#include "renderer.hpp"
#include <ft2build.h>
#include FT_FREETYPE_H

struct Glyph
{
  float advance;
  glm::ivec2 size;
  glm::ivec2 bearing;

  glm::vec2 uvOffset;
  glm::vec2 uvSize;

  FT_Bitmap bitmap;
};

class Text : public UI
{
public:
  std::string content;

  std::map<char, Glyph> glyphs;

  glm::vec3 scale = glm::vec3(1);

  bool hide = false;

  Text(Renderer &renderer, int *nextRenderingId, const std::string &text, glm::vec3 position);

  void initGraphics(Renderer &renderer) override;

  void updateText(const std::string &newText, Renderer &renderer);

  void draw(Renderer *renderer, int currentFrame, glm::mat4 transformation, glm::mat4 view, glm::mat4 projectionMatrix, VkCommandBuffer commandBuffer) override;

  void cleanup(VkDevice device, Renderer &renderer) override;

private:
  void generateVerticesFromText(Renderer &renderer);

  void generateAtlas(Renderer &renderer);
  void loadFont(Renderer &renderer);
};