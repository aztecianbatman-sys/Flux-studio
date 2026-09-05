#pragma once

#include <QImage>
#include <QPointF>
#include <QRectF>
#include <QRegion>
#include <QTransform>

class FluxSelectionEngine final {
public:
    enum class Mode { Replace, Add, Subtract, Intersect };

    void clear();
    bool isEmpty() const { return m_region.isEmpty(); }
    const QRegion& region() const { return m_region; }

    void rectangle(const QRect& rect, Mode mode = Mode::Replace);
    void lasso(const QPolygon& polygon, Mode mode = Mode::Replace);
    void contiguous(const QImage& image, const QPoint& seed, int tolerance = 24, Mode mode = Mode::Replace);
    bool contains(const QPoint& point) const { return m_region.contains(point); }

    QRect bounds() const { return m_region.boundingRect(); }
    void move(const QPoint& delta);
    void transform(const QTransform& transform, const QSize& canvasSize);
    QImage mask(const QSize& size) const;

private:
    void combine(const QRegion& incoming, Mode mode);
    QRegion m_region;
};

class FluxTransform final {
public:
    void reset(const QRectF& bounds);
    void translate(const QPointF& delta);
    void scale(qreal sx, qreal sy, const QPointF& anchor);
    void rotate(qreal degrees, const QPointF& anchor);
    void shear(qreal sh, qreal sv);
    void setWarp(const QPolygonF& quad);

    const QTransform& matrix() const { return m_matrix; }
    QRectF bounds() const { return m_bounds; }
    QPolygonF handles() const;

private:
    QRectF m_bounds;
    QTransform m_matrix;
    QPolygonF m_warp;
};
