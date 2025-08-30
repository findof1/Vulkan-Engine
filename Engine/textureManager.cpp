#include "textureManager.hpp"
#include "utils.h"
#include <stb_image.h>
#include <string>
#include <stdexcept>
#include "bufferManager.hpp"
#include "renderer.hpp"
#include <noImage.hpp>

void TextureManager::createTextureImageView(VkDevice device)
{
  albedoImageView = createImageView(albedoImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT, device);
  normalImageView = createImageView(normalImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT, device);
  heightImageView = createImageView(heightImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT, device);
  roughnessImageView = createImageView(roughnessImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT, device);
  metallicImageView = createImageView(metallicImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT, device);
  aoImageView = createImageView(aoImage, VK_FORMAT_R8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT, device);
  emissiveImageView = createImageView(emissiveImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT, device);
}

void TextureManager::createTextTextureImageView(VkDevice device)
{
  albedoImageView = createImageView(albedoImage, VK_FORMAT_R8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT, device);
  normalImageView = createImageView(normalImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT, device);
  heightImageView = createImageView(heightImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT, device);
  roughnessImageView = createImageView(roughnessImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT, device);
  metallicImageView = createImageView(metallicImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT, device);
  aoImageView = createImageView(aoImage, VK_FORMAT_R8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT, device);
  emissiveImageView = createImageView(emissiveImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT, device);
}

void TextureManager::createTextureImages(std::string albedoPath, std::string normalPath, std::string heightPath, std::string roughnessPath, std::string metallicPath, std::string aoPath, std::string emissivePath, VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue graphicsQueue)
{
  createTextureImage(albedoPath, albedoImage, albedoImageMemory, VK_FORMAT_R8G8B8A8_SRGB, device, physicalDevice, commandPool, graphicsQueue);
  createTextureImage(normalPath, normalImage, normalImageMemory, VK_FORMAT_R8G8B8A8_UNORM, device, physicalDevice, commandPool, graphicsQueue);
  createTextureImage(heightPath, heightImage, heightImageMemory, VK_FORMAT_R8G8B8A8_UNORM, device, physicalDevice, commandPool, graphicsQueue);
  createTextureImage(roughnessPath, roughnessImage, roughnessImageMemory, VK_FORMAT_R8G8B8A8_UNORM, device, physicalDevice, commandPool, graphicsQueue);
  createTextureImage(metallicPath, metallicImage, metallicImageMemory, VK_FORMAT_R8G8B8A8_UNORM, device, physicalDevice, commandPool, graphicsQueue);
  createTextureImage(aoPath, aoImage, aoImageMemory, VK_FORMAT_R8_UNORM, device, physicalDevice, commandPool, graphicsQueue);
  createTextureImage(emissivePath, emissiveImage, emissiveImageMemory, VK_FORMAT_R8G8B8A8_SRGB, device, physicalDevice, commandPool, graphicsQueue);
}

void TextureManager::createTextureImage(std::string texturePath, VkImage &image, VkDeviceMemory &imageMemory, VkFormat format, VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue graphicsQueue)
{
  int texWidth, texHeight, texChannels;

  stbi_uc *pixels = stbi_load(texturePath.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
  if (!pixels)
  {
    throw std::runtime_error("failed to load texture image! Filepath: " + texturePath);
  }

  VkDeviceSize imageSize = texWidth * texHeight * 4;

  VkBuffer stagingBuffer;
  VkDeviceMemory stagingBufferMemory;
  bufferManager.createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory, device, physicalDevice);

  void *data;
  vkMapMemory(device, stagingBufferMemory, 0, imageSize, 0, &data);
  memcpy(data, pixels, static_cast<size_t>(imageSize));
  vkUnmapMemory(device, stagingBufferMemory);

  stbi_image_free(pixels);

  createImage(texWidth, texHeight, format, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, image, imageMemory, device, physicalDevice);

  transitionImageLayout(image, format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, device, commandPool, graphicsQueue);

  copyBufferToImage(stagingBuffer, image, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight), device, commandPool, graphicsQueue);

  transitionImageLayout(image, format, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, device, commandPool, graphicsQueue);

  vkDestroyBuffer(device, stagingBuffer, nullptr);
  vkFreeMemory(device, stagingBufferMemory, nullptr);
}

