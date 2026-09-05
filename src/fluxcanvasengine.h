#pragma once

#include <QImage>
#include <QPointF>
#include <QRectF>
#include <QVector>

class QPainter;
class FluxDocument;
class BrushEngine;

class FluxCanvasEngine final {
public:
    static constexpr int TileSize = 256;

    void setDocument(FluxDocument* document);
    FluxDocument* document() const { return m_document; }

    void setZoom(qreal zoom);
    qreal zoom() const { return m_zoom; }
    void setRotation(qreal degrees);
    qreal rotation() const { return m_rotation; }
    void setMirror(bool horizontal, bool vertical = false);
    bool mirrorHorizontal() const { return m_mirrorH; }
    bool mirrorVertical() const { return m_mirrorV; }
    void panBy(const QPointF& delta);
    QPointF pan() const { return m_pan; }
    void resetView();
    void fitToViewport(const QSize& viewport);

    QPointF widgetToCanvas(const QPointF& point, const QSize& viewport) const;
    QPointF canvasToWidget(const QPointF& point, const QSize& viewport) const;
    QRectF visibleCanvasRect(const QSize& viewport) const;

    void draw(QPainter& painter, const QSize& viewport);
    void invalidate();
    void setPixelPerfect(bool enabled) { m_pixelPerfect = enabled; }
    bool pixelPerfect() const { return m_pixelPerfect; }

private:
    struct Tile { QPoint grid; QImage image; bool valid=false; };
    QImage tileFor(const QPoint& grid);
    QImage renderTile(const QPoint& grid) const;

    FluxDocument* m_document{};
    QVector<Tile> m_tiles;
    qreal m_zoom=1.0;
    qreal m_rotation=0.0;
    QPointF m_pan;
    bool m_mirrorH=false;
    bool m_mirrorV=false;
    bool m_pixelPerfect=false;
};
