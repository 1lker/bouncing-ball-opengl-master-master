#include "render.h"
#include "Globals.h"
#include <iostream>
#include <vector>
#include <sstream>
#include <iomanip>
#include "objects.h"

/**
 * Generates a rainbow color based on a time parameter
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
    
    std::vector<vec4> gridLines;
    
    // For perspective projection, create a grid in world space
    int spacing = (gridMode == GRID_BASIC) ? 2 : 1;
    
    // Create horizontal lines (in world coordinates)
    for (int y = -10; y <= 10; y += spacing) {
        gridLines.push_back(vec4(-10, y, 0, 1));
        gridLines.push_back(vec4(10, y, 0, 1));
    }
    
    // Create vertical lines
    for (int x = -10; x <= 10; x += spacing) {
        gridLines.push_back(vec4(x, -10, 0, 1));
        gridLines.push_back(vec4(x, 10, 0, 1));
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
 * Convert screen coordinates to world coordinates for perspective projection
 */
vec2 screenToWorld(float screenX, float screenY) {
    // Convert screen coordinates to normalized device coordinates (-1 to 1)
    float ndcX = (2.0f * screenX / windowWidth) - 1.0f;
    float ndcY = 1.0f - (2.0f * screenY / windowHeight);
    
    // For perspective projection, scale to world coordinates
    // Assuming the ball moves in a plane at z = 0, convert to world space
    float worldX = ndcX * 10.0f;  // Scale to world coordinates (-10 to 10)
    float worldY = ndcY * 7.5f;   // Scale maintaining aspect ratio
    
    return vec2(worldX, worldY);
}

/**
 * Draws a specific object at the given position with a specific size
 */
static void drawObject(ObjectType objType, const vec2& position, float size, const vec4& color, bool isTrajectory = false) {
    mat4 model;
    
    // Convert screen position to world position for perspective projection
    vec2 worldPos = screenToWorld(position.x, position.y);
    
    // Apply the global object scale factor to the size
    float scaledSize = (size * (isTrajectory ? 1.0f : objectScale)) * 0.01f; // Scale down for world coordinates
    
    // Select appropriate shader program based on render mode
    GLuint programToUse = currentProgram;
    if (currentRenderMode == TEXTURE_MODE) {
        programToUse = phongProgram;
    } else {
        programToUse = useGouraud ? gouraudProgram : phongProgram;
    }
    
    glUseProgram(programToUse);
    
    // Update uniform locations if program changed
    GLuint tempModelLoc = glGetUniformLocation(programToUse, "model");
    GLuint tempColorLoc = glGetUniformLocation(programToUse, "objColor");
    
    // Set object color
    glUniform4fv(tempColorLoc, 1, color);
    
    // Set polygon mode based on current render mode and drawing mode
    if (currentRenderMode == WIREFRAME_MODE || (currentMode == WIREFRAME && currentRenderMode == SHADING_MODE)) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glLineWidth(isTrajectory ? 1.0f : 2.0f);
    } else {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
    
    if (objType == SPHERE) {
        // Position sphere in world coordinates
        model = Scale(zoomScale, zoomScale, zoomScale) * 
               Translate(worldPos.x, worldPos.y, 0.0f) * 
               Scale(scaledSize, scaledSize, scaledSize);
        
        glUniformMatrix4fv(tempModelLoc, 1, GL_TRUE, model);
        
        // Set lighting direction (transform if light follows object)
        vec3 worldLightDir(0.5f, 1.0f, 0.75f);
        vec3 transformedLightDir = lightFollowsObject
                                ? normalize(vec3((model * vec4(worldLightDir, 0.0f)).x,
                                                (model * vec4(worldLightDir, 0.0f)).y,
                                                (model * vec4(worldLightDir, 0.0f)).z))
                                : worldLightDir;
        
        glUniform3fv(glGetUniformLocation(programToUse, "lightDir"), 1, &transformedLightDir[0]);
        
        // Set material properties
        float shininess = useMetallic ? metallicShininess : plasticShininess;
        float specularStrength = useMetallic ? metallicSpecularStrength : plasticSpecularStrength;
        
        glUniform1f(glGetUniformLocation(programToUse, "shininess"), shininess);
        glUniform1f(glGetUniformLocation(programToUse, "specularStrength"), specularStrength);
        
        // Update lighting component uniforms
        glUniform1i(glGetUniformLocation(programToUse, "useAmbient"), useAmbient);
        glUniform1i(glGetUniformLocation(programToUse, "useDiffuse"), useDiffuse);
        glUniform1i(glGetUniformLocation(programToUse, "useSpecular"), useSpecular);
        
        // Bind texture for texture mode
        if (currentRenderMode == TEXTURE_MODE) {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, texID);
            GLint useTextureLoc = glGetUniformLocation(programToUse, "useTexture");
            if (useTextureLoc != -1) {
                glUniform1i(useTextureLoc, 1);
            }
        } else {
            GLint useTextureLoc = glGetUniformLocation(programToUse, "useTexture");
            if (useTextureLoc != -1) {
                glUniform1i(useTextureLoc, 0);
            }
        }
        
        glBindVertexArray(vaoSphere);
        glDrawArrays(GL_TRIANGLES, 0, numSphereVertices);
    }
    else if (objType == BUNNY && bunnyLoaded) {
        model = Scale(zoomScale, zoomScale, zoomScale) *
               Translate(worldPos.x, worldPos.y, 0.0f) *
               Scale(scaledSize*0.15f, scaledSize*0.15f, scaledSize*0.15f) *
               RotateY(bunnyRotation) *
               RotateX(90.0f);
        
        glUniformMatrix4fv(tempModelLoc, 1, GL_TRUE, model);
        
        // Set lighting and material for bunny
        vec3 worldLightDir(0.5f, 1.0f, 0.75f);
        vec3 transformedLightDir = lightFollowsObject
                                ? normalize(vec3((model * vec4(worldLightDir, 0.0f)).x,
                                                (model * vec4(worldLightDir, 0.0f)).y,
                                                (model * vec4(worldLightDir, 0.0f)).z))
                                : worldLightDir;
        
        glUniform3fv(glGetUniformLocation(programToUse, "lightDir"), 1, &transformedLightDir[0]);
        
        float shininess = useMetallic ? metallicShininess : plasticShininess;
        float specularStrength = useMetallic ? metallicSpecularStrength : plasticSpecularStrength;
        
        glUniform1f(glGetUniformLocation(programToUse, "shininess"), shininess);
        glUniform1f(glGetUniformLocation(programToUse, "specularStrength"), specularStrength);
        
        glUniform1i(glGetUniformLocation(programToUse, "useAmbient"), useAmbient);
        glUniform1i(glGetUniformLocation(programToUse, "useDiffuse"), useDiffuse);
        glUniform1i(glGetUniformLocation(programToUse, "useSpecular"), useSpecular);
        
        glBindVertexArray(vaoBunny);
        glDrawArrays(GL_TRIANGLES, 0, numBunnyVertices);
    }
    else { // default: cube
        model = Scale(zoomScale, zoomScale, zoomScale) *
               Translate(worldPos.x, worldPos.y, 0.0f) *
               Scale(scaledSize, scaledSize, scaledSize) *
               RotateY(cubeRotation) * RotateX(20.0f) * RotateZ(10.0f);
        
        glUniformMatrix4fv(tempModelLoc, 1, GL_TRUE, model);
        
        // Set lighting and material for cube
        vec3 worldLightDir(0.5f, 1.0f, 0.75f);
        vec3 transformedLightDir = lightFollowsObject
                                ? normalize(vec3((model * vec4(worldLightDir, 0.0f)).x,
                                                (model * vec4(worldLightDir, 0.0f)).y,
                                                (model * vec4(worldLightDir, 0.0f)).z))
                                : worldLightDir;
        
        glUniform3fv(glGetUniformLocation(programToUse, "lightDir"), 1, &transformedLightDir[0]);
        
        float shininess = useMetallic ? metallicShininess : plasticShininess;
        float specularStrength = useMetallic ? metallicSpecularStrength : plasticSpecularStrength;
        
        glUniform1f(glGetUniformLocation(programToUse, "shininess"), shininess);
        glUniform1f(glGetUniformLocation(programToUse, "specularStrength"), specularStrength);
        
        glUniform1i(glGetUniformLocation(programToUse, "useAmbient"), useAmbient);
        glUniform1i(glGetUniformLocation(programToUse, "useDiffuse"), useDiffuse);
        glUniform1i(glGetUniformLocation(programToUse, "useSpecular"), useSpecular);
        
        glBindVertexArray(vaoCube);
        glDrawArrays(GL_TRIANGLES, 0, numCubeVertices);
    }
    
    // Reset line width
    glLineWidth(1.0f);
    
    // Switch back to current program
    glUseProgram(currentProgram);
}

