#include "input.h"
#include "Globals.h"
#include "physics.h"
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <sstream>
#include <iomanip>
#include "texture.h"

extern GLuint phongProgram, gouraudProgram, currentProgram;
static bool usePhong = true;
static int componentToggleIndex = 0;
void toggleTexture();

/**
 * Callback function for keyboard input
 */
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action != GLFW_PRESS && action != GLFW_REPEAT) return;
    
    switch(key) {
        case GLFW_KEY_Q:
        case GLFW_KEY_ESCAPE:
            glfwSetWindowShouldClose(window, GL_TRUE);
            break;
            
        // Multiple restart options
        case GLFW_KEY_F5:
        case GLFW_KEY_HOME:
        case GLFW_KEY_SPACE:
        case GLFW_KEY_ENTER:
            initBall();
            std::cout << "Simulation restarted.\n";
            break;
            
        case GLFW_KEY_C:
            if (mods & GLFW_MOD_SHIFT) {
                rainbowMode = !rainbowMode;
                std::cout << "Rainbow mode " << (rainbowMode ? "ON" : "OFF") << "\n";
            } else {
                currentColorIndex = (currentColorIndex + 1) % 8;
                std::cout << "Color index: " << currentColorIndex << "\n";
            }
            break;
            
        case GLFW_KEY_P:
            if (trajectoryMode == NONE) trajectoryMode = LINE;
            else if (trajectoryMode == LINE) trajectoryMode = STROBE;
            else trajectoryMode = NONE;
            
            // FIXED: Proper string handling for trajectory mode output
            std::cout << "Trajectory mode changed to ";
            if (trajectoryMode == NONE) std::cout << "None";
            else if (trajectoryMode == LINE) std::cout << "Line";
            else std::cout << "Strobe";
            std::cout << "\n";
            break;
            
        case GLFW_KEY_G:
            if (mods & GLFW_MOD_SHIFT)
                gravityStrength += 0.1f;
            else
                gravityStrength = std::max(0.0f, gravityStrength - 0.1f);
            std::cout << "Gravity: " << gravityStrength << "\n";
            break;
            
        case GLFW_KEY_E:
            showParticles = !showParticles;
            std::cout << "Particle effects " << (showParticles ? "ON" : "OFF") << "\n";
            break;
            
        case GLFW_KEY_R:
            // Reset settings to defaults
            currentObject = SPHERE;
            currentMode = SOLID;
            trajectoryMode = NONE;
            rainbowMode = false;
            multipleObjects = false;
            simulationSpeed = 1.0f;
            objectScale = 1.0f;
            gridMode = GRID_NONE;
            backgroundColorIndex = 0;
            backgroundColor = vec4(0.1f, 0.1f, 0.1f, 1.0f);
            zoomScale = 1.0f;  // Reset zoom
            std::cout << "Reset settings to defaults.\n";
            break;
            
        case GLFW_KEY_1:
        case GLFW_KEY_KP_1:
            currentObject = CUBE;
            std::cout << "Switched to Cube\n";
            break;
            
        case GLFW_KEY_2:
        case GLFW_KEY_KP_2:
            currentObject = SPHERE;
            std::cout << "Switched to Sphere\n";
            break;
            
        case GLFW_KEY_3:
        case GLFW_KEY_KP_3:
            if (bunnyLoaded) {
                currentObject = BUNNY;
                std::cout << "Switched to Bunny\n";
            } else {
                std::cout << "Bunny not loaded\n";
            }
            break;
            
        case GLFW_KEY_H:
        case GLFW_KEY_F1:
            printHelp();
            break;
            
        // ASSIGNMENT 3 SPECIFIC CONTROLS
        
        case GLFW_KEY_S:  // Toggle shading technique (Phong/Gouraud)
            usePhong = !usePhong;
            useGouraud = !usePhong;
            currentProgram = usePhong ? phongProgram : gouraudProgram;
            glUseProgram(currentProgram);
            
            // Update uniform locations for the new program
            modelLoc = glGetUniformLocation(currentProgram, "model");
            projectionLoc = glGetUniformLocation(currentProgram, "projection");
            objColorLoc = glGetUniformLocation(currentProgram, "objColor");
            lightDirLoc = glGetUniformLocation(currentProgram, "lightDir");
            viewPosLoc = glGetUniformLocation(currentProgram, "viewPos");
            
            std::cout << "Shading: " << (usePhong ? "Phong" : "Gouraud") << "\n";
            break;
            
        case GLFW_KEY_O:  // Toggle lighting components
            switch (componentToggleIndex) {
                case 0:
                    useAmbient = !useAmbient;
                    std::cout << "Ambient: " << (useAmbient ? "ON" : "OFF") << "\n";
                    break;
                case 1:
                    useDiffuse = !useDiffuse;
                    std::cout << "Diffuse: " << (useDiffuse ? "ON" : "OFF") << "\n";
                    break;
                case 2:
                    useSpecular = !useSpecular;
                    std::cout << "Specular: " << (useSpecular ? "ON" : "OFF") << "\n";
                    break;
            }
            componentToggleIndex = (componentToggleIndex + 1) % 3;
            
            // Update uniforms for both programs
            glUseProgram(phongProgram);
            glUniform1i(glGetUniformLocation(phongProgram, "useAmbient"), useAmbient);
            glUniform1i(glGetUniformLocation(phongProgram, "useDiffuse"), useDiffuse);
            glUniform1i(glGetUniformLocation(phongProgram, "useSpecular"), useSpecular);
            
            glUseProgram(gouraudProgram);
            glUniform1i(glGetUniformLocation(gouraudProgram, "useAmbient"), useAmbient);
            glUniform1i(glGetUniformLocation(gouraudProgram, "useDiffuse"), useDiffuse);
            glUniform1i(glGetUniformLocation(gouraudProgram, "useSpecular"), useSpecular);
            
            glUseProgram(currentProgram);  // Switch back to current program
            break;
            
        case GLFW_KEY_L:  // Toggle light movement
            lightFollowsObject = !lightFollowsObject;
            std::cout << "Light movement: " << (lightFollowsObject ? "Follows object" : "Fixed") << "\n";
            break;
            
        case GLFW_KEY_M:  // Toggle material
            useMetallic = !useMetallic;
            std::cout << "Material: " << (useMetallic ? "Metallic" : "Plastic") << "\n";
            break;
            
        case GLFW_KEY_I:  // Toggle texture
            toggleTexture();
            break;
            
        case GLFW_KEY_T:  // Toggle display mode
            currentRenderMode = static_cast<RenderMode>((currentRenderMode + 1) % 3);
            std::cout << "Render Mode: ";
            switch (currentRenderMode) {
                case WIREFRAME_MODE: std::cout << "Wireframe\n"; break;
                case SHADING_MODE:   std::cout << "Shading\n"; break;
                case TEXTURE_MODE:   std::cout << "Texture\n"; break;
            }
            break;
            
        // ZOOM CONTROLS (FIXED: No longer conflicts with object scaling)
        case GLFW_KEY_Z:  // Zoom in
            zoomScale = std::min(zoomScale + 0.1f, 3.0f);
            std::cout << "Zoom In: scale = " << zoomScale << "\n";
            break;

        case GLFW_KEY_W:  // Zoom out
            zoomScale = std::max(zoomScale - 0.1f, 0.2f);
            std::cout << "Zoom Out: scale = " << zoomScale << "\n";
            break;
            
        // Object scaling (moved to different keys to avoid zoom conflict)
        case GLFW_KEY_X:
            objectScale = std::max(objectScale - 0.1f, 0.5f);
            std::cout << "Object scale: " << objectScale << "x\n";
            break;
            
        case GLFW_KEY_V:
            objectScale = std::min(objectScale + 0.1f, 2.0f);
            std::cout << "Object scale: " << objectScale << "x\n";
            break;
            
        // Background color toggle
        case GLFW_KEY_B:
            {
                const int numColors = 5;
                vec4 bgColors[numColors] = {
                    vec4(0.1f, 0.1f, 0.1f, 1.0f),   // Dark gray
                    vec4(0.05f, 0.05f, 0.15f, 1.0f), // Dark blue
                    vec4(0.15f, 0.05f, 0.05f, 1.0f), // Dark red
                    vec4(0.05f, 0.15f, 0.05f, 1.0f), // Dark green
                    vec4(0.15f, 0.15f, 0.05f, 1.0f)  // Dark yellow
                };
                
                backgroundColorIndex = (backgroundColorIndex + 1) % numColors;
                backgroundColor = bgColors[backgroundColorIndex];
                
                std::cout << "Background color changed\n";
            }
            break;
            
        // Simulation speed control
        case GLFW_KEY_EQUAL:        // + key
        case GLFW_KEY_KP_ADD:       // Numpad +
            simulationSpeed = std::min(simulationSpeed + 0.1f, 3.0f);
            std::cout << "Simulation speed: " << simulationSpeed << "x\n";
            break;
            
        case GLFW_KEY_MINUS:        // - key
        case GLFW_KEY_KP_SUBTRACT:  // Numpad -
            simulationSpeed = std::max(simulationSpeed - 0.1f, 0.1f);
            std::cout << "Simulation speed: " << simulationSpeed << "x\n";
            break;
            
        // Take a screenshot
        case GLFW_KEY_F12:
            {
                std::time_t t = std::time(nullptr);
                std::tm* now = std::localtime(&t);
                std::stringstream ss;
                ss << "screenshot_" 
                   << (now->tm_year + 1900) 
                   << std::setfill('0') << std::setw(2) << (now->tm_mon + 1)
                   << std::setfill('0') << std::setw(2) << now->tm_mday
                   << "_"
                   << std::setfill('0') << std::setw(2) << now->tm_hour
                   << std::setfill('0') << std::setw(2) << now->tm_min
                   << std::setfill('0') << std::setw(2) << now->tm_sec
                   << ".ppm";
                
                takeScreenshot(ss.str());
            }
            break;
            
        default:
            break;
    }
}

