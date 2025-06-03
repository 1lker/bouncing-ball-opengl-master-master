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
bool useAmbient = true;
bool useDiffuse = true;
bool useSpecular = true;
float zoomScale = 1.0f;


// 2. Create a new VAO setup function for textured spheres
static void setupTexturedSphereVAO() {
    glGenVertexArrays(1, &vaoSphere);
    glBindVertexArray(vaoSphere);

    glGenBuffers(1, &vboSphere);
    glBindBuffer(GL_ARRAY_BUFFER, vboSphere);
    glBufferData(GL_ARRAY_BUFFER, sphereData.size() * sizeof(Vertex), sphereData.data(), GL_STATIC_DRAW);

    GLuint posLoc = glGetAttribLocation(currentProgram, "vPosition");
    GLuint normLoc = glGetAttribLocation(currentProgram, "vNormal");
    GLuint texLoc  = glGetAttribLocation(currentProgram, "vTexCoord"); // Add this to your shader

    glEnableVertexAttribArray(posLoc);
    glVertexAttribPointer(posLoc, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), BUFFER_OFFSET(0));

    glEnableVertexAttribArray(normLoc);
    glVertexAttribPointer(normLoc, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), BUFFER_OFFSET(sizeof(vec4)));

    glEnableVertexAttribArray(texLoc);
    glVertexAttribPointer(texLoc, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), BUFFER_OFFSET(sizeof(vec4) + sizeof(vec3)));

    glBindVertexArray(0);
}

// // 3. Replace the sphere VAO setup in main() with:
// initSphere(2); // generates sphereData
// setupTexturedSphereVAO(); // replaces setupVAO for sphere





// Setup a separate VAO/VBO for the trajectory  
static void setupTrajectoryVAO() {
    glGenVertexArrays(1, &vaoTrajectory);
    glBindVertexArray(vaoTrajectory);
    
    glGenBuffers(1, &vboTrajectory);
    glBindBuffer(GL_ARRAY_BUFFER, vboTrajectory);
    // We'll upload data dynamically each frame
    glBufferData(GL_ARRAY_BUFFER, MAX_TRAJECTORY_POINTS * sizeof(vec4), nullptr, GL_DYNAMIC_DRAW);
    
    GLuint posLoc = glGetAttribLocation(currentProgram, "vPosition");
    glEnableVertexAttribArray(posLoc);
    glVertexAttribPointer(posLoc, 4, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);
    
    // Provide a default empty normal for trajectory points
    GLuint normLoc = glGetAttribLocation(currentProgram, "vNormal");
    if (normLoc != (GLuint)-1) {
        // Create a separate VBO for normals
        GLuint normalVBO;
        glGenBuffers(1, &normalVBO);
        glBindBuffer(GL_ARRAY_BUFFER, normalVBO);
        
        // Default normals pointing toward camera (z-axis)
        std::vector<vec3> defaultNormals(MAX_TRAJECTORY_POINTS, vec3(0.0, 0.0, 1.0));
        glBufferData(GL_ARRAY_BUFFER, MAX_TRAJECTORY_POINTS * sizeof(vec3), defaultNormals.data(), GL_STATIC_DRAW);
        
        glEnableVertexAttribArray(normLoc);
        glVertexAttribPointer(normLoc, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);
    }
    
    // Reset to default state
    glBindVertexArray(0);
}

void initSphere(int subdivisions);
void setupTexturedSphereVAO();
int main() {
    if (!glfwInit()) {
        std::cerr << "GLFW init failed\n";
        return -1;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR,3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR,2);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT,GL_TRUE);
