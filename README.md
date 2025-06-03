# Enhanced Bouncing Ball Simulation - Documentation

## Overview

This application simulates a bouncing ball under the influence of gravity with a rich set of interactive features. It demonstrates several fundamental computer graphics and physics simulation concepts, including:

- 3D object rendering with OpenGL
- User interaction and event-driven programming
- Physics simulation (gravity, collisions, air resistance)
- Trajectory visualization
- Various visual effects

The simulation allows the user to choose between different 3D objects (cube, sphere, bunny), rendering modes (wireframe, solid), and offers various visualization options.

## Files and Structure

### Main Components

1. **main.cpp**
   - Program entry point and initialization
   - OpenGL context setup
   - Object and shader initialization
   - Main application loop

2. **Globals.h/cpp**
   - Global state variables and constants
   - Shared data structures used across the program
   - Help text display

3. **input.cpp**
   - Handles user input via keyboard and mouse
   - Event callback functions

4. **objects.cpp**
   - 3D object initialization (cube, sphere, bunny)
   - Geometry generation and loading

5. **physics.cpp**
   - Physics simulation logic
   - Ball movement and collision detection
   - Particle system updates

6. **render.cpp**
   - Scene rendering functions
   - Trajectory visualization
   - Visual effects

7. **shader.cpp/h**
   - Shader program loading
   - OpenGL shader management

8. **Shader files**
   - vshader.glsl: Vertex shader for 3D transformations
   - fshader.glsl: Fragment shader for lighting and coloring

## Key Features

### Object Types
- **Cube**: Simple cubic object
- **Sphere**: Subdivided icosahedron sphere 
- **Bunny**: Stanford bunny 3D model (loaded from OFF file)

### Rendering Modes
- **Solid**: Filled polygons with lighting
- **Wireframe**: Line rendering of polygon edges

### Physics Simulation
- **Gravity**: Constant downward acceleration
- **Bouncing**: Elastic collision with energy loss
- **Air Resistance**: Velocity damping over time

### Visual Effects
- **Trajectory Visualization**: Shows the path of the object
  - Line mode: Continuous line trail
  - Strobe mode: Object instances along the path
- **Particle Effects**: Particles generated on bounce
- **Color Options**: Multiple color choices with rainbow mode
- **Grid Display**: Reference grid with varying detail levels

### Enhanced Features
- **Background Color**: Multiple background options
- **Simulation Speed**: Adjustable speed control
- **Object Scaling**: Size adjustment for objects
- **Screenshot**: Save current view to image file

## Controls

### Mouse Controls
- **Left Mouse Button**: Toggle wireframe/solid mode
- **Right Mouse Button**: Cycle objects (Cube, Sphere, Bunny)
- **Middle Mouse Button**: Launch a new ball or restart simulation

### Basic Controls
- **i, F5, Home, Space**: Restart simulation
- **c**: Change color (Shift+c toggles rainbow mode)
- **p**: Cycle trajectory modes (None → Line → Strobe)
- **m**: Toggle multiple objects mode
- **g**: Decrease gravity (Shift+g to increase)
- **e**: Toggle particle effects
- **r**: Reset settings to default
- **1/NumPad1**: Switch to Cube
- **2/NumPad2**: Switch to Sphere
- **3/NumPad3**: Switch to Bunny

### Enhanced Feature Controls
- **b**: Change background color
- **+/-**: Adjust simulation speed
- **z/x**: Decrease/increase object size
- **t**: Cycle grid display modes
- **F12**: Take screenshot
- **h, F1**: Print help message
- **q, Escape**: Quit

## Technical Details

### OpenGL Implementation

The application uses modern shader-based OpenGL (OpenGL 3.2+ Core Profile) with the following features:
- Vertex Array Objects (VAOs) and Vertex Buffer Objects (VBOs)
- GLSL shaders for vertex and fragment processing
- Depth testing and alpha blending
- Dynamic buffer updates for trajectory visualization

### Physics Simulation

The physics system implements:
- Verlet integration for position updates
- Collision detection with boundaries
- Coefficient of restitution for energy loss
- Air resistance as a velocity multiplier
- Variable gravity strength

### Memory Management

- Dynamic allocation for geometry data
- Proper cleanup of OpenGL resources
- Trajectory point limiting to avoid memory overflow

## Function Reference

### main.cpp

- **main()**: Program entry point, initializes GLFW, OpenGL, and runs the main loop
- **setupVAO()**: Sets up Vertex Array Objects for 3D models
- **setupTrajectoryVAO()**: Creates VAO for trajectory visualization

### input.cpp

- **key_callback()**: Handles keyboard input events
- **mouse_button_callback()**: Handles mouse button events
- **framebuffer_size_callback()**: Handles window resize events
- **registerCallbacks()**: Registers all input callbacks with GLFW

### objects.cpp

- **initCube()**: Creates cube geometry
- **initSphere()**: Creates sphere geometry with specified subdivision level
- **loadBunnyModel()**: Loads Stanford bunny from OFF file
- **calculateBunnyNormals()**: Computes normals for bunny model

### physics.cpp

- **initBall()**: Initializes ball position and velocity
- **launchBall()**: Creates a new ball in multiple objects mode
- **updateBall()**: Updates physics for all objects
- **updateParticles()**: Updates particle effects

### render.cpp

- **display()**: Main rendering function
- **drawObject()**: Renders a specific object type
- **drawTrajectory()**: Visualizes object trajectory
- **drawGrid()**: Draws reference grid
- **getRainbowColor()**: Generates color for rainbow mode

### Globals.cpp

- **printHelp()**: Displays help information
- **takeScreenshot()**: Saves current view to image file

## Implementation Notes

### Performance Considerations

- Vertex count is managed through sphere subdivision level
- Trajectory point count is limited to maintain performance
- Grid detail adapts based on selected mode

### Extensions and Future Improvements

- Advanced lighting and shading effects
- Physics parameter adjustments (air resistance, restitution)
- Multiple simultaneous object types
- Collision between objects
- More complex environments and obstacles

## Credits

This enhanced bouncing ball simulation was developed for a computer graphics course. It builds upon fundamental OpenGL concepts and extends them with interactive features and visual enhancements.

The Stanford bunny model is courtesy of the Stanford University Computer Graphics Laboratory.