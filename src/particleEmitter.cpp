#include "particleEmitter.hpp"
#include "bufferManager.hpp"
#include "descriptorManager.hpp"
#include "textureManager.hpp"
#include "renderer.hpp"
#include <vulkan/vulkan.h>
#include <tiny_obj_loader.h>
#include <glm/gtc/quaternion.hpp>
#include <random>
#include <algorithm>
#include "mesh.hpp"

ParticleEmitter::ParticleEmitter(Renderer &renderer, int *nextRenderingId, int particleCount, glm::vec3 pos, glm::quat rotation, std::string texturePath) : textureManager(renderer.bufferManager, renderer), particles(particleCount), pos(pos), rotation(rotation)
{
  id = *nextRenderingId;
  (*nextRenderingId)++;

  std::random_device rd;
  std::default_random_engine rndEngine(rd());
  std::uniform_real_distribution<float> rndDist(0.5f, 1.5f);

  for (auto &particle : particles)
  {
    float x = (rndDist(rndEngine) - 1) / 4;
    float y = (rndDist(rndEngine) - 1) / 4;
    float z = (rndDist(rndEngine) - 1) / 4;

    particle.position = glm::vec3(x, y, z);
    particle.startPosition = glm::vec3(x, y, z);
    particle.velocity = glm::vec3(0.75 * (rndDist(rndEngine) - 1), (rndDist(rndEngine) + 0.5) * 1.2f, 0.75 * ((rndDist(rndEngine) * 2) - 4));
    particle.velocity *= rndDist(rndEngine);
    float color = rndDist(rndEngine) / 8;
    particle.color = glm::vec4(color, color, color, (rndDist(rndEngine) - 0.5) / 20);
  }

  initGraphics(renderer, texturePath);
}

void ParticleEmitter::initGraphics(Renderer &renderer, std::string texturePath)
{
  renderer.bufferManager.createShaderStorageBuffer(particles, renderer.MAX_FRAMES_IN_FLIGHT, 0, renderer.deviceManager.device, renderer.deviceManager.physicalDevice, renderer.commandPool, renderer.graphicsQueue);
  renderer.bufferManager.createComputeUniformBuffers(renderer.MAX_FRAMES_IN_FLIGHT, renderer.deviceManager.device, renderer.deviceManager.physicalDevice, 1);
  renderer.descriptorManager.addComputeDescriptorSets(renderer.deviceManager.device, renderer.MAX_FRAMES_IN_FLIGHT, 1);

  textureManager.createTextureImage(texturePath, renderer.deviceManager.device, renderer.deviceManager.physicalDevice, renderer.commandPool, renderer.graphicsQueue);
  textureManager.createTextureImageView(renderer.deviceManager.device);
  textureManager.createTextureSampler(renderer.deviceManager.device, renderer.deviceManager.physicalDevice);

  renderer.bufferManager.createUniformBuffers(renderer.MAX_FRAMES_IN_FLIGHT, renderer.deviceManager.device, renderer.deviceManager.physicalDevice, 1);

  renderer.descriptorManager.addDescriptorSets(renderer.deviceManager.device, renderer.MAX_FRAMES_IN_FLIGHT, 1, textureManager.textureImageView, textureManager.textureSampler);
}

void ParticleEmitter::draw(Renderer *renderer, int currentFrame, glm::mat4 view, glm::mat4 projectionMatrix, VkCommandBuffer commandBuffer)
{
  if (hide)
    return;

  VkBuffer vertexBuffers[] = {renderer->bufferManager.shaderStorageBuffers[currentFrame]};
  VkDeviceSize offsets[] = {0};
  vkCmdBindVertexBuffers(commandBuffer, 1, 1, vertexBuffers, offsets);

  vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, renderer->pipelineManager.pipelineLayout, 0, 1, &renderer->descriptorManager.descriptorSets[currentFrame + id * renderer->MAX_FRAMES_IN_FLIGHT], 0, nullptr);

  glm::mat4 transform = glm::mat4(1.0f);
  transform = glm::translate(transform, pos) * glm::mat4(rotation);
  transform = glm::scale(transform, glm::vec3(0.2));
  renderer->bufferManager.updateUniformBuffer(currentFrame + id * renderer->MAX_FRAMES_IN_FLIGHT, transform, view, projectionMatrix);

  MaterialData materialData;
  materialData.diffuseColor = glm::vec3(0);
  materialData.hasDiffuseMap = 1;
  materialData.isParticle = 1;

  vkCmdPushConstants(
      commandBuffer,
      renderer->pipelineManager.pipelineLayout,
      VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
      0,
      sizeof(MaterialData),
      &materialData);

  vkCmdDraw(commandBuffer, particles.size(), 1, 0, 0);
}