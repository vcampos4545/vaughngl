#ifndef OBJMESH_H
#define OBJMESH_H

#include <vaughngl/Mesh.h>
#include <glm/glm.hpp>
#include <string>
#include <vector>
#include <unordered_map>

struct Material {
  std::string name;
  glm::vec3 diffuse{0.8f, 0.8f, 0.8f};
  glm::vec3 ambient{0.2f, 0.2f, 0.2f};
  glm::vec3 specular{1.0f, 1.0f, 1.0f};
  float shininess = 32.0f;
};

struct SubMesh {
  Mesh mesh;
  Material material;
};

class OBJMesh {
public:
  OBJMesh() = default;
  ~OBJMesh() = default;

  OBJMesh(OBJMesh&&) = default;
  OBJMesh& operator=(OBJMesh&&) = default;
  OBJMesh(const OBJMesh&) = delete;
  OBJMesh& operator=(const OBJMesh&) = delete;

  bool load(const std::string& path);
  bool isLoaded() const { return !m_subMeshes.empty(); }

  const std::vector<SubMesh>& getSubMeshes() const { return m_subMeshes; }
  const std::string& getError() const { return m_error; }

private:
  bool loadMTL(const std::string& path);
  void buildMeshes(
    const std::vector<glm::vec3>& positions,
    const std::vector<glm::vec3>& normals,
    const std::vector<glm::vec2>& texCoords,
    const std::vector<std::tuple<std::string, std::vector<std::array<int, 9>>>>& materialFaces
  );

  std::vector<SubMesh> m_subMeshes;
  std::unordered_map<std::string, Material> m_materials;
  std::string m_error;
};

#endif
