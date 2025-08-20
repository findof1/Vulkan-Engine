#include "descriptorManager.hpp"
#include <array>
#include <stdexcept>
#include "utils.h"
#include "bufferManager.hpp"
#include "textureManager.hpp"

void DescriptorManager::createDescriptorSetLayout(VkDevice device)
{
  VkDescriptorSetLayoutBinding uboLayoutBinding{};
  uboLayoutBinding.binding = 0;
  uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  uboLayoutBinding.descriptorCount = 1;
  uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

  VkDescriptorSetLayoutBinding albedoSamplerLayoutBinding{};
  albedoSamplerLayoutBinding.binding = 1;
  albedoSamplerLayoutBinding.descriptorCount = 1;
  albedoSamplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  albedoSamplerLayoutBinding.pImmutableSamplers = nullptr;
  albedoSamplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

  VkDescriptorSetLayoutBinding normalSamplerLayoutBinding{};
  normalSamplerLayoutBinding.binding = 2;
  normalSamplerLayoutBinding.descriptorCount = 1;
  normalSamplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  normalSamplerLayoutBinding.pImmutableSamplers = nullptr;
  normalSamplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

  VkDescriptorSetLayoutBinding heightSamplerLayoutBinding{};
  heightSamplerLayoutBinding.binding = 3;
  heightSamplerLayoutBinding.descriptorCount = 1;
  heightSamplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  heightSamplerLayoutBinding.pImmutableSamplers = nullptr;
  heightSamplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

  VkDescriptorSetLayoutBinding roughnessSamplerLayoutBinding{};
  roughnessSamplerLayoutBinding.binding = 4;
  roughnessSamplerLayoutBinding.descriptorCount = 1;
  roughnessSamplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  roughnessSamplerLayoutBinding.pImmutableSamplers = nullptr;
  roughnessSamplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

  VkDescriptorSetLayoutBinding metallicSamplerLayoutBinding{};
  metallicSamplerLayoutBinding.binding = 5;
  metallicSamplerLayoutBinding.descriptorCount = 1;
  metallicSamplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  metallicSamplerLayoutBinding.pImmutableSamplers = nullptr;
  metallicSamplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

  VkDescriptorSetLayoutBinding aoSamplerLayoutBinding{};
  aoSamplerLayoutBinding.binding = 6;
  aoSamplerLayoutBinding.descriptorCount = 1;
  aoSamplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  aoSamplerLayoutBinding.pImmutableSamplers = nullptr;
  aoSamplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

  VkDescriptorSetLayoutBinding emissiveSamplerLayoutBinding{};
  emissiveSamplerLayoutBinding.binding = 7;
  emissiveSamplerLayoutBinding.descriptorCount = 1;
  emissiveSamplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  emissiveSamplerLayoutBinding.pImmutableSamplers = nullptr;
  emissiveSamplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

  VkDescriptorSetLayoutBinding computeUboLayoutBinding{};
  computeUboLayoutBinding.binding = 0;
  computeUboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  computeUboLayoutBinding.descriptorCount = 1;
  computeUboLayoutBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

  VkDescriptorSetLayoutBinding computeLayoutBinding1{};
  computeLayoutBinding1.binding = 1;
  computeLayoutBinding1.descriptorCount = 1;
  computeLayoutBinding1.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
  computeLayoutBinding1.pImmutableSamplers = nullptr;
  computeLayoutBinding1.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

  VkDescriptorSetLayoutBinding computeLayoutBinding2{};
  computeLayoutBinding2.binding = 2;
  computeLayoutBinding2.descriptorCount = 1;
  computeLayoutBinding2.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
  computeLayoutBinding2.pImmutableSamplers = nullptr;
  computeLayoutBinding2.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

  std::array<VkDescriptorSetLayoutBinding, 8> bindings = {uboLayoutBinding, albedoSamplerLayoutBinding, normalSamplerLayoutBinding, heightSamplerLayoutBinding, roughnessSamplerLayoutBinding, metallicSamplerLayoutBinding, aoSamplerLayoutBinding, emissiveSamplerLayoutBinding};
  VkDescriptorSetLayoutCreateInfo layoutInfo{};
  layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
  layoutInfo.pBindings = bindings.data();

  if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS)
  {
    throw std::runtime_error("failed to create descriptor set layout!");
  }

  std::array<VkDescriptorSetLayoutBinding, 3> computeBindings = {computeUboLayoutBinding, computeLayoutBinding1, computeLayoutBinding2};
  VkDescriptorSetLayoutCreateInfo computeLayoutInfo{};
  computeLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  computeLayoutInfo.bindingCount = static_cast<uint32_t>(computeBindings.size());
  computeLayoutInfo.pBindings = computeBindings.data();

  if (vkCreateDescriptorSetLayout(device, &computeLayoutInfo, nullptr, &computeDescriptorSetLayout) != VK_SUCCESS)
  {
    throw std::runtime_error("failed to create compute descriptor set layout!");
  }

  VkDescriptorSetLayoutBinding animUboLayoutBinding{};
  animUboLayoutBinding.binding = 0;
  animUboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  animUboLayoutBinding.descriptorCount = 1;
  animUboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
  animUboLayoutBinding.pImmutableSamplers = nullptr;

  VkDescriptorSetLayoutCreateInfo animLayoutInfo{};
  animLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  animLayoutInfo.bindingCount = 1;
  animLayoutInfo.pBindings = &animUboLayoutBinding;

  if (vkCreateDescriptorSetLayout(device, &animLayoutInfo, nullptr, &animDescriptorSetLayout) != VK_SUCCESS)
  {
    throw std::runtime_error("failed to create animation descriptor set layout!");
  }
}

