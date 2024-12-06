#pragma once

#include "camera.hpp"
#include "mesh.hpp"
#include "projection.hpp"

namespace qtgl {
class GLScene {
 private:
  float viewHeight;
  float viewWidth;
  GLCamera camera;
  GLProjection projection;
  std::vector<Mesh> objs;
  bool showAxis;
  std::vector<Mesh> axis;

 public:
  GLScene(float viewHeight = 768.0, float viewWidth = 1024.0)
      : viewHeight(viewHeight), viewWidth(viewWidth) {
    this->projection.height = viewHeight;
    this->projection.width = viewWidth;
    addAxis();
  }
  GLCamera& getCamera() { return this->camera; }
  GLProjection& getProjection() { return this->projection; }
  void setViewHeight(float h) {
    this->viewHeight = h;
    this->projection.height = h;
  }
  void setViewWidth(float w) {
    this->viewWidth = w;
    this->projection.width = w;
  }
  void setViewSize(float w, float h) {
    this->setViewWidth(w);
    this->setViewHeight(h);
  }
  void setShowAxis(bool b) { this->showAxis = b; }
  void addAxis() {
    Mesh xAxis, yAxis, zAxis;
    xAxis.pushVertice(0, 0, 0);
    xAxis.pushVertice(50, 0, 0);
    xAxis.addFacet({0, 1});
    yAxis.pushVertice(0, 0, 0);
    yAxis.pushVertice(0, 50, 0);
    yAxis.addFacet({0, 1});
    zAxis.pushVertice(0, 0, 0);
    zAxis.pushVertice(0, 0, 50);
    zAxis.addFacet({0, 1});
    this->axis.push_back(std::move(xAxis));
    this->axis.push_back(std::move(yAxis));
    this->axis.push_back(std::move(zAxis));
  }
  void addObj(Mesh& mesh) { objs.push_back(mesh); }
  std::vector<Mesh>& getObjs() { return this->objs; }
  Eigen::Matrix4f viewportMatrix() {
    float hw = this->viewWidth / 2;
    float hh = this->viewHeight / 2;
    Eigen::Matrix4f viewMtx;
    viewMtx << hw, 0, 0, 0,  //
        0, -hh, 0, 0,        //
        0, 0, 1, 0,          //
        hw, hh, 0, 1;        //
    return viewMtx;
  }

  Mesh meshToView(Mesh& mesh) {
    Mesh viewMesh;
    Vertices viewVertices = mesh.vertices;
    // View矩阵
    viewVertices = viewVertices * camera.viewMatrix();
    // 投影矩阵
    viewVertices = viewVertices * projection.projMatrix();
    // w归一化
    viewVertices.array().colwise() /= viewVertices.col(viewVertices.cols() - 1).array();
    // 视口变换
    viewVertices = viewVertices * viewportMatrix();
    viewMesh.vertices = viewVertices;
    viewMesh.facets = mesh.facets;
    return viewMesh;
  }
  void draw(QPainter& painter) {
    if (this->showAxis) {
      QPen oldpen = painter.pen();
      // x
      painter.setPen(QPen(QColor(255, 0, 0), 2));
      Mesh viewMesh = meshToView(axis[0]);
      viewMesh.draw(painter);
      // y
      painter.setPen(QPen(QColor(0, 255, 0), 2));
      viewMesh = meshToView(axis[1]);
      viewMesh.draw(painter);
      // z
      painter.setPen(QPen(QColor(0, 0, 255), 2));
      viewMesh = meshToView(axis[2]);
      viewMesh.draw(painter);
      // reset pen
      painter.setPen(oldpen);
    }
    for (Mesh& mesh : objs) {
      Mesh viewMesh = meshToView(mesh);
      viewMesh.draw(painter);
    }
  }
};
}  // namespace qtgl