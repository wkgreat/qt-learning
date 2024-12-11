#pragma once

#include <QPainter>
#include <QString>
#include <QStringList>
#include <fstream>
#include <iostream>
#include <map>
#include "affineutils.hpp"

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
  void rotate_x(double a) { this->vertices = AffineUtils::rotate_x(this->vertices, a); }
  void rotate_y(double a) { this->vertices = AffineUtils::rotate_y(this->vertices, a); }
  void rotate_z(double a) { this->vertices = AffineUtils::rotate_z(this->vertices, a); }
  void translate(double x, double y, double z) {
    this->vertices = AffineUtils::translate(this->vertices, x, y, z);
  }
  void scale(double x, double y, double z) {
    this->vertices = AffineUtils::scale(this->vertices, x, y, z);
  }
  virtual void draw(QPainter& painter) = 0;
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
  std::vector<std::vector<Color>> colors;

  GLMeshGroup(GLMesh* parent, std::string& name) {
    this->parent = parent;
    this->name = name;
  }

  void addIndex3(Index3 idx);

  void addIndex3(Index3 idx, Color clr0, Color clr1, Color clr2) {
    indices.conservativeResize(indices.rows() + 1, indices.cols());
    indices.row(indices.rows() - 1) = idx;
    std::vector<Color> tricolor{clr0, clr1, clr2};
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
    return g;
  }
  void draw(QPainter& painter) {
    // TODO
  }

  void rasterize(Fragments& fragments);

  void rasterizeTriangle(Triangle& t, std::vector<Color>& clrs, Fragments& fragments) {
    // mbr
    int xmin = static_cast<int>(std::min(std::min(t.x0(), t.x1()), t.x2()));
    int xmax = static_cast<int>(std::max(std::max(t.x0(), t.x1()), t.x2()));
    int ymin = static_cast<int>(std::min(std::min(t.y0(), t.y1()), t.y2()));
    int ymax = static_cast<int>(std::max(std::max(t.y0(), t.y1()), t.y2()));

    for (int x = xmin; x <= xmax; ++x) {
      if (x < 0 || x >= fragments[0].size()) continue;
      for (int y = ymin; y <= ymax; ++y) {
        if (y < 0 || y >= fragments.size()) continue;
        Triangle::BarycentricCoordnates coord = t.resovleBarycentricCoordnates(x, y);
        if (coord.alpha >= 0 && coord.beta >= 0 && coord.gamma >= 0) {
          double depth = coord.alpha * t.z0() + coord.beta * t.z1() + coord.gamma * t.z2();
          if (depth < fragments[y][x].depth) {
            Color c;
            c.R = coord.alpha * clrs[0].R + coord.beta * clrs[1].R + coord.gamma * clrs[2].R;
            c.G = coord.alpha * clrs[0].G + coord.beta * clrs[1].G + coord.gamma * clrs[2].G;
            c.B = coord.alpha * clrs[0].B + coord.beta * clrs[1].B + coord.gamma * clrs[2].B;
            fragments[y][x].color.R = c.R;
            fragments[y][x].color.G = c.G;
            fragments[y][x].color.B = c.B;
            fragments[y][x].depth = depth;
          }
        }
      }
    }
  }

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
  const static Color defaultColor;
  const static std::string defaultGroup;
  Normals normals;
  std::map<std::string, GLMeshGroup*> groups;

  GLMesh() = default;
  ~GLMesh() {
    for (auto g : groups) {
      delete g.second;
    }
  };
  GLMesh(const GLMesh& mesh) : GLObject(mesh) { groups = mesh.groups; }
  GLObject* clone() {
    GLMesh* p = new GLMesh;
    p->groups = this->groups;
    p->vertices = this->vertices;
    for (auto g : groups) {
      p->groups[g.first] = reinterpret_cast<GLMeshGroup*>((g.second)->clone());
      p->groups[g.first]->parent = p;
    }
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
  void addIndex3(Index3 idx, Color clr0, Color clr1, Color clr2) {
    addIndex3(const_cast<std::string&>(defaultGroup), idx, clr0, clr1, clr2);
  }
  void addIndex3(std::string& groupName, Index3 idx) {
    getGroup(groupName)->addIndex3(idx, GLMesh::defaultColor, GLMesh::defaultColor,
                                   GLMesh::defaultColor);
  }

  void addIndex3(std::string& groupName, Index3 idx, Color clr0, Color clr1, Color clr2) {
    GLMeshGroup* group = getGroup(groupName);
    group->indices.conservativeResize(group->indices.rows() + 1, group->indices.cols());
    group->indices.row(group->indices.rows() - 1) = idx;
    std::vector<Color> tricolor{clr0, clr1, clr2};
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

  static GLMesh* readFromObjFile(std::string fpath);

  void rasterize(Fragments& fragments);

  void draw(QPainter& painter) {
    // rasterize(painter);
    // drawSkeleton(painter);
  }
};

}  // namespace qtgl