#endif
    glfwWindowHint(GLFW_OPENGL_PROFILE,GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE,GL_TRUE);
    
    GLFWwindow* window = glfwCreateWindow(windowWidth, windowHeight, "Enhanced Bouncing Ball", nullptr, nullptr);
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

    // Very important: Disable face culling so all faces are always rendered
    glDisable(GL_CULL_FACE);
    registerCallbacks(window);
    
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL); // Change from GL_LESS to GL_LEQUAL for better z-fighting handling
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    
    // program = Angel::InitShader("vshader.glsl", "fshader.glsl");
    phongProgram = Angel::InitShader("vshader.glsl", "fshader.glsl");
    gouraudProgram = Angel::InitShader("vshader_gouraud.glsl", "fshader_gouraud.glsl");
    
    

    currentProgram = phongProgram;
    glUseProgram(currentProgram);
    modelLoc = glGetUniformLocation(currentProgram, "model");
    projectionLoc = glGetUniformLocation(currentProgram, "projection");
    objColorLoc = glGetUniformLocation(currentProgram, "objColor");
    lightDirLoc = glGetUniformLocation(currentProgram, "lightDir");
    viewPosLoc  = glGetUniformLocation(currentProgram, "viewPos");
    glUniform1i(glGetUniformLocation(currentProgram, "useAmbient"), useAmbient);
    glUniform1i(glGetUniformLocation(currentProgram, "useDiffuse"), useDiffuse);
    glUniform1i(glGetUniformLocation(currentProgram, "useSpecular"), useSpecular);
    int texWidth, texHeight;
    texID = loadPPMTexture("earth.ppm", texWidth, texHeight);

    // Tell shader to use texture unit 0 for "textureMap"
    glUniform1i(glGetUniformLocation(currentProgram, "textureMap"), 0);
    
    // #


    glUseProgram(phongProgram);
    glUniform1i(glGetUniformLocation(phongProgram, "textureMap"), 0);

    glUseProgram(gouraudProgram);
    glUniform1i(glGetUniformLocation(gouraudProgram, "textureMap"), 0);
    
    // #



    // Use multiple light sources for better illumination
    vec3 lightDir(0.5f, 1.0f, 0.75f); // Adjusted for better all-around lighting
    glUniform3fv(lightDirLoc, 1, &lightDir[0]);
    
    // Camera position (view position) - move a bit to improve lighting angles
    vec3 viewPos(windowWidth/2.0f, windowHeight/2.0f, 300.0f);
    glUniform3fv(viewPosLoc, 1, &viewPos[0]);
    
    initCube();
    
    // Reduced subdivision level for better wireframe visualization
    // Changed from 5 to 2 subdivisions for more visible wireframe
    initSphere(2);
    setupTexturedSphereVAO();     // setup VAO for sphere with texture
    
    if (loadBunnyModel("bunny.off")) {
        calculateBunnyNormals();
        bunnyLoaded = true;
    } else {
        bunnyLoaded = false;
    }
    
    // Create VAOs for each object

    // setupVAO(vaoCube, vboCube, cubeVertices, cubeNormals);

    glGenVertexArrays(1, &vaoCube);
    glBindVertexArray(vaoCube);
    
    glGenBuffers(1, &vboCube);
    glBindBuffer(GL_ARRAY_BUFFER, vboCube);
    
    std::vector<Vertex> cubeCombined;
    for (size_t i = 0; i < cubeVertices.size(); ++i) {
        cubeCombined.push_back({cubeVertices[i], cubeNormals[i], vec2(0.0f, 0.0f)}); // texcoord dummy
    }
    
    glBufferData(GL_ARRAY_BUFFER, cubeCombined.size() * sizeof(Vertex), cubeCombined.data(), GL_STATIC_DRAW);
    
    GLuint posLoc = glGetAttribLocation(currentProgram, "vPosition");
    GLuint normLoc = glGetAttribLocation(currentProgram, "vNormal");
    
    glEnableVertexAttribArray(posLoc);
    glVertexAttribPointer(posLoc, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), BUFFER_OFFSET(0));
    
    glEnableVertexAttribArray(normLoc);
    glVertexAttribPointer(normLoc, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), BUFFER_OFFSET(sizeof(vec4)));    // setupVAO(vaoSphere, vboSphere, sphereVertices, sphereNormals);
    // setupVAO(vaoBunny, vboBunny, bunnyVertices, bunnyNormals);
    // Create VAO/VBO for trajectory
    setupTrajectoryVAO();
    
    glViewport(0, 0, windowWidth, windowHeight);
    mat4 proj = Perspective(45.0, (float)windowWidth / windowHeight, 1.0, 1000.0);
    glUniformMatrix4fv(projectionLoc, 1, GL_TRUE, proj);
    
    initBall();
    printHelp();
    
    double lastTime = glfwGetTime();
    while (!glfwWindowShouldClose(window)) {
        double currentT = glfwGetTime();
        double dt = currentT - lastTime;
        lastTime = currentT;
        
        if (multipleObjects && (currentT - lastLaunchTime)>launchInterval) {
            launchBall();
            lastLaunchTime = currentT;
        }
        updateBall(dt);
        if (showParticles) updateParticles(dt);
        
        display();
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    
    // Cleanup
    glDeleteVertexArrays(1, &vaoCube);
    glDeleteBuffers(1, &vboCube);
    glDeleteVertexArrays(1, &vaoSphere);
    glDeleteBuffers(1, &vboSphere);
    glDeleteVertexArrays(1, &vaoBunny);
    glDeleteBuffers(1, &vboBunny);
    glDeleteVertexArrays(1, &vaoTrajectory);
    glDeleteBuffers(1, &vboTrajectory);
    
    glfwTerminate();
    return 0;
}