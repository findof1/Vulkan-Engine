#include "serializationUtils.hpp"
#include "engine.hpp"

void writeString(std::ofstream &out, const std::string &str)
{
  size_t len = str.size();
  out.write(reinterpret_cast<const char *>(&len), sizeof(len));
  out.write(str.data(), len);
}

void readString(std::ifstream &in, std::string &str)
{
  size_t len;
  in.read(reinterpret_cast<char *>(&len), sizeof(len));
  str.resize(len);
  in.read(&str[0], len);
}

// yes, ik I could have used templates for int and float, but its just as fast like this
void writeInt(std::ofstream &out, const int &i)
{
  out.write(reinterpret_cast<const char *>(&i), sizeof(i));
}

void readInt(std::ifstream &in, int &i)
{
  in.read(reinterpret_cast<char *>(&i), sizeof(i));
}

void writeUInt(std::ofstream &out, const uint32_t &i)
{
  out.write(reinterpret_cast<const char *>(&i), sizeof(i));
}

void readUInt(std::ifstream &in, uint32_t &i)
{
  in.read(reinterpret_cast<char *>(&i), sizeof(i));
}

void writeFloat(std::ofstream &out, const float &f)
{
  out.write(reinterpret_cast<const char *>(&f), sizeof(f));
}

void readFloat(std::ifstream &in, float &f)
{
  in.read(reinterpret_cast<char *>(&f), sizeof(f));
}

void writeBool(std::ofstream &out, const bool &b)
{
  out.write(reinterpret_cast<const char *>(&b), sizeof(b));
}

void readBool(std::ifstream &in, bool &b)
{
  in.read(reinterpret_cast<char *>(&b), sizeof(b));
}

void writeVec4(std::ofstream &out, const glm::vec4 vec)
{
  writeFloat(out, vec.x);
  writeFloat(out, vec.y);
  writeFloat(out, vec.z);
  writeFloat(out, vec.w);
}

void readVec4(std::ifstream &in, glm::vec4 &vec)
{
  readFloat(in, vec.x);
  readFloat(in, vec.y);
  readFloat(in, vec.z);
  readFloat(in, vec.w);
}

void writeVec3(std::ofstream &out, const glm::vec3 vec)
{
  writeFloat(out, vec.x);
  writeFloat(out, vec.y);
  writeFloat(out, vec.z);
}

void readVec3(std::ifstream &in, glm::vec3 &vec)
{
  readFloat(in, vec.x);
  readFloat(in, vec.y);
  readFloat(in, vec.z);
}

void writeVec2(std::ofstream &out, const glm::vec2 vec)
{
  writeFloat(out, vec.x);
  writeFloat(out, vec.y);
}

void readVec2(std::ifstream &in, glm::vec2 &vec)
{
  readFloat(in, vec.x);
  readFloat(in, vec.y);
}

using Entity = uint32_t;
void writeTransforms(std::ofstream &out, const std::unordered_map<Entity, TransformComponent> &transforms)
{
  uint32_t size = static_cast<uint32_t>(transforms.size());
  writeUInt(out, size);
  for (const auto &[key, value] : transforms)
  {
    writeUInt(out, key);
    writeVec3(out, value.position);
    writeVec3(out, value.rotationZYX);
    writeVec3(out, value.scale);
  }
}

void readTransforms(std::ifstream &in, std::unordered_map<Entity, TransformComponent> &transforms)
{
  uint32_t size;
  readUInt(in, size);
  for (uint32_t i = 0; i < size; i++)
  {
    Entity key;
    TransformComponent value;
    readUInt(in, key);
    readVec3(in, value.position);
    readVec3(in, value.rotationZYX);
    readVec3(in, value.scale);
    transforms.emplace(key, value);
  }
}

void writeMaterialData(std::ofstream &out, const MaterialData &mat)
{
  writeVec3(out, mat.albedoColor);
  writeFloat(out, mat.metallic);
  writeFloat(out, mat.roughness);
  writeFloat(out, mat.ao);
  writeFloat(out, mat.opacity);
  writeFloat(out, mat.emissiveStrength);

  writeInt(out, mat.hasAlbedoMap);
  writeInt(out, mat.hasNormalMap);
  writeInt(out, mat.hasHeightMap);
  writeInt(out, mat.hasRoughnessMap);
  writeInt(out, mat.hasMetallicMap);
  writeInt(out, mat.hasAOMap);
  writeInt(out, mat.hasEmissiveMap);

  writeInt(out, mat.isParticle);
}

void readMaterialData(std::ifstream &in, MaterialData &mat)
{
  readVec3(in, mat.albedoColor);
  readFloat(in, mat.metallic);
  readFloat(in, mat.roughness);
  readFloat(in, mat.ao);
  readFloat(in, mat.opacity);
  readFloat(in, mat.emissiveStrength);

  readInt(in, mat.hasAlbedoMap);
  readInt(in, mat.hasNormalMap);
  readInt(in, mat.hasHeightMap);
  readInt(in, mat.hasRoughnessMap);
  readInt(in, mat.hasMetallicMap);
  readInt(in, mat.hasAOMap);
  readInt(in, mat.hasEmissiveMap);

  readInt(in, mat.isParticle);
}

