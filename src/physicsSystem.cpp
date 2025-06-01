#include "physicsSystem.hpp"
#include "ECSRegistry.hpp"
#include "debugDrawer.hpp"
#include <algorithm>

void PhysicsSystem::update(float deltaTime)
{
  auto &rigidBodies = registry.rigidBodies;
  for (auto rigidBody = rigidBodies.begin(); rigidBody != rigidBodies.end(); ++rigidBody)
  {
    if (rigidBody->second.isStatic)
    {
      continue;
    }
    rigidBody->second.integrate(deltaTime);
    auto &transform = registry.transforms[rigidBody->first];
    rigidBody->second.applyVelocity(transform, deltaTime);
  }

  auto &boxColliders = registry.boxColliders;

  for (auto it1 = boxColliders.begin(); it1 != boxColliders.end(); ++it1)
  {
    drawAABB(it1->second, glm::vec3(1.0f, 0.0f, 0.0f));

    if (it1->second.justUpdated == false)
      continue;
    it1->second.justUpdated = false;
    auto it2 = it1;
    ++it2;
    for (; it2 != boxColliders.end(); ++it2)
    {
      if (AABBOverlap(it1->second, it2->second))
      {
        it1->second.justUpdated = true;
        it2->second.justUpdated = true;
        resolveCollision(it1->first, it2->first, it1->second, it2->second);
      }
    }
  }
}

void PhysicsSystem::resolveCollision(Entity entityA, Entity entityB, const BoxColliderComponent &a, const BoxColliderComponent &b)
{
  float overlapX = std::min(a.worldMax.x, b.worldMax.x) - std::max(a.worldMin.x, b.worldMin.x);
  float overlapY = std::min(a.worldMax.y, b.worldMax.y) - std::max(a.worldMin.y, b.worldMin.y);
  float overlapZ = std::min(a.worldMax.z, b.worldMax.z) - std::max(a.worldMin.z, b.worldMin.z);

  glm::vec3 centerA = (a.worldMin + a.worldMax) * 0.5f;
  glm::vec3 centerB = (b.worldMin + b.worldMax) * 0.5f;

  glm::vec3 mtv;
  if (overlapX < overlapY && overlapX < overlapZ)
    mtv = glm::vec3((centerA.x < centerB.x ? -overlapX : overlapX), 0.0f, 0.0f);
  else if (overlapY < overlapZ)
    mtv = glm::vec3(0.0f, (centerA.y < centerB.y ? -overlapY : overlapY), 0.0f);
  else
    mtv = glm::vec3(0.0f, 0.0f, (centerA.z < centerB.z ? -overlapZ : overlapZ));
  glm::vec3 halfMTV = mtv * 0.5f;

  bool bothEntitiesHaveTransforms = registry.transforms.find(entityA) != registry.transforms.end() && registry.transforms.find(entityB) != registry.transforms.end();
  bool entityAHasRigidBody = registry.rigidBodies.find(entityA) != registry.rigidBodies.end();
  bool entityBHasRigidBody = registry.rigidBodies.find(entityB) != registry.rigidBodies.end();
  if (!(bothEntitiesHaveTransforms && (entityAHasRigidBody || entityBHasRigidBody)))
  {
    return;
  }

  bool entityAStatic = registry.rigidBodies[entityA].isStatic || !entityAHasRigidBody;
  bool entityBStatic = registry.rigidBodies[entityB].isStatic || !entityBHasRigidBody;

  if (entityAStatic && entityBStatic)
  {
    return;
  }

  if (entityAStatic)
  {
    registry.transforms[entityB].justUpdated = true;
    registry.transforms[entityB].position -= mtv;
    if (entityBHasRigidBody)
    {
      registry.rigidBodies[entityB].velocity = glm::vec3(0);
    }
    return;
  }

  if (entityBStatic)
  {
    registry.transforms[entityA].justUpdated = true;
    registry.transforms[entityA].position += mtv;
    if (entityAHasRigidBody)
    {
      registry.rigidBodies[entityA].velocity = glm::vec3(0);
    }
    return;
  }

  registry.transforms[entityA].justUpdated = true;
  registry.transforms[entityB].justUpdated = true;
  registry.transforms[entityA].position += halfMTV;
  registry.transforms[entityB].position -= halfMTV;
  registry.rigidBodies[entityA].velocity = glm::vec3(0);
  registry.rigidBodies[entityB].velocity = glm::vec3(0);
}

bool PhysicsSystem::AABBOverlap(const BoxColliderComponent &a, const BoxColliderComponent &b)
{
  return (a.worldMin.x <= b.worldMax.x && a.worldMax.x >= b.worldMin.x) &&
         (a.worldMin.y <= b.worldMax.y && a.worldMax.y >= b.worldMin.y) &&
         (a.worldMin.z <= b.worldMax.z && a.worldMax.z >= b.worldMin.z);
}

void PhysicsSystem::drawAABB(const BoxColliderComponent &box, const glm::vec3 &color)
{
  glm::vec3 min = box.worldMin;
  glm::vec3 max = box.worldMax;

  std::vector<glm::vec3> corners = {
      {min.x, min.y, min.z}, {max.x, min.y, min.z}, {max.x, max.y, min.z}, {min.x, max.y, min.z}, {min.x, min.y, max.z}, {max.x, min.y, max.z}, {max.x, max.y, max.z}, {min.x, max.y, max.z}};

  auto addLine = [&](glm::vec3 a, glm::vec3 b)
  {
    debugDrawer->debugLines.push_back(Vertex{a, color});
    debugDrawer->debugLines.push_back(Vertex{b, color});
  };

  addLine(corners[0], corners[1]);
  addLine(corners[1], corners[2]);
  addLine(corners[2], corners[3]);
  addLine(corners[3], corners[0]);

  addLine(corners[4], corners[5]);
  addLine(corners[5], corners[6]);
  addLine(corners[6], corners[7]);
  addLine(corners[7], corners[4]);

  addLine(corners[0], corners[4]);
  addLine(corners[1], corners[5]);
  addLine(corners[2], corners[6]);
  addLine(corners[3], corners[7]);
}