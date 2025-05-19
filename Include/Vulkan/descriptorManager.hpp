#pragma once
#include <vulkan/vulkan.h>
#include <vector>

class BufferManager;
class TextureManager;
class DescriptorManager
{
public:
  VkDescriptorSetLayout descriptorSetLayout;
  VkDescriptorPool descriptorPool;
  std::vector<VkDescriptorSet> descriptorSets;
  VkDescriptorSetLayout computeDescriptorSetLayout;
  std::vector<VkDescriptorSet> computeDescriptorSets;
  BufferManager &bufferManager;
  DescriptorManager(BufferManager &bufferManager) : bufferManager(bufferManager)
  {
  }
  ~DescriptorManager()
  {
  }
  void createDescriptorSetLayout(VkDevice device);
  void createDescriptorPool(VkDevice device, int MAX_FRAMES_IN_FLIGHT, int count);
  void createDescriptorSets(VkDevice device, int MAX_FRAMES_IN_FLIGHT, int count, VkImageView textureImageView, VkSampler textureSampler);
  void addDescriptorSets(VkDevice device, int MAX_FRAMES_IN_FLIGHT, int count, VkImageView textureImageView, VkSampler textureSampler);

  void createComputeDescriptorSets(VkDevice device, int MAX_FRAMES_IN_FLIGHT, int count);
  void addComputeDescriptorSets(VkDevice device, int MAX_FRAMES_IN_FLIGHT, int count);

  void cleanup(VkDevice device);
};