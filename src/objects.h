#ifndef OBJECTS_H
#define OBJECTS_H

#include "Angel.h"
#include <string>

struct Vertex {
    vec4 position;
    vec3 normal;
    vec2 texCoord;
};

extern std::vector<Vertex> sphereData;

bool loadBunnyModel(const std::string& filename);
void calculateBunnyNormals();
void initCube();
void initSphere(int subdivisions);

#endif