/**
 * Callback function for mouse button events
 */
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    if (action != GLFW_PRESS) return;
    switch(button) {
        case GLFW_MOUSE_BUTTON_LEFT:
            currentMode = (currentMode == WIREFRAME ? SOLID : WIREFRAME);
            std::cout << "Drawing mode: " << (currentMode == WIREFRAME ? "Wireframe" : "Solid") << std::endl;
            break;
            
        case GLFW_MOUSE_BUTTON_RIGHT:
            if (currentObject == CUBE)
                currentObject = SPHERE;
            else if (currentObject == SPHERE)
                currentObject = (bunnyLoaded ? BUNNY : CUBE);
            else
                currentObject = CUBE;
            std::cout << "Object type: " << 
                (currentObject == CUBE ? "Cube" : 
                 (currentObject == SPHERE ? "Sphere" : "Bunny")) << std::endl;
            break;
            
        case GLFW_MOUSE_BUTTON_MIDDLE:
            initBall();
            std::cout << "Simulation restarted.\n";
            break;
            
        default:
            break;
    }
}

/**
 * Callback function for window resize events
 */
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    windowWidth = width;
    windowHeight = height;
    
    // Update viewport
    glViewport(0, 0, width, height);
    
    // FIXED: Update perspective projection with proper view matrix
    vec4 eye(0.0f, 0.0f, 15.0f, 1.0f);      // Camera position
    vec4 at(0.0f, 0.0f, 0.0f, 1.0f);        // Look at origin
    vec4 up(0.0f, 1.0f, 0.0f, 0.0f);        // Up vector
    mat4 view = LookAt(eye, at, up);
    
    mat4 projection = Perspective(45.0f, (float)width / height, 0.1f, 100.0f);
    mat4 viewProjection = projection * view;
    
    // Update view position for lighting (camera position in world space)
    vec3 viewPos(0.0f, 0.0f, 15.0f);
    
    // Apply all updates to both shaders
    glUseProgram(phongProgram);
    glUniformMatrix4fv(glGetUniformLocation(phongProgram, "projection"), 1, GL_TRUE, viewProjection);
    glUniform3fv(glGetUniformLocation(phongProgram, "viewPos"), 1, &viewPos[0]);
    
    glUseProgram(gouraudProgram);
    glUniformMatrix4fv(glGetUniformLocation(gouraudProgram, "projection"), 1, GL_TRUE, viewProjection);
    glUniform3fv(glGetUniformLocation(gouraudProgram, "viewPos"), 1, &viewPos[0]);
    
    // Switch back to current program
    glUseProgram(currentProgram);
    
    std::cout << "Window resized to " << width << "x" << height << std::endl;
}

static int currentTexture = 0;

void toggleTexture() {
    currentTexture = (currentTexture + 1) % 2;
    const char* textureFile = (currentTexture == 0) ? "earth.ppm" : "basketball.ppm";
    int w, h;
    texID = loadPPMTexture(textureFile, w, h);
    std::cout << "Switched to texture: " << textureFile << "\n";
}

/**
 * Registers all callback functions with GLFW
 */
void registerCallbacks(GLFWwindow* window) {
    glfwSetKeyCallback(window, key_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
}