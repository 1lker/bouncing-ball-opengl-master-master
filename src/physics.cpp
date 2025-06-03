#include "physics.h"
#include "Globals.h"
#include <iostream>
#include <cmath>
#include <cstdlib>

/**
 * Initialize the ball at the starting position with initial velocity
 * Clears trajectory points and multiple balls
 */
void initBall() {
    // Set the ball's initial position (e.g., top left corner)
    float margin = windowWidth * 0.05f;
    xPos = margin;
    yPos = margin;
    
    // Use the globals xVel and yVel
    xVel = initialVelocityX;
    yVel = initialVelocityY;
    
    currentTime = 0.0f;
    
    // Initialize with starting point
    trajectoryPoints.clear();
    TrajectoryPoint pt;
    pt.position = vec2(xPos, yPos);
    pt.timeStamp = currentTime;
    trajectoryPoints.push_back(pt);
    
    // Clear any existing multiple-ball objects
    balls.clear();
    
    std::cout << "Ball initialized at (" << xPos << ", " << yPos << ")" << std::endl;
}

/**
 * Launch a new ball in multiple objects mode
 * Ball is created with random properties
 */
void launchBall() {
    BallObject ball;
    
    // Set random starting position near top-left
    float marginX = windowWidth * 0.1f;
    float marginY = windowHeight * 0.1f;
    ball.x = marginX + (marginX * (rand() % 100) / 100.0f);
    ball.y = marginY + (marginY * (rand() % 100) / 100.0f);
    
    // Set random initial velocity based on global settings
    ball.vx = initialVelocityX * (0.8f + (rand() % 40) / 100.0f);
    ball.vy = initialVelocityY * (0.8f + (rand() % 40) / 100.0f);
    
    // Set random properties
    ball.colorIndex = rand() % 8;
    ball.type = static_cast<ObjectType>(rand() % (bunnyLoaded ? 3 : 2));
    ball.size = BALL_SIZE * (0.6f + (rand() % 80) / 100.0f);
    ball.launchTime = currentTime;
    
    // Add to the balls list
    balls.push_back(ball);
    
    std::cout << "Launched ball at (" << ball.x << ", " << ball.y << ")" << std::endl;
}

/**
 * Update the physics simulation for a time step
 * 
 * @param deltaTime Time step size in seconds
 */