void TextureManager::createTextureImage(const FT_Bitmap &bitmap, VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue graphicsQueue)
{
  int texWidth = bitmap.width;
  int texHeight = bitmap.rows;
  VkDeviceSize imageSize = texWidth * texHeight;

  VkBuffer stagingBuffer;
  VkDeviceMemory stagingBufferMemory;
  bufferManager.createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory, device, physicalDevice);

  void *data;
  vkMapMemory(device, stagingBufferMemory, 0, imageSize, 0, &data);
  memcpy(data, bitmap.buffer, static_cast<size_t>(imageSize));
  vkUnmapMemory(device, stagingBufferMemory);

  createImage(texWidth, texHeight, VK_FORMAT_R8_UNORM, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, albedoImage, albedoImageMemory, device, physicalDevice);

  transitionImageLayout(albedoImage, VK_FORMAT_R8_UNORM, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, device, commandPool, graphicsQueue);

  copyBufferToImage(stagingBuffer, albedoImage, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight), device, commandPool, graphicsQueue);

  transitionImageLayout(albedoImage, VK_FORMAT_R8_UNORM, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, device, commandPool, graphicsQueue);

  vkDestroyBuffer(device, stagingBuffer, nullptr);
  vkFreeMemory(device, stagingBufferMemory, nullptr);

  createTextureImage(NO_IMAGE, normalImage, normalImageMemory, VK_FORMAT_R8G8B8A8_UNORM, device, physicalDevice, commandPool, graphicsQueue);
  createTextureImage(NO_IMAGE, heightImage, heightImageMemory, VK_FORMAT_R8G8B8A8_UNORM, device, physicalDevice, commandPool, graphicsQueue);
  createTextureImage(NO_IMAGE, roughnessImage, roughnessImageMemory, VK_FORMAT_R8G8B8A8_UNORM, device, physicalDevice, commandPool, graphicsQueue);
  createTextureImage(NO_IMAGE, metallicImage, metallicImageMemory, VK_FORMAT_R8G8B8A8_UNORM, device, physicalDevice, commandPool, graphicsQueue);
  createTextureImage(NO_IMAGE, aoImage, aoImageMemory, VK_FORMAT_R8_UNORM, device, physicalDevice, commandPool, graphicsQueue);
  createTextureImage(NO_IMAGE, emissiveImage, emissiveImageMemory, VK_FORMAT_R8G8B8A8_SRGB, device, physicalDevice, commandPool, graphicsQueue);
}