void DescriptorManager::createDescriptorPool(VkDevice device, int MAX_FRAMES_IN_FLIGHT, int count)
{

  std::array<VkDescriptorPoolSize, 3> poolSizes{};
  poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  poolSizes[0].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT * count);
  poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  poolSizes[1].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT * count);
  poolSizes[2].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
  poolSizes[2].descriptorCount = static_cast<uint32_t>(2 * MAX_FRAMES_IN_FLIGHT * count);

  VkDescriptorPoolCreateInfo poolInfo{};
  poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
  poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
  poolInfo.pPoolSizes = poolSizes.data();
  poolInfo.maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT * count);
  poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;

  if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS)
  {
    throw std::runtime_error("failed to create descriptor pool!");
  }
}

void DescriptorManager::createDescriptorSets(VkDevice device, int MAX_FRAMES_IN_FLIGHT, int count, TextureMaps textureMaps)
{
  std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT * count, descriptorSetLayout);
  VkDescriptorSetAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
  allocInfo.descriptorPool = descriptorPool;
  allocInfo.descriptorSetCount = static_cast<uint32_t>(layouts.size());
  allocInfo.pSetLayouts = layouts.data();

  descriptorSets.resize(layouts.size());
  if (vkAllocateDescriptorSets(device, &allocInfo, descriptorSets.data()) != VK_SUCCESS)
  {
    throw std::runtime_error("failed to allocate descriptor sets!");
  }

  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT * count; i++)
  {
    VkDescriptorBufferInfo bufferInfo{};
    bufferInfo.buffer = bufferManager.uniformBuffers[i];
    bufferInfo.offset = 0;
    bufferInfo.range = sizeof(UniformBufferObject);

    VkDescriptorImageInfo albedoInfo{};
    albedoInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    albedoInfo.imageView = textureMaps.albedoImageView;
    albedoInfo.sampler = textureMaps.albedoSampler;

    VkDescriptorImageInfo normalInfo{};
    normalInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    normalInfo.imageView = textureMaps.normalImageView;
    normalInfo.sampler = textureMaps.normalSampler;

    VkDescriptorImageInfo heightInfo{};
    heightInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    heightInfo.imageView = textureMaps.heightImageView;
    heightInfo.sampler = textureMaps.heightSampler;

    VkDescriptorImageInfo roughnessInfo{};
    roughnessInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    roughnessInfo.imageView = textureMaps.roughnessImageView;
    roughnessInfo.sampler = textureMaps.roughnessSampler;

    VkDescriptorImageInfo metallicInfo{};
    metallicInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    metallicInfo.imageView = textureMaps.metallicImageView;
    metallicInfo.sampler = textureMaps.metallicSampler;

    VkDescriptorImageInfo aoInfo{};
    aoInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    aoInfo.imageView = textureMaps.aoImageView;
    aoInfo.sampler = textureMaps.aoSampler;

    VkDescriptorImageInfo emissiveInfo{};
    emissiveInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    emissiveInfo.imageView = textureMaps.emissiveImageView;
    emissiveInfo.sampler = textureMaps.emissiveSampler;

    std::array<VkWriteDescriptorSet, 8> descriptorWrites{};

    descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[0].dstSet = descriptorSets[i];
    descriptorWrites[0].dstBinding = 0;
    descriptorWrites[0].dstArrayElement = 0;
    descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorWrites[0].descriptorCount = 1;
    descriptorWrites[0].pBufferInfo = &bufferInfo;

    descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[1].dstSet = descriptorSets[i];
    descriptorWrites[1].dstBinding = 1;
    descriptorWrites[1].dstArrayElement = 0;
    descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptorWrites[1].descriptorCount = 1;
    descriptorWrites[1].pImageInfo = &albedoInfo;

    descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[2].dstSet = descriptorSets[i];
    descriptorWrites[2].dstBinding = 2;
    descriptorWrites[2].dstArrayElement = 0;
    descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptorWrites[2].descriptorCount = 1;
    descriptorWrites[2].pImageInfo = &normalInfo;

    descriptorWrites[3].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[3].dstSet = descriptorSets[i];
    descriptorWrites[3].dstBinding = 3;
    descriptorWrites[3].dstArrayElement = 0;
    descriptorWrites[3].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptorWrites[3].descriptorCount = 1;
    descriptorWrites[3].pImageInfo = &heightInfo;

    descriptorWrites[4].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[4].dstSet = descriptorSets[i];
    descriptorWrites[4].dstBinding = 4;
    descriptorWrites[4].dstArrayElement = 0;
    descriptorWrites[4].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptorWrites[4].descriptorCount = 1;
    descriptorWrites[4].pImageInfo = &roughnessInfo;

    descriptorWrites[5].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[5].dstSet = descriptorSets[i];
    descriptorWrites[5].dstBinding = 5;
    descriptorWrites[5].dstArrayElement = 0;
    descriptorWrites[5].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptorWrites[5].descriptorCount = 1;
    descriptorWrites[5].pImageInfo = &metallicInfo;

    descriptorWrites[6].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[6].dstSet = descriptorSets[i];
    descriptorWrites[6].dstBinding = 6;
    descriptorWrites[6].dstArrayElement = 0;
    descriptorWrites[6].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptorWrites[6].descriptorCount = 1;
    descriptorWrites[6].pImageInfo = &aoInfo;

    descriptorWrites[7].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[7].dstSet = descriptorSets[i];
    descriptorWrites[7].dstBinding = 7;
    descriptorWrites[7].dstArrayElement = 0;
    descriptorWrites[7].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptorWrites[7].descriptorCount = 1;
    descriptorWrites[7].pImageInfo = &emissiveInfo;

    vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
  }
}

