#include <Eigen/Dense>
#include <QApplication>
#include <QGridLayout>
#include <QLabel>
#include <QPainter>
#include <QString>
#include <QTimer>
#include <QWidget>
#include <cmath>
#include <iostream>

#define PI 3.1415926
#define TO_RADIANS(d) ((d) * (PI / 180))

namespace QTGL {

using Vertice = Eigen::Vector4f;
using Vertices = Eigen::Matrix<float, Eigen::Dynamic, 4>;

struct AffineUtils {
  static Vertices affine(Vertices& vtx, Eigen::Matrix4f& mtx) { return vtx * mtx; }
  static Vertices translate(Vertices& vtx, float x, float y, float z) {
    Eigen::Matrix4f m;
    m << 1, 0, 0, 0,  //
        0, 1, 0, 0,   //
        0, 0, 1, 0,   //
        x, y, z, 1;
    return affine(vtx, m);
  };
  static Vertices rotate_x(Vertices& vtx, float a) {
    Eigen::Matrix4f m;
    m << 1, 0, 0, 0,            //
        0, cos(a), sin(a), 0,   //
        0, -sin(a), cos(a), 0,  //
        0, 0, 0, 1;
    return affine(vtx, m);
  };
  static Vertices rotate_y(Vertices& vtx, float a) {
    Eigen::Matrix4f m;
    m << cos(a), 0, -sin(a), 0,  //
        0, 1, 0, 0,              //
        sin(a), 0, cos(a), 0,    //
        0, 0, 0, 1;
    return affine(vtx, m);
  };
  static Vertices rotate_z(Vertices& vtx, float a) {
    Eigen::Matrix4f m;
    m << cos(a), sin(a), 0, 0,  //
        -sin(a), cos(a), 0, 0,  //
        0, 0, 1, 0,             //
        0, 0, 0, 1;
    return affine(vtx, m);
  };
  static Vertices scale(Vertices& vtx, float x, float y, float z) {
    Eigen::Matrix4f m;
    m << x, 0, 0, 0,  //
        0, y, 0, 0,   //
        0, 0, z, 0,   //
        0, 0, 0, 1;
    return affine(vtx, m);
  };
};
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
        Vertice p1 = vertices.row(facet[i - 1]);
        Vertice p2 = vertices.row(facet[i]);
        // std::cout << p1[0] << "," << p1[1] << "," << p2[0] << "," << p2[1] << std::endl;
        painter.drawLine(p1[0], p1[1], p2[0], p2[1]);
      }
    }
  }
};
class GLCamera {
 private:
  float pos_x = 0;
  float pos_y = 0;
  float pos_z = 0;
  float heading = 0;
  float pitch = 0;
  float yaw = 0;

 public:
  GLCamera() : pos_x(0), pos_y(0), pos_z(0), heading(0), pitch(0), yaw(0) {}
  GLCamera(float pos_x, float pos_y, float pos_z, float heading, float pitch, float yaw)
      : pos_x(pos_x), pos_y(pos_y), pos_z(pos_z), heading(heading), pitch(pitch), yaw(yaw) {}
  void setPosition(float x, float y, float z) {
    this->pos_x = x;
    this->pos_y = y;
    this->pos_z = z;
  }
  void setPosture(float heading, float pitch, float yaw) {
    this->heading = heading;
    this->pitch = pitch;
    this->yaw = yaw;
  }
  void setPosX(float x) { this->pos_x = x; }
  void setPosY(float y) { this->pos_y = y; }
  void setPosZ(float z) { this->pos_z = z; }
  void setHeading(float heading) { this->heading = heading; }
  void setPitch(float pitch) { this->pitch = pitch; }
  void setYaw(float yaw) { this->yaw = yaw; }
  float getPosX() const { return pos_x; }
  float getPosY() const { return pos_y; }
  float getPosZ() const { return pos_z; }
  float getHeading() const { return heading; }
  float getPitch() const { return pitch; }
  float getYaw() const { return yaw; }

