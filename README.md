# 3D-Train-Game

A 3D train logistics game developed in **C++** using **OpenGL, GLSL, GLM, and the GFX Framework**.

The player controls the direction of a continuously moving train through a railway network containing intersections, bridges, tunnels, stations, and different resource locations. During each round, the game generates an order containing several resources that must be collected in the correct sequence and then delivered back to the main station before the timer expires.

The project combines real-time 3D rendering with gameplay logic, graph-based movement, vector mathematics, collision detection, procedural geometry, multiple cameras, resource management, and a simple scoring and objective system.

A large part of the project was focused on implementing the environment and the game mechanics directly in code rather than relying on imported 3D models or a complete game engine.

## Railway Network and Navigation

The railway system is represented as a graph made up of individual rail segments.

Each rail contains a starting position, an ending position, a rail type, and a list of neighboring rails that can be reached from it.

The network contains regular railway sections together with special bridge and tunnel sections. Several parts of the track connect at intersections, allowing the train to choose between different possible routes instead of following one fixed loop.

Connections between rail segments are created explicitly when the railway graph is initialized. This allows the locomotive to determine which tracks are available whenever it reaches the end of its current segment.

The railway therefore acts both as part of the rendered environment and as the logical structure used by the train movement system.

Intersections are also detected from the railway geometry. Points that belong to several connected rail segments are stored separately and used when rendering additional railway elements around important junctions.

## Route Selection

The player does not directly move the locomotive using free movement. Instead, the train continues moving along the railway while the player selects the direction that should be taken at the next possible intersection.

The available directions are straight, left, right, and backward.

The controls are mapped to `W`, `A`, `D`, and `S`, and the selected direction is stored until the locomotive has to choose its next rail segment.

The route-selection logic uses vector mathematics rather than relying on manually assigned left and right connections.

For every possible neighboring rail, the direction of that rail is compared with the current direction of the locomotive.

Dot products and cross products are used together with signed-angle calculations to determine how the candidate direction is positioned relative to the train.

This makes it possible to distinguish whether a possible path continues forward, turns to the left, turns to the right, or goes back relative to the current movement direction.

The railway graph and the direction calculations work together so that the same route-selection logic can be used at different intersections without having to hardcode a separate control rule for every junction.

## Train Movement

The locomotive moves continuously from the beginning of a rail segment toward its end.

Movement is based on frame delta time, so the train speed is not directly tied to the rendering frame rate.

The locomotive stores its current rail, current progress along that rail, position, rotation, speed, and movement direction.

Its position is calculated by interpolating between the start and end points of the current railway segment. The orientation of the locomotive is also updated according to the direction of the rail so that the train visually follows the track.

When the locomotive reaches the end of a segment, the route-selection system chooses the next connected rail according to the direction requested by the player.

The locomotive can therefore move continuously through the graph while changing direction naturally at railway intersections.

This part of the project required connecting gameplay input with geometric calculations and with the logical representation of the railway network.

## Wagon Following

The train contains a locomotive followed by multiple wagons.

Instead of calculating a completely separate railway path for every wagon, the implementation stores the previous positions and rotations of the locomotive using `std::deque`.

As the locomotive moves, its current state is continuously added to this history.

Each wagon then uses an older position and rotation from the stored history. Different wagons use different delays, which creates spacing between them.

This means that every wagon follows the same route previously taken by the locomotive, including turns and intersection decisions.

The result is a relatively simple wagon-following system that avoids having to independently determine the current rail and route for every train component.

Because the history also contains rotation information, the wagons reproduce both the position and orientation changes of the locomotive.

## Resource Collection and Orders

The game contains several stations associated with different resource types.

At the beginning of an objective, a new order is generated. The order contains a sequence of resource identifiers that must be collected in the correct order.

The player has to navigate the railway network and reach the station containing the resource currently required by the order.

When the locomotive enters the interaction area of the correct resource, that resource is collected and removed from the active order.

The resource then becomes temporarily unavailable and a respawn timer begins.

After enough time has passed, the resource becomes available again and can be collected during a future order.

Because the order has to be completed sequentially, visiting a station is not automatically enough to make progress. The player has to reach the resource that currently appears at the front of the active order.

This creates a route-planning element in addition to simply controlling the train.

## Delivery, Timer, and Score

Each objective is limited by a countdown timer.

A new order resets the available time, and the remaining time is updated continuously while the game is running.

The player must collect all required resources and then return to the main station to finish the delivery.

Successfully completing the objective increases the score and generates another resource order, allowing the game to continue with a new delivery.

The remaining time is displayed on screen using the text-rendering functionality provided by the framework.

If the timer reaches zero before the objective is completed, the game enters a Game Over state.

The normal scene is replaced by a Game Over screen containing text that informs the player that the available time has expired.

This adds a complete gameplay loop around the movement system: receive an order, navigate through the railway network, collect the resources, return to the main station, receive a score increase, and start another objective.

## Collision Detection

Interactions between the train and the stations are detected using **Axis-Aligned Bounding Boxes**, or AABBs.

An AABB is represented by minimum and maximum coordinates on the three spatial axes.

The project generates bounding boxes for the locomotive, stations, and collectible resources.

Two objects are considered to intersect when their coordinate intervals overlap on the X, Y, and Z axes at the same time.

This method is used to determine when the locomotive reaches a resource station and when it reaches the main delivery station.

Using bounding boxes keeps the collision system relatively simple and efficient. The project does not require exact triangle-level collision detection because the important gameplay interactions happen between large scene objects whose approximate volume can be represented sufficiently well using boxes.

The collision system is therefore closely connected to the order and resource logic rather than being used as a general-purpose physics system.

## 3D Rendering

