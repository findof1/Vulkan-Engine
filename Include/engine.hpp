#include "renderer.hpp"
#include "ParticleEmitter.hpp"
#include "UI.hpp"
#include "ECSRegistry.hpp"
#include "physicsSystem.hpp"
#include "noImage.hpp"
#include <memory>

#ifdef BUILD_ENGINE_DLL

#ifndef ENGINE_API
#define ENGINE_API __declspec(dllexport)
#endif

#else

#ifndef ENGINE_API
#define ENGINE_API __declspec(dllimport)
#endif

#endif

#ifndef DEBUG_MODE
#define DEBUG_MODE
enum DebugMode
{
  Viewport, // full debug tools along with viewport
  Tools,    // full debug tools
  Inactive, // no debug tools
};
#endif

struct ENGINE_API Input
{
  bool keys[GLFW_KEY_LAST] = {false};
  bool mouseButtons[GLFW_MOUSE_BUTTON_LAST] = {false};
  double mouseX = 0.0;
  double mouseY = 0.0;
  double scrollOffsetX = 0.0;
  double scrollOffsetY = 0.0;
};

class ENGINE_API Engine
{
public:
  Camera camera;
  uint32_t WIDTH;
  uint32_t HEIGHT;
  Input input;

  ECSRegistry registry;
  std::unordered_map<std::string, std::unique_ptr<UI>> UIElements;
  std::unordered_map<std::string, tinygltf::Model> loadedModels;
  std::unordered_map<std::string, std::shared_ptr<TextureManager>> preloadedTextures;

  std::string selectedUI = "";
  PhysicsSystem physics;

  DebugMode debugMode;

  Engine(uint32_t width = 1600, uint32_t height = 1200, DebugMode debugMode = DebugMode::Tools) : WIDTH(width), HEIGHT(height), debugMode(debugMode), camera(), renderer(camera, WIDTH, HEIGHT), registry(), physics(registry)
  {
  }

  void init(std::string windowName, std::function<void(Engine *)> startFn, std::function<void(Engine *, float)> updateFn);
  void run();
  void shutdown();

  void render();

  void clearHierarchy();

  void addTextElement(const std::string &name, glm::vec3 position, std::string text);
  void addSquareElement(const std::string &name, glm::vec3 position, glm::vec3 color, std::array<glm::vec2, 2> verticesOffsets, std::array<glm::vec2, 2> uvCoords = {glm::vec2(0, 0), glm::vec2(1, 1)}, std::string texture = NO_IMAGE);
  void addButtonElement(const std::string &name, glm::vec3 position, std::string text, std::array<glm::vec2, 2> verticesOffsets, glm::vec3 color, glm::vec3 colorHovered, glm::vec3 colorPressed, std::string texture = NO_IMAGE, std::function<void(void)> callback = []() {});

  inline void moveCamera(const glm::vec3 &offset)
  {
    camera.move(offset);
  }

  inline void moveCameraForwards(float amount)
  {
    camera.moveForwards(amount);
  }

  inline void moveCameraRight(float amount)
  {
    camera.moveRight(amount);
  }

  inline void rotateCamera(float yawOffset, float pitchOffset, bool constrainPitch = true)
  {
    camera.rotate(yawOffset, pitchOffset, constrainPitch);
  }

  inline void setCameraZoom(float zoom)
  {
    camera.setZoom(zoom);
  }

  void useFreeCamMode(bool activate)
  {
    autoFreeCam = true;
  }

  void disableCursor();
  void enableCursor();

