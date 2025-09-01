#include "text.hpp"
#include "bufferManager.hpp"
#include "descriptorManager.hpp"
#include "renderer.hpp"
#include <stdexcept>
#include <iostream>
#include <cstring>
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>
#include "mesh.hpp"

Text::Text(Renderer &renderer, int *nextRenderingId, const std::string &text, glm::vec3 position)
    : UI(renderer, nextRenderingId, position), content(text)
{
  loadFont(renderer);
  generateAtlas(renderer);
  generateVerticesFromText(renderer);
  initGraphics(renderer);
}

void Text::loadFont(Renderer &renderer)
{

  FT_Set_Pixel_Sizes(renderer.face, 0, 48);

  for (unsigned char c = 0; c < 128; c++)
  {
    if (FT_Load_Char(renderer.face, c, FT_LOAD_RENDER))
    {
      std::cerr << "Warning: Failed to load glyph for character " << c << std::endl;
      continue;
    }
    Glyph glyph;
    glyph.advance = renderer.face->glyph->advance.x >> 6;
    glyph.size = glm::ivec2(renderer.face->glyph->bitmap.width, renderer.face->glyph->bitmap.rows);
    glyph.bearing = glm::ivec2(renderer.face->glyph->bitmap_left, renderer.face->glyph->bitmap_top);

    FT_Bitmap &ftBitmap = renderer.face->glyph->bitmap;
    glyph.bitmap.width = ftBitmap.width;
    glyph.bitmap.rows = ftBitmap.rows;
    glyph.bitmap.pitch = ftBitmap.pitch;
    glyph.bitmap.pixel_mode = ftBitmap.pixel_mode;
    glyph.bitmap.num_grays = ftBitmap.num_grays;

    int bufferSize = ftBitmap.rows * std::abs(ftBitmap.pitch);
    glyph.bitmap.buffer = new unsigned char[bufferSize];
    std::memcpy(glyph.bitmap.buffer, ftBitmap.buffer, bufferSize);

    glyphs.insert(std::pair<char, Glyph>(c, glyph));
  }
}

void Text::generateVerticesFromText(Renderer &renderer)
{
  vertices.clear();

  float x = 0.0f;
  float y = 0.0f;

  for (const char &c : content)
  {
    Glyph glyph = glyphs.at(c);

    float xpos = x + glyph.bearing.x;
    float ypos = y - (glyph.size.y - glyph.bearing.y);
    float w = static_cast<float>(glyph.size.x);
    float h = static_cast<float>(glyph.size.y);

    Vertex v0;
    v0.pos = glm::vec3(xpos, ypos + h, 0.0f);
    v0.texPos = glm::vec4(glyph.uvOffset.x, glyph.uvOffset.y, 1, 1);
    v0.color = glm::vec3(0.0f);
    v0.normal = glm::vec3(-1.0f);

    Vertex v1;
    v1.pos = glm::vec3(xpos, ypos, 0.0f);
    v1.texPos = glm::vec4(glyph.uvOffset.x, glyph.uvOffset.y + glyph.uvSize.y, 1, 1);
    v1.color = glm::vec3(0.0f);
    v1.normal = glm::vec3(-1.0f);

    Vertex v2;
    v2.pos = glm::vec3(xpos + w, ypos, 0.0f);
    v2.texPos = glm::vec4(glyph.uvOffset.x + glyph.uvSize.x, glyph.uvOffset.y + glyph.uvSize.y, 1, 1);
    v2.color = glm::vec3(0.0f);
    v2.normal = glm::vec3(-1.0f);

    Vertex v3;
    v3.pos = glm::vec3(xpos + w, ypos + h, 0.0f);
    v3.texPos = glm::vec4(glyph.uvOffset.x + glyph.uvSize.x, glyph.uvOffset.y, 1, 1);
    v3.color = glm::vec3(0.0f);
    v3.normal = glm::vec3(-1.0f);

    vertices.push_back(v0);
    vertices.push_back(v1);
    vertices.push_back(v2);

    vertices.push_back(v2);
    vertices.push_back(v3);
    vertices.push_back(v0);

    x += glyph.advance;
  }
}

void Text::initGraphics(Renderer &renderer)
{
  renderer.bufferManager.createVertexBuffer(vertices, id, renderer.deviceManager.device, renderer.deviceManager.physicalDevice, renderer.commandPool, renderer.graphicsQueue);

  renderer.bufferManager.createUniformBuffers(renderer.MAX_FRAMES_IN_FLIGHT, renderer.deviceManager.device, renderer.deviceManager.physicalDevice, 1);

  TextureMaps textureMaps(textureManager.albedoImageView, textureManager.albedoSampler, textureManager.normalImageView, textureManager.normalSampler, textureManager.heightImageView, textureManager.heightSampler, textureManager.roughnessImageView, textureManager.roughnessSampler, textureManager.metallicImageView, textureManager.metallicSampler, textureManager.aoImageView, textureManager.aoSampler, textureManager.emissiveImageView, textureManager.emissiveSampler);
  renderer.descriptorManager.addDescriptorSets(renderer.deviceManager.device, renderer.MAX_FRAMES_IN_FLIGHT, 1, textureMaps);
}

void Text::updateText(const std::string &newText, Renderer &renderer)
{
  content = newText;
  generateVerticesFromText(renderer);

  renderer.bufferManager.createVertexBuffer(vertices, id, renderer.deviceManager.device, renderer.deviceManager.physicalDevice, renderer.commandPool, renderer.graphicsQueue);
}

