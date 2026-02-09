#include <vaughngl/vaughngl.h>
#include <cmath>

int main()
{
  GUI gui(800, 600, "Vaughn-GL Demo");

  gui.camera.position = {0, 2, 8};
  gui.camera.target = {0, 0, 0};

  glm::vec2 lastMousePos = gui.getMousePosition();
  glm::vec3 cubePos = {0, 1, 0};
  float moveSpeed = 0.1f;

  while (!gui.shouldClose())
  {
    gui.beginFrame();

    // Orbital camera - drag to rotate around target
    glm::vec2 mousePos = gui.getMousePosition();
    if (gui.isMouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT))
    {
      glm::vec2 delta = mousePos - lastMousePos;
      float sensitivity = 0.005f;

      // Get current spherical coordinates relative to target
      glm::vec3 offset = gui.camera.position - gui.camera.target;
      float radius = glm::length(offset);
      float theta = std::atan2(offset.x, offset.z); // horizontal angle
      float phi = std::acos(offset.y / radius);     // vertical angle

      // Update angles based on mouse movement
      theta -= delta.x * sensitivity;
      phi -= delta.y * sensitivity;

      // Clamp phi to avoid flipping
      phi = glm::clamp(phi, 0.1f, 3.04f);

      // Convert back to Cartesian coordinates
      gui.camera.position.x = gui.camera.target.x + radius * std::sin(phi) * std::sin(theta);
      gui.camera.position.y = gui.camera.target.y + radius * std::cos(phi);
      gui.camera.position.z = gui.camera.target.z + radius * std::sin(phi) * std::cos(theta);
    }
    lastMousePos = mousePos;

    // Scroll to zoom camera
    glm::vec2 scroll = gui.getScrollDelta();
    if (scroll.y != 0.0f)
    {
      glm::vec3 direction = glm::normalize(gui.camera.position - gui.camera.target);
      gui.camera.position -= direction * scroll.y * 0.5f;
    }

    // Move cube with arrow keys
    if (gui.isKeyPressed(GLFW_KEY_UP))
      cubePos.z -= moveSpeed;
    if (gui.isKeyPressed(GLFW_KEY_DOWN))
      cubePos.z += moveSpeed;
    if (gui.isKeyPressed(GLFW_KEY_LEFT))
      cubePos.x -= moveSpeed;
    if (gui.isKeyPressed(GLFW_KEY_RIGHT))
      cubePos.x += moveSpeed;

    // Draw grid lines
    int numLines = 10;
    for (int i = 0; i < numLines + 1; ++i)
    {
      // Horizontal
      gui.drawLine({-numLines / 2, 0, i - numLines / 2}, {numLines / 2, 0, i - numLines / 2}, {1, 1, 1}, 2.0f);
      // Vertical
      gui.drawLine({i - numLines / 2, 0, numLines / 2}, {i - numLines / 2, 0, -numLines / 2}, {1, 1, 1}, 2.0f);
    }

    // 3D shapes with lighting
    gui.drawSphere({-5, 0, -5}, 0.8f, {1, 0.3f, 0.3f});
    gui.drawCube(cubePos, 1.2f, {0.3f, 1, 0.3f});
    gui.drawBox({5, 0, -5}, {0.5f, 1.5f, 0.5f}, {0.3f, 0.3f, 1});

    gui.endFrame();
  }

  return 0;
}