  void removeEntity(Entity entity);
  void removeUIElement(const std::string &identifier);
  void loadMaterialAsset(std::string assetName, std::string texturePath = NO_IMAGE, std::string normalPath = NO_IMAGE, std::string heightPath = NO_IMAGE, std::string roughnessPath = NO_IMAGE, std::string metallicPath = NO_IMAGE, std::string aoPath = NO_IMAGE, std::string emissivePath = NO_IMAGE);
  void updateTextObject(const std::string &identifier, std::string text);
  void createAnimatedModelFromFile(std::string baseName, std::string path, std::string texturesDir);
  void addEmptyMeshComponent(Entity entity);
  void addMeshComponent(Entity entity, MaterialData material, const std::string &texturePath, const std::vector<Vertex> &vertices, const std::vector<uint32_t> &indices);
  void addMeshComponent(Entity entity, const std::string objPath, const std::string mtlPath);
  void addMeshToComponent(Entity entity, MaterialData material, const std::string &texturePath, const std::vector<Vertex> &vertices, const std::vector<uint32_t> &indices);
  void addMeshToComponent(Entity entity, std::string textureAssetName, MaterialData material, const std::vector<Vertex> &vertices, const std::vector<uint32_t> &indices);
  void removeMeshComponent(Entity entity);
  void addEmptyAnimatedMeshComponent(Entity entity);
  void addAnimatedMeshToComponent(Entity entity, MaterialData material, const std::string &texturePath, const std::vector<AnimatedVertex> &vertices, const std::vector<uint32_t> &indices);
  void addSkeletonComponent(Entity entity, std::vector<glm::mat4> inverseBindMatrices, std::vector<int> jointNodeIndices, tinygltf::Model *model, const tinygltf::Node *node);
  void addTransformComponent(Entity entity, glm::vec3 position, glm::vec3 rotation, glm::vec3 scale);
  void addPointLightComponent(Entity entity, glm::vec3 color, int intensity);
  void addBoxColliderComponent(Entity entity, glm::vec3 position, glm::vec3 rotation, glm::vec3 scale);
  void addBoxColliderComponent(Entity entity);
  void updateBoxCollider(Entity entity, glm::vec3 position, glm::vec3 rotation, glm::vec3 scale);
  void updateBoxCollider(Entity entity);
  void addRigidBodyComponent(Entity entity);
  void setRigidBodyComponentStatic(Entity entity, bool isStatic);
  void removeRigidBodyComponent(Entity entity);
  void setRigidBodyComponentUseGravity(Entity entity, bool useGravity);
  void setRigidBodyComponentMass(Entity entity, float mass);
  void applyRigidBodyForce(Entity entity, const glm::vec3 &force);
  Entity createEmptyGameObject(std::string name);
  Entity getGameObjectHandle(std::string name);
  TransformComponent &getTransformComponent(Entity entity);
  TransformComponent &getTransformComponentNoUpdate(Entity entity);
  const TransformComponent &getConstTransformComponent(Entity entity);
  BoxColliderComponent &getBoxColliderComponent(Entity entity);
  BoxColliderComponent &getBoxColliderComponentNoUpdate(Entity entity);
  RigidBodyComponent &getRigidBodyComponent(Entity entity);

  void serializeScene(const std::string &filePath);
  void deserializeScene(const std::string &filePath);

private:
  Renderer renderer;
  GLFWwindow *window;
  int nextRenderingId = 0;
  std::function<void(Engine *, float)> update;
  std::function<void(Engine *)> start;
  std::vector<ParticleEmitter> particleEmitters;
  bool autoFreeCam = false;

  void initWindow(std::string windowName);
  void updateBoxColliders();

  inline void transformComponentDisableJustUpdated(Entity entity)
  {
    registry.transforms[entity].justUpdated = false;
  }

  static void framebufferResizeCallback(GLFWwindow *window, int width, int height)
  {
    auto app = reinterpret_cast<Engine *>(glfwGetWindowUserPointer(window));
    app->renderer.framebufferResized = true;
  }

  static void cursor_position_callback(GLFWwindow *window, double xpos, double ypos)
  {
    auto app = reinterpret_cast<Engine *>(glfwGetWindowUserPointer(window));
    app->input.mouseX = xpos;
    app->input.mouseY = ypos;
  }

  static void scroll_callback(GLFWwindow *window, double xoffset, double yoffset)
  {
    auto app = reinterpret_cast<Engine *>(glfwGetWindowUserPointer(window));
    app->input.scrollOffsetX = xoffset;
    app->input.scrollOffsetY = yoffset;
  }

  static void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
  {
    auto app = reinterpret_cast<Engine *>(glfwGetWindowUserPointer(window));
    if (key >= 0 && key < GLFW_KEY_LAST)
    {
      app->input.keys[key] = (action != GLFW_RELEASE);
    }
  }

  static void mouse_button_callback(GLFWwindow *window, int button, int action, int mods)
  {
    auto app = reinterpret_cast<Engine *>(glfwGetWindowUserPointer(window));
    if (button >= 0 && button < GLFW_MOUSE_BUTTON_LAST)
    {
      app->input.mouseButtons[button] = (action != GLFW_RELEASE);
    }
  }

  void updateFreeCam(float dt);

  bool isRunning = true;
};