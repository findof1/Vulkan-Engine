#include "renderCommands.hpp"
#include "UI.hpp"
#include "particleEmitter.hpp"
#include "debugDrawer.hpp"

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

RenderCommand makeGameObjectCommand(GameObject *obj, Renderer *renderer, int currentFrame, glm::mat4 view, glm::mat4 proj)
{
  return {
      [=](VkCommandBuffer cmdBuf)
      {
        vkCmdBindPipeline(cmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS, renderer->pipelineManager.graphicsPipeline);
        setTriangleTopology(renderer, cmdBuf);
        enableDepthWrite(renderer, cmdBuf);
        setupViewportScissor(renderer, cmdBuf);
        obj->draw(renderer, currentFrame, view, proj, cmdBuf);
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
        if (renderer->debugDrawer)
        {
          vkCmdBindPipeline(cmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS, renderer->pipelineManager.graphicsPipeline);
          setLineListTopology(renderer, cmdBuf);
          enableDepthWrite(renderer, cmdBuf);
          setupViewportScissor(renderer, cmdBuf);
          drawer->drawDebugLines(cmdBuf, lines, view, proj, currentFrame);
        }
      }};
}
