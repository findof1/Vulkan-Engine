#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "renderer.hpp"
#include "utils.h"
#include <cstring>
#include "text.hpp"
#include "particleEmitter.hpp"
#include <imgui.h>
#include <imgui_impl_vulkan.h>

const std::vector<const char *> Renderer::validationLayers = {
    "VK_LAYER_KHRONOS_validation"};

const std::vector<const char *> Renderer::deviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME,
    VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME};

Renderer::Renderer(Camera &camera, uint32_t &WIDTH, uint32_t &HEIGHT)
    : bufferManager(), swapchainManager(), deviceManager(swapchainManager), descriptorManager(bufferManager), pipelineManager(swapchainManager, descriptorManager), camera(camera), WIDTH(WIDTH), HEIGHT(HEIGHT)
{
  if (FT_Init_FreeType(&ft))
  {
    std::cerr << "Could not initialize FreeType Library\n";
  }
  if (FT_New_Face(ft, "C:/Windows/Fonts/arial.ttf", 0, &face))
  {
    std::cerr << "Could not load font\n";
  }
}

void Renderer::initVulkan()
{
  std::cout << "DE1" << std::endl;
  createInstance();
  std::cout << "DE2" << std::endl;
  swapchainManager.createSurface(instance);
  std::cout << "DE3" << std::endl;
  deviceManager.pickPhysicalDevice(instance, deviceExtensions);
  std::cout << "DE4" << std::endl;
  deviceManager.createLogicalDevice(enableValidationLayers, deviceExtensions, validationLayers, &presentQueue, &graphicsQueue, &computeQueue);
  std::cout << "DE5" << std::endl;

  swapchainManager.createSwapChain(deviceManager.device, deviceManager.physicalDevice);
  std::cout << "DE6" << std::endl;
  swapchainManager.createImageViews(deviceManager.device);
  std::cout << "DE7" << std::endl;
  pipelineManager.createRenderPass(deviceManager.device, deviceManager.physicalDevice);
  std::cout << "DE8" << std::endl;
  pipelineManager.createOffScreenRenderPass(deviceManager.device, deviceManager.physicalDevice);
  std::cout << "DE9" << std::endl;
  pipelineManager.createColorIDRenderPass(deviceManager.device, deviceManager.physicalDevice);
  std::cout << "DE10" << std::endl;
  descriptorManager.createDescriptorSetLayout(deviceManager.device);
  std::cout << "DE11" << std::endl;
  pipelineManager.createGraphicsPipeline(deviceManager.device);
  std::cout << "DE12" << std::endl;
  pipelineManager.createColorIDPipeline(deviceManager.device);
  std::cout << "DE13" << std::endl;
  pipelineManager.createComputePipeline(deviceManager.device);
  std::cout << "DE14" << std::endl;
  createCommandPool();
  std::cout << "DE15" << std::endl;
  swapchainManager.createDepthResources(deviceManager.device, deviceManager.physicalDevice, commandPool, graphicsQueue);
  std::cout << "DE16" << std::endl;
  swapchainManager.createFramebuffers(deviceManager.device, pipelineManager.renderPass);
  std::cout << "DE17" << std::endl;

  // bufferManager.createVertexBuffer(vertices, 0, deviceManager.device, deviceManager.physicalDevice, commandPool, graphicsQueue);
  // bufferManager.createIndexBuffer(indices, 0, deviceManager.device, deviceManager.physicalDevice, commandPool, //graphicsQueue);
  // bufferManager.createUniformBuffers(MAX_FRAMES_IN_FLIGHT, deviceManager.device, deviceManager.physicalDevice, 2);
  bufferManager.createLightsUniformBuffers(MAX_FRAMES_IN_FLIGHT, deviceManager.device, deviceManager.physicalDevice);
  std::cout << "DE18" << std::endl;
  descriptorManager.createDescriptorPool(deviceManager.device, MAX_FRAMES_IN_FLIGHT, 250);
  std::cout << "DE19" << std::endl;

  // descriptorManager.createDescriptorSets(deviceManager.device, MAX_FRAMES_IN_FLIGHT, 1);
  // descriptorManager.addDescriptorSets(deviceManager.device, MAX_FRAMES_IN_FLIGHT, 1);
  createCommandBuffer();
  std::cout << "DE20" << std::endl;
  createSyncObjects();
  std::cout << "DE21" << std::endl;

  fpCmdSetPrimitiveTopology = (PFN_vkCmdSetPrimitiveTopology)vkGetDeviceProcAddr(deviceManager.device, "vkCmdSetPrimitiveTopology");
  std::cout << "DE22" << std::endl;
  vkCmdSetDepthWriteEnableEXT = (PFN_vkCmdSetDepthWriteEnableEXT)vkGetDeviceProcAddr(deviceManager.device, "vkCmdSetDepthWriteEnableEXT");
  std::cout << "DE23" << std::endl;
}

