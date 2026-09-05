#pragma once
#include <QWidget>

class QMouseEvent;
class QWheelEvent;

class FluxCanvas final : public QWidget {
    Q_OBJECT
public:
    explicit FluxCanvas(QWidget* parent=nullptr);
    void setBrushSize(int px);
    int brushSize() const { return m_brushSize; }

signals:
    void wheelRequested(const QPoint& globalPos);
    void brushSizeChanged(int px);

protected:
    void paintEvent(QPaintEvent*) override;
    void mousePressEvent(QMouseEvent*) override;
    void mouseMoveEvent(QMouseEvent*) override;
    void mouseReleaseEvent(QMouseEvent*) override;
    void wheelEvent(QWheelEvent*) override;

private:
    QPoint m_lastPoint;
    bool m_drawing=false;
    int m_brushSize=24;
    double m_zoom=1.0;
};
