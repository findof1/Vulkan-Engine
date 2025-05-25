#include "physicsSystem.hpp"
#include "ECSRegistry.hpp"
#include "debugDrawer.hpp"

void PhysicsSystem::update(float deltaTime)
{
  auto &boxColliders = registry.boxColliders;

  for (auto it1 = boxColliders.begin(); it1 != boxColliders.end(); ++it1)
  {
    drawAABB(it1->second, glm::vec3(1.0f, 0.0f, 0.0f));
    auto it2 = it1;
    ++it2;
    for (; it2 != boxColliders.end(); ++it2)
    {
      if (AABBOverlap(it1->second, it2->second))
      {
        std::cout << "Collision detected between entities "
                  << it1->first << " and " << it2->first << std::endl;
      }
    }
  }
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