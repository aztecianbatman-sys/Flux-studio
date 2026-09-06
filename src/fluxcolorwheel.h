#pragma once
#include <QColor>
#include <QWidget>

class FluxColorWheel final : public QWidget {
    Q_OBJECT
public:
    explicit FluxColorWheel(QWidget* parent=nullptr);
    QColor color() const { return m_color; }
    void setColor(const QColor& color);
signals:
    void colorChanged(const QColor& color);
protected:
    void paintEvent(QPaintEvent*) override;
    void mousePressEvent(QMouseEvent*) override;
    void mouseMoveEvent(QMouseEvent*) override;
    void mouseReleaseEvent(QMouseEvent*) override;
private:
    void pick(const QPointF& point);
    QColor m_color{42, 128, 255};
    bool m_dragging=false;
};
