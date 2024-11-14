#include <Eigen/Dense>
#include <QApplication>
#include <QPainter>
#include <QTimer>
#include <QWidget>
#include <cmath>

#define PI 3.1415926
#define TO_RADIANS(d) ((d) * (PI / 180))

// 正方形
class SquareMesh {
 public:
  double x0, y0, z0, x1, y1, z1, x2, y2, z2, x3, y3, z3;
  SquareMesh() {
    x0 = 0;
    y0 = 0;
    z0 = 0;
    x1 = 1;
    y1 = 0;
    z1 = 0;
    x2 = 1;
    y2 = 1;
    z2 = 0;
    x3 = 0;
    z3 = 0;
  }
  SquareMesh(double x, double y, double a) {
    x0 = x;
    y0 = y;
    z0 = 0;
    x1 = x + a;
    y1 = y;
    z1 = 0;
    x2 = x + a;
    y2 = y + a;
    z2 = 0;
    x3 = x;
    y3 = y + a;
    z3 = 0;
  }
  static SquareMesh fromXYRadius(double x, double y, double r) {
    SquareMesh s(x - r, y - r, 2 * r);
    return s;
  }
  // 绘制
  void draw(QPainter* painter) const {
    painter->drawLine(x0, y0, x1, y1);
    painter->drawLine(x1, y1, x2, y2);
    painter->drawLine(x2, y2, x3, y3);
    painter->drawLine(x3, y3, x0, y0);
  }

  // 仿射
  void affline(Eigen::Matrix4f& m) {
    Eigen::Vector4f p0(x0, y0, z0, 1);
    Eigen::Vector4f p1(x1, y1, z1, 1);
    Eigen::Vector4f p2(x2, y2, z2, 1);
    Eigen::Vector4f p3(x3, y3, z3, 1);
    Eigen::Vector4f np0 = p0.transpose() * m;
    Eigen::Vector4f np1 = p1.transpose() * m;
    Eigen::Vector4f np2 = p2.transpose() * m;
    Eigen::Vector4f np3 = p3.transpose() * m;
    this->x0 = np0[0];
    this->y0 = np0[1];
    this->z0 = np0[2];
    this->x1 = np1[0];
    this->y1 = np1[1];
    this->z1 = np1[2];
    this->x2 = np2[0];
    this->y2 = np2[1];
    this->z2 = np2[2];
    this->x3 = np3[0];
    this->y3 = np3[1];
    this->z3 = np3[2];
  }

  // 平移
  void translate(double dx, double dy, double dz) {
    Eigen::Matrix4f m;
    m << 1, 0, 0, 0,  //
        0, 1, 0, 0,   //
        0, 0, 1, 0,   //
        dx, dy, dz, 1;
    affline(m);
  }

  // 绕x轴旋转
  void rotate_x(double a) {
    Eigen::Matrix4f m;
    m << 1, 0, 0, 0,            //
        0, cos(a), sin(a), 0,   //
        0, -sin(a), cos(a), 0,  //
        0, 0, 0, 1;
    affline(m);
  }

  // 绕y轴旋转
  void rotate_y(double a) {
    Eigen::Matrix4f m;
    m << cos(a), 0, -sin(a), 0,  //
        0, 1, 0, 0,              //
        sin(a), 0, cos(a), 0,    //
        0, 0, 0, 1;
    affline(m);
  }

  // 绕z轴旋转
  void rotate_z(double a) {
    Eigen::Matrix4f m;
    m << cos(a), sin(a), 0, 0,  //
        -sin(a), cos(a), 0, 0,  //
        0, 0, 1, 0,             //
        0, 0, 0, 1;
    affline(m);
  }

  // 缩放
  void scale(double sx, double sy, double sz) {
    Eigen::Matrix4f m;
    m << sx, 0, 0, 0,  //
        0, sy, 0, 0,   //
        0, 0, sz, 0,   //
        0, 0, 0, 1;
    affline(m);
  }
};

class MyWidget : public QWidget {
 public:
  SquareMesh square = SquareMesh::fromXYRadius(0, 0, 200);

  MyWidget(QWidget* parent = nullptr) : QWidget(parent) {
    this->setGeometry(0, 0, 1000, 1000);

    QTimer* timer = new QTimer(this);                            // 创建定时器
    timer->setInterval(10);                                      // 定时间隔单位ms
    connect(timer, &QTimer::timeout, this, &MyWidget::refresh);  // 定时器关联refresh
    timer->start();                                              // 定时启动
  }

  void refresh() { this->update(); }
  void paintAxis(QPainter* painter) {
    painter->setPen(QPen(QColor(255, 0, 0), 2));
    painter->drawLine(-1000, 0, 1000, 0);
    painter->setPen(QPen(QColor(0, 255, 0), 2));
    painter->drawLine(0, -1000, 0, 1000);
    painter->setPen(nullptr);
  }

  // 绘图函数
  void paintEvent(QPaintEvent* event) override {
    QPainter painter(this);
    painter.eraseRect(0, 0, 1000, 1000);  // 清除画布
    painter.translate(500, 500);          // 原点移动至画布中心
    paintAxis(&painter);                  // 绘制坐标轴
    square.draw(&painter);                // 绘制正方形

    // 正方形仿射变换
    //  square.translate(1, 1, 1);
    //  square.rotate_x(TO_RADIANS(1));
    //  square.rotate_y(TO_RADIANS(1));
    //  square.scale(1.1, 1.1, 1.1);
    square.rotate_z(TO_RADIANS(1));  // 按z轴旋转
  }
};

int main(int argc, char* argv[]) {
  QApplication app(argc, argv);

  MyWidget widget;
  widget.show();

  return app.exec();
}