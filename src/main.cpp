#include <vulkan/vulkan.h>
#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#include "engine.hpp"
#include "primitives.hpp"

Engine engine;
void start()
{
}

void update(float dt)
{
}

int main()
{
    try
    {
        engine.init("Game Engine", start, update);
        engine.useFreeCamMode(true);
        MaterialData ground;
        ground.diffuseColor = {0.5, 0.5, 0.5};
        ground.hasTexture = 1;

        // engine.addGameObject("ground", glm::vec3(0, -10, 0), glm::vec3(100, 5, 100), glm::vec3(0, 0, 0), ground, "textures/wood.png", cubeVerticesNoNormals, cubeIndices);

        Entity object1 = engine.createEmptyGameObject("object");
        engine.addMeshComponent(object1, ground, "textures/wood.png", cubeVerticesNoNormals, cubeIndices);

        engine.addTextElement("titleText", glm::vec3(10.0f, -100.0f, 0.0f), "Hello, UI!");

        engine.addSquareElement(
            "square",
            glm::vec3(200.0f, -200.0f, -0.5f),
            glm::vec3(0.1f, 0.1f, 0.1f),
            {glm::vec2(-100.0f, -300.0f), glm::vec2(200.0f, 150.0f)},
            "textures/awesomeface.png");

        engine.addButtonElement(
            "button",
            glm::vec3(200.0f, -700.0f, -0.5f),
            "Button",
            {glm::vec2(-100.0f, -100.0f), glm::vec2(150.0f, 50.0f)},
            glm::vec3(0.2f, 0.6f, 0.9f),
            glm::vec3(0.3f, 0.7f, 1.0f),
            glm::vec3(0.1f, 0.3f, 0.7f),
            "textures/concrete.png");

        engine.run();
        engine.shutdown();
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
