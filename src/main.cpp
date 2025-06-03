#include "Angel.h"
#include "Globals.h"
#include "InitShader.h"
#include "input.h"
#include "objects.h"
#include "physics.h"
#include "render.h"
#include <iostream>
#include <vector>
#include <GLFW/glfw3.h>
#include "texture.h"  

extern std::vector<Vertex> sphereData;

// Setup textured sphere VAO
static void setupTexturedSphereVAO() {
    glGenVertexArrays(1, &vaoSphere);
    glBindVertexArray(vaoSphere);

    glGenBuffers(1, &vboSphere);
    glBindBuffer(GL_ARRAY_BUFFER, vboSphere);
    glBufferData(GL_ARRAY_BUFFER, sphereData.size() * sizeof(Vertex), sphereData.data(), GL_STATIC_DRAW);

    GLuint posLoc = glGetAttribLocation(currentProgram, "vPosition");
    GLuint normLoc = glGetAttribLocation(currentProgram, "vNormal");
    GLuint texLoc  = glGetAttribLocation(currentProgram, "vTexCoord");

    glEnableVertexAttribArray(posLoc);
    glVertexAttribPointer(posLoc, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), BUFFER_OFFSET(0));

    glEnableVertexAttribArray(normLoc);
    glVertexAttribPointer(normLoc, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), BUFFER_OFFSET(sizeof(vec4)));

    glEnableVertexAttribArray(texLoc);
    glVertexAttribPointer(texLoc, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), BUFFER_OFFSET(sizeof(vec4) + sizeof(vec3)));

    glBindVertexArray(0);
}

// Setup cube VAO with proper vertex structure
static void setupCubeVAO() {
    glGenVertexArrays(1, &vaoCube);
    glBindVertexArray(vaoCube);
    
    glGenBuffers(1, &vboCube);
    glBindBuffer(GL_ARRAY_BUFFER, vboCube);
    
    std::vector<Vertex> cubeCombined;
    for (size_t i = 0; i < cubeVertices.size(); ++i) {
        cubeCombined.push_back({cubeVertices[i], cubeNormals[i], vec2(0.0f, 0.0f)});
    }
    
    glBufferData(GL_ARRAY_BUFFER, cubeCombined.size() * sizeof(Vertex), cubeCombined.data(), GL_STATIC_DRAW);
    
    GLuint posLoc = glGetAttribLocation(currentProgram, "vPosition");
    GLuint normLoc = glGetAttribLocation(currentProgram, "vNormal");
    GLuint texLoc = glGetAttribLocation(currentProgram, "vTexCoord");
    
    glEnableVertexAttribArray(posLoc);
    glVertexAttribPointer(posLoc, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), BUFFER_OFFSET(0));
    
    glEnableVertexAttribArray(normLoc);
    glVertexAttribPointer(normLoc, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), BUFFER_OFFSET(sizeof(vec4)));
    
    if (texLoc != (GLuint)-1) {
        glEnableVertexAttribArray(texLoc);
        glVertexAttribPointer(texLoc, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), BUFFER_OFFSET(sizeof(vec4) + sizeof(vec3)));
    }
    
    glBindVertexArray(0);
}

// Setup bunny VAO
static void setupBunnyVAO() {
    if (!bunnyLoaded) return;
    
    glGenVertexArrays(1, &vaoBunny);
    glBindVertexArray(vaoBunny);
    
    glGenBuffers(1, &vboBunny);
    glBindBuffer(GL_ARRAY_BUFFER, vboBunny);
    
    std::vector<Vertex> bunnyCombined;
    for (size_t i = 0; i < bunnyVertices.size(); ++i) {
        bunnyCombined.push_back({bunnyVertices[i], bunnyNormals[i], vec2(0.0f, 0.0f)});
    }
    
    glBufferData(GL_ARRAY_BUFFER, bunnyCombined.size() * sizeof(Vertex), bunnyCombined.data(), GL_STATIC_DRAW);
    
    GLuint posLoc = glGetAttribLocation(currentProgram, "vPosition");
    GLuint normLoc = glGetAttribLocation(currentProgram, "vNormal");
    GLuint texLoc = glGetAttribLocation(currentProgram, "vTexCoord");
    
    glEnableVertexAttribArray(posLoc);
    glVertexAttribPointer(posLoc, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), BUFFER_OFFSET(0));
    
    glEnableVertexAttribArray(normLoc);
    glVertexAttribPointer(normLoc, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), BUFFER_OFFSET(sizeof(vec4)));
    
    if (texLoc != (GLuint)-1) {
        glEnableVertexAttribArray(texLoc);
        glVertexAttribPointer(texLoc, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), BUFFER_OFFSET(sizeof(vec4) + sizeof(vec3)));
    }
    
    glBindVertexArray(0);
}

