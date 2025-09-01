#pragma once

#ifdef BUILD_ENGINE_DLL

#ifndef ENGINE_API
#define ENGINE_API __declspec(dllexport)
#endif

#else

#ifndef ENGINE_API
#define ENGINE_API __declspec(dllimport)
#endif

#endif

#include <vulkan/vulkan_core.h>
#include <imgui.h>
#include <imgui_impl_vulkan.h>
#include <imgui_impl_glfw.h>
class Renderer;
class Engine;
class ENGINE_API EngineUI
{
public:
  bool renderToViewport = false;
  VkDescriptorPool descriptorPool;

  VkImage offscreenImage;
  VkImageView offscreenImageView;
  VkDeviceMemory offscreenImageMemory;

  VkImage offscreenDepthImage;
  VkDeviceMemory offscreenDepthImageMemory;
  VkImageView offscreenDepthImageView;

  VkFramebuffer offscreenFramebuffer;

  VkImage colorIDImage;
  VkImageView colorIDImageView;
  VkDeviceMemory colorIDImageMemory;

  VkImage colorIDDepthImage;
  VkDeviceMemory colorIDDepthImageMemory;
  VkImageView colorIDDepthImageView;

  VkFramebuffer colorIDFramebuffer;

  VkSampler offscreenSampler;
  ImTextureID offscreenImageId;
  int imageW = 1100;
  int imageH = 800;

  ImVec2 sceneMin;
  ImVec2 sceneMax;

  void createImGUIDescriptorPool(VkDevice device);

  void initImGui(Renderer *renderer);

  void render(Renderer *renderer);

  void initOffscreenResources(Renderer *renderer);
  void initColorIDResources(Renderer *renderer);
  uint32_t readColorIDPixel(Renderer *renderer, int px, int py);
  void renderImGUI(Engine *engine, Renderer *renderer);
};