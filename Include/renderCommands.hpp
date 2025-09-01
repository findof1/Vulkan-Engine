#ifndef RENDER_COMMANDS_H
#define RENDER_COMMANDS_H

#include <functional>
#include <vulkan/vulkan.h>
#include <glm/glm.hpp>
#include "ECSRegistry.hpp"
#ifndef DEBUG_MODE
#define DEBUG_MODE
enum DebugMode
{
  Viewport, // full debug tools along with viewport
  Tools,    // full debug tools
  Inactive, // no debug tools
};
#endif

#ifndef RENDER_COMMAND
#define RENDER_COMMAND

#ifdef BUILD_ENGINE_DLL

#ifndef ENGINE_API
#define ENGINE_API __declspec(dllexport)
#endif

#else

#ifndef ENGINE_API
#define ENGINE_API __declspec(dllimport)
#endif

#endif

enum RenderStage
{
  MainRender,
  ColorID, // for clicking and color picking
};

struct ENGINE_API RenderCommand
{
  std::function<void(VkCommandBuffer, RenderStage)> execute;
};

#endif
struct Vertex;
class VulkanDebugDrawer;
class ParticleEmitter;
class Renderer;
class UI;

ENGINE_API RenderCommand makeGameObjectCommand(ECSRegistry &registry, Entity e, Renderer *renderer, int currentFrame, glm::mat4 view, glm::mat4 proj, DebugMode debugMode);
ENGINE_API RenderCommand makeAnimatedGameObjectCommand(ECSRegistry &registry, Entity e, Renderer *renderer, int currentFrame, glm::mat4 view, glm::mat4 proj, DebugMode debugMode);
ENGINE_API RenderCommand makeUICommand(UI *ui, Renderer *renderer, int currentFrame, glm::mat4 model, glm::mat4 ortho, DebugMode debugMode);
ENGINE_API RenderCommand makeParticleCommand(ParticleEmitter *emitter, Renderer *renderer, int currentFrame, glm::mat4 view, glm::mat4 proj, DebugMode debugMode);
ENGINE_API RenderCommand makeDebugCommand(VulkanDebugDrawer *drawer, Renderer *renderer, const std::vector<Vertex> &lines, glm::mat4 view, glm::mat4 proj, int currentFrame, DebugMode debugMode);

#endif