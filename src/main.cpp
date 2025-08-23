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

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "engine.hpp"
#include "primitives.hpp"

void start(Engine *engine)
{
}

void update(Engine *engine, float dt)
{
    Entity object3 = engine->getGameObjectHandle("cube 2");
    if (object3 == -1)
    {
        return;
    }

    if (engine->input.keys[GLFW_KEY_B])
    {
        engine->applyRigidBodyForce(object3, glm::vec3(0, 20, 0));
    }
    if (engine->input.keys[GLFW_KEY_T])
    {
        engine->applyRigidBodyForce(object3, glm::vec3(5, 0, 0));
    }

    if (engine->input.keys[GLFW_KEY_F])
    {
        engine->applyRigidBodyForce(object3, glm::vec3(0, 0, -5));
    }

    if (engine->input.keys[GLFW_KEY_G])
    {
        engine->applyRigidBodyForce(object3, glm::vec3(-5, 0, 0));
    }

    if (engine->input.keys[GLFW_KEY_H])
    {
        engine->applyRigidBodyForce(object3, glm::vec3(0, 0, 5));
    }
}

int main()
{
    Engine engine;
    try
    {
        engine.init("Game Engine", start, update);
        engine.useFreeCamMode(true);

        MaterialData ground;
        ground.albedoColor = {0.5, 0.5, 0.5};
        ground.hasAlbedoMap = 1;

        MaterialData sky;
        sky.albedoColor = {0.5, 0.5, 0.5};
        sky.hasAlbedoMap = 1;
        sky.isSkybox = 1;

        Entity skyBox = engine.createEmptyGameObject("skyBox");
        engine.addMeshComponent(skyBox, sky, "Assets/textures/sky.png", cubeVerticesNoNormals, skyBoxIndices);
        TransformComponent &transformSky = engine.getTransformComponent(skyBox);
        transformSky.scale = glm::vec3(1000, 1000, 1000);
        transformSky.position = glm::vec3(0, 0, 0);

        Entity object1 = engine.createEmptyGameObject("object");
        engine.addMeshComponent(object1, ground, "Assets/textures/wood.png", cubeVertices, cubeIndices);
        TransformComponent &transform = engine.getTransformComponent(object1);
        transform.scale = glm::vec3(100, 30, 100);
        transform.position = glm::vec3(0, -15, 0);
        transform.rotationZYX = glm::vec3(0, 0, 0);
        engine.addBoxColliderComponent(object1);
        engine.addRigidBodyComponent(object1);
        engine.setRigidBodyComponentStatic(object1, true);

        Entity object2 = engine.createEmptyGameObject("cube 1");
        engine.addMeshComponent(object2, ground, "Assets/textures/wood.png", cubeVertices, cubeIndices);
        TransformComponent &transform2 = engine.getTransformComponent(object2);
        transform2.scale = glm::vec3(1, 1, 1);
        transform2.position = glm::vec3(10, 200, 0);
        engine.addBoxColliderComponent(object2);
        engine.addRigidBodyComponent(object2);

        Entity object3 = engine.createEmptyGameObject("cube 2");
        engine.addMeshComponent(object3, ground, "Assets/textures/wood.png", cubeVertices, cubeIndices);
        TransformComponent &transform3 = engine.getTransformComponent(object3);
        transform3.scale = glm::vec3(1, 1, 1);
        transform3.position = glm::vec3(0.1f, 150.0f, 15);
        transform3.rotationZYX = glm::vec3(45, 45, 45);
        engine.addBoxColliderComponent(object3);
        engine.addRigidBodyComponent(object3);

        Entity object4 = engine.createEmptyGameObject("cube 3");
        engine.addMeshComponent(object4, ground, "Assets/textures/wood.png", cubeVertices, cubeIndices);
        TransformComponent &transform4 = engine.getTransformComponent(object4);
        transform4.scale = glm::vec3(1, 1, 1);
        transform4.position = glm::vec3(14, 100, 0);
        engine.addBoxColliderComponent(object4);
        engine.addRigidBodyComponent(object4);

        Entity couch = engine.createEmptyGameObject("couch");
        engine.addMeshComponent(couch, "Assets/models/couch/couch1.obj", "Assets/models/couch/");
        TransformComponent &transformCouch = engine.getTransformComponent(couch);
        transformCouch.scale = glm::vec3(0.1f, 0.1f, 0.1f);
        transformCouch.position = glm::vec3(10, 100.0f, 10);
        transformCouch.rotationZYX = glm::vec3(45, 45, 45);
        engine.addBoxColliderComponent(couch);
        engine.registry.boxColliders.at(couch).localMin = glm::vec3(-10, -10, -40);
        engine.registry.boxColliders.at(couch).localMax = glm::vec3(10, 10, 40);
        engine.updateBoxCollider(couch);
        engine.addRigidBodyComponent(couch);

        Entity stump = engine.createEmptyGameObject("stump");
        engine.addMeshComponent(stump, "Assets/models/Stump/3DTreeStump001_HQ-4K-PNG.obj", "Assets/models/Stump/");
        TransformComponent &transformStump = engine.getTransformComponent(stump);
        transformStump.position = glm::vec3(5.0f, 10.0f, -5.0f);

        engine.createAnimatedModelFromFile("AnimationMan", "Assets/models/animationMan/man.gltf", "Assets/models/animationMan/");
        engine.createAnimatedModelFromFile("GLTFMan", "Assets/models/gltfMan/scene.gltf", "Assets/models/gltfMan/");
        Entity gltfMan = engine.getGameObjectHandle("GLTFMan");
        TransformComponent &transformMan = engine.getTransformComponent(gltfMan);
        transformMan.scale = glm::vec3(0.01f, 0.01f, 0.01f);

        /*
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

                engine.addButtonElement(
                    "button3",
                    glm::vec3(1000.0f, -900.0f, -0.5f),
                    "Move Up",
                    {glm::vec2(-20.0f, -50.0f), glm::vec2(250.0f, 50.0f)},
                    glm::vec3(0.7f, 0.0f, 0.0f),
                    glm::vec3(0.9f, 0.1f, 0.0f),
                    glm::vec3(1.0f, 0.2f, 0.2f),
                    "textures/concrete.png",
                    [&engine, object3]() {

                    });
        */
        // engine.deserializeScene("scene.bin");
        std::cout << "Loaded assets" << std::endl;
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
