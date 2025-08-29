#pragma once
#ifndef UTILS_H
#define UTILS_H
#include <vulkan/vulkan.h>
#include <optional>
#include <glm/glm.hpp>
#include <vector>
#define MAX_LIGHTS 100

#ifdef BUILD_ENGINE_DLL

#ifndef ENGINE_API
#define ENGINE_API __declspec(dllexport)
#endif

#else

#ifndef ENGINE_API
#define ENGINE_API __declspec(dllimport)
#endif

#endif

struct ENGINE_API QueueFamilyIndices
{
  std::optional<uint32_t> graphicsFamily;
  std::optional<uint32_t> presentFamily;
  std::optional<uint32_t> computeFamily;

  bool isComplete()
  {
    return graphicsFamily.has_value() && presentFamily.has_value() && computeFamily.has_value();
  }
};

struct ENGINE_API SwapChainSupportDetails
{
  VkSurfaceCapabilitiesKHR capabilities;
  std::vector<VkSurfaceFormatKHR> formats;
  std::vector<VkPresentModeKHR> presentModes;
};

struct ENGINE_API ComputeUniformBufferObject
{
  alignas(16) float deltaTime;
};

struct ENGINE_API UniformBufferObject
{
  alignas(16) glm::mat4 model;
  alignas(16) glm::mat4 view;
  alignas(16) glm::mat4 proj;
};

struct ENGINE_API Light
{
  alignas(16) glm::vec3 position;
  alignas(16) glm::vec3 color;
  float intensity;
};

struct ENGINE_API LightsUBO
{
  glm::vec3 cameraPos;
  int lightsCount;
  Light lights[MAX_LIGHTS];
};

struct ENGINE_API AnimatedUniformBufferObject
{
  alignas(16) glm::mat4 boneMatrices[100];
};

ENGINE_API QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface);
ENGINE_API VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, VkDevice device);

ENGINE_API uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties, VkPhysicalDevice physicalDevice);

ENGINE_API VkCommandBuffer beginSingleTimeCommands(VkDevice device, VkCommandPool commandPool);
ENGINE_API void endSingleTimeCommands(VkCommandBuffer commandBuffer, VkDevice device, VkCommandPool commandPool, VkQueue graphicsQueue);
ENGINE_API void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, VkDevice device, VkCommandPool commandPool, VkQueue graphicsQueue);
ENGINE_API void createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage &image, VkDeviceMemory &imageMemory, VkDevice device, VkPhysicalDevice physicalDevice);
ENGINE_API void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, VkDevice device, VkCommandPool commandPool, VkQueue graphicsQueue);
ENGINE_API VkFormat findSupportedFormat(const std::vector<VkFormat> &candidates, VkImageTiling tiling, VkFormatFeatureFlags features, VkPhysicalDevice physicalDevice);

#endif