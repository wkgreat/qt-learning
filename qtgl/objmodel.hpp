#pragma once

#include <QString>
#include <QStringList>
#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include "define.hpp"

namespace qtgl {

class ObjMaterial {
 public:
  std::string dirpath;
  std::string name;
  double ns;
  Color01 ka;
  Color01 kd;
  Color01 Ks;
  Color01 ke;
  double ni;
  double d;
  int illum;
  std::string map_refl;
};

class ObjMaterialLib {
 public:
  std::string dirpath;
  std::string libname;
  std::map<std::string, ObjMaterial> mtls;
};

class TexRef {
 public:
  std::string mtlname;
  Eigen::Vector3i indices;
};

class ObjModel;

class ObjModelGroup {
 public:
  std::string name;
  ObjModel* parent;
  Indices3 indices;
  NormIndices normIndices;
  std::string mtlname;
  std::vector<TexRef> texrefs;

  ObjModelGroup() = default;

  ObjModelGroup(ObjModel* parent, std::string& name) {
    this->parent = parent;
    this->name = name;
  }

  void addIndex3(Index3 idx) {
    indices.conservativeResize(indices.rows() + 1, indices.cols());
    indices.row(indices.rows() - 1) = idx;
  }

  void addNormIndex(NormIndex idx) {
    normIndices.conservativeResize(normIndices.rows() + 1, normIndices.cols());
    normIndices.row(normIndices.rows() - 1) = idx;
  }
};

class ObjModel {
 public:
  const static std::string defaultGroup;
  std::string objpath;
  std::string dirpath;
  std::string objname;
  ObjMaterialLib mtllib;
  std::map<std::string, ObjModelGroup> groups;
  Vertices vertices;
  Normals normals;
  TexCoords texcoords;

  ObjModel() = default;
  ~ObjModel() = default;

  void pushVertice(double x, double y, double z) {
    Vertice v(x, y, z, 1);
    vertices.conservativeResize(vertices.rows() + 1, vertices.cols());
    vertices.row(vertices.rows() - 1) = v;
  }

  void pushNormal(double a, double b, double c) {
    Normal n(a, b, c);
    normals.conservativeResize(normals.rows() + 1, normals.cols());
    normals.row(normals.rows() - 1) = n;
  }

  ObjModelGroup& getGroup(std::string& name) {
    if (groups.count(name)) {
      return groups[name];
    } else {
      groups[name] = ObjModelGroup(this, name);
      return groups[name];
    }
  }

  void addIndex3(std::string& groupName, Index3 idx) { getGroup(groupName).addIndex3(idx); }

  void addNormIndex(std::string& groupName, NormIndex idx) {
    getGroup(groupName).addNormIndex(idx);
  }

  static ObjModel* loadObj(const std::string& objpath);
};

}  // namespace qtgl