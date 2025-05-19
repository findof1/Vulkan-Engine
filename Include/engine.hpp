#include "renderer.hpp"
#include "ParticleEmitter.hpp"
#include "UI.hpp"
#include "GameObject.hpp"
class Engine
{
public:
  Camera camera;
  uint32_t WIDTH;
  uint32_t HEIGHT;

  Engine(uint32_t width = 1600, uint32_t height = 1200) : WIDTH(width), HEIGHT(height), camera(), renderer(camera, WIDTH, HEIGHT)
  {
  }

  void init(std::function<void()> startFn, std::function<void(float)> updateFn);
  void run();
  void shutdown();
  void render();
  void processInput();
  void enableDebug();
  void clearHierarchy();
  void addTextElement(const std::string &name, glm::vec3 position, std::string text);
  void addSquareElement(const std::string &name, glm::vec3 position, glm::vec3 color, std::array<glm::vec2, 2> verticesOffsets, std::string texture = "models/couch/diffuse.png");
  void addButtonElement(const std::string &name, glm::vec3 position, std::string text, std::array<glm::vec2, 2> verticesOffsets, glm::vec3 color, glm::vec3 colorHovered, glm::vec3 colorPressed, std::string texture = "models/couch/diffuse.png");
  void addGameObject(const std::string &name, const glm::vec3 &pos, const glm::vec3 &scale, const glm::vec3 &rotationZYX, MaterialData &material, const std::string &texturePath, const std::vector<Vertex> &vertices, const std::vector<unsigned int> &indices);

  void disableCursor();
  void enableCursor();

private:
  Renderer renderer;
  GLFWwindow *window;
  int nextRenderingId = 0;
  std::function<void(float)> update;
  std::function<void()> start;
  std::vector<ParticleEmitter> particleEmitters;
  std::unordered_map<std::string, GameObject> objects;
  std::unordered_map<std::string, std::unique_ptr<UI>> UIElements;

  void initWindow();

  static void mouse_callback(GLFWwindow *window, double xposIn, double yposIn)
  {
    auto app = reinterpret_cast<Engine *>(glfwGetWindowUserPointer(window));
  }

  static void framebufferResizeCallback(GLFWwindow *window, int width, int height)
  {
    auto app = reinterpret_cast<Engine *>(glfwGetWindowUserPointer(window));
    app->renderer.framebufferResized = true;
  }
  static void scroll_callback(GLFWwindow *window, double xoffset, double yoffset)
  {
    auto app = reinterpret_cast<Engine *>(glfwGetWindowUserPointer(window));
  }

  bool isRunning = true;
};