void DescriptorManager::createComputeDescriptorSets(VkDevice device, int MAX_FRAMES_IN_FLIGHT, int count)
{
  std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT * count, computeDescriptorSetLayout);
  VkDescriptorSetAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
  allocInfo.descriptorPool = descriptorPool;
  allocInfo.descriptorSetCount = static_cast<uint32_t>(layouts.size());
  allocInfo.pSetLayouts = layouts.data();

  computeDescriptorSets.resize(layouts.size());
  if (vkAllocateDescriptorSets(device, &allocInfo, computeDescriptorSets.data()) != VK_SUCCESS)
  {
    throw std::runtime_error("failed to allocate compute descriptor sets!");
  }

  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT * count; i++)
  {
    VkDescriptorBufferInfo storageBufferInfoLastFrame{};
    storageBufferInfoLastFrame.buffer = bufferManager.shaderStorageBuffers[(i + MAX_FRAMES_IN_FLIGHT - 1) % MAX_FRAMES_IN_FLIGHT];
    storageBufferInfoLastFrame.offset = 0;
    storageBufferInfoLastFrame.range = VK_WHOLE_SIZE;

    VkDescriptorBufferInfo storageBufferInfoCurrentFrame{};
    storageBufferInfoCurrentFrame.buffer = bufferManager.shaderStorageBuffers[i % MAX_FRAMES_IN_FLIGHT];
    storageBufferInfoCurrentFrame.offset = 0;
    storageBufferInfoCurrentFrame.range = VK_WHOLE_SIZE;

    VkDescriptorBufferInfo bufferInfo{};
    bufferInfo.buffer = bufferManager.computeUniformBuffers[i];
    bufferInfo.offset = 0;
    bufferInfo.range = sizeof(ComputeUniformBufferObject);

    std::array<VkWriteDescriptorSet, 3> descriptorWrites{};

    descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[0].dstSet = computeDescriptorSets[i];
    descriptorWrites[0].dstBinding = 0;
    descriptorWrites[0].dstArrayElement = 0;
    descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorWrites[0].descriptorCount = 1;
    descriptorWrites[0].pBufferInfo = &bufferInfo;

    descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[1].dstSet = computeDescriptorSets[i];
    descriptorWrites[1].dstBinding = 1;
    descriptorWrites[1].dstArrayElement = 0;
    descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    descriptorWrites[1].descriptorCount = 1;
    descriptorWrites[1].pBufferInfo = &storageBufferInfoLastFrame;

    descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[2].dstSet = computeDescriptorSets[i];
    descriptorWrites[2].dstBinding = 2;
    descriptorWrites[2].dstArrayElement = 0;
    descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    descriptorWrites[2].descriptorCount = 1;
    descriptorWrites[2].pBufferInfo = &storageBufferInfoCurrentFrame;

    vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
  }
}

