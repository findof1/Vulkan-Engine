#include "bufferManager.hpp"
#include "utils.h"
#include <stdexcept>
#include <chrono>

void BufferManager::createComputeUniformBuffers(int MAX_FRAMES_IN_FLIGHT, VkDevice device, VkPhysicalDevice physicalDevice, int count)
{
  VkDeviceSize bufferSize = sizeof(ComputeUniformBufferObject);

  computeUniformBuffers.resize(computeUniformBuffers.size() + MAX_FRAMES_IN_FLIGHT * count);
  computeUniformBuffersMemory.resize(computeUniformBuffersMemory.size() + MAX_FRAMES_IN_FLIGHT * count);
  computeUniformBuffersMapped.resize(computeUniformBuffersMapped.size() + MAX_FRAMES_IN_FLIGHT * count);

  for (size_t i = computeUniformBuffers.size() - MAX_FRAMES_IN_FLIGHT * count; i < computeUniformBuffers.size(); i++)
  {
    createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, computeUniformBuffers[i], computeUniformBuffersMemory[i], device, physicalDevice);

    vkMapMemory(device, computeUniformBuffersMemory[i], 0, bufferSize, 0, &computeUniformBuffersMapped[i]);
  }
}

void BufferManager::createUniformBuffers(int MAX_FRAMES_IN_FLIGHT, VkDevice device, VkPhysicalDevice physicalDevice, int count)
{
  VkDeviceSize bufferSize = sizeof(UniformBufferObject);

  uniformBuffers.resize(uniformBuffers.size() + MAX_FRAMES_IN_FLIGHT * count);
  uniformBuffersMemory.resize(uniformBuffersMemory.size() + MAX_FRAMES_IN_FLIGHT * count);
  uniformBuffersMapped.resize(uniformBuffersMapped.size() + MAX_FRAMES_IN_FLIGHT * count);

  for (size_t i = uniformBuffers.size() - MAX_FRAMES_IN_FLIGHT * count; i < uniformBuffers.size(); i++)
  {
    createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, uniformBuffers[i], uniformBuffersMemory[i], device, physicalDevice);

    vkMapMemory(device, uniformBuffersMemory[i], 0, bufferSize, 0, &uniformBuffersMapped[i]);
  }
}

void BufferManager::createAnimationUniformBuffers(int MAX_FRAMES_IN_FLIGHT, VkDevice device, VkPhysicalDevice physicalDevice, int count, int startingId)
{
  VkDeviceSize bufferSize = sizeof(AnimatedUniformBufferObject);

  for (size_t i = 0; i < count; i++)
  {
    for (int frame = 0; frame < MAX_FRAMES_IN_FLIGHT; frame++)
    {
      int id = (startingId + i) * MAX_FRAMES_IN_FLIGHT + frame;

      VkBuffer buffer;
      VkDeviceMemory memory;
      void *mapped;

      createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, buffer, memory, device, physicalDevice);

      vkMapMemory(device, memory, 0, bufferSize, 0, &mapped);

      animatedUniformBuffers[id] = buffer;
      animatedUniformBuffersMemory[id] = memory;
      animatedUniformBuffersMapped[id] = mapped;
    }
  }
}

void BufferManager::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer &buffer, VkDeviceMemory &bufferMemory, VkDevice device, VkPhysicalDevice physicalDevice)
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

