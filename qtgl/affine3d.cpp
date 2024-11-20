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

class Mesh {
 public:
  Eigen::Matrix<float, Eigen::Dynamic, 4> vertices;
  std::vector<std::vector<int>> facets;
  Mesh() {}
  void pushVertice(float x, float y, float z) {
    Eigen::Vector4f v(x, y, z, 1);
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

  void draw(QPainter& painter) {
    for (auto& facet : facets) {
      for (int i = 1; i < facet.size(); ++i) {
        Eigen::Vector4f p1 = vertices.row(facet[i - 1]);
        Eigen::Vector4f p2 = vertices.row(facet[i]);
        painter.drawLine(p1[0], p1[1], p2[0], p2[1]);
      }
    }
  }

  void affine(Eigen::Matrix4f& m) { vertices = vertices * m; }

  // 平移
  void translate(double dx, double dy, double dz) {
    Eigen::Matrix4f m;
    m << 1, 0, 0, 0,  //
        0, 1, 0, 0,   //
        0, 0, 1, 0,   //
        dx, dy, dz, 1;
    affine(m);
  }

  // 绕x轴旋转
  void rotate_x(double a) {
    Eigen::Matrix4f m;
    m << 1, 0, 0, 0,            //
        0, cos(a), sin(a), 0,   //
        0, -sin(a), cos(a), 0,  //
        0, 0, 0, 1;
    affine(m);
  }

  // 绕y轴旋转
  void rotate_y(double a) {
    Eigen::Matrix4f m;
    m << cos(a), 0, -sin(a), 0,  //
        0, 1, 0, 0,              //
        sin(a), 0, cos(a), 0,    //
        0, 0, 0, 1;
    affine(m);
  }

  // 绕z轴旋转
  void rotate_z(double a) {
    Eigen::Matrix4f m;
    m << cos(a), sin(a), 0, 0,  //
        -sin(a), cos(a), 0, 0,  //
        0, 0, 1, 0,             //
        0, 0, 0, 1;
    affine(m);
  }

  // 缩放
  void scale(double sx, double sy, double sz) {
    Eigen::Matrix4f m;
    m << sx, 0, 0, 0,  //
        0, sy, 0, 0,   //
        0, 0, sz, 0,   //
        0, 0, 0, 1;
    affine(m);
  }
};

class Affine3DWidget : public QWidget {
 public:
  Mesh mesh = Mesh::makeCube(50);

  Affine3DWidget(QWidget* parent = nullptr) : QWidget(parent) {
    this->setFixedSize(200, 200);
    QTimer* timer = new QTimer(this);                                  // 创建定时器
    timer->setInterval(10);                                            // 定时间隔单位ms
    connect(timer, &QTimer::timeout, this, &Affine3DWidget::refresh);  // 定时器关联refresh
    timer->start();                                                    // 定时启动
  }

  void refresh() { this->update(); }

  void paintEvent(QPaintEvent* event) override {
    QPainter painter(this);
    painter.eraseRect(0, 0, 200, 200);  // 清除画布
    painter.translate(100, 100);        // 原点移动至画布中心
    mesh.draw(painter);
    mesh.rotate_x(TO_RADIANS(1));
    mesh.rotate_y(TO_RADIANS(1));
    mesh.rotate_z(TO_RADIANS(1));
  }
};

int main(int argc, char* argv[]) {
  QApplication app(argc, argv);
  QWidget* window = new QWidget;
  window->setWindowTitle("2D Affine Transformation");
  window->setGeometry(0, 0, 400, 400);
  QGridLayout* layout = new QGridLayout;
  Affine3DWidget widget;
  layout->addWidget(&widget, 0, 0);
  window->setLayout(layout);
  window->show();

  return app.exec();
}