void DescriptorManager::addComputeDescriptorSets(VkDevice device, int MAX_FRAMES_IN_FLIGHT, int count)
{
  std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT * count, computeDescriptorSetLayout);
  VkDescriptorSetAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
  allocInfo.descriptorPool = descriptorPool;
  allocInfo.descriptorSetCount = static_cast<uint32_t>(layouts.size());
  allocInfo.pSetLayouts = layouts.data();

  std::vector<VkDescriptorSet> newComputeDescriptorSets;
  newComputeDescriptorSets.resize(layouts.size());
  size_t previousSetsSize = computeDescriptorSets.size();
  if (vkAllocateDescriptorSets(device, &allocInfo, newComputeDescriptorSets.data()) != VK_SUCCESS)
  {
    throw std::runtime_error("failed to allocate added compute descriptor sets!");
  }

  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT * count; i++)
  {
    uint32_t frameIndex = (i + previousSetsSize) % MAX_FRAMES_IN_FLIGHT;

    VkDescriptorBufferInfo storageBufferInfoLastFrame{};
    storageBufferInfoLastFrame.buffer = bufferManager.shaderStorageBuffers[(frameIndex + 1) % 2];
    storageBufferInfoLastFrame.offset = 0;
    storageBufferInfoLastFrame.range = VK_WHOLE_SIZE;

    VkDescriptorBufferInfo storageBufferInfoCurrentFrame{};
    storageBufferInfoCurrentFrame.buffer = bufferManager.shaderStorageBuffers[frameIndex % 2];
    storageBufferInfoCurrentFrame.offset = 0;
    storageBufferInfoCurrentFrame.range = VK_WHOLE_SIZE;

    VkDescriptorBufferInfo bufferInfo{};
    bufferInfo.buffer = bufferManager.computeUniformBuffers[i + previousSetsSize];
    bufferInfo.offset = 0;
    bufferInfo.range = sizeof(ComputeUniformBufferObject);

    std::array<VkWriteDescriptorSet, 3> descriptorWrites{};

    descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[0].dstSet = newComputeDescriptorSets[i];
    descriptorWrites[0].dstBinding = 0;
    descriptorWrites[0].dstArrayElement = 0;
    descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorWrites[0].descriptorCount = 1;
    descriptorWrites[0].pBufferInfo = &bufferInfo;

    descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[1].dstSet = newComputeDescriptorSets[i];
    descriptorWrites[1].dstBinding = 1;
    descriptorWrites[1].dstArrayElement = 0;
    descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    descriptorWrites[1].descriptorCount = 1;
    descriptorWrites[1].pBufferInfo = &storageBufferInfoLastFrame;

    descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[2].dstSet = newComputeDescriptorSets[i];
    descriptorWrites[2].dstBinding = 2;
    descriptorWrites[2].dstArrayElement = 0;
    descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    descriptorWrites[2].descriptorCount = 1;
    descriptorWrites[2].pBufferInfo = &storageBufferInfoCurrentFrame;

    vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
  }
  computeDescriptorSets.reserve(computeDescriptorSets.size() + newComputeDescriptorSets.size());
  computeDescriptorSets.insert(computeDescriptorSets.end(), newComputeDescriptorSets.begin(), newComputeDescriptorSets.end());
}

