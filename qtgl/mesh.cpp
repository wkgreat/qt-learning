#include "mesh.hpp"

namespace qtgl {

void GLMeshGroup::addIndex3(Index3 idx) {
  addIndex3(idx, GLMesh::defaultColor, GLMesh::defaultColor, GLMesh::defaultColor);
}

void GLMeshGroup::shade(GLShader* shader, std::vector<GLLight*>& lights, Vertice& cameraPos) {
  int n = indices.rows();
  if (parent->normals.rows() == 0 || normIndices.rows() == 0) {  // 无法线信息
    return;
  }
  for (int i = 0; i < n; ++i) {
    Index3 idx = indices.row(i);
    Vertice p0 = parent->vertices.row(idx[0]);
    Vertice p1 = parent->vertices.row(idx[1]);
    Vertice p2 = parent->vertices.row(idx[2]);
    Eigen::Vector3d e0 = (cameraPos.head(3) - p0.head(3)).normalized();
    Eigen::Vector3d e1 = (cameraPos.head(3) - p1.head(3)).normalized();
    Eigen::Vector3d e2 = (cameraPos.head(3) - p2.head(3)).normalized();
    NormIndex normIdx = normIndices.row(i);
    Normal n0 = parent->normals.row(normIdx[0]).normalized();
    Normal n1 = parent->normals.row(normIdx[1]).normalized();
    Normal n2 = parent->normals.row(normIdx[2]).normalized();
    Color01 d0 = colors[i][0];
    Color01 d1 = colors[i][1];
    Color01 d2 = colors[i][2];
    Color01 c0{0, 0, 0, 0}, c1{0, 0, 0, 0}, c2{0, 0, 0, 0};
    for (GLLight* lgt : lights) {
      c0 = c0 + shader->shade(lgt, n0, p0, e0, d0);
      c1 = c1 + shader->shade(lgt, n1, p1, e1, d1);
      c2 = c2 + shader->shade(lgt, n2, p2, e2, d2);
    }
    c0.clamp();
    c1.clamp();
    c2.clamp();
    colors[i][0] = c0;
    colors[i][1] = c1;
    colors[i][2] = c2;
  }
}

void GLMeshGroup::rasterize(Fragments& fragments) {
  int n = indices.rows();
  for (int i = 0; i < n; ++i) {
    Index3 idx = indices.row(i);
    Vertice p0 = parent->vertices.row(idx[0]);
    Vertice p1 = parent->vertices.row(idx[1]);
    Vertice p2 = parent->vertices.row(idx[2]);
    Triangle t(p0, p1, p2);
    std::vector<Color01> clrs = colors[i];
    rasterizeTriangle(t, clrs, fragments);
  }
}

const Color01 GLMesh::defaultColor = {1, 1, 1};
const std::string GLMesh::defaultGroup = "default";

void GLMesh::shade(GLShader* shader, std::vector<GLLight*>& lights, Vertice& cameraPos) {
  for (auto g : groups) {
    (g.second)->shade(shader, lights, cameraPos);
  }
}

void GLMesh::rasterize(Fragments& fragments) {
  for (auto g : groups) {
    (g.second)->rasterize(fragments);
  }
}

GLMesh* GLMesh::fromObjModel(ObjModel* model) {
  GLMesh* mesh = new GLMesh;
  mesh->vertices = model->vertices;
  mesh->normals = model->normals;
  for (auto g : model->groups) {
    std::string name = g.first;
    ObjModelGroup& group = g.second;
    GLMeshGroup* meshGroup = new GLMeshGroup(mesh, name);
    meshGroup->indices = group.indices;
    meshGroup->normIndices = group.normIndices;
    mesh->groups[name] = meshGroup;
    for (int i = 0; i < meshGroup->indices.rows(); ++i) {
      meshGroup->colors.push_back({Color01::random(), Color01::random(), Color01::random()});
    }
  }
  return mesh;
}

GLMesh* GLMesh::readFromObjFile(std::string fpath) {
  GLMesh* mesh = nullptr;
  ObjModel* model = ObjModel::loadObj(fpath);
  if (model) {
    mesh = fromObjModel(model);
    delete model;
  }
  return mesh;
}

}  // namespace qtgl