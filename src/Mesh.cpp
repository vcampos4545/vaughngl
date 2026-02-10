#include <vaughngl/Mesh.h>
#include <cmath>

constexpr float PI = 3.14159265359f;

Mesh::Mesh() = default;

Mesh::~Mesh() { cleanup(); }

Mesh::Mesh(Mesh&& other) noexcept
    : m_vao(other.m_vao), m_vbo(other.m_vbo), m_ebo(other.m_ebo),
      m_indexCount(other.m_indexCount), m_vertexCount(other.m_vertexCount),
      m_isLineMode(other.m_isLineMode) {
  other.m_vao = other.m_vbo = other.m_ebo = 0;
  other.m_indexCount = other.m_vertexCount = 0;
}

Mesh& Mesh::operator=(Mesh&& other) noexcept {
  if (this != &other) {
    cleanup();
    m_vao = other.m_vao;
    m_vbo = other.m_vbo;
    m_ebo = other.m_ebo;
    m_indexCount = other.m_indexCount;
    m_vertexCount = other.m_vertexCount;
    m_isLineMode = other.m_isLineMode;
    other.m_vao = other.m_vbo = other.m_ebo = 0;
    other.m_indexCount = other.m_vertexCount = 0;
  }
  return *this;
}

void Mesh::cleanup() {
  if (m_vao) glDeleteVertexArrays(1, &m_vao);
  if (m_vbo) glDeleteBuffers(1, &m_vbo);
  if (m_ebo) glDeleteBuffers(1, &m_ebo);
  m_vao = m_vbo = m_ebo = 0;
}

void Mesh::setupAttributes() {
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uv));
  glEnableVertexAttribArray(2);
}

void Mesh::upload(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices) {
  cleanup();
  m_isLineMode = false;
  m_indexCount = indices.size();

  glGenVertexArrays(1, &m_vao);
  glGenBuffers(1, &m_vbo);
  glGenBuffers(1, &m_ebo);

  glBindVertexArray(m_vao);
  glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
  glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);
  setupAttributes();
  glBindVertexArray(0);
}

