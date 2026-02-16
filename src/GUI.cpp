#include <vaughngl/GUI.h>
#include <vaughngl/EmbeddedShaders.h>
#include <stdexcept>
#include <cstdio>
#include <cmath>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

GUI::GUI(int width, int height, const char *title)
    : m_windowWidth(width), m_windowHeight(height), m_framebufferWidth(width), m_framebufferHeight(height)
{
  initGL();

  m_window = glfwCreateWindow(width, height, title, nullptr, nullptr);
  if (!m_window)
  {
    glfwTerminate();
    throw std::runtime_error("Failed to create window");
  }

  glfwMakeContextCurrent(m_window);

  if (glewInit() != GLEW_OK)
  {
    throw std::runtime_error("Failed to initialize GLEW");
  }

  glfwGetWindowSize(m_window, &m_windowWidth, &m_windowHeight);
  glfwGetFramebufferSize(m_window, &m_framebufferWidth, &m_framebufferHeight);
  glViewport(0, 0, m_framebufferWidth, m_framebufferHeight);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  setupCallbacks();
  m_shader.loadFromSource(EmbeddedShaders::defaultVert, EmbeddedShaders::defaultFrag);
  initMeshes();
}

GUI::~GUI()
{
  if (m_window)
    glfwDestroyWindow(m_window);
  glfwTerminate();
}

void GUI::initGL()
{
  if (!glfwInit())
  {
    throw std::runtime_error("Failed to initialize GLFW");
  }

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
}

void GUI::initMeshes()
{
  std::vector<Vertex> vertices;
  std::vector<unsigned int> indices;

  MeshGen::circle(vertices, indices, 32);
  m_circleMesh.upload(vertices, indices);

  MeshGen::quad(vertices, indices);
  m_quadMesh.upload(vertices, indices);

  MeshGen::cube(vertices, indices);
  m_cubeMesh.upload(vertices, indices);

  MeshGen::sphere(vertices, indices, 16, 32);
  m_sphereMesh.upload(vertices, indices);

  MeshGen::cylinder(vertices, indices, 32);
  m_cylinderMesh.upload(vertices, indices);
}

bool GUI::shouldClose() const
{
  return glfwWindowShouldClose(m_window);
}

void GUI::beginFrame()
{
  glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  m_shader.use();

  float aspect = (float)m_framebufferWidth / m_framebufferHeight;
  m_shader.setMat4("view", camera.getViewMatrix());
  m_shader.setMat4("projection", camera.getProjectionMatrix(aspect));
  m_shader.setBool("useLighting", m_useLighting);
  m_shader.setVec3("lightDir", m_lightDir);
  m_shader.setVec3("viewPos", camera.position);
}

void GUI::endFrame()
{
  glfwSwapBuffers(m_window);

  // Clear per-frame input state before polling new events
  m_keysJustPressed.clear();
  m_keysJustReleased.clear();
  m_mouseButtonsJustPressed.clear();
  m_mouseButtonsJustReleased.clear();
  m_scrollDelta = glm::vec2(0.0f);

  glfwPollEvents();
}

void GUI::setupDraw(const glm::mat4 &model, glm::vec3 color)
{
  m_shader.setMat4("model", model);
  m_shader.setVec3("color", color);
}

void GUI::drawCircle(glm::vec3 pos, float radius, glm::vec3 color)
{
  glm::mat4 model = glm::translate(glm::mat4(1.0f), pos);
  model = glm::scale(model, glm::vec3(radius));

  bool prevLighting = m_useLighting;
  m_shader.setBool("useLighting", false);
  setupDraw(model, color);
  m_circleMesh.draw();
  m_shader.setBool("useLighting", prevLighting);
}

void GUI::drawCircle(glm::vec3 pos, float radius, glm::quat rotation, glm::vec3 color)
{
  glm::mat4 model = glm::translate(glm::mat4(1.0f), pos);
  model = model * glm::mat4_cast(rotation);
  model = glm::scale(model, glm::vec3(radius));

  bool prevLighting = m_useLighting;
  m_shader.setBool("useLighting", false);
  setupDraw(model, color);
  m_circleMesh.draw();
  m_shader.setBool("useLighting", prevLighting);
}

