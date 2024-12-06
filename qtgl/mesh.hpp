#pragma once

#include <QPainter>
#include <QString>
#include <QStringList>
#include <fstream>
#include <iostream>
#include "affineutils.hpp"

namespace qtgl {
class Mesh {
 public:
  Vertices vertices;
  std::vector<std::vector<int>> facets;
  Mesh() {}
  void pushVertice(float x, float y, float z) {
    Vertice v(x, y, z, 1);
    vertices.conservativeResize(vertices.rows() + 1, vertices.cols());
    vertices.row(vertices.rows() - 1) = v;
  }
  void addFacet(std::vector<int> facet) { facets.push_back(facet); }
  void emplaceFacet(std::vector<int>& facet) { facets.push_back(facet); }
  void emplaceFacet(std::vector<int>&& facet) { facets.push_back(std::move(facet)); }

  static Mesh makeCube(int s) {
    Mesh mesh;
    mesh.pushVertice(-s, -s, -s);  // 0
    mesh.pushVertice(-s, s, -s);   // 1
    mesh.pushVertice(s, s, -s);    // 2
    mesh.pushVertice(s, -s, -s);   // 3

    mesh.pushVertice(-s, -s, s);  // 4
    mesh.pushVertice(-s, s, s);   // 5
    mesh.pushVertice(s, s, s);    // 6
    mesh.pushVertice(s, -s, s);   // 7

    mesh.addFacet({0, 1, 2, 3, 0});
    mesh.addFacet({4, 5, 6, 7, 4});
    mesh.addFacet({0, 1, 5, 4, 0});
    mesh.addFacet({1, 2, 6, 5, 1});
    mesh.addFacet({2, 3, 7, 6, 2});
    mesh.addFacet({0, 3, 7, 4, 0});
    return mesh;
  }

  static std::vector<Mesh> readFromObjFile(std::string fpath) {
    std::vector<Mesh> meshes;
    std::ifstream ifs;
    ifs.open(fpath, std::ios::in);

    if (!ifs.is_open()) {
      return {};
    }

    char buf[1024] = {0};
    bool newMesh = true;
    Mesh* mesh = nullptr;

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
          mesh = new Mesh;
        }
        QString line(buf);
        QStringList strlst = line.split(" ");

        mesh->pushVertice(strlst[1].toFloat(), strlst[2].toFloat(), strlst[3].toFloat());
        newMesh = false;
      } else {
        if (buf[0] == 'f' && buf[1] == ' ') {
          QString line(buf);
          QStringList strlst = line.split(" ");
          std::vector<int> facet;
          for (int i = 1; i < strlst.length(); ++i) {
            facet.push_back(strlst[i].split("/")[0].toInt() - 1);
          }
          facet.push_back(facet[0]);
          mesh->emplaceFacet(std::move(facet));
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

  void rotate_x(float a) { this->vertices = AffineUtils::rotate_x(this->vertices, a); }
  void rotate_y(float a) { this->vertices = AffineUtils::rotate_y(this->vertices, a); }
  void rotate_z(float a) { this->vertices = AffineUtils::rotate_z(this->vertices, a); }
  void translate(float x, float y, float z) {
    this->vertices = AffineUtils::translate(this->vertices, x, y, z);
  }
  void scale(float x, float y, float z) {
    this->vertices = AffineUtils::scale(this->vertices, x, y, z);
  }

  void draw(QPainter& painter) {
    for (auto& facet : facets) {
      for (int i = 1; i < facet.size(); ++i) {
        int nrows = vertices.rows();
        if (facet[i - 1] < 0 || facet[i - 1] >= nrows) {
          std::cout << "WARNING: OUT OF ROWS1, " << facet[i - 1] << "," << nrows << std::endl;
          continue;
        }
        if (facet[i] < 0 || facet[i] >= nrows) {
          std::cout << "WARNING: OUT OF ROWS2, " << facet[i] << "," << nrows << std::endl;
          continue;
        }
        Vertice p1 = vertices.row(facet[i - 1]);
        Vertice p2 = vertices.row(facet[i]);
        painter.drawLine(p1[0], p1[1], p2[0], p2[1]);
      }
    }
  }
};

}  // namespace qtgl