void Text::draw(Renderer *renderer, int currentFrame, glm::mat4 transformation, glm::mat4 view, glm::mat4 projectionMatrix, VkCommandBuffer commandBuffer)
{
  if (hide)
    return;

  transformation = glm::scale(transformation, scale);

  VkBuffer vertexBuffersArray[] = {renderer->bufferManager.vertexBuffers[id]};
  VkDeviceSize offsets[] = {0};
  vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffersArray, offsets);

  vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, renderer->pipelineManager.pipelineLayout, 0, 1, &renderer->descriptorManager.descriptorSets[currentFrame + id * renderer->MAX_FRAMES_IN_FLIGHT], 0, nullptr);

  renderer->bufferManager.updateUniformBuffer(currentFrame + id * renderer->MAX_FRAMES_IN_FLIGHT, transformation, view, projectionMatrix);

  MaterialData materialData;
  materialData.albedoColor = glm::vec3(1);
  materialData.hasAlbedoMap = 1;
  materialData.isSkybox = 1; // makes it ignore lighting

  vkCmdPushConstants(commandBuffer, renderer->pipelineManager.pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(MaterialData), &materialData);

  vkCmdDraw(commandBuffer, static_cast<uint32_t>(vertices.size()), 1, 0, 0);
}

void Text::generateAtlas(Renderer &renderer)
{
  const int cols = 16;
  const int rows = 8;
  int cellWidth = 0, cellHeight = 0;

  for (auto &pair : glyphs)
  {
    cellWidth = std::max(cellWidth, pair.second.size.x);
    cellHeight = std::max(cellHeight, pair.second.size.y);
  }
  int atlasWidth = cols * cellWidth;
  int atlasHeight = rows * cellHeight;
  std::vector<uint8_t> atlasBuffer(atlasWidth * atlasHeight, 0);

  int col = 0, row = 0;

  for (auto &pair : glyphs)
  {
    if (renderer.face->glyph->bitmap.pixel_mode == FT_PIXEL_MODE_MONO)
    {
      std::cout << "uh oh: " << pair.first << std::endl;
    }
    float u = (col * cellWidth) / static_cast<float>(atlasWidth);
    float v = (row * cellHeight) / static_cast<float>(atlasHeight);
    pair.second.uvOffset = glm::vec2(u, v);
    pair.second.uvSize = glm::vec2(pair.second.size.x / static_cast<float>(atlasWidth), pair.second.size.y / static_cast<float>(atlasHeight));

    int offsetX = col * cellWidth;
    int offsetY = row * cellHeight;

    int pitch = pair.second.bitmap.pitch;
    int absPitch = std::abs(pitch);

    for (int y = 0; y < pair.second.size.y; ++y)
    {
      int sy = (pitch < 0) ? (pair.second.size.y - 1 - y) : y;

      for (int x = 0; x < pair.second.size.x; ++x)
      {
        int atlasX = offsetX + x;
        int atlasY = offsetY + y;

        if (atlasX >= atlasWidth || atlasY >= atlasHeight)
          continue;

        int srcIndex = sy * absPitch + x;

        atlasBuffer[atlasY * atlasWidth + atlasX] = pair.second.bitmap.buffer[srcIndex];
      }
    }

    col++;
    if (col >= cols)
    {
      col = 0;
      row++;
    }
  }

  textureManager.createTextureImage(atlasBuffer, atlasWidth, atlasHeight, renderer.deviceManager.device, renderer.deviceManager.physicalDevice, renderer.commandPool, renderer.graphicsQueue);
  textureManager.createTextTextureImageView(renderer.deviceManager.device);
  textureManager.createTextureSampler(renderer.deviceManager.device, renderer.deviceManager.physicalDevice);
}

void Text::cleanup(VkDevice device, Renderer &renderer)
{
  for (auto &pair : glyphs)
  {
    if (pair.second.bitmap.buffer)
    {
      delete[] pair.second.bitmap.buffer;
      pair.second.bitmap.buffer = nullptr;
    }
  }

  textureManager.cleanup(device);

  if (renderer.bufferManager.vertexBuffers.size() > id && renderer.bufferManager.vertexBuffers[id] != VK_NULL_HANDLE)
  {
    vkDestroyBuffer(device, renderer.bufferManager.vertexBuffers[id], nullptr);
    renderer.bufferManager.vertexBuffers[id] = VK_NULL_HANDLE;
  }

  for (int i = 0; i < renderer.MAX_FRAMES_IN_FLIGHT; i++)
  {
    vkDestroyBuffer(device, renderer.bufferManager.uniformBuffers[id * renderer.MAX_FRAMES_IN_FLIGHT + i], nullptr);

    renderer.bufferManager.uniformBuffers[id * renderer.MAX_FRAMES_IN_FLIGHT + i] = VK_NULL_HANDLE;
  }

  uint32_t descriptorSetCount = renderer.MAX_FRAMES_IN_FLIGHT;
  uint32_t startIndex = id * renderer.MAX_FRAMES_IN_FLIGHT;
  if (renderer.descriptorManager.descriptorSets.size() >= startIndex + descriptorSetCount)
  {
    VkDescriptorSet *pDescriptorSets = renderer.descriptorManager.descriptorSets.data() + startIndex;
    VkResult result = vkFreeDescriptorSets(device, renderer.descriptorManager.descriptorPool, descriptorSetCount, pDescriptorSets);
    if (result != VK_SUCCESS)
    {
      std::cerr << "Failed to free descriptor sets for texture id " << id << std::endl;
    }
  }
}