void DescriptorManager::addDescriptorSets(VkDevice device, int MAX_FRAMES_IN_FLIGHT, int count, TextureMaps textureMaps)
{
  std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT * count, descriptorSetLayout);
  VkDescriptorSetAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
  allocInfo.descriptorPool = descriptorPool;
  allocInfo.descriptorSetCount = static_cast<uint32_t>(layouts.size());
  allocInfo.pSetLayouts = layouts.data();

  std::vector<VkDescriptorSet> newDescriptorSets;
  newDescriptorSets.resize(layouts.size());
  size_t previousSetsSize = descriptorSets.size();
  if (vkAllocateDescriptorSets(device, &allocInfo, newDescriptorSets.data()) != VK_SUCCESS)
  {
    throw std::runtime_error("failed to allocate added descriptor sets!");
  }

  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT * count; i++)
  {
    VkDescriptorBufferInfo bufferInfo{};
    bufferInfo.buffer = bufferManager.uniformBuffers[i + previousSetsSize];
    bufferInfo.offset = 0;
    bufferInfo.range = sizeof(UniformBufferObject);

    VkDescriptorImageInfo albedoInfo{};
    albedoInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    albedoInfo.imageView = textureMaps.albedoImageView;
    albedoInfo.sampler = textureMaps.albedoSampler;

    VkDescriptorImageInfo normalInfo{};
    normalInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    normalInfo.imageView = textureMaps.normalImageView;
    normalInfo.sampler = textureMaps.normalSampler;

    VkDescriptorImageInfo heightInfo{};
    heightInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    heightInfo.imageView = textureMaps.heightImageView;
    heightInfo.sampler = textureMaps.heightSampler;

    VkDescriptorImageInfo roughnessInfo{};
    roughnessInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    roughnessInfo.imageView = textureMaps.roughnessImageView;
    roughnessInfo.sampler = textureMaps.roughnessSampler;

    VkDescriptorImageInfo metallicInfo{};
    metallicInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    metallicInfo.imageView = textureMaps.metallicImageView;
    metallicInfo.sampler = textureMaps.metallicSampler;

    VkDescriptorImageInfo aoInfo{};
    aoInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    aoInfo.imageView = textureMaps.aoImageView;
    aoInfo.sampler = textureMaps.aoSampler;

    VkDescriptorImageInfo emissiveInfo{};
    emissiveInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    emissiveInfo.imageView = textureMaps.emissiveImageView;
    emissiveInfo.sampler = textureMaps.emissiveSampler;

    std::array<VkWriteDescriptorSet, 8> descriptorWrites{};

    descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[0].dstSet = newDescriptorSets[i];
    descriptorWrites[0].dstBinding = 0;
    descriptorWrites[0].dstArrayElement = 0;
    descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorWrites[0].descriptorCount = 1;
    descriptorWrites[0].pBufferInfo = &bufferInfo;

    descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[1].dstSet = newDescriptorSets[i];
    descriptorWrites[1].dstBinding = 1;
    descriptorWrites[1].dstArrayElement = 0;
    descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptorWrites[1].descriptorCount = 1;
    descriptorWrites[1].pImageInfo = &albedoInfo;

    descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[2].dstSet = newDescriptorSets[i];
    descriptorWrites[2].dstBinding = 2;
    descriptorWrites[2].dstArrayElement = 0;
    descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptorWrites[2].descriptorCount = 1;
    descriptorWrites[2].pImageInfo = &normalInfo;

    descriptorWrites[3].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[3].dstSet = newDescriptorSets[i];
    descriptorWrites[3].dstBinding = 3;
    descriptorWrites[3].dstArrayElement = 0;
    descriptorWrites[3].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptorWrites[3].descriptorCount = 1;
    descriptorWrites[3].pImageInfo = &heightInfo;

    descriptorWrites[4].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[4].dstSet = newDescriptorSets[i];
    descriptorWrites[4].dstBinding = 4;
    descriptorWrites[4].dstArrayElement = 0;
    descriptorWrites[4].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptorWrites[4].descriptorCount = 1;
    descriptorWrites[4].pImageInfo = &roughnessInfo;

    descriptorWrites[5].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[5].dstSet = newDescriptorSets[i];
    descriptorWrites[5].dstBinding = 5;
    descriptorWrites[5].dstArrayElement = 0;
    descriptorWrites[5].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptorWrites[5].descriptorCount = 1;
    descriptorWrites[5].pImageInfo = &metallicInfo;

    descriptorWrites[6].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[6].dstSet = newDescriptorSets[i];
    descriptorWrites[6].dstBinding = 6;
    descriptorWrites[6].dstArrayElement = 0;
    descriptorWrites[6].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptorWrites[6].descriptorCount = 1;
    descriptorWrites[6].pImageInfo = &aoInfo;

    descriptorWrites[7].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[7].dstSet = newDescriptorSets[i];
    descriptorWrites[7].dstBinding = 7;
    descriptorWrites[7].dstArrayElement = 0;
    descriptorWrites[7].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptorWrites[7].descriptorCount = 1;
    descriptorWrites[7].pImageInfo = &emissiveInfo;

    vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
  }
  descriptorSets.reserve(descriptorSets.size() + newDescriptorSets.size());
  descriptorSets.insert(descriptorSets.end(), newDescriptorSets.begin(), newDescriptorSets.end());
}

