#include "physicsSystem.hpp"
#include "ECSRegistry.hpp"
#include "debugDrawer.hpp"
#include <algorithm>

bool PhysicsSystem::SATCollision(Entity entityA, Entity entityB, const BoxColliderComponent &a, const BoxColliderComponent &b, glm::vec3 &mtv, glm::vec3 &collisionNormal)
{
  bool entityAHasTransform = registry.transforms.find(entityA) != registry.transforms.end();
  bool entityBHasTransform = registry.transforms.find(entityB) != registry.transforms.end();
  if (!entityAHasTransform || !entityBHasTransform)
  {
    float overlapX = std::min(a.worldMax.x, b.worldMax.x) - std::max(a.worldMin.x, b.worldMin.x);
    float overlapY = std::min(a.worldMax.y, b.worldMax.y) - std::max(a.worldMin.y, b.worldMin.y);
    float overlapZ = std::min(a.worldMax.z, b.worldMax.z) - std::max(a.worldMin.z, b.worldMin.z);

    glm::vec3 centerA = (a.worldMin + a.worldMax) * 0.5f;
    glm::vec3 centerB = (b.worldMin + b.worldMax) * 0.5f;

    if (overlapX < overlapY && overlapX < overlapZ)
      mtv = glm::vec3((centerA.x < centerB.x ? -overlapX : overlapX), 0.0f, 0.0f);
    else if (overlapY < overlapZ)
      mtv = glm::vec3(0.0f, (centerA.y < centerB.y ? -overlapY : overlapY), 0.0f);
    else
      mtv = glm::vec3(0.0f, 0.0f, (centerA.z < centerB.z ? -overlapZ : overlapZ));

    collisionNormal = getAABBCollisionNormal(overlapX, overlapY, overlapZ, centerA, centerB);

    return true; // just resolve the collision if cubes aren't transformed
  }

  const TransformComponent &ta = registry.transforms[entityA];
  const TransformComponent &tb = registry.transforms[entityB];
  auto aCorners = a.getWorldCorners(ta.position, ta.rotationZYX, ta.scale);
  auto bCorners = b.getWorldCorners(tb.position, tb.rotationZYX, tb.scale);

  glm::vec3 aAxes[3], bAxes[3];
  a.getWorldAxes(ta.rotationZYX, aAxes);
  b.getWorldAxes(tb.rotationZYX, bAxes);

  std::vector<glm::vec3> axes;

  axes.push_back(glm::normalize(aAxes[0]));
  axes.push_back(glm::normalize(aAxes[1]));
  axes.push_back(glm::normalize(aAxes[2]));

  axes.push_back(glm::normalize(bAxes[0]));
  axes.push_back(glm::normalize(bAxes[1]));
  axes.push_back(glm::normalize(bAxes[2]));

  for (int i = 0; i < 3; i++)
  {
    for (int j = 0; j < 3; j++)
    {
      glm::vec3 axis = glm::cross(aAxes[i], bAxes[j]);
      if (glm::length(axis) > 1e-6f)
        axes.push_back(glm::normalize(axis));
    }
  }

  float minOverlap = FLT_MAX;
  glm::vec3 smallestAxis;

  for (const glm::vec3 &axis : axes)
  {
    float minA = FLT_MAX, maxA = -FLT_MAX;
    float minB = FLT_MAX, maxB = -FLT_MAX;

    for (const glm::vec3 &v : aCorners)
    {
      float projection = glm::dot(v, axis);
      minA = std::min(minA, projection);
      maxA = std::max(maxA, projection);
    }

    for (const glm::vec3 &v : bCorners)
    {
      float projection = glm::dot(v, axis);
      minB = std::min(minB, projection);
      maxB = std::max(maxB, projection);
    }

    if (maxA < minB || maxB < minA)
    {
      return false;
    }
    else
    {
      float overlap = std::min(maxA, maxB) - std::max(minA, minB);
      if (overlap < minOverlap)
      {
        minOverlap = overlap;
        smallestAxis = glm::normalize(axis);
      }
    }
  }

  // This is here because if its not then if an object is fully inside another object then the whole thing breaks.
  glm::vec3 aScale = a.worldMax - a.worldMin;
  glm::vec3 bScale = b.worldMax - b.worldMin;
  float boxASmallestScale = std::min(std::min(aScale.x, aScale.y), bScale.z);
  float boxBSmallestScale = std::min(std::min(bScale.x, bScale.y), bScale.z);
  if (minOverlap > std::min(boxASmallestScale, boxBSmallestScale) / 3)
  {
    float overlapX = std::min(a.worldMax.x, b.worldMax.x) - std::max(a.worldMin.x, b.worldMin.x);
    float overlapY = std::min(a.worldMax.y, b.worldMax.y) - std::max(a.worldMin.y, b.worldMin.y);
    float overlapZ = std::min(a.worldMax.z, b.worldMax.z) - std::max(a.worldMin.z, b.worldMin.z);

    glm::vec3 centerA = (a.worldMin + a.worldMax) * 0.5f;
    glm::vec3 centerB = (b.worldMin + b.worldMax) * 0.5f;

    if (overlapX < overlapY && overlapX < overlapZ)
      mtv = glm::vec3((centerA.x < centerB.x ? -overlapX : overlapX), 0.0f, 0.0f);
    else if (overlapY < overlapZ)
      mtv = glm::vec3(0.0f, (centerA.y < centerB.y ? -overlapY : overlapY), 0.0f);
    else
      mtv = glm::vec3(0.0f, 0.0f, (centerA.z < centerB.z ? -overlapZ : overlapZ));

    collisionNormal = getAABBCollisionNormal(overlapX, overlapY, overlapZ, centerA, centerB);
    return true;
  }

  // this is what should run if they are not fully inside each other
  glm::vec3 direction = registry.transforms[entityB].position - registry.transforms[entityA].position;
  if (glm::dot(direction, smallestAxis) > 0) // make sure its >0 and not <0 or else the axis is flipped leading to a jittering effect (future referance)
    smallestAxis = -smallestAxis;

  mtv = smallestAxis * minOverlap;

  glm::vec3 bestNormal = smallestAxis;
  float bestDot = -FLT_MAX;

  for (int i = 0; i < 3; i++)
  {
    glm::vec3 normA = glm::normalize(aAxes[i]);
    glm::vec3 normB = glm::normalize(bAxes[i]);

    float dotA = std::abs(glm::dot(smallestAxis, normA));
    float dotB = std::abs(glm::dot(smallestAxis, normB));

    if (dotA > bestDot)
    {
      bestDot = dotA;
      bestNormal = (glm::dot(smallestAxis, normA) < 0) ? -normA : normA;
    }
    if (dotB > bestDot)
    {
      bestDot = dotB;
      bestNormal = (glm::dot(smallestAxis, normB) < 0) ? -normB : normB;
    }
  }

  collisionNormal = bestNormal;

  return true;
}

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
      glm::vec3 mtv;
      glm::vec3 collisionNormal;
      if (AABBOverlap(it1->second, it2->second) && SATCollision(it1->first, it2->first, it1->second, it2->second, mtv, collisionNormal))
      {
        it1->second.justUpdated = true;
        it2->second.justUpdated = true;
        resolveCollision(it1->first, it2->first, it1->second, it2->second, mtv, collisionNormal);
      }
    }
  }
}

