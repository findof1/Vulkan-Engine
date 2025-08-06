#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include <map>

class BufferManager;
class TextureManager;
class DescriptorManager
{
public:
  VkDescriptorSetLayout descriptorSetLayout;
  VkDescriptorSetLayout animDescriptorSetLayout;

  VkDescriptorPool descriptorPool;
  std::vector<VkDescriptorSet> descriptorSets;
  std::map<int, VkDescriptorSet> animDescriptorSets;
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
  void addAnimDescriptorSets(VkDevice device, int MAX_FRAMES_IN_FLIGHT, int count, int meshIdStart);
  void createComputeDescriptorSets(VkDevice device, int MAX_FRAMES_IN_FLIGHT, int count);
  void addComputeDescriptorSets(VkDevice device, int MAX_FRAMES_IN_FLIGHT, int count);

  void cleanup(VkDevice device);
};