#pragma once
#include <fstream>
#include <unordered_map>
#include <string>
#include <unordered_map>
#include "components.hpp"

// yes, ik I could have used templates for some of these, but its just as fast like this
void writeString(std::ofstream &out, const std::string &str);
void readString(std::ifstream &in, std::string &str);

void writeInt(std::ofstream &out, const int &i);
void readInt(std::ifstream &in, int &i);

void writeUInt(std::ofstream &out, const uint32_t &i);
void readUInt(std::ifstream &in, uint32_t &i);

void writeFloat(std::ofstream &out, const float &f);
void readFloat(std::ifstream &in, float &f);

void writeBool(std::ofstream &out, const bool &b);
void readBool(std::ifstream &in, bool &b);

void writeVec4(std::ofstream &out, const glm::vec4 vec);
void readVec4(std::ifstream &in, glm::vec4 &vec);

void writeVec3(std::ofstream &out, const glm::vec3 vec);
void readVec3(std::ifstream &in, glm::vec3 &vec);

void writeVec2(std::ofstream &out, const glm::vec2 vec);
void readVec2(std::ifstream &in, glm::vec2 &vec);

using Entity = uint32_t;
void writeTransforms(std::ofstream &out, const std::unordered_map<Entity, TransformComponent> &transforms);
void readTransforms(std::ifstream &in, std::unordered_map<Entity, TransformComponent> &transforms);

void writeMaterialData(std::ofstream &out, const MaterialData &mat);
void readMaterialData(std::ifstream &in, MaterialData &mat);

void writeMeshes(std::ofstream &out, const std::unordered_map<Entity, MeshComponent> &meshes);
// engine is only used for making the mesh components with nice functions that are easy to use instead of making them elsewhere
class Engine;
void readMeshes(std::ifstream &in, Engine *engine);

void writeIdentifiers(std::ofstream &out, const std::unordered_map<std::string, Entity> &entities);
void readIdentifiers(std::ifstream &in, std::unordered_map<std::string, Entity> &entities);

void writeBoxColliders(std::ofstream &out, const std::unordered_map<Entity, BoxColliderComponent> &boxColliders);
void readBoxColliders(std::ifstream &in, std::unordered_map<Entity, BoxColliderComponent> &boxColliders);

void writeRigidBodies(std::ofstream &out, const std::unordered_map<Entity, RigidBodyComponent> &rigidBodies);
void readRigidBodies(std::ifstream &in, std::unordered_map<Entity, RigidBodyComponent> &rigidBodies);