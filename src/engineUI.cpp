#include "engineUI.hpp"
#include "renderer.hpp"
#include <utils.h>

void EngineUI::createImGUIDescriptorPool(VkDevice device)
{
  VkDescriptorPoolSize pool_sizes[] =
      {
          {VK_DESCRIPTOR_TYPE_SAMPLER, 1000},
          {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000},
          {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000},
          {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000},
          {VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000},
          {VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000},
          {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000},
          {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000},
          {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000},
          {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000},
          {VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000}};

  VkDescriptorPoolCreateInfo pool_info = {};
  pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
  pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
  pool_info.maxSets = 1000 * IM_ARRAYSIZE(pool_sizes);
  pool_info.poolSizeCount = (uint32_t)IM_ARRAYSIZE(pool_sizes);
  pool_info.pPoolSizes = pool_sizes;
  vkCreateDescriptorPool(device, &pool_info, nullptr, &descriptorPool);
}

void EngineUI::initImGui(Renderer *renderer)
{
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO &io = ImGui::GetIO();

  ImGui_ImplGlfw_InitForVulkan(renderer->window, true);

  QueueFamilyIndices indices = findQueueFamilies(renderer->deviceManager.physicalDevice, renderer->deviceManager.swapchainManager.surface);
  if (!indices.isComplete())
  {
    std::cerr << "Queue Family Indices are not complete when initializing imGUI" << std::endl;
    return;
  }
  createImGUIDescriptorPool(renderer->deviceManager.device);
  ImGui_ImplVulkan_InitInfo init_info = {};
  init_info.Instance = renderer->instance;
  init_info.PhysicalDevice = renderer->deviceManager.physicalDevice;
  init_info.Device = renderer->deviceManager.device;

  init_info.QueueFamily = indices.graphicsFamily.value();

  init_info.Queue = renderer->graphicsQueue;
  init_info.DescriptorPool = descriptorPool;
  init_info.RenderPass = renderer->pipelineManager.renderPass;
  init_info.Subpass = 0;
  init_info.MinImageCount = 2;
  init_info.ImageCount = 2;
  init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
  init_info.Allocator = nullptr;

  ImGui_ImplVulkan_Init(&init_info);

  bool result = ImGui_ImplVulkan_CreateFontsTexture();
  if (!result)
  {
    std::cerr << "Font upload failed\n";
  }
  vkQueueWaitIdle(renderer->graphicsQueue);
}

void EngineUI::render(Renderer *renderer)
{
  ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), renderer->commandBuffers[0]);
}
