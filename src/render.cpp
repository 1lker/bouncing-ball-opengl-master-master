#include "render.h"
#include "Globals.h"
#include <iostream>
#include <vector>
#include <sstream>
#include <iomanip>
#include "objects.h"  // ensure this gives access to `sphereData` and `Vertex`

extern bool useGouraud;                 // <- make sure it's defined in Globals
                                         //    (bool useGouraud = false;)

#define gouraudShaderProgram gouraudProgram   // adapt old names
#define phongShaderProgram   phongProgram
// If you *don’t* have a separate texture shader, just point to phongProgram:
#define textureShaderProgram phongProgram
/**
 * Generates a rainbow color based on a time parameter
 * 
 * @param t Time parameter (0-1 range, will be wrapped)
 * @return A vec4 color with full alpha
 */
static vec4 getRainbowColor(float t) {
    t = fmod(t, 1.0f);
    float r = 0.5f + 0.5f * sin(t * 6.28318f);
    float g = 0.5f + 0.5f * sin((t + 0.33333f) * 6.28318f);
    float b = 0.5f + 0.5f * sin((t + 0.66666f) * 6.28318f);
    return vec4(r, g, b, 1.0f);
}

/**
 * Draws a grid on the screen based on the current grid mode
 */
static void drawGrid() {
    if (gridMode == GRID_NONE) return;
    
    // Create a grid of lines
    std::vector<vec4> gridLines;
    
    // Determine grid spacing based on mode
    int spacing = (gridMode == GRID_BASIC) ? 100 : 50;
    
    // Create horizontal lines
    for (int y = 0; y <= windowHeight; y += spacing) {
        gridLines.push_back(vec4(0, y, 0, 1));
        gridLines.push_back(vec4(windowWidth, y, 0, 1));
    }
    
    // Create vertical lines
    for (int x = 0; x <= windowWidth; x += spacing) {
        gridLines.push_back(vec4(x, 0, 0, 1));
        gridLines.push_back(vec4(x, windowHeight, 0, 1));
    }
    
    // Set identity model matrix for grid
    mat4 identityModel = mat4(1.0);
    glUniformMatrix4fv(modelLoc, 1, GL_TRUE, identityModel);
    
    // Bind the trajectory VAO/VBO (reusing it for grid)
    glBindVertexArray(vaoTrajectory);
    glBindBuffer(GL_ARRAY_BUFFER, vboTrajectory);
    glBufferSubData(GL_ARRAY_BUFFER, 0, gridLines.size() * sizeof(vec4), gridLines.data());
    
    // Set grid color and draw lines
    glLineWidth(1.0f);
    glUniform4fv(objColorLoc, 1, gridColor);
    glDrawArrays(GL_LINES, 0, (GLsizei)gridLines.size());
}

/**
 * Draws a specific object at the given position with a specific size
 * 
 * @param objType Type of object to draw (CUBE, SPHERE, BUNNY)
 * @param position Position (x,y) to draw the object at
 * @param size Base size of the object
 * @param color Color to use for the object
 * @param isTrajectory Whether this is a trajectory object (affects rendering settings)
 */
static void drawObject(ObjectType objType, const vec2& position, float size, const vec4& color, bool isTrajectory = false) {
    mat4 model;
    glUniform4fv(objColorLoc, 1, color);
    
    // Apply the global object scale factor to the size
    float scaledSize = size * (isTrajectory ? 1.0f : objectScale);
    if (currentRenderMode == TEXTURE_MODE) {
        // currentProgram = textureShaderProgram;
        currentProgram = phongProgram;
    } else {
        currentProgram = (useGouraud ? gouraudShaderProgram : phongShaderProgram);
    }
    glUseProgram(currentProgram);

    // Set thicker line width for wireframe mode if not a trajectory object
    if (currentMode == WIREFRAME && !isTrajectory) {
        glLineWidth(2.0f);  // Thicker lines in wireframe mode
    }

    if (currentRenderMode == WIREFRAME_MODE) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    } else {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

    
    
    if (objType == SPHERE) {
        model = Scale(zoomScale, zoomScale, zoomScale) * Translate(position.x, position.y, 0.0f) * Scale(scaledSize*0.6f, scaledSize*0.6f, scaledSize*0.6f);

        glUniformMatrix4fv(modelLoc, 1, GL_TRUE, model);
        vec3 worldLightDir(0.5f, 1.0f, 0.75f);
        vec4 tmp = model * vec4(worldLightDir, 0.0f);   // direction -> w = 0
        vec3 transformedLightDir = lightFollowsObject
                                ? vec3(tmp.x, tmp.y, tmp.z)   // or tmp.xyz if your
                                : worldLightDir;              // glsl-style swizzle is supported

        glUniform3fv(glGetUniformLocation(currentProgram, "lightDir"), 1, &transformedLightDir[0]);
        // 💡 Material properties
        float shininess = useMetallic ? metallicShininess : plasticShininess;
        float specularStrength = useMetallic ? metallicSpecularStrength : plasticSpecularStrength;
        if (currentRenderMode != TEXTURE_MODE) {
            glUniform1f(glGetUniformLocation(currentProgram, "shininess"), shininess);
            glUniform1f(glGetUniformLocation(currentProgram, "specularStrength"), specularStrength);
        }
        glBindVertexArray(vaoSphere);
        glActiveTexture(GL_TEXTURE0);        // Activate texture unit 0
        glBindTexture(GL_TEXTURE_2D, texID); // Bind our texture
        glDrawArrays(GL_TRIANGLES, 0, numSphereVertices);
    }
    else if (objType == BUNNY && bunnyLoaded) {
        model = Translate(position.x, position.y, 0.0f)
              * Scale(scaledSize*0.15f, scaledSize*0.15f, scaledSize*0.15f)
              * RotateY(bunnyRotation)
              * RotateX(90.0f);
        glUniformMatrix4fv(modelLoc, 1, GL_TRUE, model);
        glBindVertexArray(vaoBunny);
        glDrawArrays(GL_TRIANGLES, 0, numBunnyVertices);
    }
    else { // default: cube
        model = Translate(position.x, position.y, 0.0f)
              * Scale(scaledSize, scaledSize, scaledSize)
              * RotateY(cubeRotation) * RotateX(20.0f) * RotateZ(10.0f);
        glUniformMatrix4fv(modelLoc, 1, GL_TRUE, model);
        glBindVertexArray(vaoCube);
        glDrawArrays(GL_TRIANGLES, 0, numCubeVertices);
    }
    
    // Reset line width
    if (currentMode == WIREFRAME && !isTrajectory) {
        glLineWidth(1.0f);
    }
}