void BufferManager::createShaderStorageBuffer(const std::vector<Particle> &particles, int MAX_FRAMES_IN_FLIGHT, int targetBuffer, VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue graphicsQueue)
{
  if (particles.empty())
    return;

  if (shaderStorageBuffers.size() <= MAX_FRAMES_IN_FLIGHT * (targetBuffer + 1))
  {
    shaderStorageBuffers.resize(MAX_FRAMES_IN_FLIGHT * (targetBuffer + 1), VK_NULL_HANDLE);
    shaderStorageBuffersMemory.resize(MAX_FRAMES_IN_FLIGHT * (targetBuffer + 1), VK_NULL_HANDLE);
  }

  VkDeviceSize bufferSize = sizeof(particles[0]) * particles.size();
  VkBuffer stagingBuffer;
  VkDeviceMemory stagingBufferMemory;
  createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory, device, physicalDevice);

  void *data;
  vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
  memcpy(data, particles.data(), (size_t)bufferSize);
  vkUnmapMemory(device, stagingBufferMemory);
  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
  {
    createBuffer(bufferSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, shaderStorageBuffers[MAX_FRAMES_IN_FLIGHT * targetBuffer + i], shaderStorageBuffersMemory[MAX_FRAMES_IN_FLIGHT * targetBuffer + i], device, physicalDevice);

    copyBuffer(stagingBuffer, shaderStorageBuffers[MAX_FRAMES_IN_FLIGHT * targetBuffer + i], bufferSize, device, commandPool, graphicsQueue);
  }
  vkDestroyBuffer(device, stagingBuffer, nullptr);
  vkFreeMemory(device, stagingBufferMemory, nullptr);
}

void BufferManager::createIndexBuffer(const std::vector<uint32_t> &inputIndices, int targetBuffer, VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue graphicsQueue)
{
  if (indexBuffers.size() <= targetBuffer)
  {
    indexBuffers.resize(targetBuffer + 1, VK_NULL_HANDLE);
    indexBufferMemory.resize(targetBuffer + 1, VK_NULL_HANDLE);
  }

  VkDeviceSize bufferSize = sizeof(inputIndices[0]) * inputIndices.size();
  VkBuffer stagingBuffer;
  VkDeviceMemory stagingBufferMemory;
  createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory, device, physicalDevice);

  void *data;
  vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
  memcpy(data, inputIndices.data(), (size_t)bufferSize);
  vkUnmapMemory(device, stagingBufferMemory);

  createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, indexBuffers[targetBuffer], indexBufferMemory[targetBuffer], device, physicalDevice);

  copyBuffer(stagingBuffer, indexBuffers[targetBuffer], bufferSize, device, commandPool, graphicsQueue);

  vkDestroyBuffer(device, stagingBuffer, nullptr);
  vkFreeMemory(device, stagingBufferMemory, nullptr);
}

void BufferManager::createVertexBuffer(const std::vector<Vertex> &verts, int targetBuffer, VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue graphicsQueue)
{
  VkDeviceSize bufferSize = sizeof(verts[0]) * verts.size();

  if (bufferSize == 0)
  {
    return;
  }
  if (vertexBuffers.size() <= targetBuffer)
  {
    vertexBuffers.resize(targetBuffer + 1, VK_NULL_HANDLE);
    vertexBufferMemory.resize(targetBuffer + 1, VK_NULL_HANDLE);
    vertexBufferSizes.resize(targetBuffer + 1, 0);
  }

  if (vertexBuffers[targetBuffer] != VK_NULL_HANDLE)
  {
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory, device, physicalDevice);

    void *data;
    vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, verts.data(), (size_t)bufferSize);
    vkUnmapMemory(device, stagingBufferMemory);

    if (bufferSize > vertexBufferSizes[targetBuffer])
    {
      if (vertexBuffers[targetBuffer] != VK_NULL_HANDLE)
      {
        vkQueueWaitIdle(graphicsQueue);
        vkDestroyBuffer(device, vertexBuffers[targetBuffer], nullptr);
      }
      if (vertexBufferMemory[targetBuffer] != VK_NULL_HANDLE)
      {
        vkFreeMemory(device, vertexBufferMemory[targetBuffer], nullptr);
      }

      createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, vertexBuffers[targetBuffer], vertexBufferMemory[targetBuffer], device, physicalDevice);

      vertexBufferSizes[targetBuffer] = bufferSize;
    }

    copyBuffer(stagingBuffer, vertexBuffers[targetBuffer], bufferSize, device, commandPool, graphicsQueue);

    vkDestroyBuffer(device, stagingBuffer, nullptr);
    vkFreeMemory(device, stagingBufferMemory, nullptr);

    return;
  }

  vertexBufferSizes[targetBuffer] = bufferSize;
  VkBuffer stagingBuffer;
  VkDeviceMemory stagingBufferMemory;
  createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory, device, physicalDevice);

  void *data;
  vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
  memcpy(data, verts.data(), (size_t)bufferSize);
  vkUnmapMemory(device, stagingBufferMemory);

  createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, vertexBuffers[targetBuffer], vertexBufferMemory[targetBuffer], device, physicalDevice);

  copyBuffer(stagingBuffer, vertexBuffers[targetBuffer], bufferSize, device, commandPool, graphicsQueue);

  vkDestroyBuffer(device, stagingBuffer, nullptr);
  vkFreeMemory(device, stagingBufferMemory, nullptr);
}

