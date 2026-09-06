#pragma once
#include "fluxbrush.h"
#include <QColor>
#include <QImage>
#include <QPointF>
#include <QVector>
#include <QOpenGLWidget>

class QMouseEvent; class QWheelEvent; class QTabletEvent; class QResizeEvent; class QPainter;
class FluxDocument; class FluxCanvasEngine; class FluxSelectionEngine; class FluxTransform;

class FluxCanvas final : public QOpenGLWidget {
    Q_OBJECT
public:
    explicit FluxCanvas(QWidget* parent=nullptr);
    void setDocument(FluxDocument* document); FluxDocument* document() const{return m_document;}
    void setBrushSize(int px); int brushSize()const{return m_brushSize;} void setBrushColor(const QColor& color); QColor brushColor()const{return m_brushColor;}
    void setTool(const QString& tool); QString tool()const{return m_tool;} void fitCanvas(); void toggleOnionSkin(bool enabled){m_onionSkin=enabled;update();} bool onionSkin()const{return m_onionSkin;}
    void setMirrorHorizontal(bool enabled); void setMirrorVertical(bool enabled); void setCanvasRotation(qreal degrees); void setPixelPerfect(bool enabled); void setStabilization(qreal amount);
    void setSymmetry(bool horizontal,bool vertical); bool symmetryHorizontal()const{return m_symmetryH;} bool symmetryVertical()const{return m_symmetryV;}
    bool loadReference(const QString& path); void clearReference(); bool hasReference()const{return !m_referenceImage.isNull();}
    void setPerspectiveGuide(bool enabled); void setGridEnabled(bool enabled); void setRulersEnabled(bool enabled);
    double zoom()const; BrushEngine* brushEngine()const{return m_brush;}
    void undo(); void redo(); void clearSelection(); void selectAll(); void applySelectionTransform();

signals:
    void wheelRequested(const QPoint& globalPos); void brushSizeChanged(int px); void documentChanged(); void cursorInfoChanged(const QString& text); void strokeStarted(); void selectionChanged(); void zoomChanged(double zoom); void referenceChanged(); void tabletInfoChanged(const QString& text);
protected:
    void paintEvent(QPaintEvent*)override; void mousePressEvent(QMouseEvent*)override; void mouseMoveEvent(QMouseEvent*)override; void mouseReleaseEvent(QMouseEvent*)override; void wheelEvent(QWheelEvent*)override; void tabletEvent(QTabletEvent*)override; void resizeEvent(QResizeEvent*)override;
private:
    QPointF widgetToCanvas(const QPointF& p)const; QPointF canvasToWidget(const QPointF& p)const; void pushUndoState(); void drawOnion(QPainter& p,const QRectF& target,int frame,qreal opacity); void drawSelectionOverlay(QPainter& p); void drawGuides(QPainter& p);
    void drawReference(QPainter&p,const QRectF&canvasRect); void drawSymmetricStroke(const BrushInput&input); void handlePointer(const QPointF&position,qreal pressure,qreal tiltX,qreal tiltY,qreal rotation); void beginSelection(const QPointF&point); void updateSelection(const QPointF&point); void finishSelection();
    void beginTabletStroke(QTabletEvent*e); void updateTabletStroke(QTabletEvent*e); void finishTabletStroke(QTabletEvent*e); bool tabletUsesPan(QTabletEvent*e)const; bool tabletUsesWheel(QTabletEvent*e)const; QString tabletPointerName(QTabletEvent*e)const;
    bool isShapeTool()const; void drawShape(const QPointF&from,const QPointF&to); void bucketFill(const QPointF&point); void pickColor(const QPointF&point); void insertText(const QPointF&point);
    FluxDocument* m_document{}; FluxCanvasEngine* m_engine{}; FluxSelectionEngine* m_selection{}; FluxTransform* m_transform{}; BrushEngine* m_brush{};
    QPointF m_lastPoint,m_cursor,m_selectionStart,m_transformStart,m_toolStart; QVector<QPointF> m_lasso; bool m_drawing=false,m_selecting=false,m_onionSkin=true,m_grid=false,m_rulers=false,m_perspective=false,m_transforming=false,m_symmetryH=false,m_symmetryV=false;
    int m_brushSize=24; qreal m_stabilization=.12; QColor m_brushColor=Qt::black; QString m_tool=QStringLiteral("Brush"); QString m_preTabletTool; bool m_tabletEraserActive=false,m_tabletPanActive=false,m_tabletWheelActive=false; qreal m_pressure=1.0,m_tiltX=0,m_tiltY=0,m_rotationInput=0; QImage m_referenceImage; QPointF m_referencePosition; QVector<QImage> m_undo,m_redo;
};
