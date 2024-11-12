#include <QApplication>
#include <QPainter>
#include <QTimer>
#include <QWidget>

class MyWidget : public QWidget {
  void paintEvent(QPaintEvent* event) override {
    QPainter painter(this);

    // 画线
    painter.drawLine(0, 0, 100, 100);

    // 画椭圆
    painter.drawEllipse(50, 25, 100, 100);

    // 画矩形
    painter.drawRect(150, 15, 100, 50);
  }
};

int main(int argc, char* argv[]) {
  QApplication app(argc, argv);

  MyWidget widget;
  widget.show();

  return app.exec();
}