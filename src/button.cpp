#include "button.hpp"
#include "bufferManager.hpp"
#include "descriptorManager.hpp"
#include <iostream>
#include "mesh.hpp"

Button::Button(Renderer &renderer, int *nextRenderingId, const std::string &label, glm::vec3 position, std::array<glm::vec2, 2> verticesOffsets, std::string texture)
    : UI(renderer, nextRenderingId, position), label(label), verticesOffsets(verticesOffsets)
{
  if (texture != "models/couch/diffuse.png")
  {
    hasTexture = 1;
  }
  initButtonVertices();

  initGraphics(renderer, texture);
  buttonText = new Text(renderer, nextRenderingId, label, glm::vec3(position.x, position.y, position.z + 0.1));
}

void Button::initGraphics(Renderer &renderer, std::string texture)
{
  textureManager.createTextureImage(texture, renderer.deviceManager.device, renderer.deviceManager.physicalDevice, renderer.commandPool, renderer.graphicsQueue);
  textureManager.createTextureImageView(renderer.deviceManager.device);
  textureManager.createTextureSampler(renderer.deviceManager.device, renderer.deviceManager.physicalDevice);

  renderer.bufferManager.createVertexBuffer(vertices, id, renderer.deviceManager.device, renderer.deviceManager.physicalDevice, renderer.commandPool, renderer.graphicsQueue);

  renderer.bufferManager.createUniformBuffers(renderer.MAX_FRAMES_IN_FLIGHT, renderer.deviceManager.device, renderer.deviceManager.physicalDevice, 1);

  renderer.descriptorManager.addDescriptorSets(renderer.deviceManager.device, renderer.MAX_FRAMES_IN_FLIGHT, 1, textureManager.textureImageView, textureManager.textureSampler);
}

void Button::initGraphics(Renderer &renderer)
{
  textureManager.createTextureImage("models/couch/diffuse.png", renderer.deviceManager.device, renderer.deviceManager.physicalDevice, renderer.commandPool, renderer.graphicsQueue);
  textureManager.createTextureImageView(renderer.deviceManager.device);
  textureManager.createTextureSampler(renderer.deviceManager.device, renderer.deviceManager.physicalDevice);

  renderer.bufferManager.createVertexBuffer(vertices, id, renderer.deviceManager.device, renderer.deviceManager.physicalDevice, renderer.commandPool, renderer.graphicsQueue);

  renderer.bufferManager.createUniformBuffers(renderer.MAX_FRAMES_IN_FLIGHT, renderer.deviceManager.device, renderer.deviceManager.physicalDevice, 1);

  renderer.descriptorManager.addDescriptorSets(renderer.deviceManager.device, renderer.MAX_FRAMES_IN_FLIGHT, 1, textureManager.textureImageView, textureManager.textureSampler);
}

void Button::draw(Renderer *renderer, int currentFrame, glm::mat4 transformation, glm::mat4 view, glm::mat4 projectionMatrix, VkCommandBuffer commandBuffer)
{
  if (hide)
    return;

  glm::mat4 newTransformation = glm::scale(transformation, scale);

  VkBuffer vertexBuffersArray[] = {renderer->bufferManager.vertexBuffers[id]};
  VkDeviceSize offsets[] = {0};
  vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffersArray, offsets);

  vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, renderer->pipelineManager.pipelineLayout, 0, 1, &renderer->descriptorManager.descriptorSets[currentFrame + id * renderer->MAX_FRAMES_IN_FLIGHT], 0, nullptr);

  renderer->bufferManager.updateUniformBuffer(currentFrame + id * renderer->MAX_FRAMES_IN_FLIGHT, newTransformation, view, projectionMatrix);

  MaterialData materialData;
  materialData.diffuseColor = currentColor;
  materialData.hasTexture = hasTexture;

  vkCmdPushConstants(commandBuffer, renderer->pipelineManager.pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(MaterialData), &materialData);

  vkCmdDraw(commandBuffer, static_cast<uint32_t>(vertices.size()), 1, 0, 0);

  buttonText->draw(renderer, currentFrame, transformation, view, projectionMatrix, commandBuffer);
}

void Button::updateState(float mouseX, float mouseY, bool mousePressed)
{
  bool canClick = isHovered && !isPressed;
  isHovered = (mouseX > position.x + verticesOffsets[0].x && mouseX < position.x + verticesOffsets[1].x && mouseY < abs(position.y + verticesOffsets[0].y) && mouseY > abs(position.y + verticesOffsets[1].y));
  isPressed = isHovered && mousePressed;

  if (isPressed)
  {
    currentColor = pressedColor;
  }
  else if (isHovered)
  {
    currentColor = hoverColor;
  }
  else
  {
    currentColor = normalColor;
  }

  if (isPressed)
  {
    if (!clicked && canClick)
    {
      callback();
    }
    clicked = true;
  }
  else
  {
    clicked = false;
  }
}

void Button::initButtonVertices()
{
  vertices.clear();

  Vertex v0;
  v0.pos = glm::vec3(verticesOffsets[0].x, verticesOffsets[1].y, -0.1f);
  v0.color = glm::vec3(0);
  v0.normal = glm::vec3(-1.0f);
  v0.texPos = glm::vec2(0, 1);

  Vertex v1;
  v1.pos = glm::vec3(verticesOffsets[0].x, verticesOffsets[0].y, -0.1f);
  v1.color = glm::vec3(0);
  v1.normal = glm::vec3(-1.0f);
  v1.texPos = glm::vec2(0, 0);

  Vertex v2;
  v2.pos = glm::vec3(verticesOffsets[1].x, verticesOffsets[0].y, -0.1f);
  v2.color = glm::vec3(0);
  v2.normal = glm::vec3(-1.0f);
  v2.texPos = glm::vec2(1, 0);

  Vertex v3;
  v3.pos = glm::vec3(verticesOffsets[1].x, verticesOffsets[1].y, -0.1f);
  v3.color = glm::vec3(0);
  v3.normal = glm::vec3(-1.0f);
  v3.texPos = glm::vec2(1, 1);

  vertices.push_back(v0);
  vertices.push_back(v1);
  vertices.push_back(v2);
  vertices.push_back(v2);
  vertices.push_back(v3);
  vertices.push_back(v0);
}

void Button::cleanup(VkDevice device, Renderer &renderer)
{
  if (buttonText)
  {
    buttonText->cleanup(device, renderer);
    delete buttonText;
  }

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
      std::cerr << "Failed to free descriptor sets for button id " << id << std::endl;
    }
  }
}