void BufferManager::createAnimatedVertexBuffer(const std::vector<AnimatedVertex> &verts, int targetBuffer, VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue graphicsQueue)
{
  VkDeviceSize bufferSize = sizeof(verts[0]) * verts.size();

  if (bufferSize == 0)
  {
    return;
  }
  if (vertexBuffers.size() <= targetBuffer)
  {
    vertexBuffers.resize(targetBuffer + 1, VK_NULL_HANDLE);
    vertexBufferMemory.resize(targetBuffer + 1, VK_NULL_HANDLE);
    vertexBufferSizes.resize(targetBuffer + 1, 0);
  }

  if (vertexBuffers[targetBuffer] != VK_NULL_HANDLE)
  {
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory, device, physicalDevice);

    void *data;
    vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, verts.data(), (size_t)bufferSize);
    vkUnmapMemory(device, stagingBufferMemory);

    if (bufferSize > vertexBufferSizes[targetBuffer])
    {
      if (vertexBuffers[targetBuffer] != VK_NULL_HANDLE)
      {
        vkQueueWaitIdle(graphicsQueue);
        vkDestroyBuffer(device, vertexBuffers[targetBuffer], nullptr);
      }
      if (vertexBufferMemory[targetBuffer] != VK_NULL_HANDLE)
      {
        vkFreeMemory(device, vertexBufferMemory[targetBuffer], nullptr);
      }

      createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, vertexBuffers[targetBuffer], vertexBufferMemory[targetBuffer], device, physicalDevice);

      vertexBufferSizes[targetBuffer] = bufferSize;
    }

    copyBuffer(stagingBuffer, vertexBuffers[targetBuffer], bufferSize, device, commandPool, graphicsQueue);

    vkDestroyBuffer(device, stagingBuffer, nullptr);
    vkFreeMemory(device, stagingBufferMemory, nullptr);

    return;
  }

  vertexBufferSizes[targetBuffer] = bufferSize;
  VkBuffer stagingBuffer;
  VkDeviceMemory stagingBufferMemory;
  createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory, device, physicalDevice);

  void *data;
  vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
  memcpy(data, verts.data(), (size_t)bufferSize);
  vkUnmapMemory(device, stagingBufferMemory);

  createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, vertexBuffers[targetBuffer], vertexBufferMemory[targetBuffer], device, physicalDevice);

  copyBuffer(stagingBuffer, vertexBuffers[targetBuffer], bufferSize, device, commandPool, graphicsQueue);

  vkDestroyBuffer(device, stagingBuffer, nullptr);
  vkFreeMemory(device, stagingBufferMemory, nullptr);
}

void BufferManager::freeVertexBuffer(int index, VkDevice device, VkQueue graphicsQueue)
{
  vkQueueWaitIdle(graphicsQueue);
  if (index < 0 || index >= vertexBuffers.size())
  {
    return;
  }

  if (vertexBuffers[index] != VK_NULL_HANDLE)
  {
    vkDestroyBuffer(device, vertexBuffers[index], nullptr);
  }

  if (vertexBufferMemory[index] != VK_NULL_HANDLE)
  {
    vkFreeMemory(device, vertexBufferMemory[index], nullptr);
  }

  vertexBuffers.erase(vertexBuffers.begin() + index);
  vertexBufferMemory.erase(vertexBufferMemory.begin() + index);
}

