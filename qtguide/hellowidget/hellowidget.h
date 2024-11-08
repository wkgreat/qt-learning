#include <QtWidgets/QLabel>
#include <QtWidgets/QWidget>

class HelloWidget : public QWidget {
  Q_OBJECT
 public:
  explicit HelloWidget(QWidget* parent = 0);
  ~HelloWidget();
  // label
  QLabel* m_labelInfo;
};