/**
 * Draws trajectory visualization based on the current trajectory mode
 */
static void drawTrajectory() {
    if (trajectoryMode == NONE || trajectoryPoints.size() < 2) return;
    
    // Still draw a connecting line for LINE mode to show the path
    if (trajectoryMode == LINE) {
        std::vector<vec4> lineVerts;
        lineVerts.reserve(trajectoryPoints.size());
        for (auto& tp : trajectoryPoints) {
            vec2 worldPos = screenToWorld(tp.position.x, tp.position.y);
            lineVerts.push_back(vec4(worldPos.x, worldPos.y, 0.0f, 1.0f));
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
    const int maxObjectsToShow = 10;
    int stepSize = std::max(1, (int)trajectoryPoints.size() / maxObjectsToShow);
    
    // Draw objects along the trajectory
    float baseSize = BALL_SIZE * 0.6f;
    
    for (size_t i = 0; i < trajectoryPoints.size(); i += stepSize) {
        if (i == 0 || i >= trajectoryPoints.size() - 1) continue;
        
        vec2 pos = trajectoryPoints[i].position;
        float timeFactor = (float)i / trajectoryPoints.size();
        float objSize = baseSize * (0.5f + 0.5f * (1.0f - timeFactor));
        
        vec4 objColor;
        if (rainbowMode) {
            objColor = getRainbowColor(timeFactor);
        } else {
            objColor = colorPalette[currentColorIndex];
            objColor.w = 0.5f + 0.5f * timeFactor;
        }
        
        // For strobe effect, only show a few objects at wider intervals
        if (trajectoryMode == STROBE && i % (stepSize * 2) != 0) {
            continue;
        }
        
        // Draw the object at this trajectory point
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