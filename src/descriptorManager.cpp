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

  VkDescriptorSetLayoutBinding samplerLayoutBinding{};
  samplerLayoutBinding.binding = 1;
  samplerLayoutBinding.descriptorCount = 1;
  samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  samplerLayoutBinding.pImmutableSamplers = nullptr;
  samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

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

  std::array<VkDescriptorSetLayoutBinding, 2> bindings = {uboLayoutBinding, samplerLayoutBinding};
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

void DescriptorManager::createDescriptorSets(VkDevice device, int MAX_FRAMES_IN_FLIGHT, int count, VkImageView textureImageView, VkSampler textureSampler)
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

    VkDescriptorImageInfo imageInfo{};
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageInfo.imageView = textureImageView;
    imageInfo.sampler = textureSampler;

    std::array<VkWriteDescriptorSet, 4> descriptorWrites{};

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
    descriptorWrites[1].pImageInfo = &imageInfo;

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

void DescriptorManager::addDescriptorSets(VkDevice device, int MAX_FRAMES_IN_FLIGHT, int count, VkImageView textureImageView, VkSampler textureSampler)
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

    VkDescriptorImageInfo imageInfo{};
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageInfo.imageView = textureImageView;
    imageInfo.sampler = textureSampler;

    std::array<VkWriteDescriptorSet, 2> descriptorWrites{};

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
    descriptorWrites[1].pImageInfo = &imageInfo;

    vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
  }
  descriptorSets.reserve(descriptorSets.size() + newDescriptorSets.size());
  descriptorSets.insert(descriptorSets.end(), newDescriptorSets.begin(), newDescriptorSets.end());
}

void DescriptorManager::cleanup(VkDevice device)
{
  vkDestroyDescriptorPool(device, descriptorPool, nullptr);
  vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);
  vkDestroyDescriptorSetLayout(device, computeDescriptorSetLayout, nullptr);
}