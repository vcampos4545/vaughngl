#ifndef GUI_H
#define GUI_H

#include <vaughngl/Shader.h>
#include <vaughngl/Mesh.h>
#include <vaughngl/Camera.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <string>
#include <unordered_set>

class GUI {
public:
  GUI(int width, int height, const char* title = "GUI Window");
  ~GUI();

  GUI(const GUI&) = delete;
  GUI& operator=(const GUI&) = delete;

  bool shouldClose() const;
  void beginFrame();
  void endFrame();

  // 2D shapes (drawn in XY plane, can be positioned in 3D)
  void drawCircle(glm::vec3 pos, float radius, glm::vec3 color = {1, 1, 1});
  void drawRect(glm::vec3 pos, float width, float height, glm::vec3 color = {1, 1, 1});
  void drawLine(glm::vec3 start, glm::vec3 end, glm::vec3 color = {1, 1, 1}, float width = 1.0f);

  // 3D shapes
  void drawSphere(glm::vec3 pos, float radius, glm::vec3 color = {1, 1, 1});
  void drawCube(glm::vec3 pos, float size, glm::vec3 color = {1, 1, 1});
  void drawBox(glm::vec3 pos, glm::vec3 size, glm::vec3 color = {1, 1, 1});

  // Lighting control
  void setLighting(bool enabled) { m_useLighting = enabled; }
  void setLightDirection(glm::vec3 dir) { m_lightDir = glm::normalize(dir); }

  // Keyboard input
  bool isKeyPressed(int key) const;
  bool isKeyJustPressed(int key) const;
  bool isKeyJustReleased(int key) const;

  // Mouse input
  glm::vec2 getMousePosition() const;
  bool isMouseButtonPressed(int button) const;
  bool isMouseButtonJustPressed(int button) const;
  bool isMouseButtonJustReleased(int button) const;
  glm::vec2 getScrollDelta() const;

  Camera camera;

  int getWidth() const { return m_width; }
  int getHeight() const { return m_height; }
  float getAspect() const { return static_cast<float>(m_width) / m_height; }
  GLFWwindow* getWindow() const { return m_window; }

private:
  void initGL();
  void initMeshes();
  void setupCallbacks();
  void setupDraw(const glm::mat4& model, glm::vec3 color);

  // GLFW callbacks
  static void framebufferSizeCallback(GLFWwindow* window, int width, int height);
  static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
  static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
  static void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);

  GLFWwindow* m_window = nullptr;
  int m_width, m_height;

  Shader m_shader;
  Mesh m_circleMesh;
  Mesh m_quadMesh;
  Mesh m_cubeMesh;
  Mesh m_sphereMesh;
  Mesh m_lineMesh;

  bool m_useLighting = true;
  glm::vec3 m_lightDir{0.5f, 1.0f, 0.3f};

  // Input state
  std::unordered_set<int> m_keysPressed;
  std::unordered_set<int> m_keysJustPressed;
  std::unordered_set<int> m_keysJustReleased;
  std::unordered_set<int> m_mouseButtonsPressed;
  std::unordered_set<int> m_mouseButtonsJustPressed;
  std::unordered_set<int> m_mouseButtonsJustReleased;
  glm::vec2 m_scrollDelta{0.0f, 0.0f};
};

#endif