void TextureManager::createTextureImage(const std::vector<uint8_t> &textureData, int texWidth, int texHeight, VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue graphicsQueue)
{
  VkDeviceSize imageSize = texWidth * texHeight;

  VkBuffer stagingBuffer;
  VkDeviceMemory stagingBufferMemory;
  bufferManager.createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory, device, physicalDevice);
  void *data;
  vkMapMemory(device, stagingBufferMemory, 0, imageSize, 0, &data);
  memcpy(data, textureData.data(), static_cast<size_t>(imageSize));
  vkUnmapMemory(device, stagingBufferMemory);

  createImage(texWidth, texHeight, VK_FORMAT_R8_UNORM, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, albedoImage, albedoImageMemory, device, physicalDevice);

  transitionImageLayout(albedoImage, VK_FORMAT_R8_UNORM, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, device, commandPool, graphicsQueue);

  copyBufferToImage(stagingBuffer, albedoImage, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight), device, commandPool, graphicsQueue);

  transitionImageLayout(albedoImage, VK_FORMAT_R8_UNORM, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, device, commandPool, graphicsQueue);

  vkDestroyBuffer(device, stagingBuffer, nullptr);
  vkFreeMemory(device, stagingBufferMemory, nullptr);

  createTextureImage(NO_IMAGE, normalImage, normalImageMemory, VK_FORMAT_R8G8B8A8_UNORM, device, physicalDevice, commandPool, graphicsQueue);
  createTextureImage(NO_IMAGE, heightImage, heightImageMemory, VK_FORMAT_R8G8B8A8_UNORM, device, physicalDevice, commandPool, graphicsQueue);
  createTextureImage(NO_IMAGE, roughnessImage, roughnessImageMemory, VK_FORMAT_R8G8B8A8_UNORM, device, physicalDevice, commandPool, graphicsQueue);
  createTextureImage(NO_IMAGE, metallicImage, metallicImageMemory, VK_FORMAT_R8G8B8A8_UNORM, device, physicalDevice, commandPool, graphicsQueue);
  createTextureImage(NO_IMAGE, aoImage, aoImageMemory, VK_FORMAT_R8_UNORM, device, physicalDevice, commandPool, graphicsQueue);
  createTextureImage(NO_IMAGE, emissiveImage, emissiveImageMemory, VK_FORMAT_R8G8B8A8_SRGB, device, physicalDevice, commandPool, graphicsQueue);
}

void TextureManager::updateTexture(std::string newTexturePath, VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue graphicsQueue)
{
  /*
  VkSemaphoreWaitInfo waitInfo{};
  waitInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO;
  waitInfo.semaphoreCount = 1;
  waitInfo.pSemaphores = &renderer.textureUpdateSemaphore;
  uint64_t timeout = UINT64_MAX;
  waitInfo.pValues = &timeout;

  std::cout << "0" << std::endl;
  if (vkWaitSemaphores(device, &waitInfo, UINT64_MAX) != VK_SUCCESS)
  {
    throw std::runtime_error("failed to wait on texture update semaphore!");
  }
*/

  int texWidth, texHeight, texChannels;
  stbi_uc *pixels = stbi_load(newTexturePath.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
  if (!pixels)
  {
    throw std::runtime_error("Failed to load new texture image: " + newTexturePath);
  }

  VkDeviceSize imageSize = texWidth * texHeight * 4;

  VkBuffer stagingBuffer;
  VkDeviceMemory stagingBufferMemory;
  bufferManager.createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                             VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                             stagingBuffer, stagingBufferMemory, device, physicalDevice);

  void *data;
  vkMapMemory(device, stagingBufferMemory, 0, imageSize, 0, &data);
  memcpy(data, pixels, static_cast<size_t>(imageSize));
  vkUnmapMemory(device, stagingBufferMemory);
  stbi_image_free(pixels);

  transitionImageLayout(albedoImage, VK_FORMAT_R8G8B8A8_SRGB,
                        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, device, commandPool, graphicsQueue);

  copyBufferToImage(stagingBuffer, albedoImage, texWidth, texHeight, device, commandPool, graphicsQueue);

  transitionImageLayout(albedoImage, VK_FORMAT_R8G8B8A8_SRGB,
                        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, device, commandPool, graphicsQueue);

  VkSemaphoreSignalInfo signalInfo = {};
  signalInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SIGNAL_INFO;
  // signalInfo.semaphore = renderer.textureUpdateSemaphore;
  signalInfo.value = 0;

  VkResult result = vkSignalSemaphore(device, &signalInfo);

  if (result != VK_SUCCESS)
  {
    throw std::runtime_error("failed to signal texture update semaphore!");
  }

  vkDestroyBuffer(device, stagingBuffer, nullptr);
  vkFreeMemory(device, stagingBufferMemory, nullptr);

  std::cout << "updated texture" << std::endl;
}

