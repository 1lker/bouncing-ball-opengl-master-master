#include "objects.h"
#include "Globals.h"
#include <fstream>
#include <iostream>
#include <vector>
#include <cmath>
#include <cfloat>
#include <algorithm>
#include <functional>
std::vector<Vertex> sphereData;

// Helper to convert vec4 -> vec3
inline vec3 toVec3(const vec4& v) {
    return vec3(v.x, v.y, v.z);
}

// Initialize cube
void initCube() {
    vec4 vertices[8] = {
        vec4(-0.5, -0.5,  0.5, 1.0),
        vec4( 0.5, -0.5,  0.5, 1.0),
        vec4( 0.5,  0.5,  0.5, 1.0),
        vec4(-0.5,  0.5,  0.5, 1.0),
        vec4(-0.5, -0.5, -0.5, 1.0),
        vec4( 0.5, -0.5, -0.5, 1.0),
        vec4( 0.5,  0.5, -0.5, 1.0),
        vec4(-0.5,  0.5, -0.5, 1.0)
    };
    
    int indices[36] = {
        0,1,2, 0,2,3,    // front
        1,5,6, 1,6,2,    // right
        5,4,7, 5,7,6,    // back
        4,0,3, 4,3,7,    // left
        3,2,6, 3,6,7,    // top
        4,5,1, 4,1,0     // bottom
    };
    
    cubeVertices.clear();
    cubeNormals.clear();
    
    for (int i = 0; i < 36; i += 3) {
        vec4 a = vertices[indices[i]];
        vec4 b = vertices[indices[i+1]];
        vec4 c = vertices[indices[i+2]];
        vec3 ab = toVec3(b - a);
        vec3 ac = toVec3(c - a);
        vec3 normal = normalize(cross(ab, ac));
        
        cubeVertices.push_back(a);
        cubeVertices.push_back(b);
        cubeVertices.push_back(c);
        
        cubeNormals.push_back(normal);
        cubeNormals.push_back(normal);
        cubeNormals.push_back(normal);
    }
    
    // 36 indices => 12 triangles => we store them as 36 vertices => total is 36
    numCubeVertices = (int)cubeVertices.size();
    std::cout << "Cube initialized with " << numCubeVertices << " vertices\n";
}

// Initialize sphere
void initSphere(int subdivisions) {
    sphereData.clear();
    // std::vector<Vertex> sphereData;

    vec4 octaVerts[6] = {
        vec4(0.0,  1.0,  0.0, 1.0),
        vec4(0.0, -1.0,  0.0, 1.0),
        vec4(1.0,  0.0,  0.0, 1.0),
        vec4(-1.0, 0.0,  0.0, 1.0),
        vec4(0.0,  0.0,  1.0, 1.0),
        vec4(0.0,  0.0, -1.0, 1.0)
    };
    
    std::vector<int> faces = {
        0,2,4, 0,4,3, 0,3,5, 0,5,2,
        1,4,2, 1,3,4, 1,5,3, 1,2,5
    };
    sphereVertices.clear();
    sphereNormals.clear();
    
    auto normalizeVertex = [&](const vec4& v) {
        float len = std::sqrt(v.x*v.x + v.y*v.y + v.z*v.z);
        if (len < 1e-5f) len = 1e-5f;
        return vec4(v.x/len, v.y/len, v.z/len, 1.0f);
    };
    
    std::function<void(const vec4&, const vec4&, const vec4&, int)> subdivide =
    [&](const vec4& a, const vec4& b, const vec4& c, int depth) {
        if (depth == 0) {
            // sphereVertices.push_back(a);
            // sphereVertices.push_back(b);
            // sphereVertices.push_back(c);
            auto pushVertex = [](const vec4& pos) {
                vec3 n = normalize(vec3(pos.x, pos.y, pos.z));
                float theta = acos(n.y);                // polar angle
                float phi = atan2(n.z, n.x);            // azimuth
                float u = (phi + M_PI) / (2 * M_PI);
                float v = theta / M_PI;
            
                Vertex vert;
                vert.position = pos;
                vert.normal = n;
                vert.texCoord = vec2(u, v);
                sphereData.push_back(vert);
            };
            
            pushVertex(a);
            pushVertex(b);
            pushVertex(c);
        } else {
            vec4 ab = normalizeVertex(a + b);
            vec4 bc = normalizeVertex(b + c);
            vec4 ca = normalizeVertex(c + a);
            subdivide(a, ab, ca, depth-1);
            subdivide(ab, b, bc, depth-1);
            subdivide(ca, bc, c, depth-1);
            subdivide(ab, bc, ca, depth-1);
        }
    };
    
    for (size_t i = 0; i < faces.size(); i += 3) {
        vec4 v1 = octaVerts[faces[i]];
        vec4 v2 = octaVerts[faces[i+1]];
        vec4 v3 = octaVerts[faces[i+2]];
        subdivide(v1, v2, v3, subdivisions);
    }
    
    for (auto& pos : sphereVertices) {
        sphereNormals.push_back(normalize(vec3(pos.x, pos.y, pos.z)));
    }
    // numSphereVertices = (int)sphereVertices.size();
    numSphereVertices = static_cast<int>(sphereData.size());  
    std::cout << "Sphere initialized with " << numSphereVertices << " vertices\n";
}