void BufferManager::freeIndexBuffer(int index, VkDevice device, VkQueue graphicsQueue)
{
  vkQueueWaitIdle(graphicsQueue);
  if (index < 0 || index >= indexBuffers.size())
  {
    return;
  }

  if (indexBuffers[index] != VK_NULL_HANDLE)
  {
    vkDestroyBuffer(device, indexBuffers[index], nullptr);
  }

  if (indexBufferMemory[index] != VK_NULL_HANDLE)
  {
    vkFreeMemory(device, indexBufferMemory[index], nullptr);
  }

  indexBuffers.erase(indexBuffers.begin() + index);
  indexBufferMemory.erase(indexBufferMemory.begin() + index);
}

void BufferManager::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size, VkDevice device, VkCommandPool commandPool, VkQueue graphicsQueue)
{
  VkCommandBuffer commandBuffer = beginSingleTimeCommands(device, commandPool);

  VkBufferCopy copyRegion{};
  copyRegion.size = size;
  vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

  endSingleTimeCommands(commandBuffer, device, commandPool, graphicsQueue);
}

void BufferManager::cleanup(VkDevice device)
{
  for (auto &uniformBuffer : uniformBuffers)
  {
    vkDestroyBuffer(device, uniformBuffer, nullptr);
  }

  for (auto &uniformBufferMemory : uniformBuffersMemory)
  {
    vkFreeMemory(device, uniformBufferMemory, nullptr);
  }

  for (auto &indexBuffer : indexBuffers)
  {
    if (indexBuffer != VK_NULL_HANDLE)
    {
      vkDestroyBuffer(device, indexBuffer, nullptr);
      indexBuffer = VK_NULL_HANDLE;
    }
  }
  for (auto &indexBufferMem : indexBufferMemory)
  {
    if (indexBufferMem != VK_NULL_HANDLE)
    {
      vkFreeMemory(device, indexBufferMem, nullptr);
      indexBufferMem = VK_NULL_HANDLE;
    }
  }

  for (auto &vertexBuffer : vertexBuffers)
  {
    if (vertexBuffer != VK_NULL_HANDLE)
    {
      vkDestroyBuffer(device, vertexBuffer, nullptr);
      vertexBuffer = VK_NULL_HANDLE;
    }
  }
  for (auto &vertexBufferMem : vertexBufferMemory)
  {
    if (vertexBufferMem != VK_NULL_HANDLE)
    {
      vkFreeMemory(device, vertexBufferMem, nullptr);
      vertexBufferMem = VK_NULL_HANDLE;
    }
  }
}

void BufferManager::updateUniformBuffer(uint32_t currentImage, glm::mat4 transformation, glm::mat4 view, glm::mat4 proj)
{
  UniformBufferObject ubo{};

  ubo.model = transformation;
  ubo.view = view;

  ubo.proj = proj;

  ubo.proj[1][1] *= -1;

  memcpy(uniformBuffersMapped[currentImage], &ubo, sizeof(ubo));
}

void BufferManager::updateAnimationUniformBuffer(uint32_t currentImage, std::array<glm::mat4, 100> &boneMatrices)
{
  AnimatedUniformBufferObject ubo{};

  std::copy(boneMatrices.begin(), boneMatrices.end(), std::begin(ubo.boneMatrices));

  memcpy(animatedUniformBuffersMapped.at(currentImage), &ubo, sizeof(ubo));
}

void BufferManager::updateComputeUniformBuffer(uint32_t currentImage, float deltaTime)
{
  ComputeUniformBufferObject ubo{};

  ubo.deltaTime = deltaTime;

  memcpy(computeUniformBuffersMapped[currentImage], &ubo, sizeof(ubo));
}