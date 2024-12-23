#pragma once

#include <QPainter>
#include <QString>
#include <QStringList>
#include <fstream>
#include <iostream>
#include <map>
#include "affineutils.hpp"
#include "objmodel.hpp"
#include "shader.hpp"
#include "texture.hpp"

namespace qtgl {

class GLObject {
 public:
  Vertices vertices;
  GLObject() = default;
  virtual ~GLObject() = default;
  GLObject(const GLObject& obj) { vertices = obj.vertices; }
  virtual GLObject* clone() = 0;

  void pushVertice(double x, double y, double z) {
    Vertice v(x, y, z, 1);
    vertices.conservativeResize(vertices.rows() + 1, vertices.cols());
    vertices.row(vertices.rows() - 1) = v;
  }
  virtual void rotate_x(double a) { this->vertices = AffineUtils::rotate_x(this->vertices, a); }
  virtual void rotate_y(double a) { this->vertices = AffineUtils::rotate_y(this->vertices, a); }
  virtual void rotate_z(double a) { this->vertices = AffineUtils::rotate_z(this->vertices, a); }
  virtual void translate(double x, double y, double z) {
    this->vertices = AffineUtils::translate(this->vertices, x, y, z);
  }
  virtual void scale(double x, double y, double z) {
    this->vertices = AffineUtils::scale(this->vertices, x, y, z);
  }
  virtual void draw(QPainter& painter) = 0;
  virtual void shadeVertices(GLShader* shader, std::vector<GLLight*>& lights,
                             Vertice& cameraPos) = 0;
  virtual void rasterize(Fragments& fragments) = 0;
};

// TODO 使用midpoint算法进行光栅化
class GLLine : public GLObject {
 public:
  GLLine() = default;
  ~GLLine() = default;
  GLLine(const GLLine& line) : GLObject(line) {}
  virtual GLObject* clone() {
    GLLine* p = new GLLine;
    p->vertices = this->vertices;
    return p;
  }
  void draw(QPainter& painter) {
    int n = vertices.rows();
    for (int i = 1; i < n; ++i) {
      Vertice p1 = vertices.row(i - 1);
      Vertice p2 = vertices.row(i);
      painter.drawLine(p1[0], p1[1], p2[0], p2[1]);
    }
    Vertice p1 = vertices.row(n - 1);
    Vertice p2 = vertices.row(0);
    painter.drawLine(p1[0], p1[1], p2[0], p2[1]);
  }
  void shadeVertices(GLShader* shader, std::vector<GLLight*>& lights, Vertice& cameraPos) {
    // TODO
  }
  void rasterize(Fragments& fragments) {
    // TODO
  }
};

class GLMesh;

class GLMeshGroup : public GLObject {
 public:
  GLMesh* parent;
  std::string name;
  Indices3 indices;
  NormIndices normIndices;
  std::vector<std::vector<Color01>> colors;
  std::vector<TexRef> texrefs;

  GLMeshGroup(GLMesh* parent, std::string& name) {
    this->parent = parent;
    this->name = name;
  }

  void addIndex3(Index3 idx);

  void addIndex3(Index3 idx, Color01 clr0, Color01 clr1, Color01 clr2) {
    indices.conservativeResize(indices.rows() + 1, indices.cols());
    indices.row(indices.rows() - 1) = idx;
    std::vector<Color01> tricolor{clr0, clr1, clr2};
    colors.push_back(std::move(tricolor));
  }

  void addNormIndex(NormIndex idx) {
    normIndices.conservativeResize(normIndices.rows() + 1, normIndices.cols());
    normIndices.row(normIndices.rows() - 1) = idx;
  }

  GLObject* clone() {
    GLMeshGroup* g = new GLMeshGroup(this->parent, this->name);
    g->indices = indices;
    g->normIndices = normIndices;
    g->colors = colors;
    g->texrefs = texrefs;
    return g;
  }
  void draw(QPainter& painter) {
    // TODO
  }

  void shadeVertices(GLShader* shader, std::vector<GLLight*>& lights, Vertice& cameraPos);

  void rasterize(Fragments& fragments);

  void rasterizeTriangle(Triangle2& t, std::vector<Color01>& clrs, Fragments& fragments,
                         GLTexture* texture);