  Eigen::Matrix4f viewMatrix() {
    Eigen::Matrix4f eyeMtx = Eigen::Matrix4f::Identity();
    Vertices rotateMtx = static_cast<Vertices>(eyeMtx);
    rotateMtx = AffineUtils::rotate_y(rotateMtx, -heading);
    rotateMtx = AffineUtils::rotate_x(rotateMtx, -pitch);
    rotateMtx = AffineUtils::rotate_z(rotateMtx, -yaw);
    Vertices translateMtx = static_cast<Vertices>(eyeMtx);
    translateMtx = AffineUtils::translate(translateMtx, -pos_x, -pos_y, -pos_z);
    return static_cast<Eigen::Matrix4f>(translateMtx) * static_cast<Eigen::Matrix4f>(rotateMtx);
  }
};
class GLProjection {
 public:
  float height, width;
  float near, far;
  GLProjection() : height(768), width(1024), near(1), far(100) {}
  GLProjection(float height, float width, float near, float far)
      : height(height), width(width), near(near), far(far) {}
  Eigen::Matrix4f projMatrix() {
    float hfov = PI / 3;
    float vfov = hfov * (height / width);
    float right = tan(hfov / 2) * near;
    float left = -right;
    float top = tan(vfov / 2) * near;
    float bottom = -top;
    float m00 = 2 / (right - left);
    float m11 = 2 / (top - bottom);
    float m22 = (far + near) / (far - near);
    float m32 = -2 * near * far / (far - near);
    Eigen::Matrix4f projMtx;
    projMtx << m00, 0, 0, 0,  //
        0, m11, 0, 0,         //
        0, 0, m22, 1,         //
        0, 0, m32, 0;
    return projMtx;
  }
};
class GLScene {
 private:
  float viewHeight;
  float viewWidth;
  GLCamera camera;
  GLProjection projection;
  std::vector<Mesh> objs;

 public:
  GLScene(float viewHeight = 768.0, float viewWidth = 1024.0)
      : viewHeight(viewHeight), viewWidth(viewWidth) {
    this->projection.height = viewHeight;
    this->projection.width = viewWidth;
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
  void addObj(Mesh& mesh) { objs.push_back(mesh); }
  std::vector<Mesh>& getObjs() { return this->objs; }
  Eigen::Matrix4f viewMatrix() {
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
    viewVertices = viewVertices * camera.viewMatrix();
    viewVertices = viewVertices * projection.projMatrix();
    // 归一化
    viewVertices.array().colwise() /= viewVertices.col(viewVertices.cols() - 1).array();

    viewVertices = viewVertices * viewMatrix();
    viewMesh.vertices = viewVertices;
    viewMesh.facets = mesh.facets;
    return viewMesh;
  }
  void draw(QPainter& painter) {
    for (Mesh& mesh : objs) {
      Mesh viewMesh = meshToView(mesh);
      viewMesh.draw(painter);
    }
  }
};
class GLRenderWidget : public QWidget {
 public:
  GLScene scene;
  GLRenderWidget(QWidget* parent = nullptr) : QWidget(parent) {
    QTimer* timer = new QTimer(this);                                  // 创建定时器
    timer->setInterval(10);                                            // 定时间隔单位ms
    connect(timer, &QTimer::timeout, this, &GLRenderWidget::refresh);  // 定时器关联refresh
    timer->start();                                                    // 定时启动
  }

  void setFixedSize(int w, int h) {
    scene.setViewWidth(w);
    scene.setViewHeight(h);
    QWidget::setFixedSize(w, h);
  }

  void refresh() { this->update(); }

  void paintEvent(QPaintEvent* event) override {
    QPainter painter(this);
    painter.eraseRect(0, 0, this->width(), this->height());  // 清除画布
    scene.getObjs()[0].rotate_z(TO_RADIANS(1));
    scene.draw(painter);
  }
};
}  // namespace QTGL

int main(int argc, char* argv[]) {
  QApplication app(argc, argv);
  QWidget* window = new QWidget;
  window->setWindowTitle("QTGL");
  window->setGeometry(0, 0, 600, 600);
  QGridLayout* layout = new QGridLayout;

  QTGL::GLRenderWidget widget;
  widget.setFixedSize(600, 600);
  QTGL::Mesh mesh = QTGL::Mesh::makeCube(50);
  widget.scene.getCamera().setPosition(0, 0, -300);
  widget.scene.addObj(mesh);
  layout->addWidget(&widget, 0, 0);

  window->setLayout(layout);
  window->show();

  return app.exec();
}