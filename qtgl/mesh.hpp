#pragma once

#include <QPainter>
#include <QString>
#include <QStringList>
#include <fstream>
#include <iostream>
#include "affineutils.hpp"

namespace qtgl {

class GLObject {
 public:
  Vertices vertices;
  GLObject() = default;
  virtual ~GLObject() = default;
  GLObject(const GLObject& obj) { vertices = obj.vertices; }
  virtual GLObject* clone() = 0;

  void pushVertice(float x, float y, float z) {
    Vertice v(x, y, z, 1);
    vertices.conservativeResize(vertices.rows() + 1, vertices.cols());
    vertices.row(vertices.rows() - 1) = v;
  }
  void rotate_x(float a) { this->vertices = AffineUtils::rotate_x(this->vertices, a); }
  void rotate_y(float a) { this->vertices = AffineUtils::rotate_y(this->vertices, a); }
  void rotate_z(float a) { this->vertices = AffineUtils::rotate_z(this->vertices, a); }
  void translate(float x, float y, float z) {
    this->vertices = AffineUtils::translate(this->vertices, x, y, z);
  }
  void scale(float x, float y, float z) {
    this->vertices = AffineUtils::scale(this->vertices, x, y, z);
  }
  virtual void draw(QPainter& painter) = 0;
};

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
};

class GLMesh : public GLObject {
 public:
  Indices3 indices;
  GLMesh() = default;
  ~GLMesh() = default;
  GLMesh(const GLMesh& mesh) : GLObject(mesh) { indices = mesh.indices; }
  GLObject* clone() {
    GLMesh* p = new GLMesh;
    p->indices = this->indices;
    p->vertices = this->vertices;
    return p;
  }
  void addIndex3(Index3 idx) {
    indices.conservativeResize(indices.rows() + 1, indices.cols());
    indices.row(indices.rows() - 1) = idx;
  }

  static std::vector<GLMesh> readFromObjFile(std::string fpath) {
    std::vector<GLMesh> meshes;
    std::ifstream ifs;
    ifs.open(fpath, std::ios::in);

    if (!ifs.is_open()) {
      return {};
    }

    char buf[1024] = {0};
    bool newMesh = true;
    GLMesh* mesh = nullptr;

    int i = 0;

    while (ifs.getline(buf, sizeof(buf))) {
      if (i % 1000 == 0) {
        std::cout << i << std::endl;
      }
      i++;
      if (buf[0] == 'v' && buf[1] == ' ') {
        if (newMesh) {
          if (mesh) {
            meshes.push_back(*mesh);
            delete mesh;
            mesh = nullptr;
          }
          mesh = new GLMesh;
        }
        QString line(buf);
        QStringList strlst = line.split(" ");

        mesh->pushVertice(strlst[1].toFloat(), strlst[2].toFloat(), strlst[3].toFloat());
        newMesh = false;
      } else {
        if (buf[0] == 'f' && buf[1] == ' ') {
          QString line(buf);
          QStringList strlst = line.split(" ");
          std::vector<int> v;
          for (int i = 1; i < strlst.length(); ++i) {
            v.push_back(strlst[i].split("/")[0].toInt() - 1);
          }
          Index3 idx{v[0], v[1], v[2]};
          mesh->addIndex3(idx);
        }
        newMesh = true;
      }
    }

    if (mesh) {
      meshes.push_back(*mesh);
      delete mesh;
      mesh = nullptr;
    }

    return meshes;
  }

  void draw(QPainter& painter) {
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

}  // namespace qtgl