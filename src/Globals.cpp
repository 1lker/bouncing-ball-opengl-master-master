#include "Globals.h"
#include <iostream>
#include <algorithm>
#include <fstream>

/**
 * Window dimensions for the application
 */
int windowWidth = 800;
int windowHeight = 600;

/**
 * Texture and material properties
 */
GLuint texID = 0;
bool lightFollowsObject = false;
bool useMetallic = false;
float zoomScale = 1.0f;  // Zoom functionality

// FIXED: Properly define lighting component toggles
bool useAmbient = true;
bool useDiffuse = true;
bool useSpecular = true;

// Material parameters
float plasticShininess = 16.0f;
float metallicShininess = 64.0f;
float plasticSpecularStrength = 0.3f;
float metallicSpecularStrength = 0.8f;

// Render mode and shading
RenderMode currentRenderMode = SHADING_MODE; // default
bool useGouraud = false;

/**
 * Physics constants that control simulation behavior
 */
const float GRAVITY = 0.35f;          // Gravitational acceleration
const float RESTITUTION = 0.92f;      // Energy retention on bounce (1.0 = perfect bounce)
const float BALL_SIZE = 60.0f;        // Base size of objects
const float BUNNY_SCALE = 15.0f;      // Scale factor for bunny model
const int MAX_TRAJECTORY_POINTS = 150; // Maximum number of points in trajectory
const float AIR_RESISTANCE = 0.998f;  // Air resistance factor (1.0 = no resistance)

/**
 * Global state variables for object properties
 */
ObjectType currentObject = SPHERE;    // Current object type being displayed
DrawingMode currentMode = SOLID;      // Current rendering mode
TrajectoryMode trajectoryMode = NONE; // Current trajectory visualization mode
GridMode gridMode = GRID_NONE;        // Current grid display mode

/**
 * Color palette with 8 predefined colors
 */
vec4 colorPalette[8] = {
    vec4(1.0, 0.3, 0.3, 1.0),  // Red
    vec4(1.0, 0.7, 0.2, 1.0),  // Orange
    vec4(1.0, 1.0, 0.3, 1.0),  // Yellow
    vec4(0.4, 1.0, 0.4, 1.0),  // Green
    vec4(0.3, 0.6, 1.0, 1.0),  // Blue
    vec4(0.9, 0.3, 1.0, 1.0),  // Purple
    vec4(1.0, 0.5, 1.0, 1.0),  // Pink
    vec4(0.2, 1.0, 1.0, 1.0)   // Cyan
};
int currentColorIndex = 0;      // Index into color palette
bool rainbowMode = false;       // Rainbow color cycling mode
bool multipleObjects = false;   // Multiple objects mode

/**
 * Physics state for main simulation object
 */
float xPos = 0.0f, yPos = 0.0f;  // Position
float xVel = 6.0f, yVel = -2.0f; // Velocity
float initialVelocityX = 6.0f;   // Initial velocity for resets
float initialVelocityY = -2.0f;  // Initial velocity for resets
float currentTime = 0.0f;        // Current simulation time
float gravityStrength = GRAVITY; // Current gravity value

/**
 * New feature variables
 */
float simulationSpeed = 1.0f;    // Simulation speed multiplier
vec4 backgroundColor = vec4(0.1f, 0.1f, 0.1f, 1.0f); // Background color
int backgroundColorIndex = 0;    // Background color index
float objectScale = 1.0f;        // Object scaling factor
vec4 gridColor = vec4(0.3f, 0.3f, 0.3f, 0.5f); // Grid color

/**
 * Multiple objects mode variables
 */
std::vector<BallObject> balls;   // List of ball objects
float launchInterval = 1.5f;     // Time between auto-launches
float lastLaunchTime = 0.0f;     // Time of last launch

/**
 * Particle effects variables
 */
std::vector<Particle> particles; // List of particles
bool showParticles = false;      // Particle effects toggle

/**
 * Trajectory visualization variables
 */
std::deque<TrajectoryPoint> trajectoryPoints; // List of trajectory points

/**
 * OpenGL shader variables
 */
GLuint phongProgram = 0;
GLuint gouraudProgram = 0;
GLuint currentProgram = 0;
GLuint modelLoc = 0;             // Model matrix uniform location
GLuint projectionLoc = 0;        // Projection matrix uniform location
GLuint objColorLoc = 0;          // Object color uniform location
GLuint lightDirLoc = 0;          // Light direction uniform location
GLuint viewPosLoc = 0;           // View position uniform location

