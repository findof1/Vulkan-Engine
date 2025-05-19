#ifndef RENDER_COMMANDS_H
#define RENDER_COMMANDS_H

#include <functional>
#include <vulkan/vulkan.h>
#include <glm/glm.hpp>
#ifndef RENDER_COMMAND
#define RENDER_COMMAND
struct RenderCommand
{
  std::function<void(VkCommandBuffer)> execute;
};
#endif
struct Vertex;
class VulkanDebugDrawer;
class GameObject;
class ParticleEmitter;
class Renderer;
class UI;

RenderCommand makeGameObjectCommand(GameObject *obj, Renderer *renderer, int currentFrame, glm::mat4 view, glm::mat4 proj);
RenderCommand makeUICommand(UI *ui, Renderer *renderer, int currentFrame, glm::mat4 model, glm::mat4 ortho);
RenderCommand makeParticleCommand(ParticleEmitter *emitter, Renderer *renderer, int currentFrame, glm::mat4 view, glm::mat4 proj);
RenderCommand makeDebugCommand(VulkanDebugDrawer *drawer, Renderer *renderer, const std::vector<Vertex> &lines, glm::mat4 view, glm::mat4 proj, int currentFrame);

#endif