void GUI::drawRect(glm::vec3 pos, float width, float height, glm::vec3 color)
{
  glm::mat4 model = glm::translate(glm::mat4(1.0f), pos);
  model = glm::scale(model, glm::vec3(width, height, 1.0f));

  bool prevLighting = m_useLighting;
  m_shader.setBool("useLighting", false);
  setupDraw(model, color);
  m_quadMesh.draw();
  m_shader.setBool("useLighting", prevLighting);
}

void GUI::drawRect(glm::vec3 pos, float width, float height, glm::quat rotation, glm::vec3 color)
{
  glm::mat4 model = glm::translate(glm::mat4(1.0f), pos);
  model = model * glm::mat4_cast(rotation);
  model = glm::scale(model, glm::vec3(width, height, 1.0f));

  bool prevLighting = m_useLighting;
  m_shader.setBool("useLighting", false);
  setupDraw(model, color);
  m_quadMesh.draw();
  m_shader.setBool("useLighting", prevLighting);
}

void GUI::drawLine(glm::vec3 start, glm::vec3 end, glm::vec3 color, float width)
{
  std::vector<glm::vec3> points = {start, end};
  m_lineMesh.uploadLines(points);

  glLineWidth(width);
  m_shader.setBool("useLighting", false);
  setupDraw(glm::mat4(1.0f), color);
  m_lineMesh.drawLines();
  m_shader.setBool("useLighting", m_useLighting);
}

void GUI::drawArrow(glm::vec3 start, glm::vec3 end, glm::vec3 color, float width)
{
  glm::vec3 dir = end - start;
  float length = glm::length(dir);
  if (length <= 0.0001f)
    return;

  float headLength = length / 10.0f;
  float headRadius = headLength / 3.0f;

  glm::vec3 dirNorm = glm::normalize(dir);

  // Clamp head length
  headLength = glm::min(headLength, length * 0.5f);

  glm::vec3 shaftEnd = end - dirNorm * headLength;

  // Draw shaft
  drawLine(start, shaftEnd, color, width);

  // Build orthonormal basis for cone
  glm::vec3 b1, b2;
  if (std::abs(dirNorm.x) < 0.9f)
    b1 = glm::normalize(glm::cross(dirNorm, glm::vec3(1, 0, 0)));
  else
    b1 = glm::normalize(glm::cross(dirNorm, glm::vec3(0, 1, 0)));
  b2 = glm::cross(dirNorm, b1);

  // Draw cone edges
  std::vector<glm::vec3> coneLines;
  int coneSegments = 12;
  for (int i = 0; i < coneSegments; ++i)
  {
    float a0 = (float)i / coneSegments * glm::two_pi<float>();
    float a1 = (float)(i + 1) / coneSegments * glm::two_pi<float>();

    glm::vec3 p0 = shaftEnd + (std::cos(a0) * b1 + std::sin(a0) * b2) * headRadius;
    glm::vec3 p1 = shaftEnd + (std::cos(a1) * b1 + std::sin(a1) * b2) * headRadius;

    // Circle edge
    coneLines.push_back(p0);
    coneLines.push_back(p1);

    // Side edge to tip
    coneLines.push_back(p0);
    coneLines.push_back(end);
  }

  m_lineMesh.uploadLines(coneLines);
  glLineWidth(width);
  m_shader.setBool("useLighting", false);
  setupDraw(glm::mat4(1.0f), color);
  m_lineMesh.drawLines();
  m_shader.setBool("useLighting", m_useLighting);
}

void GUI::drawSphere(glm::vec3 pos, float radius, glm::vec3 color)
{
  glm::mat4 model = glm::translate(glm::mat4(1.0f), pos);
  model = glm::scale(model, glm::vec3(radius * 2.0f)); // mesh is unit diameter

  setupDraw(model, color);
  m_sphereMesh.draw();
}

void GUI::drawSphere(glm::vec3 pos, float radius, glm::quat rotation, glm::vec3 color)
{
  glm::mat4 model = glm::translate(glm::mat4(1.0f), pos);
  model = model * glm::mat4_cast(rotation);
  model = glm::scale(model, glm::vec3(radius * 2.0f));

  setupDraw(model, color);
  m_sphereMesh.draw();
}

void GUI::drawCube(glm::vec3 pos, float size, glm::vec3 color)
{
  drawBox(pos, glm::vec3(size), color);
}

void GUI::drawCube(glm::vec3 pos, float size, glm::quat rotation, glm::vec3 color)
{
  drawBox(pos, glm::vec3(size), rotation, color);
}

