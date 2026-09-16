# 3D Train Logistics Game

A 3D train logistics game developed in **C++** using **OpenGL, GLSL, GLM, and the GFX Framework**.

The project combines real-time 3D rendering, graph-based railway navigation, vector mathematics, collision detection, procedural geometry, resource management, and gameplay logic.

The player controls the route of a continuously moving train through a railway network, collects resources from different stations in a required order, and returns to the main station before the timer expires.

---

## Features

- Real-time 3D rendering using OpenGL
- Graph-based railway network with intersections
- Normal tracks, bridges, and tunnels
- Player-controlled route selection
- Ordered resource collection and delivery system
- Resource respawn mechanics
- Timed objectives and scoring
- Locomotive with multiple following wagons
- AABB collision detection
- Procedurally generated 3D geometry
- Perspective main camera
- Orthographic real-time minimap
- Custom GLSL shaders
- Interactive camera controls
- Game Over state

---

## Railway Network and Route Selection

The railway system is represented as a network of interconnected rail segments.

Each rail stores its start and end positions, rail type, and connections to other parts of the network.

When the locomotive reaches an intersection, the game determines the available outgoing tracks and selects the route that best matches the direction requested by the player.

The route-selection system uses vector mathematics, including:

- Direction vectors
- Dot products
- Cross products
- Signed angles

These calculations allow the game to classify possible directions as **straight, left, right, or backward** relative to the train's current movement direction.

This allows intersections to be handled dynamically instead of relying on hardcoded route choices.

---

## Train Movement and Wagon Following

The locomotive moves continuously between the endpoints of each rail segment.

Movement is calculated using **delta time**, making the train speed independent of the rendering frame rate.

The locomotive's position along a rail is determined using linear interpolation between the start and end points of the current segment, while its orientation is updated according to the direction of the track.

The wagons follow the locomotive using a position and rotation history stored in `std::deque`.

Instead of calculating an independent path for every wagon, each wagon reads an older state from the locomotive's movement history using a different delay.

This causes the wagons to naturally reproduce the same path previously taken by the locomotive, including turns and railway intersections.

---

## Resource and Delivery System

The game contains several stations associated with different resource types.

A new objective consists of a randomly generated sequence of resources that must be collected in the correct order.

When the train reaches the correct resource:

1. The resource is collected
2. It is removed from the active order
3. The resource temporarily disappears
4. A respawn timer is started

After the respawn timer expires, the resource becomes available again.

Once all required resources have been collected, the player must return to the main station to complete the delivery.

Completing a delivery increases the score and generates a new order.

Each objective must be completed before the timer reaches zero.

---

## Collision Detection

Interactions between the locomotive, stations, and collectible resources are implemented using **Axis-Aligned Bounding Boxes (AABB)**.

Each relevant object is approximated by a 3D bounding box defined by minimum and maximum coordinates.

Two objects are considered to intersect when their bounding boxes overlap on all three axes.

AABB collision detection is used to detect:

- Station interactions
- Resource collection
- Delivery at the main station

This provides an efficient collision-detection method without requiring more expensive mesh-level collision calculations.

---

## 3D Rendering

The project uses OpenGL for real-time graphics rendering.

Objects in the scene are positioned using transformation matrices for:

- Translation
- Rotation
- Scaling

The rendering process uses the standard **Model-View-Projection** pipeline:

- Model matrix
- View matrix
- Projection matrix

GLM is used for vector and matrix operations, while GLSL shaders are integrated into the rendering pipeline.

The main game scene uses perspective projection.

---

## Procedural Geometry

Several 3D objects are generated directly in code instead of being loaded from external models.

The project includes procedural generation for:

- Cubes
- Cylinders
- Spheres
- Cones

Vertices and triangle indices are calculated programmatically and uploaded to the GPU using OpenGL buffers.

The implementation works with:

- VAO - Vertex Array Objects
- VBO - Vertex Buffer Objects
- IBO - Index Buffer Objects

These reusable primitive meshes are transformed and combined to construct the locomotive, wagons, stations, resources, railway infrastructure, and surrounding environment.

---

## Real-Time Minimap

The game includes a real-time minimap rendered in a separate OpenGL viewport.

The main scene uses a perspective camera, while the minimap uses a second camera positioned above the environment with an orthographic projection.

The scene is rendered again from this top-down camera to display the railway network, stations, locomotive, wagons, and environment.

---

## Controls

### Train Controls

- `W` - Continue straight
- `A` - Turn left
- `D` - Turn right
- `S` - Go back

### Camera Controls

While holding the **Right Mouse Button**:

- Arrow Keys - Move the camera
- `Q` - Move downward
- `E` - Move upward
- Mouse Movement - Rotate the camera

---

## Technologies

- C++
- OpenGL
- GLSL
- GLM
- GFX Framework
- Visual Studio

---

## Project Structure

The project-specific implementation is located in:

```text
src/lab_m1/Tema2/
```

Main files:

```text
Tema2.cpp
Tema2.h
shaders/
    VertexShader.glsl
    FragmentShader.glsl
```

`Tema2.cpp` contains the main gameplay logic, railway system, train movement, rendering, collision detection, resource mechanics, minimap, and UI logic.

`Tema2.h` defines the main game class and the data structures used for rails, train components, stations, collision boxes, cameras, and game state.

---

## Framework Notice

This project was developed on top of the provided **GFX Framework**.

The project-specific implementation includes the railway network, route-selection logic, gameplay mechanics, train and wagon movement, resource system, collision detection, procedural geometry, rendering logic, minimap, shaders, camera interaction, scoring system, timer, and Game Over behavior.
