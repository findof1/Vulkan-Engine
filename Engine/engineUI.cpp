#include "engineUI.hpp"
#include "renderer.hpp"
#include <utils.h>
#include "engine.hpp"

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

  initOffscreenResources(renderer);
  initColorIDResources(renderer);
}

void EngineUI::initOffscreenResources(Renderer *renderer)
{
  createImage(imageW, imageH, VK_FORMAT_B8G8R8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, offscreenImage, offscreenImageMemory, renderer->deviceManager.device, renderer->deviceManager.physicalDevice);

  offscreenImageView = createImageView(offscreenImage, VK_FORMAT_B8G8R8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT, renderer->deviceManager.device);

  VkFormat depthFormat = findSupportedFormat({VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT}, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT, renderer->deviceManager.physicalDevice);

  createImage(imageW, imageH, depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, offscreenDepthImage, offscreenDepthImageMemory, renderer->deviceManager.device, renderer->deviceManager.physicalDevice);
  offscreenDepthImageView = createImageView(offscreenDepthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, renderer->deviceManager.device);

  transitionImageLayout(offscreenDepthImage, depthFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, renderer->deviceManager.device, renderer->commandPool, renderer->graphicsQueue);

  VkImageView attachments[] = {
      offscreenImageView,
      offscreenDepthImageView};

  VkFramebufferCreateInfo framebufferInfo{};
  framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
  framebufferInfo.renderPass = renderer->pipelineManager.offscreenRenderPass;
  framebufferInfo.attachmentCount = 2;
  framebufferInfo.pAttachments = attachments;
  framebufferInfo.width = imageW;
  framebufferInfo.height = imageH;
  framebufferInfo.layers = 1;

  vkCreateFramebuffer(renderer->deviceManager.device, &framebufferInfo, nullptr, &offscreenFramebuffer);

  VkSamplerCreateInfo samplerInfo{};
  samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;

  samplerInfo.magFilter = VK_FILTER_LINEAR;
  samplerInfo.minFilter = VK_FILTER_LINEAR;

  samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
  samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
  samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;

  samplerInfo.anisotropyEnable = VK_FALSE;
  samplerInfo.maxAnisotropy = 1.0f;

  samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;

  samplerInfo.unnormalizedCoordinates = VK_FALSE;

  samplerInfo.compareEnable = VK_FALSE;
  samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;

  samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
  samplerInfo.mipLodBias = 0.0f;
  samplerInfo.minLod = 0.0f;
  samplerInfo.maxLod = 0.0f;
  if (vkCreateSampler(renderer->deviceManager.device, &samplerInfo, nullptr, &offscreenSampler) != VK_SUCCESS)
  {
    throw std::runtime_error("failed to create offscreen image sampler!");
  }
  offscreenImageId = ImGui_ImplVulkan_AddTexture(offscreenSampler, offscreenImageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
}

void EngineUI::initColorIDResources(Renderer *renderer)
{
  createImage(imageW, imageH, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, colorIDImage, colorIDImageMemory, renderer->deviceManager.device, renderer->deviceManager.physicalDevice);

  colorIDImageView = createImageView(colorIDImage, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT, renderer->deviceManager.device);

  VkFormat depthFormat = findSupportedFormat({VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT}, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT, renderer->deviceManager.physicalDevice);

  createImage(imageW, imageH, depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, colorIDDepthImage, colorIDDepthImageMemory, renderer->deviceManager.device, renderer->deviceManager.physicalDevice);
  colorIDDepthImageView = createImageView(colorIDDepthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, renderer->deviceManager.device);

  transitionImageLayout(colorIDDepthImage, depthFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, renderer->deviceManager.device, renderer->commandPool, renderer->graphicsQueue);

  VkImageView attachments[] = {
      colorIDImageView,
      colorIDDepthImageView};

  VkFramebufferCreateInfo framebufferInfo{};
  framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
  framebufferInfo.renderPass = renderer->pipelineManager.colorIDRenderPass;
  framebufferInfo.attachmentCount = 2;
  framebufferInfo.pAttachments = attachments;
  framebufferInfo.width = imageW;
  framebufferInfo.height = imageH;
  framebufferInfo.layers = 1;

  vkCreateFramebuffer(renderer->deviceManager.device, &framebufferInfo, nullptr, &colorIDFramebuffer);
}

void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer &buffer, VkDeviceMemory &bufferMemory, VkDevice device, VkPhysicalDevice physicalDevice)
{
  VkBufferCreateInfo bufferInfo{};
  bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  bufferInfo.size = size;
  bufferInfo.usage = usage;
  bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

  if (vkCreateBuffer(device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS)
  {
    throw std::runtime_error("failed to create buffer!");
  }

  VkMemoryRequirements memRequirements;
  vkGetBufferMemoryRequirements(device, buffer, &memRequirements);

  VkMemoryAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  allocInfo.allocationSize = memRequirements.size;
  allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties, physicalDevice);

  if (vkAllocateMemory(device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS)
  {
    throw std::runtime_error("failed to allocate buffer memory!");
  }

  vkBindBufferMemory(device, buffer, bufferMemory, 0);
}

uint32_t EngineUI::readColorIDPixel(Renderer *renderer, int px, int py)
{
  VkDevice device = renderer->deviceManager.device;
  VkPhysicalDevice physicalDevice = renderer->deviceManager.physicalDevice;
  VkCommandPool commandPool = renderer->commandPool;
  VkQueue graphicsQueue = renderer->graphicsQueue;

  VkCommandBufferAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  allocInfo.commandPool = commandPool;
  allocInfo.commandBufferCount = 1;

  VkCommandBuffer commandBuffer;
  vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

  VkCommandBufferBeginInfo beginInfo{};
  beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

  vkBeginCommandBuffer(commandBuffer, &beginInfo);

  VkImageMemoryBarrier barrier{};
  barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
  barrier.oldLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
  barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
  barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
  barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
  barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.image = colorIDImage;
  barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  barrier.subresourceRange.baseMipLevel = 0;
  barrier.subresourceRange.levelCount = 1;
  barrier.subresourceRange.baseArrayLayer = 0;
  barrier.subresourceRange.layerCount = 1;

  vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr,
                       0, nullptr, 1, &barrier);

  VkDeviceSize bufferSize = 4;

  VkBuffer stagingBuffer;
  VkDeviceMemory stagingBufferMemory;

  createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory, device, physicalDevice);

  VkBufferImageCopy region{};
  region.bufferOffset = 0;
  region.bufferRowLength = 0;
  region.bufferImageHeight = 0;
  region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  region.imageSubresource.mipLevel = 0;
  region.imageSubresource.baseArrayLayer = 0;
  region.imageSubresource.layerCount = 1;
  region.imageOffset = {px, py, 0};
  region.imageExtent = {1, 1, 1};

  vkCmdCopyImageToBuffer(commandBuffer, colorIDImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, stagingBuffer, 1, &region);

  VkImageMemoryBarrier barrier2{};
  barrier2.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
  barrier2.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
  barrier2.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
  barrier2.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
  barrier2.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
  barrier2.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier2.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier2.image = colorIDImage;
  barrier2.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  barrier2.subresourceRange.baseMipLevel = 0;
  barrier2.subresourceRange.levelCount = 1;
  barrier2.subresourceRange.baseArrayLayer = 0;
  barrier2.subresourceRange.layerCount = 1;

  vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier2);

  vkEndCommandBuffer(commandBuffer);

  VkSubmitInfo submitInfo{};
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = &commandBuffer;

  VkFenceCreateInfo fenceInfo{};
  fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
  VkFence fence;
  vkCreateFence(device, &fenceInfo, nullptr, &fence);

  vkQueueSubmit(graphicsQueue, 1, &submitInfo, fence);
  vkWaitForFences(device, 1, &fence, VK_TRUE, UINT64_MAX);

  vkDestroyFence(device, fence, nullptr);
  vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);

  void *data;
  vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);

  uint32_t pixelValue = *(uint32_t *)data;

  vkUnmapMemory(device, stagingBufferMemory);

  vkDestroyBuffer(device, stagingBuffer, nullptr);
  vkFreeMemory(device, stagingBufferMemory, nullptr);

  uint8_t b = (pixelValue >> 0) & 0xFF;
  uint8_t g = (pixelValue >> 8) & 0xFF;
  uint8_t r = (pixelValue >> 16) & 0xFF;
  int objectID = (b << 16) | (g << 8) | r;

  return objectID;
}

