#include "gameObject.hpp"
#include "bufferManager.hpp"
#include "descriptorManager.hpp"
#include "textureManager.hpp"
#include "renderer.hpp"
#include <vulkan/vulkan.h>
#include <tiny_obj_loader.h>
#include <glm/gtc/quaternion.hpp>

GameObject::GameObject(Renderer &renderer, const glm::vec3 &pos, const glm::vec3 &scale, const glm::vec3 &rotationZYX) : pos(pos), scale(scale), rotationZYX(rotationZYX)
{
}

void GameObject::draw(Renderer *renderer, int currentFrame, glm::mat4 view, glm::mat4 projectionMatrix, VkCommandBuffer commandBuffer)
{
  if (hide)
    return;

  glm::mat4 transformation = glm::mat4(1.0f);
  transformation = glm::translate(transformation, pos);
  transformation = glm::rotate(transformation, glm::radians(rotationZYX.x), glm::vec3(0.0f, 0.0f, 1.0f));
  transformation = glm::rotate(transformation, glm::radians(rotationZYX.y), glm::vec3(0.0f, 1.0f, 0.0f));
  transformation = glm::rotate(transformation, glm::radians(rotationZYX.z), glm::vec3(1.0f, 0.0f, 0.0f));
  transformation = glm::scale(transformation, scale);

  for (auto &mesh : meshes)
  {
    mesh.draw(renderer, currentFrame, transformation, view, projectionMatrix, commandBuffer);
  }
}

void GameObject::addMesh(Renderer &renderer, int *nextRenderingId, MaterialData &material, std::string texturePath, const std::vector<Vertex> &meshVertices, const std::vector<uint32_t> &meshIndices)
{
  if (texturePath.empty())
  {
    texturePath = "models/couch/diffuse.png";
  }

  Mesh mesh(renderer, nextRenderingId, material, meshVertices, meshIndices);
  mesh.initGraphics(renderer, texturePath);
  meshes.emplace_back(std::move(mesh));
}

void GameObject::loadModel(Renderer &renderer, int *nextRenderingId, const std::string objPath, const std::string mtlPath)
{
  tinyobj::attrib_t attrib;
  std::vector<tinyobj::shape_t> shapes;
  std::vector<tinyobj::material_t> materials;
  std::string err;

  bool ret = tinyobj::LoadObj(
      &attrib,
      &shapes,
      &materials,
      &err,
      objPath.c_str(), mtlPath.c_str());

  if (!ret)
  {
    throw std::runtime_error("Failed to load OBJ: " + err);
  }

  for (const auto &shape : shapes)
  {
    std::vector<Vertex> meshVertices;
    std::vector<uint32_t> meshIndices;
    for (const auto &index : shape.mesh.indices)
    {
      Vertex vertex{};

      size_t vIdx = 3 * index.vertex_index;
      if (vIdx + 2 >= attrib.vertices.size())
      {
        throw std::runtime_error("Vertex index out of range in model: " + objPath);
      }
      vertex.pos = {
          attrib.vertices[vIdx + 0],
          attrib.vertices[vIdx + 1],
          attrib.vertices[vIdx + 2]};

      if (!attrib.normals.empty() && (3 * index.normal_index + 2 < attrib.normals.size()))
      {
        vertex.normal = {
            attrib.normals[3 * index.normal_index + 0],
            attrib.normals[3 * index.normal_index + 1],
            attrib.normals[3 * index.normal_index + 2]};
      }
      else
      {
        vertex.normal = {0.0f, 0.0f, 0.0f};
      }

      if (!attrib.texcoords.empty() && (2 * index.texcoord_index + 1 < attrib.texcoords.size()))
      {
        vertex.texPos = {
            attrib.texcoords[2 * index.texcoord_index + 0],
            1.0f - attrib.texcoords[2 * index.texcoord_index + 1]};
      }
      else
      {
        vertex.texPos = {0.0f, 0.0f};
      }

      vertex.color = {-1.0f, -1.0f, -1.0f};

      meshVertices.push_back(vertex);
      meshIndices.push_back(static_cast<uint32_t>(meshIndices.size()));
    }
    MaterialData material;
    material.diffuseColor = {0.5f, 0.5f, 0.5f};
    material.hasTexture = 1;
    std::string texturePath = "";
    std::string fullPath = "";

    if (!materials.empty() && !shape.mesh.material_ids.empty())
    {
      int matId = shape.mesh.material_ids[0];
      if (matId < 0 || matId >= static_cast<int>(materials.size()))
      {
        throw std::runtime_error("Material index out of range in model: " + objPath);
      }

      if (!materials[matId].diffuse_texname.empty())
      {
        texturePath = materials[matId].diffuse_texname;
        fullPath = mtlPath + texturePath;
      }

      material.diffuseColor = {
          materials[matId].diffuse[0],
          materials[matId].diffuse[1],
          materials[matId].diffuse[2]};

      material.specularColor = {
          materials[matId].specular[0],
          materials[matId].specular[1],
          materials[matId].specular[2]};

      material.ambientColor = {
          materials[matId].ambient[0],
          materials[matId].ambient[1],
          materials[matId].ambient[2]};

      material.shininess = materials[matId].shininess;
      material.emissionColor = {
          materials[matId].emission[0],
          materials[matId].emission[1],
          materials[matId].emission[2]};

      material.refractiveIndex = materials[matId].ior;
      material.illuminationModel = materials[matId].illum;

      material.opacity = (materials[matId].diffuse[3] != 0) ? materials[matId].diffuse[3] : 1.0f;
    }

    if (texturePath.empty())
    {
      material.hasTexture = 0;
      fullPath = "models/couch/diffuse.png";
    }

    Mesh mesh(renderer, nextRenderingId, material, meshVertices, meshIndices);

    try
    {
      mesh.initGraphics(renderer, fullPath);
    }
    catch (const std::exception &e)
    {
      throw std::runtime_error("Mesh graphics initialization failed: " + std::string(e.what()));
    }
    meshes.push_back(std::move(mesh));
  }
}

void GameObject::cleanupMeshes(VkDevice device, Renderer &renderer)
{
  for (auto &mesh : meshes)
  {
    mesh.cleanup(device, renderer);
  }
}

void GameObject::setScale(const glm::vec3 &newScale)
{
  scale = newScale;
}

void GameObject::setPosition(const glm::vec3 &newPosition)
{
  pos = newPosition;
}