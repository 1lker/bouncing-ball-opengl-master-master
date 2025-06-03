#include "input.h"
#include "Globals.h"
#include "physics.h"
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <sstream>
#include <iomanip>
#include "texture.h"

extern GLuint phongProgram, gouraudProgram, currentProgram;  // Declare external shader program handles
static bool usePhong = true;  // Initial shading mode
static int componentToggleIndex = 0;
void toggleTexture();  // 👈 Add this line
/**
 * Callback function for keyboard input
 * Handles all keyboard commands for the application
 * 
 * @param window GLFW window handle
 * @param key Key code of the pressed key
 * @param scancode System-specific scancode of the key
 * @param action GLFW_PRESS, GLFW_RELEASE or GLFW_REPEAT
 * @param mods Bit field describing which modifier keys were held down
 */
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action != GLFW_PRESS && action != GLFW_REPEAT) return;
    
    // Debug print for key detection - uncomment to check which key codes are being received
    // std::cout << "Key pressed: " << key << " (scancode: " << scancode << ")" << std::endl;
    
    switch(key) {
        case GLFW_KEY_Q:
        case GLFW_KEY_ESCAPE:
            glfwSetWindowShouldClose(window, GL_TRUE);
            break;
            
        // Multiple restart options for different keyboard layouts
        // case GLFW_KEY_I:
        case GLFW_KEY_F5:            // Added F5 as a restart option
        case GLFW_KEY_HOME:          // Added Home key as a restart option  
        case GLFW_KEY_KP_0:          // Added numpad 0 as a restart option
            initBall();              // reinitialize
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
            std::cout << "Trajectory mode changed to " <<
                (trajectoryMode == NONE ? "None" : (trajectoryMode == LINE ? "Line" : "Strobe")) << "\n";
            break;
            
        // case GLFW_KEY_M:
        //     multipleObjects = !multipleObjects;
        //     std::cout << "Multiple objects " << (multipleObjects ? "ON" : "OFF") << "\n";
        //     break;
            
        case GLFW_KEY_SPACE:
        case GLFW_KEY_ENTER:         // Added Enter as an alternative
            if (multipleObjects) launchBall();
            else initBall();         // Also restart on Space/Enter when not in multiple mode
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
            // 'r' key now just resets settings without restarting
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
            std::cout << "Reset settings to defaults.\n";
            break;
            
        case GLFW_KEY_1:
        case GLFW_KEY_KP_1:          // Added numpad 1
            currentObject = CUBE;
            std::cout << "Switched to Cube\n";
            break;
            
        case GLFW_KEY_2:
        case GLFW_KEY_KP_2:          // Added numpad 2
            currentObject = SPHERE;
            std::cout << "Switched to Sphere\n";
            break;
            
        case GLFW_KEY_3:
        case GLFW_KEY_KP_3:          // Added numpad 3
            if (bunnyLoaded) {
                currentObject = BUNNY;
                std::cout << "Switched to Bunny\n";
            } else {
                std::cout << "Bunny not loaded\n";
            }
            break;
            
        case GLFW_KEY_H:
        case GLFW_KEY_F1:            // Added F1 as help key
            printHelp();
            break;
            
        // NEW FEATURES
        
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
            
        // Object scaling
        case GLFW_KEY_Z:
            objectScale = std::max(objectScale - 0.1f, 0.5f);
            std::cout << "Object scale: " << objectScale << "x\n";
            break;
            
        case GLFW_KEY_X:
            objectScale = std::min(objectScale + 0.1f, 2.0f);
            std::cout << "Object scale: " << objectScale << "x\n";
            break;
            
        // // Toggle grid display
        // case GLFW_KEY_T:
        //     if (gridMode == GRID_NONE) gridMode = GRID_BASIC;
        //     else if (gridMode == GRID_BASIC) gridMode = GRID_DETAILED;
        //     else gridMode = GRID_NONE;
        //     std::cout << "Grid mode changed to " <<
        //         (gridMode == GRID_NONE ? "None" : (gridMode == GRID_BASIC ? "Basic" : "Detailed")) << "\n";
        //     break;
        
        case GLFW_KEY_I:
            toggleTexture();
            break;
        
        case GLFW_KEY_M:
            useMetallic = !useMetallic;
            std::cout << "Material: "<< (useMetallic ? "Metallic" : "Plastic") << "\n";
            break;

        case GLFW_KEY_S:  // Toggle shading technique
            usePhong = !usePhong;
            currentProgram = usePhong ? phongProgram : gouraudProgram;
            glUseProgram(currentProgram);  // Important: activate the new program immediately
            std::cout << "Shading: " << (usePhong ? "Phong" : "Gouraud") << "\n";
            break;
        // case GLFW_KEY_Z:
        //     zoomScale = std::min(zoomScale + 0.1f, 3.0f);
        //     std::cout << "Zoom In: scale = " << zoomScale << "\n";
        //     break;

        // case GLFW_KEY_W:
        //     zoomScale = std::max(zoomScale - 0.1f, 0.2f);
        //     std::cout << "Zoom Out: scale = " << zoomScale << "\n";
        //     break;
        
        case GLFW_KEY_T:
            currentRenderMode = static_cast<RenderMode>((currentRenderMode + 1) % 3);
            std::cout << "Render Mode: ";
            switch (currentRenderMode) {
                case WIREFRAME_MODE: std::cout << "Wireframe\n"; break;
                case SHADING_MODE:   std::cout << "Shading\n"; break;
                case TEXTURE_MODE:   std::cout << "Texture\n"; break;
            }
            break;

        case GLFW_KEY_O:
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
            break;

        case GLFW_KEY_L:
            lightFollowsObject = !lightFollowsObject;
            std::cout << "Light movement: " << (lightFollowsObject ? "Follows object" : "Fixed") << "\n";
            break;
            
        // Take a screenshot
        case GLFW_KEY_F12:
            {
                // Generate a timestamp for the filename
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
            // For any other key, check if it's the launch/restart key for Turkish keyboard
            if (scancode == 23 || scancode == 31) {  // Common scancodes for 'i' variants
                initBall();
                std::cout << "Simulation restarted (using scancode).\n";
            }
            break;
    }
}