void EngineUI::renderImGUI(Engine *engine, Renderer *renderer)
{
  ImGui_ImplVulkan_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();

  Entity *selected = &engine->registry.selected;

  ImGui::Begin("Hierarchy");

  if (ImGui::BeginTable("Game Objects", 1, ImGuiTableFlags_Borders))
  {
    for (const auto &[key, entity] : engine->registry.entities)
    {
      ImGui::TableNextRow();
      ImGui::TableNextColumn();

      bool isSelected = (*selected == entity);
      if (ImGui::Selectable(key.c_str(), isSelected))
      {
        engine->selectedUI = "";
        *selected = entity;
      }
    }
    for (const auto &[key, _] : engine->UIElements)
    {
      ImGui::TableNextRow();
      ImGui::TableNextColumn();
      bool isSelected = (*selected == -1 && engine->selectedUI == key);
      if (ImGui::Selectable(key.c_str(), isSelected))
      {
        engine->selectedUI = key;
        *selected = -1;
      }
    }
    ImGui::EndTable();
  }

  ImGui::End();

  ImGui::Begin("Inspector");
  if (*selected != -1)
  {
    if (engine->registry.transforms.find(*selected) != engine->registry.transforms.end())
    {
      ImGui::Text("Transform");

      TransformComponent &transform = engine->getTransformComponentNoUpdate(*selected);
      bool updated = false;
      updated |= ImGui::DragFloat3("Position", &transform.position.x, 0.1f);
      updated |= ImGui::DragFloat3("Rotation", &transform.rotationZYX.x, 0.1f);
      updated |= ImGui::DragFloat3("Scale", &transform.scale.x, 0.1f);
      if (updated)
      {
        transform.justUpdated = true;
      }
    }
    if (engine->registry.boxColliders.find(*selected) != engine->registry.boxColliders.end())
    {
      ImGui::Text("Box Collider");

      BoxColliderComponent &boxCollider = engine->getBoxColliderComponentNoUpdate(*selected);
      bool updated = false;
      updated |= ImGui::DragFloat3("Local Min", &boxCollider.localMin.x, 0.1f);
      updated |= ImGui::DragFloat3("Local Max", &boxCollider.localMax.x, 0.1f);
      updated |= ImGui::Checkbox("Auto Update To Transform", &boxCollider.autoUpdate);
      if (!boxCollider.autoUpdate)
      {
        updated |= ImGui::DragFloat3("Position", &boxCollider.position.x, 0.1f);
        updated |= ImGui::DragFloat3("Rotation", &boxCollider.rotationZYX.x, 0.1f);
        updated |= ImGui::DragFloat3("Scale", &boxCollider.scale.x, 0.1f);
        boxCollider.updateWorldAABB(boxCollider.position, boxCollider.rotationZYX, boxCollider.scale);
      }
      if (updated)
      {
        boxCollider.justUpdated = true;
      }
    }
    if (engine->registry.rigidBodies.find(*selected) != engine->registry.rigidBodies.end())
    {
      ImGui::Text("Rigid Body");

      RigidBodyComponent &rigidBody = engine->getRigidBodyComponent(*selected);
      bool updated = false;
      updated |= ImGui::DragFloat("Mass", &rigidBody.mass, 0.1f);
      updated |= ImGui::Checkbox("Is Static", &rigidBody.isStatic);
      updated |= ImGui::Checkbox("Use Gravity", &rigidBody.useGravity);
    }
  }
  else if (!engine->selectedUI.empty())
  {
    auto it = engine->UIElements.find(engine->selectedUI);
    if (it != engine->UIElements.end() && it->second)
    {
      ImGui::DragFloat3("Position", &it->second->position.x, 0.1f);
    }
  }
  ImGui::End();

  ImGui::SetNextWindowSize(ImVec2(1200, 900));
  ImGui::Begin("Viewport");
  ImGui::Image(offscreenImageId, ImVec2(imageW, imageH));

  ImVec2 imageMin = ImGui::GetItemRectMin();
  ImVec2 imageMax = ImGui::GetItemRectMax();
  sceneMin = imageMin;
  sceneMax = imageMax;

  ImVec2 mousePos = ImGui::GetMousePos();

  bool insideImage = (mousePos.x >= imageMin.x && mousePos.x < imageMax.x &&
                      mousePos.y >= imageMin.y && mousePos.y < imageMax.y);

  int px = static_cast<int>(mousePos.x - imageMin.x);
  int py = static_cast<int>(mousePos.y - imageMin.y);

  if (insideImage && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
  {
    uint32_t pickedColorID = readColorIDPixel(renderer, px, py);
    if (pickedColorID < engine->registry.getNextEntity())
      *selected = pickedColorID;
  }

  ImGui::End();

  ImGui::Render();
}