// Setup a separate VAO/VBO for the trajectory  
static void setupTrajectoryVAO() {
    glGenVertexArrays(1, &vaoTrajectory);
    glBindVertexArray(vaoTrajectory);
    
    glGenBuffers(1, &vboTrajectory);
    glBindBuffer(GL_ARRAY_BUFFER, vboTrajectory);
    glBufferData(GL_ARRAY_BUFFER, MAX_TRAJECTORY_POINTS * sizeof(vec4), nullptr, GL_DYNAMIC_DRAW);
    
    GLuint posLoc = glGetAttribLocation(currentProgram, "vPosition");
    glEnableVertexAttribArray(posLoc);
    glVertexAttribPointer(posLoc, 4, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);
    
    GLuint normLoc = glGetAttribLocation(currentProgram, "vNormal");
    if (normLoc != (GLuint)-1) {
        GLuint normalVBO;
        glGenBuffers(1, &normalVBO);
        glBindBuffer(GL_ARRAY_BUFFER, normalVBO);
        
        std::vector<vec3> defaultNormals(MAX_TRAJECTORY_POINTS, vec3(0.0, 0.0, 1.0));
        glBufferData(GL_ARRAY_BUFFER, MAX_TRAJECTORY_POINTS * sizeof(vec3), defaultNormals.data(), GL_STATIC_DRAW);
        
        glEnableVertexAttribArray(normLoc);
        glVertexAttribPointer(normLoc, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);
    }
    
    glBindVertexArray(0);
}

void initSphere(int subdivisions);

