#include "renderer.hpp"
#include "ParticleEmitter.hpp"
#include "UI.hpp"
#include "ECSRegistry.hpp"

struct Input
{
  bool keys[GLFW_KEY_LAST] = {false};
  bool mouseButtons[GLFW_MOUSE_BUTTON_LAST] = {false};
  double mouseX = 0.0;
  double mouseY = 0.0;
  double scrollOffsetX = 0.0;
  double scrollOffsetY = 0.0;
};

class Engine
{
public:
  Camera camera;
  uint32_t WIDTH;
  uint32_t HEIGHT;
  Input input;

  ECSRegistry registry;

  Engine(uint32_t width = 1600, uint32_t height = 1200) : WIDTH(width), HEIGHT(height), camera(), renderer(camera, WIDTH, HEIGHT)
  {
  }

  void init(std::string windowName, std::function<void()> startFn, std::function<void(float)> updateFn);
  void run();
  void shutdown();

  void render();
  void enableDebug();

  void clearHierarchy();

  void addTextElement(const std::string &name, glm::vec3 position, std::string text);
  void addSquareElement(const std::string &name, glm::vec3 position, glm::vec3 color, std::array<glm::vec2, 2> verticesOffsets, std::string texture = "models/couch/diffuse.png");
  void addButtonElement(const std::string &name, glm::vec3 position, std::string text, std::array<glm::vec2, 2> verticesOffsets, glm::vec3 color, glm::vec3 colorHovered, glm::vec3 colorPressed, std::string texture = "models/couch/diffuse.png", std::function<void(void)> callback = []() {});

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

  void addMeshComponent(Entity entity, MaterialData &material, const std::string &texturePath, const std::vector<Vertex> &vertices, const std::vector<uint32_t> &indices);
  void addMeshComponent(Entity entity, const std::string objPath, const std::string mtlPath);
  void removeMeshComponent(Entity entity);
  void addTransformComponent(Entity entity, glm::vec3 position, glm::vec3 rotation, glm::vec3 scale);
  Entity createEmptyGameObject(std::string name);
  Entity getGameObjectHandle(std::string name);

private:
  Renderer renderer;
  GLFWwindow *window;
  int nextRenderingId = 0;
  std::function<void(float)> update;
  std::function<void()> start;
  std::vector<ParticleEmitter> particleEmitters;
  std::unordered_map<std::string, std::unique_ptr<UI>> UIElements;
  bool autoFreeCam = false;

  void initWindow(std::string windowName);

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