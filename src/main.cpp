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

void start()
{
}

void update(float dt)
{
}

int main()
{
    Engine engine;
    try
    {
        engine.init("Game Engine", start, update);
        engine.useFreeCamMode(true);
        engine.enableDebug();
        MaterialData ground;
        ground.diffuseColor = {0.5, 0.5, 0.5};
        ground.hasTexture = 1;

        // engine.addGameObject("ground", glm::vec3(0, -10, 0), glm::vec3(100, 5, 100), glm::vec3(0, 0, 0), ground, "textures/wood.png", cubeVerticesNoNormals, cubeIndices);

        Entity object1 = engine.createEmptyGameObject("object");
        engine.addMeshComponent(object1, ground, "textures/wood.png", cubeVerticesNoNormals, cubeIndices);
        TransformComponent &transform = engine.getTransformComponent(object1);
        transform.scale = glm::vec3(100, 5, 100);
        transform.position = glm::vec3(0, -10, 0);
        engine.addBoxColliderComponent(object1);

        Entity object2 = engine.createEmptyGameObject("cube");
        engine.addMeshComponent(object2, ground, "textures/wood.png", cubeVerticesNoNormals, cubeIndices);
        TransformComponent &transform2 = engine.getTransformComponent(object2);
        transform2.scale = glm::vec3(1, 1, 1);
        transform2.position = glm::vec3(0, 0, 0);
        engine.addBoxColliderComponent(object2);

        Entity object3 = engine.createEmptyGameObject("cube2");
        engine.addMeshComponent(object3, ground, "textures/wood.png", cubeVerticesNoNormals, cubeIndices);
        TransformComponent &transform3 = engine.getTransformComponent(object3);
        transform3.scale = glm::vec3(1, 1, 1);
        transform3.position = glm::vec3(5.5f, 0.5f, 0);
        engine.addBoxColliderComponent(object3);

        engine.addTextElement("titleText", glm::vec3(10.0f, -100.0f, 0.0f), "Hello, UI!");

        engine.addSquareElement(
            "square",
            glm::vec3(300.0f, -100.0f, -0.5f),
            glm::vec3(1.0f, 1.0f, 1.0f),
            {glm::vec2(-50.0f, -50.0f), glm::vec2(50.0f, 50.0f)},
            "textures/awesomeface.png");

        engine.addButtonElement(
            "button1",
            glm::vec3(200.0f, -900.0f, -0.5f),
            "Increase Size",
            {glm::vec2(-20.0f, -100.0f), glm::vec2(350.0f, 100.0f)},
            glm::vec3(0.0f, 0.7f, 0.0f),
            glm::vec3(0.1f, 0.9f, 0.0f),
            glm::vec3(0.1f, 1.0f, 0.2f),
            "textures/concrete.png",
            [&engine, object1]()
            {
                auto &transform = engine.getTransformComponent(object1);
                transform.scale = transform.scale * 1.2f;
            });

        engine.addButtonElement(
            "button2",
            glm::vec3(600.0f, -900.0f, -0.5f),
            "Decrease Size",
            {glm::vec2(-20.0f, -100.0f), glm::vec2(350.0f, 100.0f)},
            glm::vec3(0.7f, 0.0f, 0.0f),
            glm::vec3(0.9f, 0.1f, 0.0f),
            glm::vec3(1.0f, 0.2f, 0.2f),
            "textures/concrete.png",
            [&engine, object1]()
            {
                auto &transform = engine.getTransformComponent(object1);
                transform.scale = transform.scale / 1.2f;
            });
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
