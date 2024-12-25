#pragma once

#include "camera.hpp"
#include "mesh.hpp"
#include "projection.hpp"
#include "shader.hpp"

namespace qtgl {
class GLScene {
 private:
  double viewHeight;
  double viewWidth;
  GLCamera camera;
  GLProjection projection;
  std::vector<GLObject*> objs;
  bool showAxis;
  std::vector<GLObject*> axis;
  GLShader* shader;
  std::vector<GLLight*> lights;
  Fragments fragments;

  Eigen::Matrix4d transformMatrix;
  Eigen::Matrix4d invTransformMatrix;

 public:
  GLScene(double viewHeight = 768.0, double viewWidth = 1024.0)
      : viewHeight(viewHeight), viewWidth(viewWidth) {
    this->projection.height = viewHeight;
    this->projection.width = viewWidth;
    shader = new LambertianPhongGLShader;
    addAxis();
  }
  ~GLScene() {
    for (GLObject* obj : objs) {
      delete obj;
    }
    for (GLObject* obj : axis) {
      delete obj;
    }
    for (GLLight* lgt : lights) {
      delete lgt;
    }
    delete shader;
  }
  GLCamera& getCamera() { return this->camera; }
  Fragments& getFragments() { return this->fragments; }
  GLProjection& getProjection() { return this->projection; }
  void setViewHeight(double h) {
    this->viewHeight = h;
    this->projection.height = h;
  }
  void setViewWidth(double w) {
    this->viewWidth = w;
    this->projection.width = w;
  }
  void setViewSize(double w, double h) {
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
  void setShader(GLShader* shader) {
    if (this->shader) {
      delete this->shader;
      this->shader = nullptr;
    }
    this->shader = shader;
  }
  GLShader* getShader() { return this->shader; }
  void addLight(GLLight* lgt) { lights.push_back(lgt); }
  std::vector<GLLight*>& getLights() { return this->lights; }
  std::vector<GLObject*>& getObjs() { return this->objs; }
  Eigen::Matrix4d viewportMatrix() {
    double hw = this->viewWidth / 2;
    double hh = this->viewHeight / 2;
    Eigen::Matrix4d viewMtx;
    viewMtx << hw, 0, 0, 0,  //
        0, hh, 0, 0,         //
        0, 0, 1, 0,          //
        hw, hh, 0, 1;        //
    return viewMtx;
  }

  void calculateTransformMatrix() {
    // 视图变换矩阵 * 投影变换矩阵 * 视口变换矩阵
    transformMatrix = camera.viewMatrix() * projection.projMatrix() * viewportMatrix();
    // 逆变换矩阵 用于将屏幕坐标转换为世界坐标
    invTransformMatrix = transformMatrix.inverse();
  }

  // screen coordinator back to world coordinator
  Vertice screenVerticeBackToWorldVertice(double x, double y, double z, double w) {
    Vertice v(x, y, z, w);
    v = v.transpose() * invTransformMatrix;
    v /= v[3];
    return v;
  }

  Vertice screenVerticeBackToWorldVertice(Vertice& v) {
    return screenVerticeBackToWorldVertice(v[0], v[1], v[2], v[3]);
  }

  void meshTransformToScreen(GLObject* obj) {
    // // 渲染 TODO 渲染移动至光栅化期间
    // Vertice cameraPos{camera.getPosX(), camera.getPosY(), camera.getPosZ(), 0};
    // obj->shadeVertices(shader, lights, cameraPos);

    // 计算变换矩阵(视图变换+投影变换+视口变换)及逆矩阵
    calculateTransformMatrix();

    // 准备变换
    obj->prepareTransform();
    // 模型变换
    obj->transformWithModelMatrix();
    // 视图变换 + 投影变换 + 视口变换
    obj->transformWithMatrix(this->transformMatrix);
  }

  Fragments initFragmentsBuffer() {
    Fragments fs(this->viewHeight, std::vector<Fragment>(this->viewWidth, Fragment::init()));
    return fs;
  }

  void draw(QPainter& painter) {
    fragments = initFragmentsBuffer();  // TODO clear rather than init new
    Vertice cameraPos{camera.getPosX(), camera.getPosY(), camera.getPosZ(), 0};
    // if (this->showAxis) {
    //   GLObject* viewObj;
    //   // x
    //   viewObj = meshToView(axis[0]);
    //   viewObj->rasterize(fragments);
    //   delete viewObj;
    //   viewObj = nullptr;
    //   // y
    //   viewObj = meshToView(axis[1]);
    //   viewObj->rasterize(fragments);
    //   delete viewObj;
    //   viewObj = nullptr;
    //   // z
    //   viewObj = meshToView(axis[2]);
    //   viewObj->rasterize(fragments);
    //   delete viewObj;
    //   viewObj = nullptr;
    // }
    for (GLObject* obj : objs) {
      meshTransformToScreen(obj);
      obj->rasterize(fragments);
    }
    for (int h = 0; h < this->viewHeight; ++h) {
      for (int w = 0; w < this->viewWidth; ++w) {
        Fragment fragment = fragments[h][w];
        if (fragment.depth < Fragment::DEPTH_INF) {  // clip
          QPen oldpen = painter.pen();
          painter.setPen(QPen(
              QColor(fragment.color.R * 255, fragment.color.G * 255, fragment.color.B * 255), 1));
          painter.drawPoint(w, h);
          painter.setPen(oldpen);
        }
      }
    }
  }
};
}  // namespace qtgl