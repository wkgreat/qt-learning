#include <QApplication>
#include "render.hpp"

int main(int argc, char* argv[]) {
  QApplication app(argc, argv);
  QWidget* window = new QWidget;
  window->setWindowTitle("QTGL");
  window->setGeometry(0, 0, 1000, 1000);
  QGridLayout* layout = new QGridLayout;

  qtgl::GLRenderWidget widget;
  widget.setFixedSize(1000, 1000);

  std::string fpath = "E:\\codes\\practice\\qt-learning\\data\\minicooper.obj";
  // std::string fpath = "E:\\codes\\practice\\qt-learning\\data\\teapot.obj";

  qtgl::GLMesh* mesh = qtgl::GLMesh::readFromObjFile(fpath);
  widget.getScene().addObj(mesh);

  widget.getScene().getCamera().lookAt(1000, 1000, 1000, 0, 0, 0);
  widget.getScene().getProjection().mode = qtgl::GLProjectionMode::PRESPECTIVE;
  layout->addWidget(&widget, 0, 0);

  qtgl::SceneHelper helper;
  helper.setScene(&(widget.getScene()));
  layout->addWidget(&helper, 1, 0);

  window->setLayout(layout);
  window->show();

  return app.exec();
}