void GUI::drawBox(glm::vec3 pos, glm::vec3 size, glm::vec3 color)
{
  glm::mat4 model = glm::translate(glm::mat4(1.0f), pos);
  model = glm::scale(model, size);

  setupDraw(model, color);
  m_cubeMesh.draw();
}

void GUI::drawBox(glm::vec3 pos, glm::vec3 size, glm::quat rotation, glm::vec3 color)
{
  glm::mat4 model = glm::translate(glm::mat4(1.0f), pos);
  model = model * glm::mat4_cast(rotation);
  model = glm::scale(model, size);

  setupDraw(model, color);
  m_cubeMesh.draw();
}

void GUI::drawCylinder(glm::vec3 pos, float radius, float length, glm::vec3 color)
{
  glm::mat4 model = glm::translate(glm::mat4(1.0f), pos);
  model = glm::scale(model, glm::vec3(radius * 2.0f, length, radius * 2.0f));

  setupDraw(model, color);
  m_cylinderMesh.draw();
}

void GUI::drawCylinder(glm::vec3 pos, float radius, float length, glm::quat rotation, glm::vec3 color)
{
  glm::mat4 model = glm::translate(glm::mat4(1.0f), pos);
  model = model * glm::mat4_cast(rotation);
  model = glm::scale(model, glm::vec3(radius * 2.0f, length, radius * 2.0f));

  setupDraw(model, color);
  m_cylinderMesh.draw();
}

void GUI::drawCylinder(glm::vec3 pos, float radius, float length, glm::vec3 axis, glm::quat rotation, glm::vec3 color)
{
  axis = glm::normalize(axis);
  glm::vec3 defaultAxis(0, 1, 0);

  // Compute rotation from default Y-axis to specified axis
  glm::quat axisRot(1, 0, 0, 0);
  float d = glm::dot(defaultAxis, axis);
  if (d < 0.9999f)
  {
    if (d < -0.9999f)
    {
      // 180 degree rotation (opposite direction)
      axisRot = glm::angleAxis(glm::pi<float>(), glm::vec3(1, 0, 0));
    }
    else
    {
      glm::vec3 rotAxis = glm::normalize(glm::cross(defaultAxis, axis));
      float angle = acos(d);
      axisRot = glm::angleAxis(angle, rotAxis);
    }
  }

  glm::mat4 model = glm::translate(glm::mat4(1.0f), pos);
  model = model * glm::mat4_cast(rotation * axisRot);
  model = glm::scale(model, glm::vec3(radius * 2.0f, length, radius * 2.0f));

  setupDraw(model, color);
  m_cylinderMesh.draw();
}

// --- OBJ Mesh drawing ---

void GUI::drawOBJMesh(OBJMesh &mesh, glm::vec3 pos, float scale)
{
  drawOBJMesh(mesh, pos, glm::vec3(scale), glm::quat(1, 0, 0, 0));
}

void GUI::drawOBJMesh(OBJMesh &mesh, glm::vec3 pos, float scale, glm::quat rotation)
{
  drawOBJMesh(mesh, pos, glm::vec3(scale), rotation);
}

void GUI::drawOBJMesh(OBJMesh &mesh, glm::vec3 pos, glm::vec3 scale)
{
  drawOBJMesh(mesh, pos, scale, glm::quat(1, 0, 0, 0));
}

void GUI::drawOBJMesh(OBJMesh &mesh, glm::vec3 pos, glm::vec3 scale, glm::quat rotation)
{
  if (!mesh.isLoaded())
    return;

  glm::mat4 model = glm::translate(glm::mat4(1.0f), pos);
  model = model * glm::mat4_cast(rotation);
  model = glm::scale(model, scale);

  for (const auto &subMesh : mesh.getSubMeshes())
  {
    setupDraw(model, subMesh.material.diffuse);
    subMesh.mesh.draw();
  }
}

void GUI::drawOBJMesh(OBJMesh &mesh, glm::vec3 pos, float scale, glm::vec3 color)
{
  drawOBJMesh(mesh, pos, glm::vec3(scale), glm::quat(1, 0, 0, 0), color);
}

void GUI::drawOBJMesh(OBJMesh &mesh, glm::vec3 pos, float scale, glm::quat rotation, glm::vec3 color)
{
  drawOBJMesh(mesh, pos, glm::vec3(scale), rotation, color);
}

