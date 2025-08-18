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
#include <serializationUtils.hpp>
#include <imgui_impl_vulkan.h>
#include <imgui_impl_glfw.h>
#include <imgui.h>
#include "tiny_gltf.h"
#include "json.hpp"
#include <glm/gtc/type_ptr.hpp>
#include <noImage.hpp>

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

void Engine::init(std::string windowName, std::function<void(Engine *)> startFn, std::function<void(Engine *, float)> updateFn)
{
  start = std::move(startFn);
  update = std::move(updateFn);

  initWindow(windowName);
  renderer.initVulkan();
  particleEmitters.emplace_back(renderer, &nextRenderingId, 512, glm::vec3(-1000));
  particleEmitters.at(0).hide = true;
  renderer.engineUI.initImGui(&renderer);
  physics.debugDrawer = new VulkanDebugDrawer(renderer, nextRenderingId, true);
}

void Engine::run()
{
  float lastFrame = 0.0f;
  float deltaTime = 0.0f;
  glfwSetTime(0);

  start(this);

  while (isRunning && !glfwWindowShouldClose(window))
  {
    float currentFrame = static_cast<float>(glfwGetTime());
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    if (autoFreeCam)
      updateFreeCam(deltaTime);

    update(this, deltaTime);

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

        btn->updateState(xpos, ypos, pressed, renderer.engineUI.sceneMin, renderer.engineUI.sceneMax, renderer.WIDTH, renderer.HEIGHT);
      }
    }

    updateBoxColliders();

    physics.debugDrawer->clearLines();

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

  for (auto &[e, _] : registry.animatedMeshes)
  {
    renderer.renderQueue.push_back(makeAnimatedGameObjectCommand(registry, e, &renderer, renderer.getCurrentFrame(), view, proj));
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

  renderer.renderQueue.push_back(makeDebugCommand(physics.debugDrawer, &renderer, physics.debugDrawer->debugLines, view, proj, renderer.getCurrentFrame()));

  renderer.engineUI.renderImGUI(this, &renderer);

  renderer.drawFrame();
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

void Engine::addEmptyMeshComponent(Entity entity)
{
  if (registry.meshes.find(entity) != registry.meshes.end())
    return;

  MeshComponent meshComp;
  meshComp.loadedFromFile = false;
  registry.meshes.emplace(entity, std::move(meshComp));
}

void Engine::addEmptyAnimatedMeshComponent(Entity entity)
{
  if (registry.animatedMeshes.find(entity) != registry.animatedMeshes.end())
    return;

  AnimatedMeshComponent meshComp;
  registry.animatedMeshes.emplace(entity, std::move(meshComp));
}

void Engine::addMeshComponent(Entity entity, MaterialData material, const std::string &texturePath, const std::vector<Vertex> &vertices, const std::vector<uint32_t> &indices)
{
  if (registry.meshes.find(entity) != registry.meshes.end())
    return;

  Mesh mesh(renderer, &nextRenderingId, material, vertices, indices);
  mesh.initGraphics(renderer, texturePath.empty() ? NO_IMAGE : texturePath);

  MeshComponent meshComp;
  meshComp.loadedFromFile = false;
  meshComp.meshes.emplace_back(std::move(mesh));
  registry.meshes.emplace(entity, std::move(meshComp));
}

void Engine::addMeshToComponent(Entity entity, MaterialData material, const std::string &texturePath, const std::vector<Vertex> &vertices, const std::vector<uint32_t> &indices)
{
  if (registry.meshes.find(entity) == registry.meshes.end())
    return;

  MeshComponent &meshComp = registry.meshes.at(entity);
  if (meshComp.loadedFromFile)
    return;

  Mesh mesh(renderer, &nextRenderingId, material, vertices, indices);
  mesh.initGraphics(renderer, texturePath.empty() ? NO_IMAGE : texturePath);
  meshComp.meshes.emplace_back(std::move(mesh));
}

void Engine::addAnimatedMeshToComponent(Entity entity, MaterialData material, const std::string &texturePath, const std::vector<AnimatedVertex> &vertices, const std::vector<uint32_t> &indices)
{
  if (registry.animatedMeshes.find(entity) == registry.animatedMeshes.end())
    return;

  AnimatedMeshComponent &meshComp = registry.animatedMeshes.at(entity);

  AnimatedMesh mesh(renderer, &nextRenderingId, material, vertices, indices);
  mesh.initGraphics(renderer, texturePath.empty() ? NO_IMAGE : texturePath);
  meshComp.meshes.emplace_back(std::move(mesh));
}

void Engine::addSkeletonComponent(Entity entity, std::vector<glm::mat4> inverseBindMatrices, std::vector<int> jointNodeIndices, tinygltf::Model *model, const tinygltf::Node *node)
{
  if (registry.animationSkeletons.find(entity) != registry.animationSkeletons.end())
    return;

  SkeletonComponent skeleton;
  skeleton.inverseBindMats = inverseBindMatrices;
  skeleton.jointNodeIndices = jointNodeIndices;
  skeleton.model = model;
  skeleton.node = node;
  registry.animationSkeletons.emplace(entity, std::move(skeleton));
}

void recordParentHierarchy(const tinygltf::Model *model, std::unordered_map<int, int> &nodeToParent, int nodeIndex, int parentIndex = -1)
{
  nodeToParent[nodeIndex] = parentIndex;

  const tinygltf::Node &node = model->nodes[nodeIndex];
  for (int child : node.children)
  {
    recordParentHierarchy(model, nodeToParent, child, nodeIndex);
  }
}

void Engine::createAnimatedModelFromFile(std::string baseName, std::string path, std::string texturesDir)
{

  tinygltf::Model &model = loadedModels[path];
  tinygltf::TinyGLTF loader;
  std::string err, warn;

  if (!loader.LoadASCIIFromFile(&model, &err, &warn, path))
  {
    std::cerr << "Failed to load glTF: " << err << std::endl;
    return;
  }
  if (!warn.empty())
    std::cout << "Warn: " << warn << std::endl;
  if (!err.empty())
    std::cerr << "Err: " << err << std::endl;

  std::cout << "Nodes: " << model.nodes.size() << std::endl;
  std::cout << "Scenes: " << model.scenes.size() << std::endl;
  std::cout << "Meshes: " << model.meshes.size() << std::endl;

  int i = 0;

  Entity baseEntity = createEmptyGameObject(baseName);

  for (const auto &node : model.nodes)
  {
    Entity entity = registry.createEntity(baseName + "_" + std::to_string(i));
    i++;

    ParentComponent parentComp;
    parentComp.parent = baseEntity;
    registry.parents.emplace(entity, parentComp);

    glm::vec3 position = glm::vec3(0.0f);
    glm::quat rotation = glm::quat(1, 0, 0, 0);
    glm::vec3 scale = glm::vec3(1.0f);

    if (!node.translation.empty())
      position = glm::make_vec3(node.translation.data());
    if (!node.rotation.empty())
      rotation = glm::make_quat(node.rotation.data());
    if (!node.scale.empty())
      scale = glm::make_vec3(node.scale.data());

    glm::vec3 euler = glm::eulerAngles(rotation);

    addTransformComponent(entity, position, euler, scale);

    if (node.mesh >= 0)
    {
      addEmptyAnimatedMeshComponent(entity);
      const tinygltf::Mesh &mesh = model.meshes[node.mesh];

      for (const auto &primitive : mesh.primitives)
      {
        std::vector<AnimatedVertex> vertices;
        std::vector<uint32_t> indices;
        if (primitive.indices >= 0)
        {
          const tinygltf::Accessor &accessor = model.accessors[primitive.indices];
          const tinygltf::BufferView &bufferView = model.bufferViews[accessor.bufferView];
          const tinygltf::Buffer &buffer = model.buffers[bufferView.buffer];

          const unsigned char *dataPtr = buffer.data.data() + bufferView.byteOffset + accessor.byteOffset;
          size_t indexCount = accessor.count;

          indices.resize(indexCount);

          switch (accessor.componentType)
          {
          case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
          {
            const uint8_t *src = reinterpret_cast<const uint8_t *>(dataPtr);
            for (size_t i = 0; i < indexCount; i++)
              indices[i] = static_cast<uint32_t>(src[i]);
            break;
          }
          case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
          {
            const uint16_t *src = reinterpret_cast<const uint16_t *>(dataPtr);
            for (size_t i = 0; i < indexCount; i++)
              indices[i] = static_cast<uint32_t>(src[i]);
            break;
          }
          case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
          {
            const uint32_t *src = reinterpret_cast<const uint32_t *>(dataPtr);
            for (size_t i = 0; i < indexCount; i++)
              indices[i] = src[i];
            break;
          }
          default:
            std::cerr << "Unsupported index component type: " << accessor.componentType << "\n";
            break;
          }
        }

        if (primitive.attributes.find("POSITION") != primitive.attributes.end())
        {
          const tinygltf::Accessor &posAccessor = model.accessors[primitive.attributes.at("POSITION")];
          size_t vertexCount = posAccessor.count;
          vertices.resize(vertexCount);

          const tinygltf::BufferView &bufferView = model.bufferViews[posAccessor.bufferView];
          const tinygltf::Buffer &buffer = model.buffers[bufferView.buffer];
          const float *posData = reinterpret_cast<const float *>(&(buffer.data[bufferView.byteOffset + posAccessor.byteOffset]));

          const float *normalData = nullptr;
          const float *texCoordData = nullptr;
          const float *weightData = nullptr;
          const unsigned char *jointIndexData = nullptr;

          // a->accessor v->view b->buffer
          if (primitive.attributes.count("NORMAL"))
          {
            const auto &a = model.accessors[primitive.attributes.at("NORMAL")];
            const auto &v = model.bufferViews[a.bufferView];
            const auto &b = model.buffers[v.buffer];
            normalData = reinterpret_cast<const float *>(&b.data[v.byteOffset + a.byteOffset]);
          }

          if (primitive.attributes.count("TEXCOORD_0"))
          {
            const auto &a = model.accessors[primitive.attributes.at("TEXCOORD_0")];
            const auto &v = model.bufferViews[a.bufferView];
            const auto &b = model.buffers[v.buffer];
            texCoordData = reinterpret_cast<const float *>(&b.data[v.byteOffset + a.byteOffset]);
          }

          if (primitive.attributes.count("WEIGHTS_0"))
          {
            const auto &a = model.accessors[primitive.attributes.at("WEIGHTS_0")];
            const auto &v = model.bufferViews[a.bufferView];
            const auto &b = model.buffers[v.buffer];
            weightData = reinterpret_cast<const float *>(&b.data[v.byteOffset + a.byteOffset]);
          }
          else
          {
            std::cout << "This animation has no WEIGHTS: " << path << std::endl;
            return;
          }

          int jointComponentSize = 0;
          if (primitive.attributes.count("JOINTS_0"))
          {
            const auto &a = model.accessors[primitive.attributes.at("JOINTS_0")];
            const auto &v = model.bufferViews[a.bufferView];
            const auto &b = model.buffers[v.buffer];
            const unsigned char *raw = &b.data[v.byteOffset + a.byteOffset];

            size_t vertexCount = a.count;

            if (a.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE)
              jointComponentSize = sizeof(uint8_t);
            else if (a.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT)
              jointComponentSize = sizeof(uint16_t);
            else
            {
              std::cerr << "Unsupported JOINTS_0 component type: " << a.componentType << "\n";
              return;
            }

            jointIndexData = raw;
          }
          else
          {
            std::cout << "This animation has no JOINTS: " << path << std::endl;
            return;
          }

          for (size_t i = 0; i < vertexCount; i++)
          {
            AnimatedVertex v{};
            v.pos = glm::make_vec3(posData + i * 3);
            v.color = texCoordData ? glm::vec3(-1.0f) : glm::vec3(1.0f);
            v.normal = normalData ? glm::make_vec3(normalData + i * 3) : glm::vec3(0.0f);
            v.texPos = texCoordData ? glm::make_vec2(texCoordData + i * 2) : glm::vec2(0.0f);

            if (jointIndexData)
            {
              glm::uvec4 joints(0);
              if (jointComponentSize == 1) // uint8_t
              {
                const uint8_t *j = reinterpret_cast<const uint8_t *>(jointIndexData + i * 4);
                joints = glm::uvec4(j[0], j[1], j[2], j[3]);
              }
              else if (jointComponentSize == 2) // uint16_t
              {
                const uint16_t *j = reinterpret_cast<const uint16_t *>(jointIndexData + i * 4);
                joints = glm::uvec4(j[0], j[1], j[2], j[3]);
              }

              for (int k = 0; k < 4; ++k)
                if (joints[k] >= 100)
                  joints[k] = 0;

              v.jointIndices = joints;
            }

            if (weightData)
            {
              v.jointWeights = glm::make_vec4(weightData + i * 4);
              float sum = glm::dot(v.jointWeights, glm::vec4(1.0f));
              if (sum > 0.0f)
                v.jointWeights /= sum;
            }

            vertices[i] = v;
          }

          MaterialData meshMaterial;
          meshMaterial.hasDiffuseMap = false;
          meshMaterial.diffuseColor = glm::vec3(1);
          std::string textureName = NO_IMAGE;

          if (primitive.material >= 0)
          {
            const tinygltf::Material &material = model.materials[primitive.material];

            if (material.pbrMetallicRoughness.baseColorTexture.index >= 0)
            {
              const tinygltf::Texture &texture = model.textures[material.pbrMetallicRoughness.baseColorTexture.index];

              if (texture.source >= 0)
              {
                const tinygltf::Image &image = model.images[texture.source];

                textureName = texturesDir + image.uri;
                meshMaterial.hasDiffuseMap = true;
              }
            }
          }
          addAnimatedMeshToComponent(entity, meshMaterial, textureName, vertices, indices);
        }
      }
    }

    if (node.skin >= 0)
    {
      const tinygltf::Skin &skin = model.skins[node.skin];

      const std::vector<int> &joints = skin.joints;
      size_t jointCount = joints.size();

      std::vector<glm::mat4> inverseBindMatrices;

      if (skin.inverseBindMatrices >= 0)
      {
        const tinygltf::Accessor &accessor = model.accessors[skin.inverseBindMatrices];
        const tinygltf::BufferView &view = model.bufferViews[accessor.bufferView];
        const tinygltf::Buffer &buffer = model.buffers[view.buffer];

        const float *data = reinterpret_cast<const float *>(&buffer.data[view.byteOffset + accessor.byteOffset]);

        for (size_t i = 0; i < accessor.count; i++)
        {
          glm::mat4 mat = glm::make_mat4(data + i * 16);
          inverseBindMatrices.push_back(mat);
        }
      }
      addSkeletonComponent(entity, inverseBindMatrices, joints, &model, &node);
      SkeletonComponent &skeleton = registry.animationSkeletons.at(entity);
      for (int rootNode : skeleton.model->scenes[skeleton.model->defaultScene].nodes)
      {
        recordParentHierarchy(skeleton.model, skeleton.nodeToParent, rootNode);
      }

      skeleton.nodeTransforms.resize(model.nodes.size());
      for (size_t i = 0; i < model.nodes.size(); ++i)
      {
        const tinygltf::Node &node = model.nodes[i];
        SkeletonComponent::NodeLocalTransform transform;

        if (!node.translation.empty())
        {
          transform.translation = glm::vec3(
              node.translation[0],
              node.translation[1],
              node.translation[2]);
        }

        if (!node.rotation.empty())
        {
          transform.rotation = glm::vec4(
              node.rotation[3],
              node.rotation[0],
              node.rotation[1],
              node.rotation[2]);
        }

        if (!node.scale.empty())
        {
          transform.scale = glm::vec3(
              node.scale[0],
              node.scale[1],
              node.scale[2]);
        }

        skeleton.nodeTransforms[i] = transform;
      }
    }

    if (!model.animations.empty())
    {
      AnimationComponent &animationComponent = registry.animationComponents[entity];

      for (const tinygltf::Animation &gltfAnim : model.animations)
      {
        Animation animation;
        animation.name = gltfAnim.name;

        for (const tinygltf::AnimationSampler &gltfSampler : gltfAnim.samplers)
        {
          AnimationSampler sampler;

          {
            const tinygltf::Accessor &accessor = model.accessors[gltfSampler.input];
            const tinygltf::BufferView &bufferView = model.bufferViews[accessor.bufferView];
            const tinygltf::Buffer &buffer = model.buffers[bufferView.buffer];

            const float *dataPtr = reinterpret_cast<const float *>(
                &buffer.data[bufferView.byteOffset + accessor.byteOffset]);

            sampler.inputTimes.assign(dataPtr, dataPtr + accessor.count);
            if (!sampler.inputTimes.empty())
              animation.duration = std::max(animation.duration, sampler.inputTimes.back());
          }

          {
            const tinygltf::Accessor &accessor = model.accessors[gltfSampler.output];
            const tinygltf::BufferView &bufferView = model.bufferViews[accessor.bufferView];
            const tinygltf::Buffer &buffer = model.buffers[bufferView.buffer];

            const float *dataPtr = reinterpret_cast<const float *>(
                &buffer.data[bufferView.byteOffset + accessor.byteOffset]);

            for (size_t i = 0; i < accessor.count; ++i)
            {
              glm::vec4 value(0.0f);
              if (accessor.type == TINYGLTF_TYPE_VEC3)
              {
                value = glm::vec4(dataPtr[i * 3 + 0], dataPtr[i * 3 + 1], dataPtr[i * 3 + 2], 0.0f);
              }
              else if (accessor.type == TINYGLTF_TYPE_VEC4)
              {
                value = glm::vec4(dataPtr[i * 4 + 0], dataPtr[i * 4 + 1], dataPtr[i * 4 + 2], dataPtr[i * 4 + 3]);
              }
              sampler.outputValues.push_back(value);
            }
          }

          sampler.interpolation = gltfSampler.interpolation;
          animation.samplers.push_back(sampler);
        }

        for (const tinygltf::AnimationChannel &gltfChannel : gltfAnim.channels)
        {
          AnimationChannel channel;
          channel.nodeIndex = gltfChannel.target_node;
          channel.path = gltfChannel.target_path;
          channel.samplerIndex = gltfChannel.sampler;
          animation.channels.push_back(channel);
        }

        animationComponent.animations.push_back(animation);
      }
    }
  }
}

void Engine::addMeshComponent(Entity entity, const std::string objPath, const std::string mtlPath)
{
  if (registry.meshes.find(entity) != registry.meshes.end())
    return;

  MeshComponent meshComp;
  meshComp.loadedFromFile = true;
  meshComp.objPath = objPath;
  meshComp.mtlPath = mtlPath;

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
    material.hasDiffuseMap = 1;
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
      material.hasDiffuseMap = 0;
      fullPath = NO_IMAGE;
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
  registry.meshes.emplace(entity, std::move(meshComp));
}

void Engine::removeMeshComponent(Entity entity)
{
  MeshComponent &meshComp = registry.meshes[entity];
  for (auto &mesh : meshComp.meshes)
  {
    mesh.cleanup(renderer.deviceManager.device, renderer);
  }

  registry.meshes.erase(entity);
}

void Engine::addRigidBodyComponent(Entity entity)
{
  if (registry.rigidBodies.find(entity) != registry.rigidBodies.end())
    return;
  registry.rigidBodies.emplace(entity, RigidBodyComponent());
}

void Engine::removeRigidBodyComponent(Entity entity)
{
  registry.rigidBodies.erase(entity);
}

void Engine::setRigidBodyComponentStatic(Entity entity, bool isStatic)
{
  auto it = registry.rigidBodies.find(entity);
  if (it != registry.rigidBodies.end())
    it->second.isStatic = isStatic;
}

void Engine::setRigidBodyComponentUseGravity(Entity entity, bool useGravity)
{
  auto it = registry.rigidBodies.find(entity);
  if (it != registry.rigidBodies.end())
    it->second.useGravity = useGravity;
}

void Engine::setRigidBodyComponentMass(Entity entity, float mass)
{
  auto it = registry.rigidBodies.find(entity);
  if (it != registry.rigidBodies.end())
    it->second.mass = mass;
}

void Engine::applyRigidBodyForce(Entity entity, const glm::vec3 &force)
{
  auto it = registry.rigidBodies.find(entity);
  if (it != registry.rigidBodies.end())
    it->second.applyForce(force);
}

TransformComponent &Engine::getTransformComponent(Entity entity)
{
  registry.transforms[entity].justUpdated = true;
  return registry.transforms[entity];
}

TransformComponent &Engine::getTransformComponentNoUpdate(Entity entity)
{
  return registry.transforms[entity];
}

const TransformComponent &Engine::getConstTransformComponent(Entity entity)
{
  return registry.transforms[entity];
}

BoxColliderComponent &Engine::getBoxColliderComponent(Entity entity)
{
  registry.boxColliders[entity].justUpdated = true;
  return registry.boxColliders[entity];
}

BoxColliderComponent &Engine::getBoxColliderComponentNoUpdate(Entity entity)
{
  return registry.boxColliders[entity];
}

RigidBodyComponent &Engine::getRigidBodyComponent(Entity entity)
{
  return registry.rigidBodies[entity];
}

void Engine::addTransformComponent(Entity entity, glm::vec3 position, glm::vec3 rotation, glm::vec3 scale)
{
  if (registry.transforms.find(entity) != registry.transforms.end())
    return;

  TransformComponent transformComp;
  transformComp.position = position;
  transformComp.rotationZYX = rotation;
  transformComp.scale = scale;
  registry.transforms.emplace(entity, std::move(transformComp));
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

void Engine::updateBoxCollider(Entity entity, glm::vec3 position, glm::vec3 rotation, glm::vec3 scale)
{
  if (registry.boxColliders.find(entity) == registry.boxColliders.end())
    return;

  BoxColliderComponent &boxCollider = registry.boxColliders[entity];
  boxCollider.updateWorldAABB(position, rotation, scale);
}

void Engine::updateBoxCollider(Entity entity)
{
  if (registry.boxColliders.find(entity) == registry.boxColliders.end() || registry.transforms.find(entity) == registry.transforms.end())
    return;

  BoxColliderComponent &boxCollider = registry.boxColliders[entity];
  TransformComponent &transform = getTransformComponent(entity);
  boxCollider.updateWorldAABB(transform.position, transform.rotationZYX, transform.scale);
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

void Engine::serializeScene(const std::string &filePath)
{
  std::ofstream out(filePath, std::ios::binary);
  if (!out)
  {
    std::cerr << "Failed to open " << filePath << " for serialization." << std::endl;
    return;
  }

  int serializationVersion = 1;
  writeInt(out, serializationVersion);

  writeUInt(out, registry.getNextEntity());

  writeTransforms(out, registry.transforms);
  writeMeshes(out, registry.meshes);
  writeIdentifiers(out, registry.entities);
  writeBoxColliders(out, registry.boxColliders);
  writeRigidBodies(out, registry.rigidBodies);
}

void Engine::deserializeScene(const std::string &filePath)
{
  registry.transforms.clear();
  registry.meshes.clear();
  registry.entities.clear();
  registry.boxColliders.clear();
  registry.rigidBodies.clear();

  std::ifstream in(filePath, std::ios::binary);
  if (!in)
  {
    std::cerr << "Failed to open " << filePath << " for deserialization." << std::endl;
    return;
  }

  int serializationVersion;
  readInt(in, serializationVersion);
  if (serializationVersion != 1)
  {
    std::cerr << "Unsupported version for deserialization." << std::endl;
    return;
  }

  Entity nextEntity;
  readUInt(in, nextEntity);
  registry.setNextEntity(nextEntity);

  readTransforms(in, registry.transforms);
  readMeshes(in, this);
  readIdentifiers(in, registry.entities);
  readBoxColliders(in, registry.boxColliders);

  for (auto &box : registry.boxColliders)
  {
    if (box.second.autoUpdate)
    {
      if (registry.transforms.find(box.first) == registry.transforms.end())
        continue;

      TransformComponent &t = registry.transforms.at(box.first);
      box.second.updateWorldAABB(t.position, t.rotationZYX, t.scale);
    }
    else
    {
      box.second.updateWorldAABB(box.second.position, box.second.rotationZYX, box.second.scale);
    }
  }

  readRigidBodies(in, registry.rigidBodies);
}