/**
 * Draws trajectory visualization based on the current trajectory mode
 */
static void drawTrajectory() {
    if (trajectoryMode == NONE || trajectoryPoints.size() < 2) return;
    
    // Still draw a connecting line for LINE mode to show the path
    if (trajectoryMode == LINE) {
        // Convert trajectory points to vec4
        std::vector<vec4> lineVerts;
        lineVerts.reserve(trajectoryPoints.size());
        for (auto& tp : trajectoryPoints) {
            lineVerts.push_back(vec4(tp.position.x, tp.position.y, 0.0f, 1.0f));
        }
        
        // Set identity model matrix for trajectory
        mat4 identityModel = mat4(1.0);
        glUniformMatrix4fv(modelLoc, 1, GL_TRUE, identityModel);
        
        // Bind the trajectory VAO/VBO and upload data
        glBindVertexArray(vaoTrajectory);
        glBindBuffer(GL_ARRAY_BUFFER, vboTrajectory);
        glBufferSubData(GL_ARRAY_BUFFER, 0, lineVerts.size()*sizeof(vec4), lineVerts.data());
        
        // Set line width and color
        glLineWidth(2.0f);
        vec4 lineColor(0.7, 0.7, 0.7, 0.5); // Semitransparent line
        glUniform4fv(objColorLoc, 1, lineColor);
        glDrawArrays(GL_LINE_STRIP, 0, (GLsizei)lineVerts.size());
        glLineWidth(1.0f);
    }
    
    // Determine spacing for objects along trajectory
    const int maxObjectsToShow = 10; // Adjust based on performance needs
    int stepSize = std::max(1, (int)trajectoryPoints.size() / maxObjectsToShow);
    
    // Draw objects along the trajectory
    float baseSize = BALL_SIZE * 0.6f; // Smaller than the main object
    
    for (size_t i = 0; i < trajectoryPoints.size(); i += stepSize) {
        // Skip first and last points for cleaner look
        if (i == 0 || i >= trajectoryPoints.size() - 1) continue;
        
        vec2 pos = trajectoryPoints[i].position;
        float timeFactor = (float)i / trajectoryPoints.size();
        float objSize = baseSize * (0.5f + 0.5f * (1.0f - timeFactor)); // Size decreases with time
        
        vec4 objColor;
        if (rainbowMode) {
            objColor = getRainbowColor(timeFactor);
        } else {
            objColor = colorPalette[currentColorIndex];
            objColor.w = 0.5f + 0.5f * timeFactor; // Fade in with time
        }
        
        // For strobe effect, only show a few objects at wider intervals
        if (trajectoryMode == STROBE && i % (stepSize * 2) != 0) {
            continue;
        }
        
        // Draw the object at this trajectory point (mark as trajectory to use different settings)
        drawObject(currentObject, pos, objSize, objColor, true);
    }
}

/**
 * Main display function that renders all elements of the scene
 */
void display() {
    // Set background color
    glClearColor(backgroundColor.x, backgroundColor.y, backgroundColor.z, backgroundColor.w);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    // Set polygon mode 
    if (currentMode == WIREFRAME) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }
    else {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
    
    // Draw grid first (if enabled)
    drawGrid();
    
    // Then draw trajectory objects
    drawTrajectory();
    
    // Then draw the main object
    vec4 mainColor = colorPalette[currentColorIndex];
    if (rainbowMode) {
        mainColor = getRainbowColor(currentTime * 0.3f);
    }
    
    drawObject(currentObject, vec2(xPos, yPos), BALL_SIZE, mainColor);
    
    glFlush();
}