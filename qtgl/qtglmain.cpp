#include <QApplication>
#include "render.hpp"

int main(int argc, char* argv[]) {
  QApplication app(argc, argv);
  QWidget* window = new QWidget;
  window->setWindowTitle("QTGL");
  window->setGeometry(0, 0, 600, 600);
  QGridLayout* layout = new QGridLayout;

  qtgl::GLRenderWidget widget;
  widget.setFixedSize(600, 600);

  // QTGL::Mesh mesh = QTGL::Mesh::makeCube(50);
  // widget.getScene().addObj(mesh);

  std::vector<qtgl::Mesh> objs =
      qtgl::Mesh::readFromObjFile("E:\\codes\\practice\\qt-learning\\data\\teapot.obj");
  for (qtgl::Mesh& mesh : objs) {
    widget.getScene().addObj(mesh);
  }

  widget.getScene().getCamera().lookAt(50, 50, 50, 0, 0, 0);
  widget.getScene().setShowAxis(true);
  widget.getScene().getProjection().mode = qtgl::GLProjectionMode::PRESPECTIVE;
  layout->addWidget(&widget, 0, 0);

  qtgl::SceneHelper helper;
  helper.setScene(&(widget.getScene()));
  layout->addWidget(&helper, 1, 0);

  window->setLayout(layout);
  window->show();

  return app.exec();
}