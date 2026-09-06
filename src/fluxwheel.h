#pragma once
#include <QWidget>
#include <QPoint>

class QMouseEvent;
class FluxWheel final : public QWidget {
    Q_OBJECT
public:
    explicit FluxWheel(QWidget* parent=nullptr);
public slots:
    void openAt(const QPoint& globalPos);
signals:
    void commandTriggered(int index);
protected:
    void paintEvent(QPaintEvent*) override;
    void mousePressEvent(QMouseEvent*) override;
    void mouseMoveEvent(QMouseEvent*) override;
private:
    QPoint m_center;
    bool m_open=false;
    int m_hover=-1;
};