void Renderer::recreateSwapChain()
{
  int width = 0, height = 0;
  glfwGetFramebufferSize(window, &width, &height);
  while (width == 0 || height == 0)
  {
    glfwGetFramebufferSize(window, &width, &height);
    glfwWaitEvents();
  }
  WIDTH = width;
  HEIGHT = height;
  vkDeviceWaitIdle(deviceManager.device);

  swapchainManager.cleanupSwapChain(deviceManager.device);

  swapchainManager.createSwapChain(deviceManager.device, deviceManager.physicalDevice);
  swapchainManager.createImageViews(deviceManager.device);
  swapchainManager.createDepthResources(deviceManager.device, deviceManager.physicalDevice, commandPool, graphicsQueue);
  swapchainManager.createFramebuffers(deviceManager.device, pipelineManager.renderPass);
}

void Renderer::createSyncObjects()
{
  imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
  renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
  inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
  VkSemaphoreCreateInfo semaphoreInfo{};
  semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

  VkFenceCreateInfo fenceInfo{};
  fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
  fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
  {
    if (vkCreateSemaphore(deviceManager.device, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
        vkCreateSemaphore(deviceManager.device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS ||
        vkCreateFence(deviceManager.device, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS)
    {
      throw std::runtime_error("failed to create synchronization objects!");
    }
  }

  computeFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
  computeInFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
  {
    if (vkCreateSemaphore(deviceManager.device, &semaphoreInfo, nullptr, &computeFinishedSemaphores[i]) != VK_SUCCESS ||
        vkCreateFence(deviceManager.device, &fenceInfo, nullptr, &computeInFlightFences[i]) != VK_SUCCESS)
    {
      throw std::runtime_error("failed to create synchronization objects for compute!");
    }
  }
}

void Renderer::createCommandBuffer()
{
  commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);
  VkCommandBufferAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  allocInfo.commandPool = commandPool;
  allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  allocInfo.commandBufferCount = (uint32_t)commandBuffers.size();

  if (vkAllocateCommandBuffers(deviceManager.device, &allocInfo, commandBuffers.data()) != VK_SUCCESS)
  {
    throw std::runtime_error("failed to allocate command buffers!");
  }

  computeCommandBuffers.resize(MAX_FRAMES_IN_FLIGHT);
  VkCommandBufferAllocateInfo computeAllocInfo{};
  computeAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  computeAllocInfo.commandPool = commandPool;
  computeAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  computeAllocInfo.commandBufferCount = (uint32_t)computeCommandBuffers.size();

  if (vkAllocateCommandBuffers(deviceManager.device, &computeAllocInfo, computeCommandBuffers.data()) != VK_SUCCESS)
  {
    throw std::runtime_error("failed to allocate compute command buffers!");
  }
}

void Renderer::beginCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex)
{
  VkCommandBufferBeginInfo beginInfo{};
  beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  beginInfo.flags = 0;
  beginInfo.pInheritanceInfo = nullptr;

  if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
  {
    throw std::runtime_error("failed to begin recording command buffer!");
  }
}

void Renderer::beginOffscreenRenderPass(VkCommandBuffer commandBuffer)
{
  VkImageMemoryBarrier barrier{};
  barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
  barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  barrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
  barrier.srcAccessMask = 0;
  barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
  barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.image = engineUI.offscreenImage;
  barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  barrier.subresourceRange.baseMipLevel = 0;
  barrier.subresourceRange.levelCount = 1;
  barrier.subresourceRange.baseArrayLayer = 0;
  barrier.subresourceRange.layerCount = 1;

  vkCmdPipelineBarrier(
      commandBuffer,
      VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
      VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
      0,
      0, nullptr,
      0, nullptr,
      1, &barrier);

  VkRenderPassBeginInfo renderPassInfo{};
  renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
  renderPassInfo.renderPass = pipelineManager.offscreenRenderPass;
  renderPassInfo.framebuffer = engineUI.offscreenFramebuffer;

  VkExtent2D offscreenExtent;
  offscreenExtent.width = engineUI.imageW;
  offscreenExtent.height = engineUI.imageH;
  renderPassInfo.renderArea.offset = {0, 0};
  renderPassInfo.renderArea.extent = offscreenExtent;

  VkClearValue clearValues[2];
  clearValues[0].color = {0.0f, 0.0f, 0.0f, 1.0f};
  clearValues[1].depthStencil = {1.0f, 0};

  renderPassInfo.clearValueCount = 2;
  renderPassInfo.pClearValues = clearValues;

  vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
}

void Renderer::beginColorIDRenderPass(VkCommandBuffer commandBuffer)
{
  VkImageMemoryBarrier barrier{};
  barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
  barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  barrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
  barrier.srcAccessMask = 0;
  barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
  barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.image = engineUI.colorIDImage;
  barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  barrier.subresourceRange.baseMipLevel = 0;
  barrier.subresourceRange.levelCount = 1;
  barrier.subresourceRange.baseArrayLayer = 0;
  barrier.subresourceRange.layerCount = 1;

  vkCmdPipelineBarrier(
      commandBuffer,
      VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
      VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
      0,
      0, nullptr,
      0, nullptr,
      1, &barrier);

  VkRenderPassBeginInfo renderPassInfo{};
  renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
  renderPassInfo.renderPass = pipelineManager.colorIDRenderPass;
  renderPassInfo.framebuffer = engineUI.colorIDFramebuffer;

  VkExtent2D offscreenExtent;
  offscreenExtent.width = engineUI.imageW;
  offscreenExtent.height = engineUI.imageH;
  renderPassInfo.renderArea.offset = {0, 0};
  renderPassInfo.renderArea.extent = offscreenExtent;

  VkClearValue clearValues[2];
  clearValues[0].color = {0.0f, 0.0f, 0.0f, 1.0f};
  clearValues[1].depthStencil = {1.0f, 0};

  renderPassInfo.clearValueCount = 2;
  renderPassInfo.pClearValues = clearValues;

  vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
}

void Renderer::beginRenderPass(VkCommandBuffer commandBuffer, uint32_t imageIndex)
{
  VkImageMemoryBarrier barrier = {};
  barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
  barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  barrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
  barrier.srcAccessMask = 0;
  barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
  barrier.image = swapchainManager.swapChainImages[imageIndex];
  barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  barrier.subresourceRange.baseArrayLayer = 0;
  barrier.subresourceRange.layerCount = 1;
  barrier.subresourceRange.levelCount = 1;

  vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);

  VkRenderPassBeginInfo renderPassInfo{};
  renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
  renderPassInfo.renderPass = pipelineManager.renderPass;
  renderPassInfo.framebuffer = swapchainManager.swapChainFramebuffers[imageIndex];

  renderPassInfo.renderArea.offset = {0, 0};
  renderPassInfo.renderArea.extent = swapchainManager.swapChainExtent;

  std::array<VkClearValue, 2> clearValues{};
  clearValues[0].color = {{0.0f, 0.0f, 0.0f, 1.0f}};
  clearValues[1].depthStencil = {1.0f, 0};

  renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
  renderPassInfo.pClearValues = clearValues.data();

  vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
}

