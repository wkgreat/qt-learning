#include "mesh.hpp"

namespace qtgl {

void GLMeshGroup::addIndex3(Index3 idx) {
  addIndex3(idx, GLMesh::defaultColor, GLMesh::defaultColor, GLMesh::defaultColor);
}

void GLMeshGroup::shadeVertices(GLShader* shader, std::vector<GLLight*>& lights,
                                Vertice& cameraPos) {
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
    NormIndex normIdx = normIndices.row(i);
    TexRef ref = texrefs[i];
    Vertice p0 = parent->vertices.row(idx[0]);
    Vertice p1 = parent->vertices.row(idx[1]);
    Vertice p2 = parent->vertices.row(idx[2]);
    Normal n0 = parent->normals.row(normIdx[0]);
    Normal n1 = parent->normals.row(normIdx[1]);
    Normal n2 = parent->normals.row(normIdx[2]);

    if (ref.indices[0] != -1 && ref.indices[1] != -1 && ref.indices[2] != -1) {
      GLTexture* texture = parent->textures[ref.mtlname];
      TexCoord t0 = parent->texcoords.row(ref.indices[0]);
      TexCoord t1 = parent->texcoords.row(ref.indices[1]);
      TexCoord t2 = parent->texcoords.row(ref.indices[2]);
      Triangle2 t(p0, p1, p2, n0, n1, n2, t0, t1, t2);
      std::vector<Color01> clrs = colors[i];
      rasterizeTriangle(t, clrs, fragments, texture);
    } else {
      Triangle2 t(p0, p1, p2, n0, n1, n2);
      std::vector<Color01> clrs = colors[i];
      rasterizeTriangle(t, clrs, fragments, nullptr);
    }
  }
}

void GLMeshGroup::rasterizeTriangle(Triangle2& t, std::vector<Color01>& clrs, Fragments& fragments,
                                    GLTexture* texture) {
  // mbr
  int xmin = static_cast<int>(std::min(std::min(t.hx0(), t.hx1()), t.hx2()));
  int xmax = static_cast<int>(std::max(std::max(t.hx0(), t.hx1()), t.hx2()));
  int ymin = static_cast<int>(std::min(std::min(t.hy0(), t.hy1()), t.hy2()));
  int ymax = static_cast<int>(std::max(std::max(t.hy0(), t.hy1()), t.hy2()));

  double depth;
  Color01 color;
  Triangle2::BarycentricCoordnates coord;

  for (int x = xmin; x <= xmax; ++x) {
    if (x < 0 || x >= fragments[0].size()) continue;
    for (int y = ymin; y <= ymax; ++y) {
      if (y < 0 || y >= fragments.size()) continue;
      coord = t.resovleBarycentricCoordnates(x, y);
      if (coord.alpha >= 0 && coord.beta >= 0 && coord.gamma >= 0) {
        depth = coord.alpha * t.hz0() + coord.beta * t.hz1() + coord.gamma * t.hz2();
        if (depth < fragments[y][x].depth) {
          color.R = coord.alpha * clrs[0].R + coord.beta * clrs[1].R + coord.gamma * clrs[2].R;
          color.G = coord.alpha * clrs[0].G + coord.beta * clrs[1].G + coord.gamma * clrs[2].G;
          color.B = coord.alpha * clrs[0].B + coord.beta * clrs[1].B + coord.gamma * clrs[2].B;

          if (t.getHasTexture() && texture) {
            TexCoord tc = coord.alpha * t.getTexCoord0() + coord.beta * t.getTexCoord1() +
                          coord.gamma * t.getTexCoord2();
            Color01 texColor = texture->sample(tc);
            color.R *= texColor.R;
            color.G *= texColor.G;
            color.B *= texColor.B;
          }

          fragments[y][x].color = color;
          fragments[y][x].depth = depth;
        }
      }
    }
  }
}

const Color01 GLMesh::defaultColor = {1, 1, 1};
const std::string GLMesh::defaultGroup = "default";

void GLMesh::shadeVertices(GLShader* shader, std::vector<GLLight*>& lights, Vertice& cameraPos) {
  for (auto g : groups) {
    (g.second)->shadeVertices(shader, lights, cameraPos);
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
  mesh->texcoords = model->texcoords;
  for (auto g : model->groups) {
    std::string name = g.first;
    ObjModelGroup& group = g.second;
    GLMeshGroup* meshGroup = new GLMeshGroup(mesh, name);
    meshGroup->indices = group.indices;
    meshGroup->normIndices = group.normIndices;
    meshGroup->texrefs = group.texrefs;
    mesh->groups[name] = meshGroup;
    Color01 white = {1, 1, 1, 1};
    for (int i = 0; i < meshGroup->indices.rows(); ++i) {
      meshGroup->colors.push_back({white, white, white});
    }
  }
  if (model->mtllib) {
    for (auto mtl : model->mtllib->mtls) {
      std::string name = mtl.first;
      ObjMaterial& material = mtl.second;
      if (!material.map_kd.empty()) {
        GLTexture* texture = mtl.second.toTexture();
        std::cout << "load texture: " << material.map_kd << std::endl;
        mesh->textures[name] = texture;
      }
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