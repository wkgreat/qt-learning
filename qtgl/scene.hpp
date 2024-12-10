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
  std::vector<GLObject*> objs;
  bool showAxis;
  std::vector<GLObject*> axis;
  std::vector<std::vector<float>> zbuffer;

 public:
  GLScene(float viewHeight = 768.0, float viewWidth = 1024.0)
      : viewHeight(viewHeight), viewWidth(viewWidth) {
    this->projection.height = viewHeight;
    this->projection.width = viewWidth;
    addAxis();
  }
  ~GLScene() {
    for (GLObject* obj : objs) {
      delete obj;
    }
    for (GLObject* obj : axis) {
      delete obj;
    }
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
    GLLine* xAxis = new GLLine;
    GLLine* yAxis = new GLLine;
    GLLine* zAxis = new GLLine;

    xAxis->pushVertice(0, 0, 0);
    xAxis->pushVertice(50, 0, 0);

    yAxis->pushVertice(0, 0, 0);
    yAxis->pushVertice(0, 50, 0);

    zAxis->pushVertice(0, 0, 0);
    zAxis->pushVertice(0, 0, 50);

    this->axis.push_back(xAxis);
    this->axis.push_back(yAxis);
    this->axis.push_back(zAxis);
  }
  void addObj(GLObject* obj) { objs.push_back(obj); }
  std::vector<GLObject*>& getObjs() { return this->objs; }
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

  GLObject* meshToView(GLObject* obj) {
    GLObject* viewObj = obj->clone();
    // View矩阵
    viewObj->vertices = viewObj->vertices * camera.viewMatrix();
    // 投影矩阵
    viewObj->vertices = viewObj->vertices * projection.projMatrix();
    // w归一化
    viewObj->vertices.array().colwise() /=
        viewObj->vertices.col(viewObj->vertices.cols() - 1).array();
    // 视口变换
    viewObj->vertices = viewObj->vertices * viewportMatrix();
    return viewObj;
  }

  Fragments initFragmentsBuffer() {
    Fragments fs(this->viewHeight, std::vector<Fragment>(this->viewWidth, Fragment::init()));
    return fs;
  }

  void draw(QPainter& painter) {
    Fragments fragments = initFragmentsBuffer();
    if (this->showAxis) {
      QPen oldpen = painter.pen();
      GLObject* viewObj;
      // x
      painter.setPen(QPen(QColor(255, 0, 0), 2));
      viewObj = meshToView(axis[0]);
      viewObj->draw(painter);
      delete viewObj;
      viewObj = nullptr;
      // y
      painter.setPen(QPen(QColor(0, 255, 0), 2));
      viewObj = meshToView(axis[1]);
      viewObj->draw(painter);
      delete viewObj;
      viewObj = nullptr;
      // z
      painter.setPen(QPen(QColor(0, 0, 255), 2));
      viewObj = meshToView(axis[2]);
      viewObj->draw(painter);
      delete viewObj;
      viewObj = nullptr;
      // reset pen
      painter.setPen(oldpen);
    }
    for (GLObject* obj : objs) {
      GLObject* viewObj = meshToView(obj);
      viewObj->rasterize(fragments);
      delete viewObj;
    }
    for (int h = 0; h < this->viewHeight; ++h) {
      for (int w = 0; w < this->viewWidth; ++w) {
        Fragment fragment = fragments[h][w];
        if (fragment.depth <= 1) {
          QPen oldpen = painter.pen();
          painter.setPen(QPen(QColor(fragment.color.R, fragment.color.G, fragment.color.B), 1));
          painter.drawPoint(w, h);
          painter.setPen(oldpen);
        }
      }
    }
  }
};
}  // namespace qtgl