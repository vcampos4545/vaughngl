#include <vaughngl/OBJMesh.h>
#include <fstream>
#include <sstream>
#include <algorithm>

static std::string getDirectory(const std::string& path) {
  size_t pos = path.find_last_of("/\\");
  return (pos == std::string::npos) ? "" : path.substr(0, pos + 1);
}

static std::vector<std::string> split(const std::string& s, char delim) {
  std::vector<std::string> result;
  std::stringstream ss(s);
  std::string item;
  while (std::getline(ss, item, delim)) {
    if (!item.empty()) result.push_back(item);
  }
  return result;
}

bool OBJMesh::loadMTL(const std::string& path) {
  std::ifstream file(path);
  if (!file.is_open()) {
    return false; // MTL file is optional
  }

  Material* currentMat = nullptr;
  std::string line;

  while (std::getline(file, line)) {
    // Remove carriage return if present
    if (!line.empty() && line.back() == '\r') {
      line.pop_back();
    }

    std::istringstream iss(line);
    std::string token;
    iss >> token;

    if (token == "newmtl") {
      std::string name;
      iss >> name;
      m_materials[name] = Material{name};
      currentMat = &m_materials[name];
    } else if (currentMat) {
      if (token == "Kd") {
        iss >> currentMat->diffuse.r >> currentMat->diffuse.g >> currentMat->diffuse.b;
      } else if (token == "Ka") {
        iss >> currentMat->ambient.r >> currentMat->ambient.g >> currentMat->ambient.b;
      } else if (token == "Ks") {
        iss >> currentMat->specular.r >> currentMat->specular.g >> currentMat->specular.b;
      } else if (token == "Ns") {
        iss >> currentMat->shininess;
      }
    }
  }

  return true;
}

bool OBJMesh::load(const std::string& path) {
  std::ifstream file(path);
  if (!file.is_open()) {
    m_error = "Failed to open file: " + path;
    return false;
  }

  std::string directory = getDirectory(path);

  std::vector<glm::vec3> positions;
  std::vector<glm::vec3> normals;
  std::vector<glm::vec2> texCoords;

  // Faces grouped by material: (materialName, faces)
  // Each face is 3 vertices, each vertex has 3 indices: position/texcoord/normal
  std::vector<std::tuple<std::string, std::vector<std::array<int, 9>>>> materialFaces;
  std::string currentMaterial = "";

  std::string line;
  while (std::getline(file, line)) {
    // Remove carriage return if present
    if (!line.empty() && line.back() == '\r') {
      line.pop_back();
    }

    std::istringstream iss(line);
    std::string token;
    iss >> token;

    if (token == "mtllib") {
      std::string mtlFile;
      iss >> mtlFile;
      loadMTL(directory + mtlFile);
    } else if (token == "usemtl") {
      iss >> currentMaterial;
      // Start a new group for this material
      materialFaces.push_back({currentMaterial, {}});
    } else if (token == "v") {
      glm::vec3 pos;
      iss >> pos.x >> pos.y >> pos.z;
      positions.push_back(pos);
    } else if (token == "vn") {
      glm::vec3 normal;
      iss >> normal.x >> normal.y >> normal.z;
      normals.push_back(normal);
    } else if (token == "vt") {
      glm::vec2 uv;
      iss >> uv.x >> uv.y;
      texCoords.push_back(uv);
    } else if (token == "f") {
      // Parse face - support triangles and quads
      std::vector<std::array<int, 3>> faceVerts;
      std::string vertStr;

      while (iss >> vertStr) {
        std::array<int, 3> indices = {0, 0, 0}; // pos, uv, normal
        auto parts = split(vertStr, '/');

        if (parts.size() >= 1 && !parts[0].empty()) {
          indices[0] = std::stoi(parts[0]);
        }
        if (parts.size() >= 2 && !parts[1].empty()) {
          indices[1] = std::stoi(parts[1]);
        }
        if (parts.size() >= 3 && !parts[2].empty()) {
          indices[2] = std::stoi(parts[2]);
        }

        faceVerts.push_back(indices);
      }

      // Ensure we have a material group
      if (materialFaces.empty()) {
        materialFaces.push_back({"", {}});
      }

      auto& currentFaces = std::get<1>(materialFaces.back());

      // Triangulate: fan triangulation for convex polygons
      for (size_t i = 1; i + 1 < faceVerts.size(); ++i) {
        std::array<int, 9> tri;
        // Vertex 0
        tri[0] = faceVerts[0][0];
        tri[1] = faceVerts[0][1];
        tri[2] = faceVerts[0][2];
        // Vertex 1
        tri[3] = faceVerts[i][0];
        tri[4] = faceVerts[i][1];
        tri[5] = faceVerts[i][2];
        // Vertex 2
        tri[6] = faceVerts[i + 1][0];
        tri[7] = faceVerts[i + 1][1];
        tri[8] = faceVerts[i + 1][2];
        currentFaces.push_back(tri);
      }
    }
  }

  if (positions.empty()) {
    m_error = "No vertices found in file";
    return false;
  }

  buildMeshes(positions, normals, texCoords, materialFaces);
  return true;
}

void OBJMesh::buildMeshes(
  const std::vector<glm::vec3>& positions,
  const std::vector<glm::vec3>& normals,
  const std::vector<glm::vec2>& texCoords,
  const std::vector<std::tuple<std::string, std::vector<std::array<int, 9>>>>& materialFaces
) {
  m_subMeshes.clear();

  for (const auto& [matName, faces] : materialFaces) {
    if (faces.empty()) continue;

    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    // For each triangle
    for (const auto& tri : faces) {
      for (int v = 0; v < 3; ++v) {
        int posIdx = tri[v * 3 + 0];
        int uvIdx = tri[v * 3 + 1];
        int normIdx = tri[v * 3 + 2];

        Vertex vert;

        // OBJ indices are 1-based, negative means relative to end
        if (posIdx > 0) {
          vert.position = positions[posIdx - 1];
        } else if (posIdx < 0) {
          vert.position = positions[positions.size() + posIdx];
        }

        if (uvIdx > 0 && static_cast<size_t>(uvIdx) <= texCoords.size()) {
          vert.uv = texCoords[uvIdx - 1];
        } else if (uvIdx < 0) {
          vert.uv = texCoords[texCoords.size() + uvIdx];
        } else {
          vert.uv = {0, 0};
        }

        if (normIdx > 0 && static_cast<size_t>(normIdx) <= normals.size()) {
          vert.normal = normals[normIdx - 1];
        } else if (normIdx < 0) {
          vert.normal = normals[normals.size() + normIdx];
        } else {
          vert.normal = {0, 1, 0}; // default up normal
        }

        indices.push_back(static_cast<unsigned int>(vertices.size()));
        vertices.push_back(vert);
      }
    }

    SubMesh subMesh;
    subMesh.mesh.upload(vertices, indices);

    // Find material
    auto it = m_materials.find(matName);
    if (it != m_materials.end()) {
      subMesh.material = it->second;
    } else {
      subMesh.material.name = matName;
    }

    m_subMeshes.push_back(std::move(subMesh));
  }
}