void TextureManager::createTextureSampler(VkDevice device, VkPhysicalDevice physicalDevice)
{
  VkSamplerCreateInfo samplerInfo{};
  samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
  samplerInfo.magFilter = VK_FILTER_NEAREST;
  samplerInfo.minFilter = VK_FILTER_NEAREST;
  samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  samplerInfo.anisotropyEnable = VK_TRUE;

  VkPhysicalDeviceProperties properties{};
  vkGetPhysicalDeviceProperties(physicalDevice, &properties);
  samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;

  samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
  samplerInfo.unnormalizedCoordinates = VK_FALSE;
  samplerInfo.compareEnable = VK_FALSE;
  samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
  samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
  samplerInfo.mipLodBias = 0.0f;
  samplerInfo.minLod = 0.0f;
  samplerInfo.maxLod = 0.0f;

  auto createSafeSampler = [&](VkSampler &sampler)
  {
    if (vkCreateSampler(device, &samplerInfo, nullptr, &sampler) != VK_SUCCESS)
    {
      throw std::runtime_error("failed to create texture sampler!");
    }
  };

  if (albedoImageView != VK_NULL_HANDLE)
    createSafeSampler(albedoSampler);
  else
    albedoSampler = VK_NULL_HANDLE;

  if (normalImageView != VK_NULL_HANDLE)
    createSafeSampler(normalSampler);
  else
    normalSampler = VK_NULL_HANDLE;

  if (heightImageView != VK_NULL_HANDLE)
    createSafeSampler(heightSampler);
  else
    heightSampler = VK_NULL_HANDLE;

  if (roughnessImageView != VK_NULL_HANDLE)
    createSafeSampler(roughnessSampler);
  else
    roughnessSampler = VK_NULL_HANDLE;

  if (metallicImageView != VK_NULL_HANDLE)
    createSafeSampler(metallicSampler);
  else
    metallicSampler = VK_NULL_HANDLE;

  if (aoImageView != VK_NULL_HANDLE)
    createSafeSampler(aoSampler);
  else
    aoSampler = VK_NULL_HANDLE;

  if (emissiveImageView != VK_NULL_HANDLE)
    createSafeSampler(emissiveSampler);
  else
    emissiveSampler = VK_NULL_HANDLE;
}

