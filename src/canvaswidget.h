#pragma once
#include <QColor>
#include <QImage>
#include <QPointF>
#include <QVector>
#include <QOpenGLWidget>

class QMouseEvent; class QWheelEvent; class QTabletEvent; class QResizeEvent; class QPainter;
class FluxDocument; class FluxCanvasEngine; class FluxSelectionEngine; class FluxTransform; class BrushEngine;

class FluxCanvas final : public QOpenGLWidget {
    Q_OBJECT
public:
    explicit FluxCanvas(QWidget* parent=nullptr);
    void setDocument(FluxDocument* document);
    FluxDocument* document() const { return m_document; }
    void setBrushSize(int px); int brushSize() const { return m_brushSize; }
    void setBrushColor(const QColor& color); QColor brushColor() const { return m_brushColor; }
    void setTool(const QString& tool); QString tool() const { return m_tool; }
    void fitCanvas(); void toggleOnionSkin(bool enabled){m_onionSkin=enabled;update();} bool onionSkin() const{return m_onionSkin;}
    void setMirrorHorizontal(bool enabled); void setMirrorVertical(bool enabled); void setCanvasRotation(qreal degrees); void setPixelPerfect(bool enabled); void setStabilization(qreal amount);
    double zoom() const; BrushEngine* brushEngine() const{return m_brush;} 
    void undo(); void redo(); void clearSelection(); void selectAll(); void applySelectionTransform();

signals:
    void wheelRequested(const QPoint& globalPos); void brushSizeChanged(int px); void documentChanged(); void cursorInfoChanged(const QString& text); void strokeStarted(); void selectionChanged(); void zoomChanged(double zoom);
protected:
    void paintEvent(QPaintEvent*) override; void mousePressEvent(QMouseEvent*) override; void mouseMoveEvent(QMouseEvent*) override; void mouseReleaseEvent(QMouseEvent*) override; void wheelEvent(QWheelEvent*) override; void tabletEvent(QTabletEvent*) override; void resizeEvent(QResizeEvent*) override;
private:
    QPointF widgetToCanvas(const QPointF& p) const; QPointF canvasToWidget(const QPointF& p) const; void pushUndoState(); void drawOnion(QPainter& p,const QRectF& target,int frame,qreal opacity); void drawSelectionOverlay(QPainter& p); void drawGuides(QPainter& p);
    void handlePointer(const QPointF& position,qreal pressure,qreal tiltX,qreal tiltY,qreal rotation); void beginSelection(const QPointF& point); void updateSelection(const QPointF& point); void finishSelection();
    FluxDocument* m_document{}; FluxCanvasEngine* m_engine{}; FluxSelectionEngine* m_selection{}; FluxTransform* m_transform{}; BrushEngine* m_brush{};
    QPointF m_lastPoint,m_cursor,m_selectionStart,m_transformStart; QVector<QPointF> m_lasso; bool m_drawing=false,m_selecting=false,m_onionSkin=true,m_grid=true,m_rulers=true,m_transforming=false;
    int m_brushSize=24; qreal m_stabilization=.12; QColor m_brushColor=Qt::black; QString m_tool=QStringLiteral("Brush"); qreal m_pressure=1.0,m_tiltX=0,m_tiltY=0,m_rotationInput=0; QVector<QImage> m_undo,m_redo;
};
