#pragma once
#include <unordered_map>
#include <cstdint>

using Entity = uint32_t;
class ECSRegistry;
class SerializationManager
{
public:
  ECSRegistry &registry;
};