void TextureManager::cleanup(VkDevice device)
{
  if (albedoSampler != VK_NULL_HANDLE)
  {
    vkDestroySampler(device, albedoSampler, nullptr);
    albedoSampler = VK_NULL_HANDLE;
  }

  if (albedoImageView != VK_NULL_HANDLE)
  {
    vkDestroyImageView(device, albedoImageView, nullptr);
    albedoImageView = VK_NULL_HANDLE;
  }

  if (albedoImage != VK_NULL_HANDLE)
  {
    vkDestroyImage(device, albedoImage, nullptr);
    albedoImage = VK_NULL_HANDLE;
  }

  if (albedoImageMemory != VK_NULL_HANDLE)
  {
    vkFreeMemory(device, albedoImageMemory, nullptr);
    albedoImageMemory = VK_NULL_HANDLE;
  }

  if (normalSampler != VK_NULL_HANDLE)
  {
    vkDestroySampler(device, normalSampler, nullptr);
    normalSampler = VK_NULL_HANDLE;
  }

  if (normalImageView != VK_NULL_HANDLE)
  {
    vkDestroyImageView(device, normalImageView, nullptr);
    normalImageView = VK_NULL_HANDLE;
  }

  if (normalImage != VK_NULL_HANDLE)
  {
    vkDestroyImage(device, normalImage, nullptr);
    normalImage = VK_NULL_HANDLE;
  }

  if (normalImageMemory != VK_NULL_HANDLE)
  {
    vkFreeMemory(device, normalImageMemory, nullptr);
    normalImageMemory = VK_NULL_HANDLE;
  }

  if (heightSampler != VK_NULL_HANDLE)
  {
    vkDestroySampler(device, heightSampler, nullptr);
    heightSampler = VK_NULL_HANDLE;
  }

  if (heightImageView != VK_NULL_HANDLE)
  {
    vkDestroyImageView(device, heightImageView, nullptr);
    heightImageView = VK_NULL_HANDLE;
  }

  if (heightImage != VK_NULL_HANDLE)
  {
    vkDestroyImage(device, heightImage, nullptr);
    heightImage = VK_NULL_HANDLE;
  }

  if (heightImageMemory != VK_NULL_HANDLE)
  {
    vkFreeMemory(device, heightImageMemory, nullptr);
    heightImageMemory = VK_NULL_HANDLE;
  }

  if (roughnessSampler != VK_NULL_HANDLE)
  {
    vkDestroySampler(device, roughnessSampler, nullptr);
    roughnessSampler = VK_NULL_HANDLE;
  }

  if (roughnessImageView != VK_NULL_HANDLE)
  {
    vkDestroyImageView(device, roughnessImageView, nullptr);
    roughnessImageView = VK_NULL_HANDLE;
  }

  if (roughnessImage != VK_NULL_HANDLE)
  {
    vkDestroyImage(device, roughnessImage, nullptr);
    roughnessImage = VK_NULL_HANDLE;
  }

  if (roughnessImageMemory != VK_NULL_HANDLE)
  {
    vkFreeMemory(device, roughnessImageMemory, nullptr);
    roughnessImageMemory = VK_NULL_HANDLE;
  }

  if (metallicSampler != VK_NULL_HANDLE)
  {
    vkDestroySampler(device, metallicSampler, nullptr);
    metallicSampler = VK_NULL_HANDLE;
  }

  if (metallicImageView != VK_NULL_HANDLE)
  {
    vkDestroyImageView(device, metallicImageView, nullptr);
    metallicImageView = VK_NULL_HANDLE;
  }

  if (metallicImage != VK_NULL_HANDLE)
  {
    vkDestroyImage(device, metallicImage, nullptr);
    metallicImage = VK_NULL_HANDLE;
  }

  if (metallicImageMemory != VK_NULL_HANDLE)
  {
    vkFreeMemory(device, metallicImageMemory, nullptr);
    metallicImageMemory = VK_NULL_HANDLE;
  }

  if (aoSampler != VK_NULL_HANDLE)
  {
    vkDestroySampler(device, aoSampler, nullptr);
    aoSampler = VK_NULL_HANDLE;
  }

  if (aoImageView != VK_NULL_HANDLE)
  {
    vkDestroyImageView(device, aoImageView, nullptr);
    aoImageView = VK_NULL_HANDLE;
  }

  if (aoImage != VK_NULL_HANDLE)
  {
    vkDestroyImage(device, aoImage, nullptr);
    aoImage = VK_NULL_HANDLE;
  }

  if (aoImageMemory != VK_NULL_HANDLE)
  {
    vkFreeMemory(device, aoImageMemory, nullptr);
    aoImageMemory = VK_NULL_HANDLE;
  }

  if (emissiveSampler != VK_NULL_HANDLE)
  {
    vkDestroySampler(device, emissiveSampler, nullptr);
    emissiveSampler = VK_NULL_HANDLE;
  }

  if (emissiveImageView != VK_NULL_HANDLE)
  {
    vkDestroyImageView(device, emissiveImageView, nullptr);
    emissiveImageView = VK_NULL_HANDLE;
  }

  if (emissiveImage != VK_NULL_HANDLE)
  {
    vkDestroyImage(device, emissiveImage, nullptr);
    emissiveImage = VK_NULL_HANDLE;
  }

  if (emissiveImageMemory != VK_NULL_HANDLE)
  {
    vkFreeMemory(device, emissiveImageMemory, nullptr);
    emissiveImageMemory = VK_NULL_HANDLE;
  }
}