/**
 * Cube geometry data
 */
std::vector<vec4> cubeVertices;  // Cube vertex positions
std::vector<vec3> cubeNormals;   // Cube vertex normals
GLuint vaoCube = 0, vboCube = 0; // Cube VAO and VBO handles
int numCubeVertices = 0;         // Number of vertices in cube

/**
 * Sphere geometry data
 */
std::vector<vec4> sphereVertices; // Sphere vertex positions
std::vector<vec3> sphereNormals;  // Sphere vertex normals
GLuint vaoSphere = 0, vboSphere = 0; // Sphere VAO and VBO handles
int numSphereVertices = 0;       // Number of vertices in sphere

/**
 * Bunny geometry data
 */
std::vector<vec4> bunnyVertices; // Bunny vertex positions
std::vector<vec3> bunnyNormals;  // Bunny vertex normals
GLuint vaoBunny = 0, vboBunny = 0; // Bunny VAO and VBO handles
int numBunnyVertices = 0;        // Number of vertices in bunny
bool bunnyLoaded = false;        // Flag indicating if bunny was loaded

/**
 * Object rotation variables
 */
float bunnyRotation = 0.0f;      // Bunny rotation angle
float cubeRotation = 0.0f;       // Cube rotation angle

/**
 * Trajectory rendering variables
 */
GLuint vaoTrajectory = 0, vboTrajectory = 0; // Trajectory VAO and VBO handles

/**
 * Prints help information to the console
 */
void printHelp() {
    std::cout << "======== Enhanced Bouncing Ball Simulation ========\n";
    std::cout << "  Assignment 3 - Shading and Texture Mapping\n";
    std::cout << "\n  Basic Controls:\n";
    std::cout << "    h, F1: Print this help message\n";
    std::cout << "    q, Escape: Quit\n";
    std::cout << "    Space, F5: Restart simulation\n";
    std::cout << "\n  Shading Controls:\n";
    std::cout << "    S: Toggle between Phong and Gouraud shading\n";
    std::cout << "    O: Toggle lighting components (Ambient->Diffuse->Specular)\n";
    std::cout << "    L: Toggle light movement (Fixed/Follow object)\n";
    std::cout << "    M: Toggle material (Plastic/Metallic)\n";
    std::cout << "\n  Display Controls:\n";
    std::cout << "    T: Toggle display mode (Wireframe->Shading->Texture)\n";
    std::cout << "    I: Toggle texture images\n";
    std::cout << "    Z: Zoom in\n";
    std::cout << "    W: Zoom out\n";
    std::cout << "\n  Object Controls:\n";
    std::cout << "    1: Switch to Cube\n";
    std::cout << "    2: Switch to Sphere\n";
    std::cout << "    3: Switch to Bunny\n";
    std::cout << "    c: Change color\n";
    std::cout << "\n  Mouse Controls:\n";
    std::cout << "    Left: Toggle wireframe/solid\n";
    std::cout << "    Right: Cycle objects\n";
    std::cout << "    Middle: Restart simulation\n";
    std::cout << "=================================================\n";
}

/**
 * Takes a screenshot and saves it to the specified file
 */
void takeScreenshot(const std::string& filename) {
    // Allocate memory for the pixel data
    unsigned char* pixels = new unsigned char[3 * windowWidth * windowHeight];
    
    // Read pixels from framebuffer
    glReadPixels(0, 0, windowWidth, windowHeight, GL_RGB, GL_UNSIGNED_BYTE, pixels);
    
    // Open file for writing
    std::ofstream file(filename.c_str(), std::ios::binary);
    if (!file) {
        std::cerr << "Failed to open file for screenshot: " << filename << std::endl;
        delete[] pixels;
        return;
    }
    
    // Write PPM header
    file << "P6\n" << windowWidth << " " << windowHeight << "\n255\n";
    
    // Write pixel data (flipping vertically since OpenGL's origin is bottom-left)
    for (int y = windowHeight - 1; y >= 0; y--) {
        for (int x = 0; x < windowWidth; x++) {
            int pos = (y * windowWidth + x) * 3;
            file.write(reinterpret_cast<char*>(&pixels[pos]), 3);
        }
    }
    
    file.close();
    delete[] pixels;
    
    std::cout << "Screenshot saved to " << filename << std::endl;
}