#pragma once

#include <fstream>
#include <unordered_map>
#include <string>
#include <unordered_map>
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

// yes, ik I could have used templates for some of these, but its just as fast like this
ENGINE_API void writeString(std::ofstream &out, const std::string &str);
ENGINE_API void readString(std::ifstream &in, std::string &str);

ENGINE_API void writeInt(std::ofstream &out, const int &i);
ENGINE_API void readInt(std::ifstream &in, int &i);

ENGINE_API void writeUInt(std::ofstream &out, const uint32_t &i);
ENGINE_API void readUInt(std::ifstream &in, uint32_t &i);

ENGINE_API void writeFloat(std::ofstream &out, const float &f);
ENGINE_API void readFloat(std::ifstream &in, float &f);

ENGINE_API void writeBool(std::ofstream &out, const bool &b);
ENGINE_API void readBool(std::ifstream &in, bool &b);

ENGINE_API void writeVec4(std::ofstream &out, const glm::vec4 vec);
ENGINE_API void readVec4(std::ifstream &in, glm::vec4 &vec);

ENGINE_API void writeVec3(std::ofstream &out, const glm::vec3 vec);
ENGINE_API void readVec3(std::ifstream &in, glm::vec3 &vec);

ENGINE_API void writeVec2(std::ofstream &out, const glm::vec2 vec);
ENGINE_API void readVec2(std::ifstream &in, glm::vec2 &vec);

using Entity = uint32_t;
ENGINE_API void writeTransforms(std::ofstream &out, const std::unordered_map<Entity, TransformComponent> &transforms);
ENGINE_API void readTransforms(std::ifstream &in, std::unordered_map<Entity, TransformComponent> &transforms);

ENGINE_API void writeMaterialData(std::ofstream &out, const MaterialData &mat);
ENGINE_API void readMaterialData(std::ifstream &in, MaterialData &mat);

ENGINE_API void writeMeshes(std::ofstream &out, const std::unordered_map<Entity, MeshComponent> &meshes);
// engine is only used for making the mesh components with nice functions that are easy to use instead of making them elsewhere
class Engine;
ENGINE_API void readMeshes(std::ifstream &in, Engine *engine);

ENGINE_API void writeIdentifiers(std::ofstream &out, const std::unordered_map<std::string, Entity> &entities);
ENGINE_API void readIdentifiers(std::ifstream &in, std::unordered_map<std::string, Entity> &entities);

ENGINE_API void writeBoxColliders(std::ofstream &out, const std::unordered_map<Entity, BoxColliderComponent> &boxColliders);
ENGINE_API void readBoxColliders(std::ifstream &in, std::unordered_map<Entity, BoxColliderComponent> &boxColliders);

ENGINE_API void writeRigidBodies(std::ofstream &out, const std::unordered_map<Entity, RigidBodyComponent> &rigidBodies);
ENGINE_API void readRigidBodies(std::ifstream &in, std::unordered_map<Entity, RigidBodyComponent> &rigidBodies);