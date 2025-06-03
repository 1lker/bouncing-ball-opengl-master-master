#ifndef INITSHADER_H
#define INITSHADER_H

#include "Angel.h"

namespace Angel {
    // Loads and compiles the vertex and fragment shader files,
    // links them into a shader program, and returns the program ID.
    GLuint InitShader(const char* vertexShaderFile, const char* fragmentShaderFile);
}

#endif // INITSHADER_H