int main() {
    if (!glfwInit()) {
        std::cerr << "GLFW init failed\n";
        return -1;
    }
    
    // Request OpenGL 4.1 Core Profile as required
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
    glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);
    
    GLFWwindow* window = glfwCreateWindow(windowWidth, windowHeight, 
                                          "COMP 410/510 Assignment 3 - Shading and Texture Mapping", 
                                          nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create window\n";
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    
#ifndef __APPLE__
    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    if (err != GLEW_OK) {
        std::cerr << "GLEW error: " << glewGetErrorString(err) << "\n";
        glfwTerminate();
        return -1;
    }
#endif

    // Register callbacks early
    registerCallbacks(window);
    
    // ASSIGNMENT REQUIREMENT: Enable depth test and culling
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_CULL_FACE);  // Enable culling as required
    glCullFace(GL_BACK);     // Cull back-facing triangles
    glFrontFace(GL_CCW);     // Counter-clockwise is front-facing
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    
    // Initialize shaders
    phongProgram = Angel::InitShader("vshader.glsl", "fshader.glsl");
    gouraudProgram = Angel::InitShader("vshader_gouraud.glsl", "fshader_gouraud.glsl");
    
    // Start with Phong shading (default)
    currentProgram = phongProgram;
    glUseProgram(currentProgram);
    
    // Get uniform locations for current program
    modelLoc = glGetUniformLocation(currentProgram, "model");
    projectionLoc = glGetUniformLocation(currentProgram, "projection");
    objColorLoc = glGetUniformLocation(currentProgram, "objColor");
    lightDirLoc = glGetUniformLocation(currentProgram, "lightDir");
    viewPosLoc = glGetUniformLocation(currentProgram, "viewPos");
    
    // Initialize lighting component uniforms for both programs
    glUseProgram(phongProgram);
    glUniform1i(glGetUniformLocation(phongProgram, "useAmbient"), useAmbient);
    glUniform1i(glGetUniformLocation(phongProgram, "useDiffuse"), useDiffuse);
    glUniform1i(glGetUniformLocation(phongProgram, "useSpecular"), useSpecular);
    glUniform1i(glGetUniformLocation(phongProgram, "textureMap"), 0);
    
    glUseProgram(gouraudProgram);
    glUniform1i(glGetUniformLocation(gouraudProgram, "useAmbient"), useAmbient);
    glUniform1i(glGetUniformLocation(gouraudProgram, "useDiffuse"), useDiffuse);
    glUniform1i(glGetUniformLocation(gouraudProgram, "useSpecular"), useSpecular);
    glUniform1i(glGetUniformLocation(gouraudProgram, "textureMap"), 0);
    
    // Load default texture
    int texWidth, texHeight;
    texID = loadPPMTexture("earth.ppm", texWidth, texHeight);
    
    // Set directional light
    vec3 lightDir(0.5f, 1.0f, 0.75f);
    glUseProgram(phongProgram);
    glUniform3fv(glGetUniformLocation(phongProgram, "lightDir"), 1, &lightDir[0]);
    
    glUseProgram(gouraudProgram);
    glUniform3fv(glGetUniformLocation(gouraudProgram, "lightDir"), 1, &lightDir[0]);
    
    // Set camera position (view position) - FIXED for perspective projection
    vec3 viewPos(0.0f, 0.0f, 15.0f);  // Camera positioned back from the scene
    glUseProgram(phongProgram);
    glUniform3fv(glGetUniformLocation(phongProgram, "viewPos"), 1, &viewPos[0]);
    
    glUseProgram(gouraudProgram);
    glUniform3fv(glGetUniformLocation(gouraudProgram, "viewPos"), 1, &viewPos[0]);
    
    // Initialize objects
    initCube();
    initSphere(2);  // Use subdivision level 2 for better performance
    
    // Try to load bunny model
    if (loadBunnyModel("bunny.off")) {
        calculateBunnyNormals();
        bunnyLoaded = true;
        std::cout << "Bunny model loaded successfully\n";
    } else {
        bunnyLoaded = false;
        std::cout << "Bunny model not found, continuing without it\n";
    }
    
    // Set current program back to default
    glUseProgram(currentProgram);
    
    // Setup VAOs for all objects
    setupTexturedSphereVAO();
    setupCubeVAO();
    setupBunnyVAO();
    setupTrajectoryVAO();
    
    // FIXED: Setup proper perspective projection and view matrix
    glViewport(0, 0, windowWidth, windowHeight);
    
    // Create view matrix (camera looking at origin from positive Z)
    vec4 eye(0.0f, 0.0f, 15.0f, 1.0f);      // Camera position
    vec4 at(0.0f, 0.0f, 0.0f, 1.0f);        // Look at origin
    vec4 up(0.0f, 1.0f, 0.0f, 0.0f);        // Up vector
    mat4 view = LookAt(eye, at, up);
    
    // Create perspective projection matrix
    mat4 projection = Perspective(45.0f, (float)windowWidth / windowHeight, 0.1f, 100.0f);
    
    // Combine view and projection (since shader expects just "projection" matrix)
    mat4 viewProjection = projection * view;
    
    // Update projection matrix for both programs
    glUseProgram(phongProgram);
    glUniformMatrix4fv(glGetUniformLocation(phongProgram, "projection"), 1, GL_TRUE, viewProjection);
    
    glUseProgram(gouraudProgram);
    glUniformMatrix4fv(glGetUniformLocation(gouraudProgram, "projection"), 1, GL_TRUE, viewProjection);
    
    glUseProgram(currentProgram);
    
    // Initialize ball physics
    initBall();
    
    // Print help
    printHelp();
    std::cout << "\nAssignment 3 initialized successfully!\n";
    std::cout << "Default mode: Shading (Phong)\n";
    std::cout << "Press 'h' for help\n\n";
    
    // Main loop
    double lastTime = glfwGetTime();
    while (!glfwWindowShouldClose(window)) {
        double currentT = glfwGetTime();
        double dt = currentT - lastTime;
        lastTime = currentT;
        
        // Update physics
        updateBall(dt);
        if (showParticles) updateParticles(dt);
        
        // Render
        display();
        
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    
    // Cleanup
    glDeleteVertexArrays(1, &vaoCube);
    glDeleteBuffers(1, &vboCube);
    glDeleteVertexArrays(1, &vaoSphere);
    glDeleteBuffers(1, &vboSphere);
    if (bunnyLoaded) {
        glDeleteVertexArrays(1, &vaoBunny);
        glDeleteBuffers(1, &vboBunny);
    }
    glDeleteVertexArrays(1, &vaoTrajectory);
    glDeleteBuffers(1, &vboTrajectory);
    glDeleteTextures(1, &texID);
    
    glfwTerminate();
    return 0;
}