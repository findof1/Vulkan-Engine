#pragma once

#include <vulkan/vulkan_core.h>
#include <imgui.h>
#include <imgui_impl_vulkan.h>
#include <imgui_impl_glfw.h>
class Renderer;
class EngineUI
{
public:
  VkDescriptorPool descriptorPool;

  void createImGUIDescriptorPool(VkDevice device);

  void initImGui(Renderer *renderer);

  void render(Renderer *renderer);
};