void PhysicsSystem::resolveCollision(Entity entityA, Entity entityB, const BoxColliderComponent &a, const BoxColliderComponent &b, glm::vec3 &mtv, glm::vec3 &collisionNormal)
{

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
      registry.rigidBodies[entityB].velocity = removeVelocityAlongAxis(registry.rigidBodies[entityB].velocity, collisionNormal);
    }
    return;
  }

  if (entityBStatic)
  {
    registry.transforms[entityA].justUpdated = true;
    registry.transforms[entityA].position += mtv;
    if (entityAHasRigidBody)
    {
      registry.rigidBodies[entityA].velocity = removeVelocityAlongAxis(registry.rigidBodies[entityA].velocity, collisionNormal);
    }
    return;
  }

  registry.transforms[entityA].justUpdated = true;
  registry.transforms[entityB].justUpdated = true;
  registry.transforms[entityA].position += halfMTV;
  registry.transforms[entityB].position -= halfMTV;
  registry.rigidBodies[entityA].velocity = removeVelocityAlongAxis(registry.rigidBodies[entityA].velocity, collisionNormal);
  registry.rigidBodies[entityB].velocity = removeVelocityAlongAxis(registry.rigidBodies[entityB].velocity, collisionNormal);
}

glm::vec3 PhysicsSystem::removeVelocityAlongAxis(const glm::vec3 &velocity, const glm::vec3 &axis)
{
  glm::vec3 axisNormalized = glm::normalize(axis);
  float projection = glm::dot(velocity, axisNormalized);
  glm::vec3 vec = velocity - projection * axisNormalized;
  return velocity - projection * axisNormalized;
}

glm::vec3 PhysicsSystem::getAABBCollisionNormal(float overlapX, float overlapY, float overlapZ, glm::vec3 centerA, glm::vec3 centerB)
{
  if (overlapX < overlapY && overlapX < overlapZ)
    return glm::vec3(centerA.x < centerB.x ? -1 : 1, 0, 0);
  else if (overlapY < overlapZ)
    return glm::vec3(0, centerA.y < centerB.y ? -1 : 1, 0);
  else
    return glm::vec3(0, 0, centerA.z < centerB.z ? -1 : 1);
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