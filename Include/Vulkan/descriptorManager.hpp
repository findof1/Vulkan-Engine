#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include <map>

struct TextureMaps
{
  TextureMaps(VkImageView &albedoImageView,
              VkSampler &albedoSampler,
              VkImageView &normalImageView,
              VkSampler &normalSampler,
              VkImageView &heightImageView,
              VkSampler &heightSampler,
              VkImageView &roughnessImageView,
              VkSampler &roughnessSampler,
              VkImageView &metallicImageView,
              VkSampler &metallicSampler,
              VkImageView &aoImageView,
              VkSampler &aoSampler,
              VkImageView &emissiveImageView,
              VkSampler &emissiveSampler)
      : albedoImageView(albedoImageView), albedoSampler(albedoSampler),
        normalImageView(normalImageView), normalSampler(normalSampler),
        heightImageView(heightImageView), heightSampler(heightSampler),
        roughnessImageView(roughnessImageView), roughnessSampler(roughnessSampler),
        metallicImageView(metallicImageView), metallicSampler(metallicSampler),
        aoImageView(aoImageView), aoSampler(aoSampler),
        emissiveImageView(emissiveImageView), emissiveSampler(emissiveSampler)
  {
  }

  VkImageView &albedoImageView;
  VkSampler &albedoSampler;

  VkImageView &normalImageView;
  VkSampler &normalSampler;

  VkImageView &heightImageView;
  VkSampler &heightSampler;

  VkImageView &roughnessImageView;
  VkSampler &roughnessSampler;

  VkImageView &metallicImageView;
  VkSampler &metallicSampler;

  VkImageView &aoImageView;
  VkSampler &aoSampler;

  VkImageView &emissiveImageView;
  VkSampler &emissiveSampler;
};

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
  void createDescriptorSets(VkDevice device, int MAX_FRAMES_IN_FLIGHT, int count, TextureMaps textureMaps);
  void addDescriptorSets(VkDevice device, int MAX_FRAMES_IN_FLIGHT, int count, TextureMaps textureMaps);
  void addAnimDescriptorSets(VkDevice device, int MAX_FRAMES_IN_FLIGHT, int count, int meshIdStart);
  void createComputeDescriptorSets(VkDevice device, int MAX_FRAMES_IN_FLIGHT, int count);
  void addComputeDescriptorSets(VkDevice device, int MAX_FRAMES_IN_FLIGHT, int count);

  void cleanup(VkDevice device);
};