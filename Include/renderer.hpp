#pragma once
#include <string>
#include <iostream>
#include <vector>
#include <cstdint>
#include <vulkan/vulkan.h>
#include <unordered_map>
#include <chrono>
#include <GLFW/glfw3.h>
#include <memory>
#include "swapchainManager.hpp"
#include "deviceManager.hpp"
#include "textureManager.hpp"
#include "descriptorManager.hpp"
#include "pipelineManager.hpp"
#include "camera.h"
#include "bufferManager.hpp"
#include "mesh.hpp"
#include "vertex.h"
#include <ft2build.h>
#include <functional>
#include "engineUI.hpp"
#include FT_FREETYPE_H

class Camera;
class SwapchainManager;
class BufferManager;
class DescriptorManager;
class PipelineManager;
class DeviceManager;
struct Vertex;
class GameObject;
class ParticleEmitter;
class UI;

#ifndef RENDER_COMMAND
#define RENDER_COMMAND

enum RenderStage
{
  MainRender,
  ColorID, // for clicking and color picking
};

struct RenderCommand
{
  std::function<void(VkCommandBuffer, RenderStage)> execute;
};

#endif

class Renderer
{
public:
  const int MAX_FRAMES_IN_FLIGHT = 2;

  const std::vector<const char *> validationLayers = {
      "VK_LAYER_KHRONOS_validation"};

  const std::vector<const char *> deviceExtensions = {
      VK_KHR_SWAPCHAIN_EXTENSION_NAME,
      VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME};

  const bool enableValidationLayers = false;

  GLFWwindow *window;

  void initVulkan();

  Renderer(Camera &camera, uint32_t &WIDTH, uint32_t &HEIGHT);

  EngineUI engineUI;

  Camera &camera;
  VkInstance instance;
  VkQueue graphicsQueue;
  VkQueue presentQueue;
  VkQueue computeQueue;

  std::vector<RenderCommand> renderQueue;

  uint32_t &WIDTH;
  uint32_t &HEIGHT;

  FT_Library ft;
  FT_Face face;

  PFN_vkCmdSetPrimitiveTopology fpCmdSetPrimitiveTopology = nullptr;
  PFN_vkCmdSetDepthWriteEnableEXT vkCmdSetDepthWriteEnableEXT = nullptr;

  SwapchainManager swapchainManager;
  BufferManager bufferManager;
  DescriptorManager descriptorManager;
  PipelineManager pipelineManager;
  DeviceManager deviceManager;

  VkCommandPool commandPool;
  std::vector<VkCommandBuffer> commandBuffers;
  std::vector<VkCommandBuffer> computeCommandBuffers;

  std::vector<VkSemaphore> imageAvailableSemaphores;
  std::vector<VkSemaphore> renderFinishedSemaphores;
  std::vector<VkFence> inFlightFences;

  std::vector<VkSemaphore> computeFinishedSemaphores;
  std::vector<VkFence> computeInFlightFences;

  bool framebufferResized = true;

  void drawFrame();

  void cleanup();

  uint32_t getCurrentFrame();

private:
  std::vector<Vertex> vertices;
  std::vector<uint32_t> indices;
  uint32_t currentFrame = 0;

  void createInstance();
  bool checkValidationLayerSupport();

  void recreateSwapChain();

  void createSyncObjects();

  void createCommandBuffer();
  void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);
  void createCommandPool();

  void recordComputeCommandBuffer(VkCommandBuffer commandBuffer, uint32_t currentFrame);

  void beginCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);
  void beginOffscreenRenderPass(VkCommandBuffer commandBuffer);
  void beginColorIDRenderPass(VkCommandBuffer commandBuffer);
  void beginRenderPass(VkCommandBuffer commandBuffer, uint32_t imageIndex);
  void endCommandBuffer(VkCommandBuffer commandBuffer);
  void endRenderPass(VkCommandBuffer commandBuffer);
};