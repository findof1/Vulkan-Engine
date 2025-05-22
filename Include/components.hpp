#pragma once
#include <glm/glm.hpp>
#include "mesh.hpp"

struct TransformComponent
{
  glm::vec3 position;
  glm::vec3 rotationZYX;
  glm::vec3 scale = glm::vec3(1.0f);
};

struct MeshComponent
{
  bool hide = false;
  std::vector<Mesh> meshes;
};