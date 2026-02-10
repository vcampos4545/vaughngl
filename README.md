# vaughngl

Simple 2D/3D shape rendering library wrapping OpenGL.

## Dependencies

```bash
# macOS
brew install glfw glew glm

# Ubuntu
sudo apt install libglfw3-dev libglew-dev libglm-dev
```

## Use in Your Project

### CMake FetchContent (Recommended)

```cmake
include(FetchContent)

FetchContent_Declare(
  vaughngl
  GIT_REPOSITORY https://github.com/vaughngl/vaughngl.git
  GIT_TAG main
)
FetchContent_MakeAvailable(vaughngl)

target_link_libraries(your_app vaughngl)
```

### Git Submodule

```bash
git submodule add https://github.com/vaughngl/vaughngl.git external/vaughngl
```

```cmake
add_subdirectory(external/vaughngl)
target_link_libraries(your_app vaughngl)
```

## Quick Start

```cpp
#include <vaughngl/vaughngl.h>

int main() {
  GUI gui(800, 600, "My App");

  gui.camera.setPosition({0, 2, 8})
            .setTarget({0, 0, 0})
            .setFOV(45.0f);

  while (!gui.shouldClose()) {
    gui.beginFrame();

    // 3D shapes
    gui.drawSphere({0, 0, 0}, 1.0f, {1, 0, 0});
    gui.drawCube({2, 0, 0}, 1.0f, {0, 1, 0});
    gui.drawBox({-2, 0, 0}, {1, 2, 0.5f}, {0, 0, 1});

    // 2D shapes (in 3D space)
    gui.drawCircle({0, 2, 0}, 0.5f, {1, 1, 0});
    gui.drawRect({0, 3, 0}, 1.0f, 0.5f, {0, 1, 1});
    gui.drawLine({-3, 0, 0}, {3, 0, 0}, {1, 1, 1});

    gui.endFrame();
  }
}
```

### Rotation with Quaternions

```cpp
glm::quat rotation = glm::angleAxis(glm::radians(45.0f), glm::vec3(0, 1, 0));
gui.drawCube({0, 0, 0}, 1.0f, rotation, {1, 0, 0});
gui.drawSphere({2, 0, 0}, 0.5f, rotation, {0, 1, 0});
gui.drawBox({-2, 0, 0}, {1, 2, 1}, rotation, {0, 0, 1});
```

### OBJ Model Loading

```cpp
OBJMesh model;
if (model.load("models/spaceship.obj")) {
  // Draw with material colors from .mtl file
  gui.drawOBJMesh(model, {0, 0, 0}, 1.0f);

  // Draw with rotation
  gui.drawOBJMesh(model, {0, 0, 0}, 1.0f, rotation);

  // Draw with color override
  gui.drawOBJMesh(model, {0, 0, 0}, 1.0f, {1, 0, 0});

  // Non-uniform scale
  gui.drawOBJMesh(model, {0, 0, 0}, glm::vec3(1, 2, 1));
}
```

### Input Handling

```cpp
// Keyboard
if (gui.isKeyPressed(GLFW_KEY_W)) { /* held down */ }
if (gui.isKeyJustPressed(GLFW_KEY_SPACE)) { /* pressed this frame */ }
if (gui.isKeyJustReleased(GLFW_KEY_ESCAPE)) { /* released this frame */ }

// Mouse
glm::vec2 mousePos = gui.getMousePosition();
if (gui.isMouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT)) { /* held */ }
if (gui.isMouseButtonJustPressed(GLFW_MOUSE_BUTTON_RIGHT)) { /* clicked */ }

// Scroll wheel
glm::vec2 scroll = gui.getScrollDelta();
```

### Camera Control

```cpp
// Chainable setters
gui.camera.setPosition({0, 5, 10})
          .setTarget({0, 0, 0})
          .setFOV(60.0f)
          .setClipPlanes(0.1f, 1000.0f);

// Or set directly
gui.camera.position = {0, 5, 10};
gui.camera.target = {0, 0, 0};
gui.camera.fov = 60.0f;
gui.camera.nearPlane = 0.1f;
gui.camera.farPlane = 1000.0f;

// Utility methods
glm::vec3 dir = gui.camera.getDirection();
float dist = gui.camera.getDistance();
gui.camera.setDistance(20.0f);
```

### Lighting

```cpp
gui.setLighting(true);  // Enable/disable
gui.setLightDirection({1, 1, 0.5f});
```

## Run Example

```bash
mkdir build && cd build
cmake ..
make
./example
```

### Example Controls

- **Left mouse drag**: Orbit camera
- **Scroll**: Zoom in/out
- **Arrow keys**: Move the green cube
