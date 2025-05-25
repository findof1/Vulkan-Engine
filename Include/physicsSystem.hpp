#pragma once
#include <vector>
#include <glm/glm.hpp>
#include "components.hpp"

class ECSRegistry;
class VulkanDebugDrawer;
class PhysicsSystem
{
public:
  glm::vec3 gravity = glm::vec3(0.0f, -9.81f, 0.0f);
  ECSRegistry &registry;
  VulkanDebugDrawer *debugDrawer = nullptr;
  PhysicsSystem(ECSRegistry &registry) : registry(registry)
  {
  }

  void update(float deltaTime);

private:
  // void applyPhysics(float deltaTime);

  void handleCollisions();

  bool AABBOverlap(const BoxColliderComponent &a, const BoxColliderComponent &b);

  void drawAABB(const BoxColliderComponent &box, const glm::vec3 &color);

  // void resolveCollision(Entity& e1, Entity& e2);
};