void GUI::drawOBJMesh(OBJMesh &mesh, glm::vec3 pos, glm::vec3 scale, glm::vec3 color)
{
  drawOBJMesh(mesh, pos, scale, glm::quat(1, 0, 0, 0), color);
}

void GUI::drawOBJMesh(OBJMesh &mesh, glm::vec3 pos, glm::vec3 scale, glm::quat rotation, glm::vec3 color)
{
  if (!mesh.isLoaded())
    return;

  glm::mat4 model = glm::translate(glm::mat4(1.0f), pos);
  model = model * glm::mat4_cast(rotation);
  model = glm::scale(model, scale);

  for (const auto &subMesh : mesh.getSubMeshes())
  {
    setupDraw(model, color);
    subMesh.mesh.draw();
  }
}

// --- Callbacks ---

void GUI::setupCallbacks()
{
  glfwSetWindowUserPointer(m_window, this);
  glfwSetFramebufferSizeCallback(m_window, framebufferSizeCallback);
  glfwSetWindowSizeCallback(m_window, windowSizeCallback);
  glfwSetKeyCallback(m_window, keyCallback);
  glfwSetMouseButtonCallback(m_window, mouseButtonCallback);
  glfwSetScrollCallback(m_window, scrollCallback);
}

void GUI::framebufferSizeCallback(GLFWwindow *window, int width, int height)
{
  GUI *gui = static_cast<GUI *>(glfwGetWindowUserPointer(window));

  gui->m_framebufferWidth = width;
  gui->m_framebufferHeight = height;

  glViewport(0, 0, width, height);
}

void GUI::windowSizeCallback(GLFWwindow *window, int width, int height)
{
  GUI *gui = static_cast<GUI *>(glfwGetWindowUserPointer(window));

  gui->m_windowWidth = width;
  gui->m_windowHeight = height;
}

void GUI::keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
  (void)scancode;
  (void)mods;
  GUI *gui = static_cast<GUI *>(glfwGetWindowUserPointer(window));

  if (action == GLFW_PRESS)
  {
    gui->m_keysPressed.insert(key);
    gui->m_keysJustPressed.insert(key);
  }
  else if (action == GLFW_RELEASE)
  {
    gui->m_keysPressed.erase(key);
    gui->m_keysJustReleased.insert(key);
  }
}

void GUI::mouseButtonCallback(GLFWwindow *window, int button, int action, int mods)
{
  (void)mods;
  GUI *gui = static_cast<GUI *>(glfwGetWindowUserPointer(window));

  if (action == GLFW_PRESS)
  {
    gui->m_mouseButtonsPressed.insert(button);
    gui->m_mouseButtonsJustPressed.insert(button);
  }
  else if (action == GLFW_RELEASE)
  {
    gui->m_mouseButtonsPressed.erase(button);
    gui->m_mouseButtonsJustReleased.insert(button);
  }
}

void GUI::scrollCallback(GLFWwindow *window, double xoffset, double yoffset)
{
  GUI *gui = static_cast<GUI *>(glfwGetWindowUserPointer(window));
  gui->m_scrollDelta.x += static_cast<float>(xoffset);
  gui->m_scrollDelta.y += static_cast<float>(yoffset);
}

// --- Keyboard input ---

bool GUI::isKeyPressed(int key) const
{
  return m_keysPressed.count(key) > 0;
}

bool GUI::isKeyJustPressed(int key) const
{
  return m_keysJustPressed.count(key) > 0;
}

bool GUI::isKeyJustReleased(int key) const
{
  return m_keysJustReleased.count(key) > 0;
}

// --- Mouse input ---

glm::vec2 GUI::getMousePosition() const
{
  double x, y;
  glfwGetCursorPos(m_window, &x, &y);
  return glm::vec2(static_cast<float>(x), static_cast<float>(y));
}

bool GUI::isMouseButtonPressed(int button) const
{
  return m_mouseButtonsPressed.count(button) > 0;
}

bool GUI::isMouseButtonJustPressed(int button) const
{
  return m_mouseButtonsJustPressed.count(button) > 0;
}

bool GUI::isMouseButtonJustReleased(int button) const
{
  return m_mouseButtonsJustReleased.count(button) > 0;
}

glm::vec2 GUI::getScrollDelta() const
{
  return m_scrollDelta;
}
