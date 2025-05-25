#include "renderCommands.hpp"
#include "UI.hpp"
#include "particleEmitter.hpp"
#include "debugDrawer.hpp"
#include <glm/glm.hpp>

void setupViewportScissor(Renderer *renderer, VkCommandBuffer commandBuffer)
{
  VkViewport viewport{};

  viewport.x = 0.0f;
  viewport.y = 0.0f;

  viewport.width = static_cast<float>(renderer->swapchainManager.swapChainExtent.width);
  viewport.height = static_cast<float>(renderer->swapchainManager.swapChainExtent.height);

  viewport.minDepth = 0.0f;
  viewport.maxDepth = 1.0f;

  vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

  VkRect2D scissor{};
  scissor.offset = {0, 0};
  scissor.extent = renderer->swapchainManager.swapChainExtent;
  vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
}

void setTriangleTopology(Renderer *renderer, VkCommandBuffer commandBuffer)
{
  if (!renderer->fpCmdSetPrimitiveTopology)
  {
    PFN_vkCmdSetPrimitiveTopologyEXT vkCmdSetPrimitiveTopologyEXT =
        (PFN_vkCmdSetPrimitiveTopologyEXT)vkGetDeviceProcAddr(renderer->deviceManager.device, "vkCmdSetPrimitiveTopologyEXT");

    if (!vkCmdSetPrimitiveTopologyEXT)
    {
      std::cerr << "vkCmdSetPrimitiveTopology is not available and failed to load vkCmdSetPrimitiveTopologyEXT!" << std::endl;
    }
    else
    {
      vkCmdSetPrimitiveTopologyEXT(commandBuffer, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
    }
  }
  else
  {
    vkCmdSetPrimitiveTopology(commandBuffer, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
  }
}

void setLineListTopology(Renderer *renderer, VkCommandBuffer commandBuffer)
{
  if (!renderer->fpCmdSetPrimitiveTopology)
  {
    PFN_vkCmdSetPrimitiveTopologyEXT vkCmdSetPrimitiveTopologyEXT =
        (PFN_vkCmdSetPrimitiveTopologyEXT)vkGetDeviceProcAddr(renderer->deviceManager.device, "vkCmdSetPrimitiveTopologyEXT");

    if (!vkCmdSetPrimitiveTopologyEXT)
    {
      std::cerr << "vkCmdSetPrimitiveTopology is not available and failed to load vkCmdSetPrimitiveTopologyEXT!" << std::endl;
    }
    else
    {
      vkCmdSetPrimitiveTopologyEXT(commandBuffer, VK_PRIMITIVE_TOPOLOGY_LINE_LIST);
    }
  }
  else
  {
    vkCmdSetPrimitiveTopology(commandBuffer, VK_PRIMITIVE_TOPOLOGY_LINE_LIST);
  }
}

void setPointListTopology(Renderer *renderer, VkCommandBuffer commandBuffer)
{
  if (!renderer->fpCmdSetPrimitiveTopology)
  {
    PFN_vkCmdSetPrimitiveTopologyEXT vkCmdSetPrimitiveTopologyEXT =
        (PFN_vkCmdSetPrimitiveTopologyEXT)vkGetDeviceProcAddr(renderer->deviceManager.device, "vkCmdSetPrimitiveTopologyEXT");

    if (!vkCmdSetPrimitiveTopologyEXT)
    {
      std::cerr << "vkCmdSetPrimitiveTopology is not available and failed to load vkCmdSetPrimitiveTopologyEXT!" << std::endl;
    }
    else
    {
      vkCmdSetPrimitiveTopologyEXT(commandBuffer, VK_PRIMITIVE_TOPOLOGY_POINT_LIST);
    }
  }
  else
  {
    vkCmdSetPrimitiveTopology(commandBuffer, VK_PRIMITIVE_TOPOLOGY_POINT_LIST);
  }
}

void enableDepthWrite(Renderer *renderer, VkCommandBuffer commandBuffer)
{
  if (!renderer->vkCmdSetDepthWriteEnableEXT)
  {
    std::cerr << "vkCmdSetPrimitiveTopology is not available and failed to load vkCmdSetPrimitiveTopologyEXT!" << std::endl;
  }
  else
  {
    renderer->vkCmdSetDepthWriteEnableEXT(commandBuffer, VK_TRUE);
  }
}

void disableDepthWrite(Renderer *renderer, VkCommandBuffer commandBuffer)
{
  if (!renderer->vkCmdSetDepthWriteEnableEXT)
  {
    std::cerr << "vkCmdSetPrimitiveTopology is not available and failed to load vkCmdSetPrimitiveTopologyEXT!" << std::endl;
  }
  else
  {
    renderer->vkCmdSetDepthWriteEnableEXT(commandBuffer, VK_FALSE);
  }
}

RenderCommand makeGameObjectCommand(ECSRegistry &registry, Entity e, Renderer *renderer, int currentFrame, glm::mat4 view, glm::mat4 proj)
{
  return {
      [&registry, renderer, e, currentFrame, view, proj](VkCommandBuffer cmdBuf)
      {
        vkCmdBindPipeline(cmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS, renderer->pipelineManager.graphicsPipeline);
        setTriangleTopology(renderer, cmdBuf);
        enableDepthWrite(renderer, cmdBuf);
        setupViewportScissor(renderer, cmdBuf);

        auto meshIt = registry.meshes.find(e);
        if (meshIt == registry.meshes.end() || meshIt->second.hide)
          return;

        MeshComponent &meshComp = registry.meshes.at(e);

        if (meshComp.hide)
          return;

        auto transformIt = registry.transforms.find(e);
        glm::mat4 transformation(1.0f);
        if (transformIt != registry.transforms.end())
        {
          const TransformComponent &transform = transformIt->second;

          transformation = glm::translate(transformation, transform.position);
          transformation = glm::rotate(transformation, glm::radians(transform.rotationZYX.x), glm::vec3(0.0f, 0.0f, 1.0f));
          transformation = glm::rotate(transformation, glm::radians(transform.rotationZYX.y), glm::vec3(0.0f, 1.0f, 0.0f));
          transformation = glm::rotate(transformation, glm::radians(transform.rotationZYX.z), glm::vec3(1.0f, 0.0f, 0.0f));
          transformation = glm::scale(transformation, transform.scale);
        }

        for (auto &mesh : meshComp.meshes)
        {
          mesh.draw(renderer, currentFrame, transformation, view, proj, cmdBuf);
        }
      }};
}

RenderCommand makeUICommand(UI *ui, Renderer *renderer, int currentFrame, glm::mat4 model, glm::mat4 ortho)
{
  return {
      [=](VkCommandBuffer cmdBuf)
      {
        vkCmdBindPipeline(cmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS, renderer->pipelineManager.graphicsPipeline);
        setTriangleTopology(renderer, cmdBuf);
        enableDepthWrite(renderer, cmdBuf);
        setupViewportScissor(renderer, cmdBuf);
        ui->draw(renderer, currentFrame, model, glm::mat4(1.0f), ortho, cmdBuf);
      }};
}

RenderCommand makeParticleCommand(ParticleEmitter *emitter, Renderer *renderer, int currentFrame, glm::mat4 view, glm::mat4 proj)
{
  return {
      [=](VkCommandBuffer cmdBuf)
      {
        vkCmdBindPipeline(cmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS, renderer->pipelineManager.graphicsParticlePipeline);
        setPointListTopology(renderer, cmdBuf);
        disableDepthWrite(renderer, cmdBuf);
        setupViewportScissor(renderer, cmdBuf);
        emitter->draw(renderer, currentFrame, view, proj, cmdBuf);
      }};
}

RenderCommand makeDebugCommand(VulkanDebugDrawer *drawer, Renderer *renderer, const std::vector<Vertex> &lines, glm::mat4 view, glm::mat4 proj, int currentFrame)
{
  return {
      [=](VkCommandBuffer cmdBuf)
      {
        if (drawer)
        {
          vkCmdBindPipeline(cmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS, renderer->pipelineManager.graphicsPipeline);
          setLineListTopology(renderer, cmdBuf);
          enableDepthWrite(renderer, cmdBuf);
          setupViewportScissor(renderer, cmdBuf);
          drawer->drawDebugLines(cmdBuf, lines, view, proj, currentFrame);
        }
      }};
}