void updateBall(float deltaTime) {
    // Apply simulation speed to delta time
    float scaledDeltaTime = deltaTime * simulationSpeed;
    
    // Update time and rotations
    currentTime += scaledDeltaTime;
    bunnyRotation += scaledDeltaTime * 30.0f;
    cubeRotation += scaledDeltaTime * 20.0f;
    
    if (multipleObjects) {
        // Multiple objects mode: update each ball independently
        for (auto it = balls.begin(); it != balls.end(); ) {
            // Apply gravity
            it->vy += gravityStrength;
            
            // Apply air resistance
            it->vx *= AIR_RESISTANCE;
            it->vy *= AIR_RESISTANCE;
            
            // Update position using the ball's velocity values
            it->x += it->vx * simulationSpeed;
            it->y += it->vy * simulationSpeed;
            
            // Define boundaries (using a 10% margin at the bottom)
            float bottom = windowHeight * 0.9f;
            if (it->y > bottom) {
                // Bounce with energy loss
                it->vy = -it->vy * RESTITUTION;
                it->y = bottom;
                if (fabs(it->vy) < 0.5f) {
                    it->vy = 0.0f;
                }
                
                // Generate particles on bounce if enabled
                if (showParticles) {
                    // Create 5-10 particles on bounce
                    int numParticles = 5 + (rand() % 6);
                    for (int i = 0; i < numParticles; i++) {
                        Particle p;
                        p.position = vec2(it->x, bottom);
                        // Random velocity, mostly upward
                        p.velocity = vec2(
                            (rand() % 200 - 100) / 10.0f,  // -10 to 10
                            -(rand() % 100) / 10.0f - 5.0f  // -15 to -5
                        );
                        // Use the ball's color
                        p.color = colorPalette[it->colorIndex];
                        p.color.w = 0.7f;  // Semi-transparent
                        p.life = 0.5f + (rand() % 100) / 100.0f;  // 0.5 to 1.5 seconds
                        p.size = 3.0f + (rand() % 50) / 10.0f;  // 3 to 8 pixels
                        particles.push_back(p);
                    }
                }
            }
            
            // Check side boundaries
            float left = windowWidth * 0.05f;
            float right = windowWidth * 0.95f;
            if (it->x < left) {
                it->x = left;
                it->vx = -it->vx * RESTITUTION;
            }
            if (it->x > right) {
                it->x = right;
                it->vx = -it->vx * RESTITUTION;
            }
            
            // Remove ball if its lifetime exceeds 30 seconds or energy is too low
            float lifetime = currentTime - it->launchTime;
            float energy = fabs(it->vx) + fabs(it->vy);
            if (lifetime > 30.0f || (it->y >= bottom - 1.0f && energy < 0.1f)) {
                it = balls.erase(it);
            } else {
                ++it;
            }
        }
    } else {
        // Single ball mode: update global xVel and yVel
        yVel += gravityStrength;
        xVel *= AIR_RESISTANCE;
        yVel *= AIR_RESISTANCE;
        xPos += xVel * simulationSpeed;
        yPos += yVel * simulationSpeed;
        
        float bottom = windowHeight * 0.9f;
        if (yPos > bottom) {
            yVel = -yVel * RESTITUTION;
            yPos = bottom;
            
            if (fabs(yVel) < 0.5f)
                yVel = 0.0f;
                
            // Generate particles on bounce if enabled
            if (showParticles) {
                // Create 5-10 particles on bounce
                int numParticles = 5 + (rand() % 6);
                for (int i = 0; i < numParticles; i++) {
                    Particle p;
                    p.position = vec2(xPos, bottom);
                    // Random velocity, mostly upward
                    p.velocity = vec2(
                        (rand() % 200 - 100) / 10.0f,  // -10 to 10
                        -(rand() % 100) / 10.0f - 5.0f  // -15 to -5
                    );
                    // Use the current color
                    p.color = colorPalette[currentColorIndex];
                    p.color.w = 0.7f;  // Semi-transparent
                    p.life = 0.5f + (rand() % 100) / 100.0f;  // 0.5 to 1.5 seconds
                    p.size = 3.0f + (rand() % 50) / 10.0f;  // 3 to 8 pixels
                    particles.push_back(p);
                }
            }
        }
        
        float left = windowWidth * 0.05f;
        float right = windowWidth * 0.95f;
        if (xPos < left) {
            xPos = left;
            xVel = -xVel * RESTITUTION;
        }
        if (xPos > right) {
            xPos = right;
            xVel = -xVel * RESTITUTION;
        }
        
        // Add trajectory recording regardless of current mode to build up trajectory data
        // This ensures trajectory points are always recorded for when user enables display
        if (trajectoryPoints.empty() ||
            length(vec2(xPos, yPos) - trajectoryPoints.back().position) > 5.0f) {
            TrajectoryPoint pt;
            pt.position = vec2(xPos, yPos);
            pt.timeStamp = currentTime;
            trajectoryPoints.push_back(pt);
            
            // Limit the number of points to avoid excessive memory usage
            while (trajectoryPoints.size() > MAX_TRAJECTORY_POINTS)
                trajectoryPoints.pop_front();
        }
    }
}

/**
 * Update all particles in the scene
 * 
 * @param deltaTime Time step size in seconds
 */
void updateParticles(float deltaTime) {
    // Apply simulation speed to delta time
    float scaledDeltaTime = deltaTime * simulationSpeed;
    
    for (auto it = particles.begin(); it != particles.end(); ) {
        // Apply gravity to particles (half strength for visual appeal)
        it->velocity.y += gravityStrength * 0.5f;
        
        // Update position
        it->position += it->velocity * simulationSpeed;
        
        // Decrease life span
        it->life -= scaledDeltaTime;
        
        if (it->life <= 0.0f)
            it = particles.erase(it);
        else {
            // Fade out as life decreases
            it->color.w = it->life;
            ++it;
        }
    }
}