void Renderer::endCommandBuffer(VkCommandBuffer commandBuffer)
{
  if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
  {
    throw std::runtime_error("failed to record command buffer!");
  }
}

void Renderer::endRenderPass(VkCommandBuffer commandBuffer)
{
  vkCmdEndRenderPass(commandBuffer);
}

void Renderer::recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex)
{
  beginCommandBuffer(commandBuffer, imageIndex);
  beginOffscreenRenderPass(commandBuffer);

  for (const auto &cmd : renderQueue)
  {
    cmd.execute(commandBuffer, RenderStage::MainRender);
  }

  endRenderPass(commandBuffer);

  VkImageMemoryBarrier barrier{};
  barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
  barrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
  barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
  barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
  barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
  barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.image = engineUI.offscreenImage;
  barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  barrier.subresourceRange.baseMipLevel = 0;
  barrier.subresourceRange.levelCount = 1;
  barrier.subresourceRange.baseArrayLayer = 0;
  barrier.subresourceRange.layerCount = 1;

  vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);

  beginColorIDRenderPass(commandBuffer);

  for (const auto &cmd : renderQueue)
  {
    cmd.execute(commandBuffer, RenderStage::ColorID);
  }

  endRenderPass(commandBuffer);

  VkImageMemoryBarrier readBarrier{};
  readBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
  readBarrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
  readBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
  readBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
  readBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
  readBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  readBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  readBarrier.image = engineUI.colorIDImage;
  readBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  readBarrier.subresourceRange.baseMipLevel = 0;
  readBarrier.subresourceRange.levelCount = 1;
  readBarrier.subresourceRange.baseArrayLayer = 0;
  readBarrier.subresourceRange.layerCount = 1;

  vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &readBarrier);

  beginRenderPass(commandBuffer, imageIndex);

  vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineManager.graphicsPipeline);
  ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);

  endRenderPass(commandBuffer);

  endCommandBuffer(commandBuffer);
}

