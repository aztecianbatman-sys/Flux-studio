#pragma once
#include <QColor>
#include <QPoint>
#include <QWidget>

class QMouseEvent;
class QWheelEvent;
class QPainter;
class FluxDocument;

class FluxCanvas final : public QWidget {
    Q_OBJECT
public:
    explicit FluxCanvas(QWidget* parent=nullptr);
    void setDocument(FluxDocument* document);
    FluxDocument* document() const { return m_document; }
    void setBrushSize(int px);
    int brushSize() const { return m_brushSize; }
    void setBrushColor(const QColor& color);
    QColor brushColor() const { return m_brushColor; }
    void setTool(const QString& tool);
    QString tool() const { return m_tool; }
    void fitCanvas();
    double zoom() const { return m_zoom; }

signals:
    void wheelRequested(const QPoint& globalPos);
    void brushSizeChanged(int px);
    void documentChanged();
    void cursorInfoChanged(const QString& text);

protected:
    void paintEvent(QPaintEvent*) override;
    void mousePressEvent(QMouseEvent*) override;
    void mouseMoveEvent(QMouseEvent*) override;
    void mouseReleaseEvent(QMouseEvent*) override;
    void wheelEvent(QWheelEvent*) override;

private:
    QPointF widgetToImage(const QPointF& p) const;
    QRectF canvasRect() const;
    void drawStroke(const QPointF& a, const QPointF& b);
    void drawToolPreview(QPainter& p);

    FluxDocument* m_document{};
    QPointF m_lastPoint;
    QPointF m_cursor;
    bool m_drawing=false;
    int m_brushSize=24;
    QColor m_brushColor=Qt::black;
    QString m_tool=QStringLiteral("Brush");
    double m_zoom=1.0;
};
