#include <Eigen/Dense>
#include <QApplication>
#include <QGridLayout>
#include <QLabel>
#include <QObject>
#include <QPainter>
#include <QSlider>
#include <QString>
#include <QTimer>
#include <QVBoxLayout>
#include <QWidget>
#include <cmath>
#include <fstream>
#include <iostream>

namespace QTGL {

using Vertice = Eigen::Vector4f;
using Vertices = Eigen::Matrix<float, Eigen::Dynamic, 4>;

struct MathUtils {
  const static float PI;
  inline static float toRadians(float d) { return d * (PI / 180); }
  inline static float toDegree(float r) { return r * (180 / PI); }
};

const float MathUtils::PI = 3.1415926f;

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

/*
Heading : Yaw
Pitch : Pitch
Roll : Roll
*/
class GLCamera {
 private:
  float pos_x = 0;
  float pos_y = 0;
  float pos_z = 0;
  float heading = 0;
  float pitch = 0;
  float roll = 0;

 public:
  GLCamera() : pos_x(0), pos_y(0), pos_z(0), heading(0), pitch(0), roll(0) {}
  GLCamera(float pos_x, float pos_y, float pos_z, float heading, float pitch, float roll)
      : pos_x(pos_x), pos_y(pos_y), pos_z(pos_z), heading(heading), pitch(pitch), roll(roll) {}
  void setPosition(float x, float y, float z) {
    this->pos_x = x;
    this->pos_y = y;
    this->pos_z = z;
  }
  void setPosture(float heading, float pitch, float roll) {
    this->heading = heading;
    this->pitch = pitch;
    this->roll = roll;
  }
  void setPosX(float x) { this->pos_x = x; }
  void setPosY(float y) { this->pos_y = y; }
  void setPosZ(float z) { this->pos_z = z; }
  void setHeading(float heading) { this->heading = heading; }
  void setPitch(float pitch) { this->pitch = pitch; }
  void setRoll(float roll) { this->roll = roll; }
  float getPosX() const { return this->pos_x; }
  float getPosY() const { return this->pos_y; }
  float getPosZ() const { return this->pos_z; }
  float getHeading() const { return this->heading; }
  float getPitch() const { return this->pitch; }
  float getRool() const { return this->roll; }

  void lookAt(float fx, float fy, float fz, float tx, float ty, float tz) {
    // https://stackoverflow.com/a/33790309
    // TODO how to set z axis up
    Eigen::Vector3f d(tx - fx, ty - fy, tz - fz);
    d.normalize();
    this->pitch = asinf(-d[1]);
    this->heading = atan2f(d[0], d[2]);
    this->roll = 0;
    this->setPosition(fx, fy, fz);
  }

