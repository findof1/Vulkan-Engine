#include "engine.hpp"
#include "renderer.hpp"
#include "particleEmitter.hpp"
#include "UI.hpp"
#include "text.hpp"
#include "square.hpp"
#include "button.hpp"
#include "renderCommands.hpp"
#include "debugDrawer.hpp"

void Engine::initWindow()
{
  glfwInit();

  // glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

  window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
  glfwSetWindowUserPointer(window, this);
  glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
  glfwSetScrollCallback(window, scroll_callback);
  glfwSetCursorPosCallback(window, mouse_callback);
  renderer.window = window;
  renderer.swapchainManager.setWindow(window);
}

void Engine::disableCursor()
{
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

void Engine::enableCursor()
{
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
}

void Engine::init(std::function<void()> startFn, std::function<void(float)> updateFn)
{
  start = std::move(startFn);
  update = std::move(updateFn);

  initWindow();
  renderer.initVulkan();
  particleEmitters.emplace_back(renderer, &nextRenderingId, 512, glm::vec3(-1000));
}

void Engine::run()
{
  float lastFrame = 0.0f;
  float deltaTime = 0.0f;

  while (isRunning && !glfwWindowShouldClose(window))
  {
    float currentFrame = static_cast<float>(glfwGetTime());
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    processInput();
    update(deltaTime);

    for (auto &[key, ui_ptr] : UIElements)
    {
      Button *btn = dynamic_cast<Button *>(ui_ptr.get());
      if (btn)
      {
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
        int state = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
        bool pressed = false;
        if (state == GLFW_PRESS)
        {
          pressed = true;
        }

        btn->updateState(xpos, ypos, pressed);
      }
    }

    if (renderer.debugDrawer)
    {
      renderer.debugDrawer->clearLines();
    }
    // physics stuff goes here
    render();

    glfwPollEvents();
  }
  vkDeviceWaitIdle(renderer.deviceManager.device);
}

void Engine::clearHierarchy()
{
  vkQueueWaitIdle(renderer.graphicsQueue);

  for (auto &gameObject : objects)
  {
    gameObject.second.cleanupMeshes(renderer.deviceManager.device, renderer);
  }
  objects.clear();
  for (auto &[_, element] : UIElements)
  {
    element->cleanup(renderer.deviceManager.device, renderer);
  }

  for (int i = 0; i < particleEmitters.size(); i++)
  {
    particleEmitters.at(i).hide = true;
    particleEmitters.at(i).textureManager.cleanup(renderer.deviceManager.device);
  }
}

void Engine::addTextElement(const std::string &name, glm::vec3 position, std::string text)
{
  UIElements.emplace(name, std::make_unique<Text>(renderer, &nextRenderingId, text, position));
}

void Engine::addSquareElement(const std::string &name, glm::vec3 position, glm::vec3 color, std::array<glm::vec2, 2> verticesOffsets, std::string texture)
{
  UIElements.emplace(name, std::make_unique<Square>(renderer, &nextRenderingId, position, verticesOffsets, color, texture));
}

void Engine::addButtonElement(const std::string &name, glm::vec3 position, std::string text, std::array<glm::vec2, 2> verticesOffsets, glm::vec3 color, glm::vec3 colorHovered, glm::vec3 colorPressed, std::string texture)
{
  auto btn = std::make_unique<Button>(renderer, &nextRenderingId, text, position, verticesOffsets, texture);
  btn->normalColor = color;
  btn->hoverColor = colorHovered;
  btn->pressedColor = colorPressed;
  UIElements.emplace(name, std::move(btn));
}

void Engine::shutdown()
{
  clearHierarchy();
  renderer.renderQueue.clear();
  renderer.cleanup();
}

void Engine::render()
{
  renderer.renderQueue.clear();
  glm::mat4 view = camera.getViewMatrix();
  glm::mat4 proj = glm::perspective(glm::radians(camera.Zoom), (float)WIDTH / HEIGHT, 0.1f, 10000.0f);
  glm::mat4 ortho = glm::ortho(0.0f, (float)WIDTH, 0.0f, (float)HEIGHT, -10.0f, 10.0f);

  for (auto &[_, obj] : objects)
    renderer.renderQueue.push_back(makeGameObjectCommand(&obj, &renderer, renderer.getCurrentFrame(), view, proj));

  for (auto &[_, element] : UIElements)
  {
    glm::mat4 model = glm::translate(glm::mat4(1.0f), element->position);
    renderer.renderQueue.push_back(makeUICommand(element.get(), &renderer, renderer.getCurrentFrame(), model, ortho));
  }

  for (ParticleEmitter &emitter : particleEmitters)
  {
    if (!emitter.hide)
      renderer.renderQueue.push_back(makeParticleCommand(&emitter, &renderer, renderer.getCurrentFrame(), view, proj));
  }

  if (renderer.debugDrawer)
    renderer.renderQueue.push_back(makeDebugCommand(renderer.debugDrawer, &renderer, renderer.debugDrawer->debugLines, view, proj, renderer.getCurrentFrame()));

  renderer.drawFrame();
}

void Engine::enableDebug()
{
  VulkanDebugDrawer debugDrawer(renderer, nextRenderingId, true);
  renderer.debugDrawer = &debugDrawer;
}

void Engine::processInput()
{
}

void Engine::addGameObject(const std::string &name, const glm::vec3 &pos, const glm::vec3 &scale, const glm::vec3 &rotationZYX, MaterialData &material, const std::string &texturePath, const std::vector<Vertex> &vertices, const std::vector<unsigned int> &indices)
{
  objects.emplace(name, GameObject(renderer, pos, scale, rotationZYX));
  objects.at(name).addMesh(renderer, &nextRenderingId, material, texturePath, vertices, indices);
}