// Load bunny
bool loadBunnyModel(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Failed to open bunny file: " << filename << "\n";
        return false;
    }
    std::string header;
    file >> header;
    if (header != "OFF") {
        std::cerr << "Invalid OFF file format\n";
        file.close();
        return false;
    }
    
    int numVerts, numFaces, numEdges;
    file >> numVerts >> numFaces >> numEdges;
    if (numVerts <= 0 || numFaces <= 0) {
        std::cerr << "Invalid bunny mesh data\n";
        file.close();
        return false;
    }
    std::cout << "Loading bunny model with " << numVerts << " vertices and "
              << numFaces << " faces\n";
    
    std::vector<vec4> tempVerts(numVerts);
    for (int i = 0; i < numVerts; i++) {
        float x,y,z;
        file >> x >> y >> z;
        tempVerts[i] = vec4(x,y,z,1.0f);
    }
    
    vec4 minv(FLT_MAX, FLT_MAX, FLT_MAX, 1.0f);
    vec4 maxv(-FLT_MAX, -FLT_MAX, -FLT_MAX, 1.0f);
    for (auto& v : tempVerts) {
        minv.x = std::min(minv.x, v.x);
        minv.y = std::min(minv.y, v.y);
        minv.z = std::min(minv.z, v.z);
        maxv.x = std::max(maxv.x, v.x);
        maxv.y = std::max(maxv.y, v.y);
        maxv.z = std::max(maxv.z, v.z);
    }
    vec4 center((minv.x+maxv.x)*0.5f, (minv.y+maxv.y)*0.5f, (minv.z+maxv.z)*0.5f, 1.0f);
    float scaleVal = BUNNY_SCALE / std::max({maxv.x-minv.x, maxv.y-minv.y, maxv.z-minv.z});
    
    bunnyVertices.clear();
    for (int i = 0; i < numFaces; i++) {
        int n, idx1, idx2, idx3;
        file >> n;
        if (n == 3) {
            file >> idx1 >> idx2 >> idx3;
            if (idx1 >= 0 && idx1 < numVerts &&
                idx2 >= 0 && idx2 < numVerts &&
                idx3 >= 0 && idx3 < numVerts) {
                vec4 v1 = tempVerts[idx1];
                vec4 v2 = tempVerts[idx2];
                vec4 v3 = tempVerts[idx3];
                v1 = vec4(scaleVal*(v1.x - center.x), scaleVal*(v1.y - center.y), scaleVal*(v1.z - center.z), 1.0f);
                v2 = vec4(scaleVal*(v2.x - center.x), scaleVal*(v2.y - center.y), scaleVal*(v2.z - center.z), 1.0f);
                v3 = vec4(scaleVal*(v3.x - center.x), scaleVal*(v3.y - center.y), scaleVal*(v3.z - center.z), 1.0f);
                bunnyVertices.push_back(v1);
                bunnyVertices.push_back(v2);
                bunnyVertices.push_back(v3);
            } else {
                // skip
            }
        } else {
            for (int j = 0; j < n; j++) {
                int dummy;
                file >> dummy;
            }
        }
    }
    file.close();
    numBunnyVertices = (int)bunnyVertices.size();
    std::cout << "Bunny model loaded with " << numBunnyVertices << " vertices\n";
    return (numBunnyVertices>0);
}

// Compute bunny normals
void calculateBunnyNormals() {
    bunnyNormals.clear();
    bunnyNormals.resize(bunnyVertices.size());
    for (size_t i = 0; i < bunnyVertices.size(); i+=3) {
        vec3 v1(bunnyVertices[i].x,   bunnyVertices[i].y,   bunnyVertices[i].z);
        vec3 v2(bunnyVertices[i+1].x, bunnyVertices[i+1].y, bunnyVertices[i+1].z);
        vec3 v3(bunnyVertices[i+2].x, bunnyVertices[i+2].y, bunnyVertices[i+2].z);
        vec3 normal = normalize(cross(v2-v1, v3-v1));
        bunnyNormals[i]   = normal;
        bunnyNormals[i+1] = normal;
        bunnyNormals[i+2] = normal;
    }
    std::cout << "Calculated " << bunnyNormals.size() << " normals for bunny model\n";
}
