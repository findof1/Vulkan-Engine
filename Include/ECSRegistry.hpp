#pragma once
#include <unordered_map>
#include <cstdint>
#include "components.hpp"

#ifdef BUILD_ENGINE_DLL

#ifndef ENGINE_API
#define ENGINE_API __declspec(dllexport)
#endif

#else

#ifndef ENGINE_API
#define ENGINE_API __declspec(dllimport)
#endif

#endif

using Entity = uint32_t;

class ENGINE_API ECSRegistry
{
private:
  Entity nextEntity = 1;

public:
  std::unordered_map<Entity, TransformComponent> transforms;
  std::unordered_map<Entity, SkeletonComponent> animationSkeletons;
  std::unordered_map<Entity, AnimatedMeshComponent> animatedMeshes;
  std::unordered_map<Entity, AnimationComponent> animationComponents;
  std::unordered_map<Entity, ParentComponent> parents;
  std::unordered_map<Entity, MeshComponent> meshes;
  std::unordered_map<Entity, PointLightComponent> pointLights;
  std::unordered_map<Entity, BoxColliderComponent> boxColliders;
  std::unordered_map<Entity, RigidBodyComponent> rigidBodies;
  std::unordered_map<std::string, Entity> entities;
  Entity selected = -1;

  Entity createEntity(std::string name)
  {
    uint32_t e = nextEntity++;
    entities.emplace(name, e);
    return e;
  }

  Entity getEntity(std::string name)
  {
    if (entities.find(name) != entities.end())
      return entities.at(name);
    return -1;
  }

  void resetNextEntity()
  {
    nextEntity = 1;
  }

  void destroyEntity(Entity e)
  {
    transforms.erase(e);
    meshes.erase(e);
    boxColliders.erase(e);
  }

  // only use these for serialization stuff :)
  void setNextEntity(Entity newEntity)
  {
    nextEntity = newEntity;
  }

  const int getNextEntity()
  {
    return nextEntity;
  }
};