void Mesh::uploadLines(const std::vector<glm::vec3>& points) {
  cleanup();
  m_isLineMode = true;
  m_vertexCount = points.size();

  glGenVertexArrays(1, &m_vao);
  glGenBuffers(1, &m_vbo);

  glBindVertexArray(m_vao);
  glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
  glBufferData(GL_ARRAY_BUFFER, points.size() * sizeof(glm::vec3), points.data(), GL_DYNAMIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
  glEnableVertexAttribArray(0);
  glBindVertexArray(0);
}

void Mesh::draw() const {
  if (!m_vao || m_isLineMode) return;
  glBindVertexArray(m_vao);
  glDrawElements(GL_TRIANGLES, m_indexCount, GL_UNSIGNED_INT, 0);
  glBindVertexArray(0);
}

void Mesh::drawLines() const {
  if (!m_vao || !m_isLineMode || m_vertexCount < 2) return;
  glBindVertexArray(m_vao);
  glDrawArrays(GL_LINE_STRIP, 0, m_vertexCount);
  glBindVertexArray(0);
}

// Mesh generators

namespace MeshGen {

void circle(std::vector<Vertex>& vertices, std::vector<unsigned int>& indices, int segments) {
  vertices.clear();
  indices.clear();

  // Center vertex
  vertices.push_back({{0, 0, 0}, {0, 0, 1}, {0.5f, 0.5f}});

  for (int i = 0; i <= segments; ++i) {
    float angle = 2.0f * PI * i / segments;
    float x = cos(angle);
    float y = sin(angle);
    vertices.push_back({{x, y, 0}, {0, 0, 1}, {(x + 1) * 0.5f, (y + 1) * 0.5f}});
  }

  for (int i = 1; i <= segments; ++i) {
    indices.push_back(0);
    indices.push_back(i);
    indices.push_back(i + 1);
  }
}

void quad(std::vector<Vertex>& vertices, std::vector<unsigned int>& indices) {
  vertices.clear();
  indices.clear();

  vertices = {
    {{-0.5f, -0.5f, 0}, {0, 0, 1}, {0, 0}},
    {{ 0.5f, -0.5f, 0}, {0, 0, 1}, {1, 0}},
    {{ 0.5f,  0.5f, 0}, {0, 0, 1}, {1, 1}},
    {{-0.5f,  0.5f, 0}, {0, 0, 1}, {0, 1}}
  };

  indices = {0, 1, 2, 0, 2, 3};
}

void cube(std::vector<Vertex>& vertices, std::vector<unsigned int>& indices) {
  vertices.clear();
  indices.clear();

  // Each face has its own vertices for correct normals
  glm::vec3 normals[6] = {
    { 0,  0,  1}, { 0,  0, -1},
    { 1,  0,  0}, {-1,  0,  0},
    { 0,  1,  0}, { 0, -1,  0}
  };

  // Face vertex positions (CCW winding)
  float positions[6][4][3] = {
    // Front (+Z)
    {{-0.5f, -0.5f,  0.5f}, { 0.5f, -0.5f,  0.5f}, { 0.5f,  0.5f,  0.5f}, {-0.5f,  0.5f,  0.5f}},
    // Back (-Z)
    {{ 0.5f, -0.5f, -0.5f}, {-0.5f, -0.5f, -0.5f}, {-0.5f,  0.5f, -0.5f}, { 0.5f,  0.5f, -0.5f}},
    // Right (+X)
    {{ 0.5f, -0.5f,  0.5f}, { 0.5f, -0.5f, -0.5f}, { 0.5f,  0.5f, -0.5f}, { 0.5f,  0.5f,  0.5f}},
    // Left (-X)
    {{-0.5f, -0.5f, -0.5f}, {-0.5f, -0.5f,  0.5f}, {-0.5f,  0.5f,  0.5f}, {-0.5f,  0.5f, -0.5f}},
    // Top (+Y)
    {{-0.5f,  0.5f,  0.5f}, { 0.5f,  0.5f,  0.5f}, { 0.5f,  0.5f, -0.5f}, {-0.5f,  0.5f, -0.5f}},
    // Bottom (-Y)
    {{-0.5f, -0.5f, -0.5f}, { 0.5f, -0.5f, -0.5f}, { 0.5f, -0.5f,  0.5f}, {-0.5f, -0.5f,  0.5f}}
  };

  float uvs[4][2] = {{0, 0}, {1, 0}, {1, 1}, {0, 1}};

  for (int face = 0; face < 6; ++face) {
    unsigned int base = vertices.size();
    for (int v = 0; v < 4; ++v) {
      vertices.push_back({
        {positions[face][v][0], positions[face][v][1], positions[face][v][2]},
        normals[face],
        {uvs[v][0], uvs[v][1]}
      });
    }
    indices.push_back(base);
    indices.push_back(base + 1);
    indices.push_back(base + 2);
    indices.push_back(base);
    indices.push_back(base + 2);
    indices.push_back(base + 3);
  }
}

void sphere(std::vector<Vertex>& vertices, std::vector<unsigned int>& indices, int rings, int sectors) {
  vertices.clear();
  indices.clear();

  for (int r = 0; r <= rings; ++r) {
    float phi = PI * r / rings;
    float y = cos(phi);
    float ringRadius = sin(phi);

    for (int s = 0; s <= sectors; ++s) {
      float theta = 2.0f * PI * s / sectors;
      float x = ringRadius * cos(theta);
      float z = ringRadius * sin(theta);

      glm::vec3 pos(x, y, z);
      vertices.push_back({
        pos * 0.5f,  // radius 0.5 so diameter = 1 (unit sphere)
        glm::normalize(pos),
        {(float)s / sectors, (float)r / rings}
      });
    }
  }

  for (int r = 0; r < rings; ++r) {
    for (int s = 0; s < sectors; ++s) {
      unsigned int cur = r * (sectors + 1) + s;
      unsigned int next = cur + sectors + 1;

      indices.push_back(cur);
      indices.push_back(next);
      indices.push_back(cur + 1);

      indices.push_back(cur + 1);
      indices.push_back(next);
      indices.push_back(next + 1);
    }
  }
}

void cylinder(std::vector<Vertex>& vertices, std::vector<unsigned int>& indices, int segments) {
  vertices.clear();
  indices.clear();

  // Unit cylinder: radius 0.5, height 1, centered at origin, extending along Y-axis
  const float radius = 0.5f;
  const float halfHeight = 0.5f;

  // Top cap center
  unsigned int topCenterIdx = vertices.size();
  vertices.push_back({{0, halfHeight, 0}, {0, 1, 0}, {0.5f, 0.5f}});

  // Top cap ring
  unsigned int topRingStart = vertices.size();
  for (int i = 0; i <= segments; ++i) {
    float angle = 2.0f * PI * i / segments;
    float x = radius * cos(angle);
    float z = radius * sin(angle);
    vertices.push_back({{x, halfHeight, z}, {0, 1, 0}, {(cos(angle) + 1) * 0.5f, (sin(angle) + 1) * 0.5f}});
  }

  // Top cap triangles
  for (int i = 0; i < segments; ++i) {
    indices.push_back(topCenterIdx);
    indices.push_back(topRingStart + i + 1);
    indices.push_back(topRingStart + i);
  }

  // Bottom cap center
  unsigned int bottomCenterIdx = vertices.size();
  vertices.push_back({{0, -halfHeight, 0}, {0, -1, 0}, {0.5f, 0.5f}});

  // Bottom cap ring
  unsigned int bottomRingStart = vertices.size();
  for (int i = 0; i <= segments; ++i) {
    float angle = 2.0f * PI * i / segments;
    float x = radius * cos(angle);
    float z = radius * sin(angle);
    vertices.push_back({{x, -halfHeight, z}, {0, -1, 0}, {(cos(angle) + 1) * 0.5f, (sin(angle) + 1) * 0.5f}});
  }

  // Bottom cap triangles (reversed winding)
  for (int i = 0; i < segments; ++i) {
    indices.push_back(bottomCenterIdx);
    indices.push_back(bottomRingStart + i);
    indices.push_back(bottomRingStart + i + 1);
  }

  // Side surface - top ring vertices (with outward normals)
  unsigned int sideTopStart = vertices.size();
  for (int i = 0; i <= segments; ++i) {
    float angle = 2.0f * PI * i / segments;
    float x = cos(angle);
    float z = sin(angle);
    vertices.push_back({{x * radius, halfHeight, z * radius}, {x, 0, z}, {(float)i / segments, 1}});
  }

  // Side surface - bottom ring vertices (with outward normals)
  unsigned int sideBottomStart = vertices.size();
  for (int i = 0; i <= segments; ++i) {
    float angle = 2.0f * PI * i / segments;
    float x = cos(angle);
    float z = sin(angle);
    vertices.push_back({{x * radius, -halfHeight, z * radius}, {x, 0, z}, {(float)i / segments, 0}});
  }

  // Side surface triangles
  for (int i = 0; i < segments; ++i) {
    unsigned int top0 = sideTopStart + i;
    unsigned int top1 = sideTopStart + i + 1;
    unsigned int bot0 = sideBottomStart + i;
    unsigned int bot1 = sideBottomStart + i + 1;

    indices.push_back(top0);
    indices.push_back(bot0);
    indices.push_back(bot1);

    indices.push_back(top0);
    indices.push_back(bot1);
    indices.push_back(top1);
  }
}

}