void DescriptorManager::addAnimDescriptorSets(VkDevice device, int MAX_FRAMES_IN_FLIGHT, int count, int meshIdStart)
{
  std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT * count, animDescriptorSetLayout);

  VkDescriptorSetAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
  allocInfo.descriptorPool = descriptorPool;
  allocInfo.descriptorSetCount = static_cast<uint32_t>(layouts.size());
  allocInfo.pSetLayouts = layouts.data();

  std::vector<VkDescriptorSet> newDescriptorSets(layouts.size());

  if (vkAllocateDescriptorSets(device, &allocInfo, newDescriptorSets.data()) != VK_SUCCESS)
  {
    throw std::runtime_error("failed to allocate animation descriptor sets!");
  }

  for (int objectIdx = 0; objectIdx < count; objectIdx++)
  {
    for (int frame = 0; frame < MAX_FRAMES_IN_FLIGHT; frame++)
    {
      int globalIndex = (meshIdStart + objectIdx) * MAX_FRAMES_IN_FLIGHT + frame;
      int localIndex = (objectIdx)*MAX_FRAMES_IN_FLIGHT + frame;

      VkDescriptorBufferInfo bufferInfo{};
      bufferInfo.buffer = bufferManager.animatedUniformBuffers[globalIndex];
      bufferInfo.offset = 0;
      bufferInfo.range = sizeof(AnimatedUniformBufferObject);

      std::array<VkWriteDescriptorSet, 1> descriptorWrites{};

      descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
      descriptorWrites[0].dstSet = newDescriptorSets[localIndex];
      descriptorWrites[0].dstBinding = 0;
      descriptorWrites[0].dstArrayElement = 0;
      descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
      descriptorWrites[0].descriptorCount = 1;
      descriptorWrites[0].pBufferInfo = &bufferInfo;

      vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);

      int descriptorKey = (meshIdStart + objectIdx) * MAX_FRAMES_IN_FLIGHT + frame;
      animDescriptorSets[descriptorKey] = newDescriptorSets[localIndex];
    }
  }
}

void DescriptorManager::cleanup(VkDevice device)
{
  vkDestroyDescriptorPool(device, descriptorPool, nullptr);
  vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);
  vkDestroyDescriptorSetLayout(device, computeDescriptorSetLayout, nullptr);
}