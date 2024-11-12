#include <QApplication>
#include <QPainter>
#include <QTimer>
#include <QWidget>
#include <iostream>

class MyWidget : public QWidget {
 public:
  MyWidget(QWidget* parent = nullptr) : QWidget(parent) {
    this->setGeometry(0, 0, 1000, 1000);                         // 设置窗体大小
    QTimer* timer = new QTimer(this);                            // 创建定时器
    timer->setInterval(100);                                     // 定时间隔单位ms
    connect(timer, &QTimer::timeout, this, &MyWidget::refresh);  // 定时器关联refresh
    timer->start();                                              // 定时启动
  }

  int a = 0;

  // 刷新函数
  void refresh() { this->update(); }

  // 绘图函数
  void paintEvent(QPaintEvent* event) override {
    QPainter painter(this);                    // painter
    painter.eraseRect(0, 0, 1000, 1000);       // 清除
    painter.drawLine(0, 0, a % 100, a % 100);  // 画矩形
    a += 10;
  }
};

int main(int argc, char* argv[]) {
  QApplication app(argc, argv);

  MyWidget widget;
  widget.show();

  return app.exec();
}