void Renderer::recordComputeCommandBuffer(VkCommandBuffer commandBuffer, uint32_t currentFrame)
{

  VkCommandBufferBeginInfo beginInfo{};
  beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

  if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
  {
    throw std::runtime_error("failed to begin recording compute command buffer!");
  }

  vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipelineManager.computePipeline);

  vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipelineManager.computePipelineLayout, 0, 1, &descriptorManager.computeDescriptorSets[currentFrame], 0, nullptr);

  vkCmdDispatch(commandBuffer, 512 / 256, 1, 1);

  VkMemoryBarrier memoryBarrier{};
  memoryBarrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
  memoryBarrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
  memoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

  vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 0, 1, &memoryBarrier, 0, nullptr, 0, nullptr);

  if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
  {
    throw std::runtime_error("failed to record compute command buffer!");
  }
}

void Renderer::createCommandPool()
{
  QueueFamilyIndices queueFamilyIndices = findQueueFamilies(deviceManager.physicalDevice, swapchainManager.surface);

  VkCommandPoolCreateInfo poolInfo{};
  poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
  poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

  if (vkCreateCommandPool(deviceManager.device, &poolInfo, nullptr, &commandPool) != VK_SUCCESS)
  {
    throw std::runtime_error("failed to create command pool!");
  }
}