void writeMeshes(std::ofstream &out, const std::unordered_map<Entity, MeshComponent> &meshes)
{
  uint32_t size = static_cast<uint32_t>(meshes.size());
  writeUInt(out, size);
  for (const auto &[key, value] : meshes)
  {
    writeUInt(out, key);
    writeBool(out, value.hide);
    writeBool(out, value.loadedFromFile);
    if (value.loadedFromFile)
    {
      writeString(out, value.objPath);
      writeString(out, value.mtlPath);
    }
    else
    {
      size = static_cast<uint32_t>(value.meshes.size());
      writeUInt(out, size);
      for (const auto &mesh : value.meshes)
      {
        writeMaterialData(out, mesh.material);

        writeString(out, mesh.texPath);

        size = static_cast<uint32_t>(mesh.vertices.size());
        writeUInt(out, size);
        for (const Vertex &v : mesh.vertices)
        {
          writeVec3(out, v.pos);
          writeVec3(out, v.color);
          writeVec3(out, v.normal);
          writeVec4(out, v.texPos);
        }

        size = static_cast<uint32_t>(mesh.indices.size());
        writeUInt(out, size);
        for (uint32_t i : mesh.indices)
        {
          writeUInt(out, i);
        }
      }
    }
  }
}

void readMeshes(std::ifstream &in, Engine *engine)
{
  uint32_t meshCount;
  readUInt(in, meshCount);
  for (int i = 0; i < meshCount; i++)
  {
    Entity key;
    bool hide;
    bool loadedFromFile;
    readUInt(in, key);
    readBool(in, hide);
    readBool(in, loadedFromFile);
    if (loadedFromFile)
    {
      std::string obj;
      std::string mtl;
      readString(in, obj);
      readString(in, mtl);
      engine->addMeshComponent(key, obj, mtl);
      engine->registry.meshes.at(key).hide = hide;
    }
    else
    {
      uint32_t submeshCount;
      readUInt(in, submeshCount);
      engine->addEmptyMeshComponent(key);
      for (int j = 0; j < submeshCount; j++)
      {
        MaterialData mat;
        readMaterialData(in, mat);

        std::string texPath;
        readString(in, texPath);

        uint32_t verticesCount;
        readUInt(in, verticesCount);
        std::vector<Vertex> vertices;
        for (int k = 0; k < verticesCount; k++)
        {
          Vertex v;
          readVec3(in, v.pos);
          readVec3(in, v.color);
          readVec3(in, v.normal);
          readVec4(in, v.texPos);
          vertices.emplace_back(v);
        }

        uint32_t indicesCount;
        readUInt(in, indicesCount);
        std::vector<uint32_t> indices;
        for (int k = 0; k < indicesCount; k++)
        {
          uint32_t num;
          readUInt(in, num);
          indices.emplace_back(num);
        }
        engine->addMeshToComponent(key, mat, texPath, vertices, indices);
        engine->registry.meshes.at(key).hide = hide;
      }
    }
  }
}

void writeIdentifiers(std::ofstream &out, const std::unordered_map<std::string, Entity> &entities)
{
  uint32_t size = static_cast<uint32_t>(entities.size());
  writeUInt(out, size);
  for (const auto &[key, value] : entities)
  {
    writeString(out, key);
    writeUInt(out, value);
  }
}

void readIdentifiers(std::ifstream &in, std::unordered_map<std::string, Entity> &entities)
{
  uint32_t size;
  readUInt(in, size);
  for (int i = 0; i < size; i++)
  {
    std::string key;
    Entity value;
    readString(in, key);
    readUInt(in, value);
    entities.emplace(key, value);
  }
}

void writeBoxColliders(std::ofstream &out, const std::unordered_map<Entity, BoxColliderComponent> &boxColliders)
{
  uint32_t size = static_cast<uint32_t>(boxColliders.size());
  writeUInt(out, size);
  for (const auto &[key, value] : boxColliders)
  {
    writeUInt(out, key);
    writeVec3(out, value.localMin);
    writeVec3(out, value.localMax);
    writeBool(out, value.autoUpdate);
    if (!value.autoUpdate)
    {
      writeVec3(out, value.position);
      writeVec3(out, value.rotationZYX);
      writeVec3(out, value.scale);
    }
  }
}

void readBoxColliders(std::ifstream &in, std::unordered_map<Entity, BoxColliderComponent> &boxColliders)
{
  uint32_t size;
  readUInt(in, size);
  for (int i = 0; i < size; i++)
  {
    Entity key;
    BoxColliderComponent value;
    glm::vec3 localMin;
    glm::vec3 localMax;
    readUInt(in, key);
    readVec3(in, value.localMin);
    readVec3(in, value.localMax);
    readBool(in, value.autoUpdate);
    if (!value.autoUpdate)
    {
      readVec3(in, value.position);
      readVec3(in, value.rotationZYX);
      readVec3(in, value.scale);
    }
    boxColliders.emplace(key, value);
  }
}

void writeRigidBodies(std::ofstream &out, const std::unordered_map<Entity, RigidBodyComponent> &rigidBodies)
{
  uint32_t size = static_cast<uint32_t>(rigidBodies.size());
  writeUInt(out, size);
  for (const auto &[key, value] : rigidBodies)
  {
    writeUInt(out, key);
    writeFloat(out, value.mass);
    writeBool(out, value.useGravity);
    writeBool(out, value.isStatic);
  }
}

void readRigidBodies(std::ifstream &in, std::unordered_map<Entity, RigidBodyComponent> &rigidBodies)
{
  uint32_t size;
  readUInt(in, size);
  for (int i = 0; i < size; i++)
  {
    Entity key;
    RigidBodyComponent value;
    glm::vec3 localMin;
    glm::vec3 localMax;
    readUInt(in, key);
    readFloat(in, value.mass);
    readBool(in, value.useGravity);
    readBool(in, value.isStatic);
    rigidBodies.emplace(key, value);
  }
}