  void drawSkeleton(QPainter& painter) {
    int n = indices.rows();
    for (int i = 0; i < n; ++i) {
      Index3 idx = indices.row(i);
      Vertice p1 = vertices.row(idx[0]);
      Vertice p2 = vertices.row(idx[1]);
      painter.drawLine(p1[0], p1[1], p2[0], p2[1]);

      p1 = vertices.row(idx[1]);
      p2 = vertices.row(idx[2]);
      painter.drawLine(p1[0], p1[1], p2[0], p2[1]);

      p1 = vertices.row(idx[2]);
      p2 = vertices.row(idx[0]);
      painter.drawLine(p1[0], p1[1], p2[0], p2[1]);
    }
  }
};

class GLMesh : public GLObject {
 public:
  const static Color01 defaultColor;
  const static std::string defaultGroup;
  Normals normals;
  TexCoords texcoords;
  std::map<std::string, GLMeshGroup*> groups;
  std::map<std::string, GLTexture*> textures;

  GLMesh() = default;
  ~GLMesh() {
    for (auto g : groups) {
      delete g.second;
    }
    // TODO 设置公共纹理缓冲
    //  for (auto t : textures) {
    //    delete t.second;
    //  }
  };
  GLMesh(const GLMesh& mesh) : GLObject(mesh) { groups = mesh.groups; }
  GLObject* clone() {
    GLMesh* p = new GLMesh;
    p->groups = this->groups;
    p->vertices = this->vertices;
    p->normals = this->normals;
    for (auto g : groups) {
      p->groups[g.first] = reinterpret_cast<GLMeshGroup*>((g.second)->clone());
      p->groups[g.first]->parent = p;
    }
    p->texcoords = this->texcoords;
    p->textures = this->textures;
    return p;
  }

  GLMeshGroup* getGroup(std::string& name) {
    if (groups.count(name)) {
      return groups[name];
    } else {
      groups[name] = new GLMeshGroup(this, name);
      return groups[name];
    }
  }

  void addIndex3(Index3 idx) { addIndex3(const_cast<std::string&>(defaultGroup), idx); }
  void addIndex3(Index3 idx, Color01 clr0, Color01 clr1, Color01 clr2) {
    addIndex3(const_cast<std::string&>(defaultGroup), idx, clr0, clr1, clr2);
  }
  void addIndex3(std::string& groupName, Index3 idx) {
    getGroup(groupName)->addIndex3(idx, GLMesh::defaultColor, GLMesh::defaultColor,
                                   GLMesh::defaultColor);
  }

  void addIndex3(std::string& groupName, Index3 idx, Color01 clr0, Color01 clr1, Color01 clr2) {
    GLMeshGroup* group = getGroup(groupName);
    group->indices.conservativeResize(group->indices.rows() + 1, group->indices.cols());
    group->indices.row(group->indices.rows() - 1) = idx;
    std::vector<Color01> tricolor{clr0, clr1, clr2};
    group->colors.push_back(std::move(tricolor));
  }

  void pushNormal(double a, double b, double c) {
    Normal n(a, b, c);
    normals.conservativeResize(normals.rows() + 1, normals.cols());
    normals.row(normals.rows() - 1) = n;
  }

  void addNormIndex(std::string& groupName, NormIndex idx) {
    GLMeshGroup* group = getGroup(groupName);
    group->addNormIndex(idx);
  }

  void pushTexCoord(double u, double v) {
    TexCoord t(u, v);
    texcoords.conservativeResize(texcoords.rows() + 1, texcoords.cols());
    texcoords.row(texcoords.rows() - 1) = t;
  }

  void rotate_x(double a) {
    this->vertices = AffineUtils::rotate_x(this->vertices, a);
    this->normals = AffineUtils::normal_rotate_x(this->normals, a);
  }
  void rotate_y(double a) {
    this->vertices = AffineUtils::rotate_y(this->vertices, a);
    this->normals = AffineUtils::normal_rotate_y(this->normals, a);
  }
  void rotate_z(double a) {
    this->vertices = AffineUtils::rotate_z(this->vertices, a);
    this->normals = AffineUtils::normal_rotate_z(this->normals, a);
  }
  void translate(double x, double y, double z) {
    this->vertices = AffineUtils::translate(this->vertices, x, y, z);
    this->normals = AffineUtils::normal_translate(this->normals, x, y, z);
  }
  void scale(double x, double y, double z) {
    this->vertices = AffineUtils::scale(this->vertices, x, y, z);
    this->normals = AffineUtils::norm_scale(this->normals, x, y, z);
  }

  static GLMesh* readFromObjFile(std::string fpath);

  static GLMesh* fromObjModel(ObjModel* model);

  void shadeVertices(GLShader* shader, std::vector<GLLight*>& lights, Vertice& cameraPos);

  void rasterize(Fragments& fragments);

  void draw(QPainter& painter) {
    // rasterize(painter);
    // drawSkeleton(painter);
  }
};

}  // namespace qtgl