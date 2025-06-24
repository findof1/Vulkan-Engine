#pragma once
#include <vector>
#include <glm/glm.hpp>
#include "components.hpp"

using Entity = uint32_t;
class ECSRegistry;
class VulkanDebugDrawer;
class PhysicsSystem
{
public:
  glm::vec3 gravity = glm::vec3(0.0f, -9.81f, 0.0f);
  ECSRegistry &registry;
  VulkanDebugDrawer *debugDrawer = nullptr;
  bool doDebugDraw = false;
  PhysicsSystem(ECSRegistry &registry) : registry(registry)
  {
  }

  void update(float deltaTime);

private:
  void handleCollisions();

  bool AABBOverlap(const BoxColliderComponent &a, const BoxColliderComponent &b);

  void drawAABB(const BoxColliderComponent &box, const glm::vec3 &color);

  void resolveCollision(Entity entityA, Entity entityB, const BoxColliderComponent &a, const BoxColliderComponent &b, glm::vec3 &mtv, glm::vec3 &collisionNormal);
  glm::vec3 removeVelocityAlongAxis(const glm::vec3 &velocity, const glm::vec3 &axis);
  glm::vec3 getAABBCollisionNormal(float overlapX, float overlapY, float overlapZ, glm::vec3 centerA, glm::vec3 centerB);

  bool SATCollision(Entity entityA, Entity entityB, const BoxColliderComponent &a, const BoxColliderComponent &b, glm::vec3 &mtv, glm::vec3 &collisionNormal);
};