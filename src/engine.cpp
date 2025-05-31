#include "engine.hpp"
#include "renderer.hpp"
#include "particleEmitter.hpp"
#include "UI.hpp"
#include "text.hpp"
#include "square.hpp"
#include "button.hpp"
#include "renderCommands.hpp"
#include "debugDrawer.hpp"
#include <tiny_obj_loader.h>

void Engine::initWindow(std::string windowName)
{
  glfwInit();

  // glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

  window = glfwCreateWindow(WIDTH, HEIGHT, windowName.c_str(), nullptr, nullptr);
  glfwSetWindowUserPointer(window, this);
  glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
  glfwSetKeyCallback(window, key_callback);
  glfwSetMouseButtonCallback(window, mouse_button_callback);
  glfwSetCursorPosCallback(window, cursor_position_callback);
  glfwSetScrollCallback(window, scroll_callback);
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

void Engine::init(std::string windowName, std::function<void()> startFn, std::function<void(float)> updateFn)
{
  start = std::move(startFn);
  update = std::move(updateFn);

  initWindow(windowName);
  renderer.initVulkan();
  particleEmitters.emplace_back(renderer, &nextRenderingId, 512, glm::vec3(-1000));
  particleEmitters.at(0).hide = true;
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

    if (autoFreeCam)
      updateFreeCam(deltaTime);

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

    updateBoxColliders();

    if (physics.debugDrawer)
    {
      physics.debugDrawer->clearLines();
    }
    physics.update(deltaTime);
    render();

    glfwPollEvents();
  }
  vkDeviceWaitIdle(renderer.deviceManager.device);
}

void Engine::updateBoxColliders()
{
  for (auto &box : registry.boxColliders)
  {
    if (!box.second.autoUpdate)
    {
      continue;
    }
    const TransformComponent &transform = getConstTransformComponent(box.first);
    if (transform.justUpdated)
    {
      box.second.updateWorldAABB(transform.position, transform.rotationZYX, transform.scale);
      transformComponentDisableJustUpdated(box.first);
    }
  }
}

