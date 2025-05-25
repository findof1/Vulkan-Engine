#pragma once
#include <unordered_map>
#include <cstdint>
#include "components.hpp"

using Entity = uint32_t;

class ECSRegistry
{
private:
  Entity nextEntity = 1;

public:
  std::unordered_map<Entity, TransformComponent> transforms;
  std::unordered_map<Entity, MeshComponent> meshes;
  std::unordered_map<Entity, BoxColliderComponent> boxColliders;
  std::unordered_map<std::string, Entity> entities;

  Entity createEntity(std::string name)
  {
    uint32_t e = nextEntity++;
    entities.emplace(name, e);
    return e;
  }

  Entity getEntity(std::string name)
  {
    return entities.at(name);
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
};
