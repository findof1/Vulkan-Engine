#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include "mesh.hpp"
#include "animatedMesh.hpp"
#include "tiny_gltf.h"

#ifdef BUILD_ENGINE_DLL

#ifndef ENGINE_API
#define ENGINE_API __declspec(dllexport)
#endif

#else

#ifndef ENGINE_API
#define ENGINE_API __declspec(dllimport)
#endif

#endif

struct ENGINE_API TransformComponent
{
  glm::vec3 position;
  glm::vec3 rotationZYX;
  glm::vec3 scale = glm::vec3(1.0f);
  bool justUpdated = true;
};

struct ENGINE_API PointLightComponent
{
  glm::vec3 color;
  int intensity;
};

struct ENGINE_API MeshComponent
{
  bool hide = false;
  std::vector<Mesh> meshes;

  bool loadedFromFile = false;
  std::string objPath;
  std::string mtlPath;
};

struct ENGINE_API ParentComponent
{
  int parent;
};

struct ENGINE_API AnimatedMeshComponent
{
  bool hide = false;
  std::vector<AnimatedMesh> meshes;
};

struct ENGINE_API SkeletonComponent
{
  tinygltf::Model *model;
  const tinygltf::Node *node;
  std::vector<int> jointNodeIndices;
  std::vector<glm::mat4> inverseBindMats;
  std::unordered_map<int, int> nodeToParent;
  std::array<glm::mat4, 100> finalBoneMatrices;
  bool computeFinalBoneMatrices = true;

  struct NodeLocalTransform
  {
    glm::vec3 translation = glm::vec3(0.0f);
    glm::vec4 rotation = glm::vec4(1, 0, 0, 0);
    glm::vec3 scale = glm::vec3(1.0f);
  };
  std::vector<NodeLocalTransform> nodeTransforms;
};

struct ENGINE_API AnimationSampler
{
  std::vector<float> inputTimes;
  std::vector<glm::vec4> outputValues;
  std::string interpolation;
};

struct ENGINE_API AnimationChannel
{
  int nodeIndex;
  std::string path;
  int samplerIndex;
};

struct ENGINE_API Animation
{
  std::string name;
  std::vector<AnimationSampler> samplers;
  std::vector<AnimationChannel> channels;
  float duration = 0.0f;
};

struct ENGINE_API AnimationComponent
{
  std::vector<Animation> animations;
  int currentAnimationIndex = -1;
  float currentTime = 0.0f;
  bool playing = false;
};

struct ENGINE_API BoxColliderComponent
{
  bool justUpdated = true;

  glm::vec3 localMin = glm::vec3(-0.5f);
  glm::vec3 localMax = glm::vec3(0.5f);

  glm::vec3 worldMin;
  glm::vec3 worldMax;

  bool autoUpdate = false;

  // serialization stuff
  glm::vec3 position;
  glm::vec3 rotationZYX;
  glm::vec3 scale;

  void updateWorldAABB(const glm::vec3 &positionIn, const glm::vec3 &rotationZYXIn, const glm::vec3 &scaleIn)
  {
    position = positionIn;
    rotationZYX = rotationZYXIn;
    scale = scaleIn;

    justUpdated = true;
    glm::quat rotation = glm::quat(glm::radians(rotationZYX));

    glm::vec3 scaledMin = localMin * scale;
    glm::vec3 scaledMax = localMax * scale;

    glm::vec3 corners[8] = {
        glm::vec3(scaledMin.x, scaledMin.y, scaledMin.z),
        glm::vec3(scaledMax.x, scaledMin.y, scaledMin.z),
        glm::vec3(scaledMin.x, scaledMax.y, scaledMin.z),
        glm::vec3(scaledMax.x, scaledMax.y, scaledMin.z),
        glm::vec3(scaledMin.x, scaledMin.y, scaledMax.z),
        glm::vec3(scaledMax.x, scaledMin.y, scaledMax.z),
        glm::vec3(scaledMin.x, scaledMax.y, scaledMax.z),
        glm::vec3(scaledMax.x, scaledMax.y, scaledMax.z)};

    worldMin = glm::vec3(FLT_MAX);
    worldMax = glm::vec3(-FLT_MAX);

    for (const auto &corner : corners)
    {
      glm::vec3 rotatedCorner = rotation * corner;
      glm::vec3 worldCorner = rotatedCorner + position;

      worldMin = glm::min(worldMin, worldCorner);
      worldMax = glm::max(worldMax, worldCorner);
    }
  }

  std::vector<glm::vec3> getWorldCorners(const glm::vec3 &position, const glm::vec3 &rotationZYX, const glm::vec3 &scale) const
  {
    glm::quat rotation = glm::quat(glm::radians(rotationZYX));
    glm::vec3 scaledMin = localMin * scale;
    glm::vec3 scaledMax = localMax * scale;

    glm::vec3 corners[8] = {
        glm::vec3(scaledMin.x, scaledMin.y, scaledMin.z),
        glm::vec3(scaledMax.x, scaledMin.y, scaledMin.z),
        glm::vec3(scaledMin.x, scaledMax.y, scaledMin.z),
        glm::vec3(scaledMax.x, scaledMax.y, scaledMin.z),
        glm::vec3(scaledMin.x, scaledMin.y, scaledMax.z),
        glm::vec3(scaledMax.x, scaledMin.y, scaledMax.z),
        glm::vec3(scaledMin.x, scaledMax.y, scaledMax.z),
        glm::vec3(scaledMax.x, scaledMax.y, scaledMax.z),
    };

    std::vector<glm::vec3> worldCorners;
    for (const glm::vec3 &corner : corners)
    {
      glm::vec3 rotatedCorner = rotation * corner;
      worldCorners.push_back(rotatedCorner + position);
    }
    return worldCorners;
  }

  void getWorldAxes(const glm::vec3 &rotationZYX, glm::vec3 axes[3]) const
  {
    glm::quat rotation = glm::quat(glm::radians(rotationZYX));
    axes[0] = rotation * glm::vec3(1, 0, 0);
    axes[1] = rotation * glm::vec3(0, 1, 0);
    axes[2] = rotation * glm::vec3(0, 0, 1);
  }

  bool containsPoint(const glm::vec3 &point) const
  {
    return (point.x >= worldMin.x && point.x <= worldMax.x) &&
           (point.y >= worldMin.y && point.y <= worldMax.y) &&
           (point.z >= worldMin.z && point.z <= worldMax.z);
  }
};

struct ENGINE_API RigidBodyComponent
{
  glm::vec3 velocity = glm::vec3(0.0f);

  glm::vec3 acceleration = glm::vec3(0.0f);

  float mass = 1.0f;

  bool useGravity = true;

  bool isStatic = false;

  void applyForce(const glm::vec3 &force)
  {
    if (isStatic || mass <= 0.0f)
      return;
    acceleration += force / mass;
  }

  void integrate(float deltaTime)
  {
    if (isStatic)
      return;

    if (useGravity)
    {
      glm::vec3 gravity(0.0f, -9.81f, 0.0f);
      acceleration += gravity;
    }

    velocity += acceleration * deltaTime;

    acceleration = glm::vec3(0.0f);
  }

  void applyVelocity(TransformComponent &transform, float deltaTime)
  {
    if (isStatic)
      return;
    transform.position += velocity * deltaTime;
    transform.justUpdated = true;
  }
};