#pragma once
#include <vector>
#include "vertex.h"

#ifdef BUILD_ENGINE_DLL

#ifndef ENGINE_API
#define ENGINE_API __declspec(dllexport)
#endif

#else

#ifndef ENGINE_API
#define ENGINE_API __declspec(dllimport)
#endif

#endif

extern std::vector<Vertex> cubeVertices;
extern std::vector<Vertex> cubeVerticesNoNormals;
extern std::vector<uint32_t> cubeIndices;
extern std::vector<uint32_t> skyBoxIndices;
extern std::vector<Vertex> squareVertices;
extern std::vector<uint32_t> squareIndices;