  Eigen::Matrix4f viewMatrix() {
    Eigen::Matrix4f eyeMtx = Eigen::Matrix4f::Identity();
    Vertices rotateMtx = static_cast<Vertices>(eyeMtx);
    rotateMtx = AffineUtils::rotate_y(rotateMtx, -heading);
    rotateMtx = AffineUtils::rotate_x(rotateMtx, -pitch);
    rotateMtx = AffineUtils::rotate_z(rotateMtx, -roll);
    Vertices translateMtx = static_cast<Vertices>(eyeMtx);
    translateMtx = AffineUtils::translate(translateMtx, -pos_x, -pos_y, -pos_z);
    Eigen::Matrix4f viewMtx =
        static_cast<Eigen::Matrix4f>(translateMtx) * static_cast<Eigen::Matrix4f>(rotateMtx);
    return viewMtx;
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
    float hfov = MathUtils::PI / 3;
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
class GLRenderWidget : public QWidget {
 private:
  GLScene scene;

 public:
  GLRenderWidget(QWidget* parent = nullptr) : QWidget(parent) {
    QTimer* timer = new QTimer(this);                                  // 创建定时器
    timer->setInterval(20);                                            // 定时间隔单位ms
    connect(timer, &QTimer::timeout, this, &GLRenderWidget::refresh);  // 定时器关联refresh
    timer->start();                                                    // 定时启动
  }

  void setFixedSize(int w, int h) {
    scene.setViewWidth(w);
    scene.setViewHeight(h);
    QWidget::setFixedSize(w, h);
  }

  GLScene& getScene() { return scene; }

  void refresh() { this->update(); }

  void paintEvent(QPaintEvent* event) override {
    QPainter painter(this);
    painter.eraseRect(0, 0, this->width(), this->height());  // 清除画布
    scene.getObjs()[0].rotate_y(MathUtils::toRadians(1));
    scene.draw(painter);
  }
};

class SceneHelper : public QWidget {
 public:
  GLScene* scene;
  QLabel label1;
  QLabel label2;
  QLabel label3;
  QSlider slider1;
  QSlider slider2;
  QSlider slider3;

  QLabel label4;
  QLabel label5;
  QLabel label6;
  QSlider slider4;
  QSlider slider5;
  QSlider slider6;

  QGridLayout layout;
  SceneHelper(QWidget* parent = nullptr) : QWidget(parent) {}
  void setScene(GLScene* scene) {
    this->scene = scene;

    label1.setText(QString("Heading: ") + QString::number(QTGL::MathUtils::toDegree(
                                              this->scene->getCamera().getHeading())));
    label2.setText(QString("Pitch: ") +
                   QString::number(QTGL::MathUtils::toDegree(this->scene->getCamera().getPitch())));
    label3.setText(QString("Roll: ") +
                   QString::number(QTGL::MathUtils::toDegree(this->scene->getCamera().getRool())));
    label4.setText(QString("PosX: ") + QString::number(this->scene->getCamera().getPosX()));
    label5.setText(QString("PosY: ") + QString::number(this->scene->getCamera().getPosY()));
    label6.setText(QString("PosZ: ") + QString::number(this->scene->getCamera().getPosZ()));

    slider1.setOrientation(Qt::Horizontal);
    slider1.setMinimum(-360);
    slider1.setMaximum(360);
    slider1.setSingleStep(1);
    slider1.setValue(QTGL::MathUtils::toDegree(this->scene->getCamera().getHeading()));
    slider1.setTracking(true);
    connect(&slider1, &QSlider::valueChanged, [&](int value) {
      this->scene->getCamera().setHeading(QTGL::MathUtils::toRadians(value));
      label1.setText(QString("Heading: ") + QString::number(QTGL::MathUtils::toDegree(
                                                this->scene->getCamera().getHeading())));
    });

    slider2.setOrientation(Qt::Horizontal);
    slider2.setMinimum(-360);
    slider2.setMaximum(360);
    slider2.setSingleStep(1);
    slider2.setValue(QTGL::MathUtils::toDegree(this->scene->getCamera().getPitch()));
    slider2.setTracking(true);
    connect(&slider2, &QSlider::valueChanged, [&](int value) {
      this->scene->getCamera().setPitch(QTGL::MathUtils::toRadians(value));
      label2.setText(QString("Pitch: ") + QString::number(QTGL::MathUtils::toDegree(
                                              this->scene->getCamera().getPitch())));
    });

    slider3.setOrientation(Qt::Horizontal);
    slider3.setMinimum(-360);
    slider3.setMaximum(360);
    slider3.setSingleStep(1);
    slider3.setValue(QTGL::MathUtils::toDegree(this->scene->getCamera().getRool()));
    slider3.setTracking(true);

    connect(&slider3, &QSlider::valueChanged, [&](int value) {
      this->scene->getCamera().setRoll(QTGL::MathUtils::toRadians(value));
      label3.setText(QString("Roll: ") + QString::number(QTGL::MathUtils::toDegree(
                                             this->scene->getCamera().getRool())));
    });

    slider4.setOrientation(Qt::Horizontal);
    slider4.setMinimum(-600);
    slider4.setMaximum(600);
    slider4.setSingleStep(1);
    slider4.setValue(this->scene->getCamera().getPosX());
    slider4.setTracking(true);

    connect(&slider4, &QSlider::valueChanged, [&](int value) {
      this->scene->getCamera().setPosX(value);
      label4.setText(QString("PosX: ") + QString::number(this->scene->getCamera().getPosX()));
    });

    slider5.setOrientation(Qt::Horizontal);
    slider5.setMinimum(-600);
    slider5.setMaximum(600);
    slider5.setSingleStep(1);
    slider5.setValue(this->scene->getCamera().getPosY());
    slider5.setTracking(true);

    connect(&slider5, &QSlider::valueChanged, [&](int value) {
      this->scene->getCamera().setPosY(value);
      label5.setText(QString("PosY: ") + QString::number(this->scene->getCamera().getPosY()));
    });

    slider6.setOrientation(Qt::Horizontal);
    slider6.setMinimum(-600);
    slider6.setMaximum(600);
    slider6.setSingleStep(1);
    slider6.setValue(this->scene->getCamera().getPosZ());
    slider6.setTracking(true);

    connect(&slider6, &QSlider::valueChanged, [&](int value) {
      this->scene->getCamera().setPosZ(value);
      label6.setText(QString("PosZ: ") + QString::number(this->scene->getCamera().getPosZ()));
    });

    layout.addWidget(&label1, 0, 0);
    layout.addWidget(&slider1, 0, 1);
    layout.addWidget(&label2, 1, 0);
    layout.addWidget(&slider2, 1, 1);
    layout.addWidget(&label3, 2, 0);
    layout.addWidget(&slider3, 2, 1);

    layout.addWidget(&label4, 0, 2);
    layout.addWidget(&slider4, 0, 3);
    layout.addWidget(&label5, 1, 2);
    layout.addWidget(&slider5, 1, 3);
    layout.addWidget(&label6, 2, 2);
    layout.addWidget(&slider6, 2, 3);

    this->setLayout(&layout);
  }
  ~SceneHelper() = default;
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

  // QTGL::Mesh mesh = QTGL::Mesh::makeCube(50);
  // widget.getScene().addObj(mesh);

  std::vector<QTGL::Mesh> objs =
      QTGL::Mesh::readFromObjFile("E:\\codes\\practice\\qt-learning\\data\\teapot.obj");
  for (QTGL::Mesh& mesh : objs) {
    widget.getScene().addObj(mesh);
  }

  widget.getScene().getCamera().lookAt(5, 5, 5, 0, 0, 0);
  widget.getScene().setShowAxis(true);
  layout->addWidget(&widget, 0, 0);

  QTGL::SceneHelper helper;
  helper.setScene(&(widget.getScene()));
  layout->addWidget(&helper, 1, 0);

  window->setLayout(layout);
  window->show();

  return app.exec();
}