void Engine::clearHierarchy()
{
  vkQueueWaitIdle(renderer.graphicsQueue);

  registry.meshes.clear();
  registry.transforms.clear();
  registry.boxColliders.clear();
  registry.resetNextEntity();

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

void Engine::addButtonElement(const std::string &name, glm::vec3 position, std::string text, std::array<glm::vec2, 2> verticesOffsets, glm::vec3 color, glm::vec3 colorHovered, glm::vec3 colorPressed, std::string texture, std::function<void(void)> callback)
{
  auto btn = std::make_unique<Button>(renderer, &nextRenderingId, text, position, verticesOffsets, texture);
  btn->normalColor = color;
  btn->hoverColor = colorHovered;
  btn->pressedColor = colorPressed;
  btn->callback = callback;
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

  for (auto &[e, _] : registry.meshes)
  {
    renderer.renderQueue.push_back(makeGameObjectCommand(registry, e, &renderer, renderer.getCurrentFrame(), view, proj));
  }

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

  if (physics.debugDrawer)
    renderer.renderQueue.push_back(makeDebugCommand(physics.debugDrawer, &renderer, physics.debugDrawer->debugLines, view, proj, renderer.getCurrentFrame()));

  renderer.drawFrame();
}

void Engine::enableDebug()
{
  physics.debugDrawer = new VulkanDebugDrawer(renderer, nextRenderingId, true);
}

void Engine::updateFreeCam(float dt)
{
  float moveSpeed = 15.0f;
  const float mouseSensitivity = 0.1f;
  static bool firstMouse = true;
  static double lastX = 0.0, lastY = 0.0;
  if (input.keys[GLFW_KEY_LEFT_SHIFT])
    moveSpeed *= 2;

  if (input.keys[GLFW_KEY_W])
    moveCameraForwards(+moveSpeed * dt);
  if (input.keys[GLFW_KEY_S])
    moveCameraForwards(-moveSpeed * dt);
  if (input.keys[GLFW_KEY_D])
    moveCameraRight(+moveSpeed * dt);
  if (input.keys[GLFW_KEY_A])
    moveCameraRight(-moveSpeed * dt);
  if (input.keys[GLFW_KEY_SPACE])
    moveCamera({0.0f, +moveSpeed * dt, 0.0f});
  if (input.keys[GLFW_KEY_LEFT_CONTROL])
    moveCamera({0.0f, -moveSpeed * dt, 0.0f});

  if (input.mouseButtons[GLFW_MOUSE_BUTTON_RIGHT])
  {
    disableCursor();
    if (firstMouse)
    {
      lastX = input.mouseX;
      lastY = input.mouseY;
      firstMouse = false;
    }

    float xoffset = static_cast<float>(input.mouseX - lastX);
    float yoffset = static_cast<float>(lastY - input.mouseY);

    lastX = input.mouseX;
    lastY = input.mouseY;

    rotateCamera(xoffset * mouseSensitivity, yoffset * mouseSensitivity);
  }
  else
  {
    enableCursor();
    firstMouse = true;
  }

  static float zoom = 45.0f;
  zoom -= input.scrollOffsetY;
  if (zoom < 1.0f)
    zoom = 1.0f;
  if (zoom > 90.0f)
    zoom = 90.0f;
  setCameraZoom(zoom);

  input.scrollOffsetX = 0.0;
  input.scrollOffsetY = 0.0;
}

void Engine::addMeshComponent(Entity entity, MaterialData &material, const std::string &texturePath, const std::vector<Vertex> &vertices, const std::vector<uint32_t> &indices)
{
  if (registry.meshes.find(entity) != registry.meshes.end())
    return;

  Mesh mesh(renderer, &nextRenderingId, material, vertices, indices);
  mesh.initGraphics(renderer, texturePath.empty() ? "models/couch/diffuse.png" : texturePath);

  MeshComponent &meshComp = registry.meshes[entity];
  meshComp.meshes.emplace_back(std::move(mesh));
}

void Engine::addMeshComponent(Entity entity, const std::string objPath, const std::string mtlPath)
{
  if (registry.meshes.find(entity) != registry.meshes.end())
    return;

  MeshComponent &meshComp = registry.meshes[entity];

  tinyobj::attrib_t attrib;
  std::vector<tinyobj::shape_t> shapes;
  std::vector<tinyobj::material_t> materials;
  std::string err;

  bool ret = tinyobj::LoadObj(
      &attrib,
      &shapes,
      &materials,
      &err,
      objPath.c_str(), mtlPath.c_str());

  if (!ret)
  {
    throw std::runtime_error("Failed to load OBJ: " + err);
  }

  for (const auto &shape : shapes)
  {
    std::vector<Vertex> meshVertices;
    std::vector<uint32_t> meshIndices;
    for (const auto &index : shape.mesh.indices)
    {
      Vertex vertex{};

      size_t vIdx = 3 * index.vertex_index;
      if (vIdx + 2 >= attrib.vertices.size())
      {
        throw std::runtime_error("Vertex index out of range in model: " + objPath);
      }
      vertex.pos = {
          attrib.vertices[vIdx + 0],
          attrib.vertices[vIdx + 1],
          attrib.vertices[vIdx + 2]};

      if (!attrib.normals.empty() && (3 * index.normal_index + 2 < attrib.normals.size()))
      {
        vertex.normal = {
            attrib.normals[3 * index.normal_index + 0],
            attrib.normals[3 * index.normal_index + 1],
            attrib.normals[3 * index.normal_index + 2]};
      }
      else
      {
        vertex.normal = {0.0f, 0.0f, 0.0f};
      }

      if (!attrib.texcoords.empty() && (2 * index.texcoord_index + 1 < attrib.texcoords.size()))
      {
        vertex.texPos = {
            attrib.texcoords[2 * index.texcoord_index + 0],
            1.0f - attrib.texcoords[2 * index.texcoord_index + 1]};
      }
      else
      {
        vertex.texPos = {0.0f, 0.0f};
      }

      vertex.color = {-1.0f, -1.0f, -1.0f};

      meshVertices.push_back(vertex);
      meshIndices.push_back(static_cast<uint32_t>(meshIndices.size()));
    }
    MaterialData material;
    material.diffuseColor = {0.5f, 0.5f, 0.5f};
    material.hasTexture = 1;
    std::string texturePath = "";
    std::string fullPath = "";

    if (!materials.empty() && !shape.mesh.material_ids.empty())
    {
      int matId = shape.mesh.material_ids[0];
      if (matId < 0 || matId >= static_cast<int>(materials.size()))
      {
        throw std::runtime_error("Material index out of range in model: " + objPath);
      }

      if (!materials[matId].diffuse_texname.empty())
      {
        texturePath = materials[matId].diffuse_texname;
        fullPath = mtlPath + texturePath;
      }

      material.diffuseColor = {
          materials[matId].diffuse[0],
          materials[matId].diffuse[1],
          materials[matId].diffuse[2]};

      material.specularColor = {
          materials[matId].specular[0],
          materials[matId].specular[1],
          materials[matId].specular[2]};

      material.ambientColor = {
          materials[matId].ambient[0],
          materials[matId].ambient[1],
          materials[matId].ambient[2]};

      material.shininess = materials[matId].shininess;
      material.emissionColor = {
          materials[matId].emission[0],
          materials[matId].emission[1],
          materials[matId].emission[2]};

      material.refractiveIndex = materials[matId].ior;
      material.illuminationModel = materials[matId].illum;

      material.opacity = (materials[matId].diffuse[3] != 0) ? materials[matId].diffuse[3] : 1.0f;
    }

    if (texturePath.empty())
    {
      material.hasTexture = 0;
      fullPath = "models/couch/diffuse.png";
    }

    Mesh mesh(renderer, &nextRenderingId, material, meshVertices, meshIndices);

    try
    {
      mesh.initGraphics(renderer, fullPath);
    }
    catch (const std::exception &e)
    {
      throw std::runtime_error("Mesh graphics initialization failed: " + std::string(e.what()));
    }
    meshComp.meshes.push_back(std::move(mesh));
  }
}

void Engine::removeMeshComponent(Entity entity)
{
  MeshComponent &meshComp = registry.meshes[entity];
  for (auto &mesh : meshComp.meshes)
  {
    mesh.cleanup(renderer.deviceManager.device, renderer);
  }
}

TransformComponent &Engine::getTransformComponent(Entity entity)
{
  registry.transforms[entity].justUpdated = true;
  return registry.transforms[entity];
}

const TransformComponent &Engine::getConstTransformComponent(Entity entity)
{
  return registry.transforms[entity];
}

void Engine::addTransformComponent(Entity entity, glm::vec3 position, glm::vec3 rotation, glm::vec3 scale)
{
  if (registry.transforms.find(entity) != registry.transforms.end())
    return;

  TransformComponent &transformComp = registry.transforms[entity];
  transformComp.position = position;
  transformComp.rotationZYX = rotation;
  transformComp.scale = scale;
}

void Engine::addBoxColliderComponent(Entity entity, glm::vec3 position, glm::vec3 rotation, glm::vec3 scale)
{
  if (registry.boxColliders.find(entity) != registry.boxColliders.end())
    return;

  BoxColliderComponent &boxCollider = registry.boxColliders[entity];
  boxCollider.updateWorldAABB(position, rotation, scale);
}

void Engine::addBoxColliderComponent(Entity entity)
{
  if (registry.boxColliders.find(entity) != registry.boxColliders.end())
    return;

  BoxColliderComponent &boxCollider = registry.boxColliders[entity];
  TransformComponent &transform = getTransformComponent(entity);
  boxCollider.updateWorldAABB(transform.position, transform.rotationZYX, transform.scale);
  boxCollider.autoUpdate = true;
}

Entity Engine::createEmptyGameObject(std::string name)
{
  Entity e = registry.createEntity(name);
  addTransformComponent(e, glm::vec3(0), glm::vec3(0), glm::vec3(1));
  return e;
}

Entity Engine::getGameObjectHandle(std::string name)
{
  return registry.getEntity(name);
}