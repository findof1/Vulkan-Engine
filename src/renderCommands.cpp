#include "renderCommands.hpp"
#include "UI.hpp"
#include "particleEmitter.hpp"
#include "debugDrawer.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>

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

void setupViewportScissor(VkCommandBuffer commandBuffer, VkExtent2D extent)
{
  VkViewport viewport{};
  viewport.x = 0.0f;
  viewport.y = 0.0f;
  viewport.width = static_cast<float>(extent.width);
  viewport.height = static_cast<float>(extent.height);
  viewport.minDepth = 0.0f;
  viewport.maxDepth = 1.0f;

  vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

  VkRect2D scissor{};
  scissor.offset = {0, 0};
  scissor.extent = extent;

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

glm::mat4 getLocalTransform(SkeletonComponent &skeleton, int nodeIndex)
{

  glm::mat4 translation = glm::mat4(1.0f);
  glm::mat4 rotation = glm::mat4(1.0f);
  glm::mat4 scale = glm::mat4(1.0f);

  translation = glm::translate(translation, glm::vec3(skeleton.nodeTransforms[nodeIndex].translation[0], skeleton.nodeTransforms[nodeIndex].translation[1], skeleton.nodeTransforms[nodeIndex].translation[2]));

  glm::quat q = glm::quat(skeleton.nodeTransforms[nodeIndex].rotation[0], skeleton.nodeTransforms[nodeIndex].rotation[1], skeleton.nodeTransforms[nodeIndex].rotation[2], skeleton.nodeTransforms[nodeIndex].rotation[3]);
  rotation = glm::mat4_cast(q);

  scale = glm::scale(scale, glm::vec3(skeleton.nodeTransforms[nodeIndex].scale[0], skeleton.nodeTransforms[nodeIndex].scale[1], skeleton.nodeTransforms[nodeIndex].scale[2]));

  return translation * rotation * scale;
}

glm::mat4 getGlobalTransform(int nodeIndex, SkeletonComponent &skeleton, glm::mat4 startMatrix = glm::mat4(1.0f))
{
  glm::mat4 transform = startMatrix;

  while (nodeIndex >= 0)
  {
    transform = getLocalTransform(skeleton, nodeIndex) * transform;

    auto it = skeleton.nodeToParent.find(nodeIndex);
    if (it == skeleton.nodeToParent.end())
      break;
    nodeIndex = it->second;
  }

  return transform;
}

glm::mat4 getWorldTransform(ECSRegistry &registry, Entity e)
{
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

  auto parentIt = registry.parents.find(e);
  if (parentIt != registry.parents.end())
  {
    Entity parent = parentIt->second.parent;
    glm::mat4 parentTransform = getWorldTransform(registry, parent);
    transformation = parentTransform * transformation;
  }

  return transformation;
}

RenderCommand makeGameObjectCommand(ECSRegistry &registry, Entity e, Renderer *renderer, int currentFrame, glm::mat4 view, glm::mat4 proj)
{
  return {
      [&registry, renderer, e, currentFrame, view, proj](VkCommandBuffer cmdBuf, RenderStage renderStage)
      {
        auto meshIt = registry.meshes.find(e);
        if (meshIt == registry.meshes.end() || meshIt->second.hide)
        {
          return;
        }

        if (renderStage == MainRender)
        {
          vkCmdBindPipeline(cmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS, renderer->pipelineManager.graphicsPipeline);
          setTriangleTopology(renderer, cmdBuf);
          enableDepthWrite(renderer, cmdBuf);

          VkExtent2D offscreenExtent;
          offscreenExtent.width = renderer->engineUI.imageW;
          offscreenExtent.height = renderer->engineUI.imageH;
          setupViewportScissor(cmdBuf, offscreenExtent);
        }
        else if (renderStage == ColorID)
        {
          vkCmdBindPipeline(cmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS, renderer->pipelineManager.colorIDPipeline);
          setTriangleTopology(renderer, cmdBuf);
          enableDepthWrite(renderer, cmdBuf);

          VkExtent2D offscreenExtent;
          offscreenExtent.width = renderer->engineUI.imageW;
          offscreenExtent.height = renderer->engineUI.imageH;
          setupViewportScissor(cmdBuf, offscreenExtent);
        }

        MeshComponent &meshComp = registry.meshes.at(e);

        if (meshComp.hide)
          return;

        glm::mat4 transformation = getWorldTransform(registry, e);

        for (auto &mesh : meshComp.meshes)
        {
          mesh.draw(renderer, currentFrame, transformation, view, proj, cmdBuf, renderStage == MainRender ? -1 : e);
        }
      }};
}

RenderCommand makeAnimatedGameObjectCommand(ECSRegistry &registry, Entity e, Renderer *renderer, int currentFrame, glm::mat4 view, glm::mat4 proj)
{
  return {
      [&registry, renderer, e, currentFrame, view, proj](VkCommandBuffer cmdBuf, RenderStage renderStage)
      {
        auto animMeshIt = registry.animatedMeshes.find(e);
        if (animMeshIt == registry.animatedMeshes.end() || animMeshIt->second.hide)
        {
          return;
        }

        if (renderStage == ColorID) // Need to add support for color picking with animated meshes later
        {
          return;
        }
        vkCmdBindPipeline(cmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS, renderer->pipelineManager.animationPipeline);
        setTriangleTopology(renderer, cmdBuf);
        enableDepthWrite(renderer, cmdBuf);

        VkExtent2D offscreenExtent;
        offscreenExtent.width = renderer->engineUI.imageW;
        offscreenExtent.height = renderer->engineUI.imageH;
        setupViewportScissor(cmdBuf, offscreenExtent);

        AnimatedMeshComponent &animMeshComp = registry.animatedMeshes.at(e);

        if (animMeshComp.hide)
          return;

        glm::mat4 transformation = getWorldTransform(registry, e);

        SkeletonComponent &skeleton = registry.animationSkeletons.at(e);
        if (registry.animationComponents.find(e) != registry.animationComponents.end())
        {
          float currentTime = glfwGetTime();
          if (currentTime > 0)
          {
            AnimationComponent &animations = registry.animationComponents.at(e);
            Animation &animation = animations.animations.at(0);
            currentTime = fmod(currentTime, animation.duration);

            for (auto &channel : animation.channels)
            {
              AnimationSampler &sampler = animation.samplers[channel.samplerIndex];
              int frameIndex = 0;
              for (size_t i = 0; i < sampler.inputTimes.size(); ++i)
              {
                if (sampler.inputTimes[i] > currentTime)
                  break;
                frameIndex = i;
              }

              if (channel.path == "translation")
              {
                skeleton.nodeTransforms[channel.nodeIndex].translation = glm::vec3(sampler.outputValues[frameIndex]);
                skeleton.computeFinalBoneMatrices = true;
              }
              else if (channel.path == "rotation")
              {
                skeleton.nodeTransforms[channel.nodeIndex].rotation = glm::vec4(sampler.outputValues[frameIndex][3], sampler.outputValues[frameIndex][0], sampler.outputValues[frameIndex][1], sampler.outputValues[frameIndex][2]);
                skeleton.computeFinalBoneMatrices = true;
              }
              else if (channel.path == "scale")
              {
                skeleton.nodeTransforms[channel.nodeIndex].scale = glm::vec3(sampler.outputValues[frameIndex]);
                skeleton.computeFinalBoneMatrices = true;
              }
            }
          }
        }

        if (skeleton.computeFinalBoneMatrices)
        {
          skeleton.computeFinalBoneMatrices = !skeleton.computeFinalBoneMatrices;
          skeleton.finalBoneMatrices = {};

          const tinygltf::Skin &skin = skeleton.model->skins[skeleton.node->skin];
          for (int i = 0; i < skin.joints.size(); i++)
          {
            int jointNodeIndex = skin.joints[i];

            glm::mat4 jointGlobalTransform = getGlobalTransform(jointNodeIndex, skeleton);
            glm::mat4 inverseBind = skeleton.inverseBindMats[i];
            skeleton.finalBoneMatrices[i] = jointGlobalTransform * inverseBind;
          }
        }

        for (auto &mesh : animMeshComp.meshes)
        {
          mesh.draw(renderer, currentFrame, transformation, view, proj, skeleton.finalBoneMatrices, cmdBuf, renderStage == MainRender ? -1 : e);
        }
      }};
}

RenderCommand makeUICommand(UI *ui, Renderer *renderer, int currentFrame, glm::mat4 model, glm::mat4 ortho)
{
  return {
      [=](VkCommandBuffer cmdBuf, RenderStage renderStage)
      {
        if (renderStage != MainRender)
        {
          return;
        }
        vkCmdBindPipeline(cmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS, renderer->pipelineManager.graphicsPipeline);
        setTriangleTopology(renderer, cmdBuf);
        enableDepthWrite(renderer, cmdBuf);

        VkExtent2D offscreenExtent;
        offscreenExtent.width = renderer->engineUI.imageW;
        offscreenExtent.height = renderer->engineUI.imageH;
        setupViewportScissor(cmdBuf, offscreenExtent);
        ui->draw(renderer, currentFrame, model, glm::mat4(1.0f), ortho, cmdBuf);
      }};
}

RenderCommand makeParticleCommand(ParticleEmitter *emitter, Renderer *renderer, int currentFrame, glm::mat4 view, glm::mat4 proj)
{
  return {
      [=](VkCommandBuffer cmdBuf, RenderStage renderStage)
      {
        if (renderStage != MainRender)
        {
          return;
        }

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
      [=](VkCommandBuffer cmdBuf, RenderStage renderStage)
      {
        if (renderStage != MainRender)
        {
          return;
        }

        if (drawer)
        {
          vkCmdBindPipeline(cmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS, renderer->pipelineManager.graphicsPipeline);
          setLineListTopology(renderer, cmdBuf);
          enableDepthWrite(renderer, cmdBuf);

          VkExtent2D offscreenExtent;
          offscreenExtent.width = renderer->engineUI.imageW;
          offscreenExtent.height = renderer->engineUI.imageH;
          setupViewportScissor(cmdBuf, offscreenExtent);
          drawer->drawDebugLines(cmdBuf, lines, view, proj, currentFrame);
        }
      }};
}
