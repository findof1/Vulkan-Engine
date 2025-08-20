#pragma once
#include <vulkan/vulkan.h>
#include <memory>
#include <vector>
class BufferManager;
class Renderer;
struct FT_Bitmap_;
typedef FT_Bitmap_ FT_Bitmap;
class TextureManager
{
public:
  VkImage albedoImage;
  VkDeviceMemory albedoImageMemory;
  VkImageView albedoImageView;
  VkSampler albedoSampler;

  VkImage normalImage;
  VkDeviceMemory normalImageMemory;
  VkImageView normalImageView;
  VkSampler normalSampler;

  VkImage heightImage;
  VkDeviceMemory heightImageMemory;
  VkImageView heightImageView;
  VkSampler heightSampler;

  VkImage roughnessImage;
  VkDeviceMemory roughnessImageMemory;
  VkImageView roughnessImageView;
  VkSampler roughnessSampler;

  VkImage metallicImage;
  VkDeviceMemory metallicImageMemory;
  VkImageView metallicImageView;
  VkSampler metallicSampler;

  VkImage aoImage;
  VkDeviceMemory aoImageMemory;
  VkImageView aoImageView;
  VkSampler aoSampler;

  VkImage emissiveImage;
  VkDeviceMemory emissiveImageMemory;
  VkImageView emissiveImageView;
  VkSampler emissiveSampler;

  BufferManager &bufferManager;
  Renderer &renderer;
  TextureManager(BufferManager &bufferManager, Renderer &renderer) : bufferManager(bufferManager), renderer(renderer)
  {
  }
  ~TextureManager()
  {
  }
  void createTextureImageView(VkDevice device);
  void createTextTextureImageView(VkDevice device);

  void createTextureImages(std::string albedoPath, std::string normalPath, std::string heightPath, std::string roughnessPath, std::string metallicPath, std::string aoPath, std::string emissivePath, VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue graphicsQueue);
  void createTextureImage(std::string texturePath, VkImage &image, VkDeviceMemory &imageMemory, VkFormat format, VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue graphicsQueue);
  void createTextureImage(const FT_Bitmap &bitmap, VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue graphicsQueue);
  void createTextureImage(const std::vector<uint8_t> &textureData, int texWidth, int texHeight, VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue graphicsQueue);

  void createTextureSampler(VkDevice device, VkPhysicalDevice physicalDevice);

  void updateTexture(std::string newTexturePath, VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue graphicsQueue);
  void cleanup(VkDevice device);
};