void Renderer::drawFrame()
{
  vkWaitForFences(deviceManager.device, 1, &computeInFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

  bufferManager.updateComputeUniformBuffer(currentFrame, 0.0005);

  vkResetFences(deviceManager.device, 1, &computeInFlightFences[currentFrame]);
  vkResetCommandBuffer(computeCommandBuffers[currentFrame], 0);

  recordComputeCommandBuffer(computeCommandBuffers[currentFrame], currentFrame);

  VkSubmitInfo computeSubmitInfo{};
  computeSubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

  computeSubmitInfo.commandBufferCount = 1;
  computeSubmitInfo.pCommandBuffers = &computeCommandBuffers[currentFrame];
  computeSubmitInfo.signalSemaphoreCount = 1;
  computeSubmitInfo.pSignalSemaphores = &computeFinishedSemaphores[currentFrame];

  if (vkQueueSubmit(computeQueue, 1, &computeSubmitInfo, computeInFlightFences[currentFrame]) != VK_SUCCESS)
  {
    throw std::runtime_error("failed to submit compute command buffer!");
  };

  vkWaitForFences(deviceManager.device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

  uint32_t imageIndex;
  VkResult result = vkAcquireNextImageKHR(deviceManager.device, swapchainManager.swapChain, UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

  if (result == VK_ERROR_OUT_OF_DATE_KHR)
  {
    recreateSwapChain();
    return;
  }
  else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
  {
    throw std::runtime_error("failed to acquire swap chain image!");
  }

  vkResetFences(deviceManager.device, 1, &inFlightFences[currentFrame]);

  vkResetCommandBuffer(commandBuffers[currentFrame], 0);

  recordCommandBuffer(commandBuffers[currentFrame], imageIndex);

  VkSubmitInfo submitInfo{};
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

  VkSemaphore waitSemaphores[] = {computeFinishedSemaphores[currentFrame], imageAvailableSemaphores[currentFrame]};
  VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT};

  submitInfo.waitSemaphoreCount = 2;
  submitInfo.pWaitSemaphores = waitSemaphores;
  submitInfo.pWaitDstStageMask = waitStages;

  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = &commandBuffers[currentFrame];

  VkSemaphore signalSemaphores[] = {renderFinishedSemaphores[currentFrame]};
  submitInfo.signalSemaphoreCount = 1;
  submitInfo.pSignalSemaphores = signalSemaphores;

  if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS)
  {
    throw std::runtime_error("failed to submit draw command buffer!");
  }

  VkPresentInfoKHR presentInfo{};
  presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

  presentInfo.waitSemaphoreCount = 1;
  presentInfo.pWaitSemaphores = signalSemaphores;

  VkSwapchainKHR swapChains[] = {swapchainManager.swapChain};
  presentInfo.swapchainCount = 1;
  presentInfo.pSwapchains = swapChains;
  presentInfo.pImageIndices = &imageIndex;
  presentInfo.pResults = nullptr;

  result = vkQueuePresentKHR(presentQueue, &presentInfo);

  if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebufferResized)
  {
    framebufferResized = false;
    recreateSwapChain();
  }
  else if (result != VK_SUCCESS)
  {
    throw std::runtime_error("failed to present swap chain image!");
  }

  currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

uint32_t Renderer::getCurrentFrame()
{
  return currentFrame;
}

void Renderer::cleanup()
{
  swapchainManager.cleanupDepthImages(deviceManager.device);
  swapchainManager.cleanupSwapChain(deviceManager.device);

  bufferManager.cleanup(deviceManager.device);

  descriptorManager.cleanup(deviceManager.device);

  pipelineManager.cleanup(deviceManager.device);

  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
  {
    vkDestroySemaphore(deviceManager.device, renderFinishedSemaphores[i], nullptr);
    vkDestroySemaphore(deviceManager.device, imageAvailableSemaphores[i], nullptr);
    vkDestroyFence(deviceManager.device, inFlightFences[i], nullptr);
  }

  vkDestroyCommandPool(deviceManager.device, commandPool, nullptr);
  vkDestroyDevice(deviceManager.device, nullptr);
  vkDestroySurfaceKHR(instance, swapchainManager.surface, nullptr);
  vkDestroyInstance(instance, nullptr);

  glfwDestroyWindow(window);

  glfwTerminate();
}

void Renderer::createInstance()
{
  std::cout << "RE" << std::endl;
  if (enableValidationLayers && !checkValidationLayerSupport())
  {
    throw std::runtime_error("validation layers requested, but not available!");
  }
  std::cout << "RE1" << std::endl;

  VkApplicationInfo appInfo{};
  appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  appInfo.pApplicationName = "Vulkan";
  appInfo.applicationVersion = VK_MAKE_API_VERSION(0, 1, 0, 0);
  appInfo.pEngineName = "No Engine";
  appInfo.engineVersion = VK_MAKE_API_VERSION(0, 1, 0, 0);
  appInfo.apiVersion = VK_API_VERSION_1_3;
  std::cout << "RE2" << std::endl;

  VkInstanceCreateInfo createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  createInfo.pApplicationInfo = &appInfo;
  std::cout << "RE3" << std::endl;

  uint32_t glfwExtensionCount = 0;
  const char **glfwExtensions;
  std::cout << "RE4" << std::endl;

  glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
  std::cout << "RE5" << std::endl;

  createInfo.enabledExtensionCount = glfwExtensionCount;
  createInfo.ppEnabledExtensionNames = glfwExtensions;
  std::cout << "RE6" << std::endl;

  if (enableValidationLayers)
  {
    createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
    createInfo.ppEnabledLayerNames = validationLayers.data();
    std::cout << "RE7" << std::endl;
  }
  else
  {
    createInfo.enabledLayerCount = 0;
  }

  std::cout << "RE8" << std::endl;
  VkResult result = vkCreateInstance(&createInfo, nullptr, &instance);
  if (result != VK_SUCCESS)
  {
    std::cerr << "failed to create instance!" << result << std::endl;
    exit(EXIT_FAILURE);
  }
  std::cout << "RE9" << std::endl;
}

bool Renderer::checkValidationLayerSupport()
{
  std::cout << "GE" << std::endl;
  uint32_t layerCount;
  VkResult result = vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
  if (result != VK_SUCCESS)
  {
    throw std::runtime_error("failed to enumerate instance layers (count)!");
  }
  std::cout << "GE1" << std::endl;

  std::vector<VkLayerProperties> availableLayers(layerCount);
  vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());
  std::cout << "GE2" << std::endl;
  std::cout << "Layer count: " << layerCount << std::endl;
  std::cout << "validationLayers count: " << validationLayers.size() << std::endl;
  for (const char *layerName : validationLayers)
  {
    std::cout << "GE3" << std::endl;
    bool layerFound = false;

    for (const auto &layerProperties : availableLayers)
    {
      std::cout << "GE4" << std::endl;
      if (strcmp(layerName, layerProperties.layerName) == 0)
      {
        std::cout << "GE5" << std::endl;
        layerFound = true;
        break;
      }
    }

    if (!layerFound)
    {
      std::cout << "GE6" << std::endl;
      return false;
    }
  }
  return true;
}