The scene is rendered using OpenGL and the rendering functionality provided by the GFX Framework.

Objects are positioned using model transformations that combine translation, rotation, and scaling.

The rendering pipeline uses the standard Model-View-Projection approach.

The model matrix describes the position and transformation of an individual object, the view matrix represents the camera, and the projection matrix transforms the scene according to the selected perspective.

GLM is used throughout the project for vectors, matrices, transformations, interpolation, distances, normalization, and other mathematical operations.

The main game camera uses a perspective projection, giving the player a normal 3D view of the environment.

The project also defines custom GLSL shaders. The vertex shader applies the model, view, and projection matrices and forwards vertex color information to the fragment shader.

The fragment shader can mix the original object color with an alert color using a time factor, allowing visual effects to be controlled from the application.

## Procedural Geometry

Many of the visible objects are created directly in C++ instead of being loaded from external model files.

The project includes functions for generating cubes, cylinders, spheres, and cones.

For each primitive, the required vertices and triangle indices are calculated programmatically.

The generated data is then uploaded to OpenGL buffers.

The implementation creates and manages Vertex Array Objects, Vertex Buffer Objects, and Index Buffer Objects for the generated meshes.

These primitives are reused with different colors, transformations, positions, and scales to build the environment and gameplay objects.

The locomotive, wagons, railway elements, terrain, stations, resources, bridge components, tunnel areas, and intersection elements can therefore be constructed from reusable geometric pieces.

This approach made procedural mesh generation and low-level GPU buffer management an important part of the project rather than treating the scene only as a collection of imported assets.

## Environment and Railway Types

The game world contains more than a simple flat railway.

Different areas of the terrain are visually represented using different colors and geometric objects, including grass, water, and mountain regions.

The railway itself also contains several track types.

Normal railway segments are rendered differently from bridge and tunnel sections.

Bridge areas include additional structural elements around the tracks, while tunnel sections are visually integrated with the surrounding environment.

Intersections also contain additional objects that make important junctions easier to recognize.

These elements give the railway graph a visual representation that corresponds to the logical rail types stored by the program.

## Minimap and Multiple Cameras

The game includes a real-time minimap rendered using a second camera.

The main camera uses perspective projection and displays the normal gameplay view.

The minimap camera is positioned above the environment and uses an orthographic projection.

During rendering, the scene is drawn again from this top-down camera into a different OpenGL viewport.

Because the minimap uses the same scene data as the main view, it can display the railway layout and the important objects from above without requiring a separate representation of the world.

This part of the project demonstrates the use of multiple cameras, multiple projection types, and multiple viewports inside the same rendered frame.

The perspective camera is intended for navigating and observing the 3D scene, while the orthographic camera gives the player a clearer overview of the railway network.

## Camera and Player Controls

The train is controlled using directional input that influences the route chosen at railway intersections.

The main train controls are:

- `W` — continue straight
- `A` — choose the left route
- `D` — choose the right route
- `S` — go back

The camera can also be moved independently from the train.

While using the right mouse button, the player can rotate the camera using mouse movement and adjust its position using the available keyboard controls.

This separation between train control and camera control makes it possible to inspect the environment while the locomotive continues moving through the railway network.

## Game State and UI

The project maintains several pieces of gameplay state, including the current score, remaining time, active resource order, availability of resources, selected train direction, and whether the game has ended.

Text rendering is used to display information such as the remaining time and the Game Over state.

Resource availability is also represented visually in the scene, and the current objective changes as resources are collected.

The update loop brings these systems together by advancing train movement, updating resource respawn timers, checking interactions, updating the countdown, and controlling the transition to the Game Over state.

The rendering loop then uses the resulting state to display the environment, train, stations, resources, minimap, and interface.

## Project Structure

The project-specific implementation is located under `src/lab_m1/Tema2/`.

The main files are:

- `Tema2.cpp` — main game implementation, railway creation, movement, gameplay systems, rendering, collisions, procedural geometry, minimap, and UI
- `Tema2.h` — game class definition and the structures used for rails, stations, train components, collision boxes, cameras, and gameplay state
- `shaders/VertexShader.glsl` — custom vertex shader used for transforming and rendering vertices
- `shaders/FragmentShader.glsl` — custom fragment shader used for vertex colors and the time-based color effect

The game is implemented as a scene inside the provided GFX Framework.

The repository contains the project-specific source code rather than a completely standalone rendering framework, so the scene depends on framework components such as the base scene class, camera implementation, text renderer, mesh system, shader management, and window/input handling.

## Build and Run

The project was developed using the **GFX Framework** environment and depends on the framework's rendering, camera, window, input, and text-rendering components.

The repository contains the project-specific implementation under `src/lab_m1/Tema2`, so it is intended to be integrated and built inside the corresponding GFX Framework project rather than compiled as an independent C++ application from the files in this repository alone.

Once added to the framework and selected as the active scene, the game initializes the railway graph, procedural meshes, stations, cameras, train state, resource order, timer, shaders, and user interface before entering the normal update and rendering loop.

## Technologies and Concepts

- C++
- OpenGL
- GLSL
- GLM
- GFX Framework
- Real-time 3D graphics
- Model-View-Projection transformations
- Perspective projection
- Orthographic projection
- Multiple cameras
- Multiple OpenGL viewports
- Graph-based navigation
- Vector mathematics
- Dot products
- Cross products
- Signed angles
- Delta-time movement
- Linear interpolation
- Procedural geometry
- VAO, VBO, and IBO management
- Custom shaders
- AABB collision detection
- Resource management
- Timed gameplay
- Scoring systems
- `std::vector`
- `std::deque`
- Pointer-based graph structures
- Interactive camera controls
