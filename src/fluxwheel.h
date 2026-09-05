#pragma once
#include <QWidget>

class FluxWheel final : public QWidget {
    Q_OBJECT
public:
    explicit FluxWheel(QWidget* parent=nullptr);
public slots:
    void openAt(const QPoint& globalPos);
protected:
    void paintEvent(QPaintEvent*) override;
    void mousePressEvent(QMouseEvent*) override;
private:
    QPoint m_center;
    bool m_open=false;
};