/**
 * Callback function for mouse button events
 * Handles all mouse button interactions
 * 
 * @param window GLFW window handle
 * @param button The mouse button that was pressed or released
 * @param action GLFW_PRESS or GLFW_RELEASE
 * @param mods Bit field describing which modifier keys were held down
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
            if (multipleObjects) {
                launchBall();
            } else {
                initBall();  // Also restart on middle click when not in multiple mode
                std::cout << "Simulation restarted.\n";
            }
            break;
            
        default:
            break;
    }
}

/**
 * Callback function for window resize events
 * Updates viewport, projection, and lighting when the window is resized
 * 
 * @param window GLFW window handle
 * @param width New width of the window
 * @param height New height of the window
 */
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    windowWidth = width;
    windowHeight = height;
    
    // Update viewport
    glViewport(0, 0, width, height);
    
    // Update projection matrix
    mat4 proj = Ortho(0.0f, (float)width, (float)height, 0.0f, -1000.0f, 1000.0f);
    
    // Update view position to match new window dimensions
    vec3 viewPos(width/2.0f, height/2.0f, 300.0f);
    
    // Apply all updates to shader
    glUseProgram(currentProgram);
    glUniformMatrix4fv(projectionLoc, 1, GL_TRUE, proj);
    glUniform3fv(viewPosLoc, 1, &viewPos[0]);
    
    // Update lighting to match new dimensions
    vec3 lightDir(0.5f, 1.0f, 0.75f);
    glUniform3fv(lightDirLoc, 1, &lightDir[0]);
    
    std::cout << "Window resized to " << width << "x" << height << std::endl;
    
    // Adjust starting position if window was resized before simulation started
    if (xPos == 0.0f && yPos == 0.0f) {
        float margin = width * 0.05f;
        xPos = margin;
        yPos = margin;
    }
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
 * 
 * @param window GLFW window handle to register callbacks for
 */
void registerCallbacks(GLFWwindow* window) {
    glfwSetKeyCallback(window, key_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
}