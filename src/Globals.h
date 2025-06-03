#ifndef GLOBALS_H
#define GLOBALS_H

#include "Angel.h"
#include <vector>
#include <deque>
#include <string>

// Shader program handles
extern GLuint phongProgram, gouraudProgram, currentProgram;

// Lighting and material properties
extern bool lightFollowsObject;
extern bool useMetallic;
extern float zoomScale;  // For zoom functionality

// Lighting component toggles - FIXED: Now properly declared
extern bool useAmbient;
extern bool useDiffuse; 
extern bool useSpecular;

// Render modes
enum RenderMode { WIREFRAME_MODE, SHADING_MODE, TEXTURE_MODE };
extern RenderMode currentRenderMode;

// Material parameters
extern float metallicShininess;
extern float plasticShininess;
extern float metallicSpecularStrength;
extern float plasticSpecularStrength;
extern bool useGouraud;

// Window dimensions
extern int windowWidth;
extern int windowHeight;

// Constants
extern const float GRAVITY;
extern const float RESTITUTION;
extern const float BALL_SIZE;
extern const float BUNNY_SCALE;
extern const int MAX_TRAJECTORY_POINTS;
extern const float AIR_RESISTANCE;

// Enumerations
enum ObjectType { CUBE, SPHERE, BUNNY };
enum DrawingMode { WIREFRAME, SOLID };
enum TrajectoryMode { NONE, LINE, STROBE };
enum GridMode { GRID_NONE, GRID_BASIC, GRID_DETAILED };

// Global state
extern ObjectType currentObject;
extern DrawingMode currentMode;
extern TrajectoryMode trajectoryMode;

// Color palette (8 colors)
extern vec4 colorPalette[8];
extern int currentColorIndex;
extern bool rainbowMode;
extern bool multipleObjects;

// Physics for a single object
extern float xPos, yPos;
extern float xVel, yVel;
extern float initialVelocityX, initialVelocityY;
extern float currentTime;
extern float gravityStrength;

// Simulation speed control
extern float simulationSpeed;

// Background color options
extern vec4 backgroundColor;
extern int backgroundColorIndex;

// Object scaling
extern float objectScale;

// Grid display
extern GridMode gridMode;
extern vec4 gridColor;
extern GLuint texID;

// Multi-object mode
struct BallObject {
    float x, y;
    float vx, vy;
    int colorIndex;
    ObjectType type;
    float size;
    float launchTime;
};
extern std::vector<BallObject> balls;
extern float launchInterval;
extern float lastLaunchTime;

// Particle effect
struct Particle {
    vec2 position;
    vec2 velocity;
    vec4 color;
    float life;
    float size;
};
extern std::vector<Particle> particles;
extern bool showParticles;

// Trajectory visualization
struct TrajectoryPoint {
    vec2 position;
    float timeStamp;
};
extern std::deque<TrajectoryPoint> trajectoryPoints;

// OpenGL variables
extern GLuint modelLoc, projectionLoc, objColorLoc;
extern GLuint lightDirLoc, viewPosLoc;

// Cube data
extern std::vector<vec4> cubeVertices;
extern std::vector<vec3> cubeNormals;
extern GLuint vaoCube, vboCube;
extern int numCubeVertices;

// Sphere data
extern std::vector<vec4> sphereVertices;
extern std::vector<vec3> sphereNormals;
extern GLuint vaoSphere, vboSphere;
extern int numSphereVertices;

// Bunny data
extern std::vector<vec4> bunnyVertices;
extern std::vector<vec3> bunnyNormals;
extern GLuint vaoBunny, vboBunny;
extern int numBunnyVertices;
extern bool bunnyLoaded;

// Rotations
extern float bunnyRotation;
extern float cubeRotation;

// Trajectory buffers
extern GLuint vaoTrajectory, vboTrajectory;

// Function to print help information
void printHelp();

// Function to take a screenshot
void takeScreenshot(const std::string& filename);

#endif