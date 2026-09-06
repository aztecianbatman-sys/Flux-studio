#pragma once
#include <QColor>
#include <QPainterPath>
#include <QPointF>
#include <QPolygonF>
#include <QTransform>
#include <QVector>

struct FluxPathNode { QPointF position; QPointF inTangent; QPointF outTangent; bool smooth=false; };
struct FluxVectorStyle { QColor fill=Qt::transparent; QColor stroke=Qt::black; qreal strokeWidth=2.0; bool closed=true; };

class FluxVectorPath final {
public:
    void clear();
    int addNode(const QPointF& p);
    void moveNode(int index,const QPointF& delta);
    void setNode(int index,const QPointF& p);
    void setTangent(int index,const QPointF& in,const QPointF& out);
    void setStyle(const FluxVectorStyle& style){m_style=style;}
    const QVector<FluxPathNode>& nodes() const { return m_nodes; }
    QPainterPath painterPath() const;
    QPolygonF boundsPolygon() const;
    QPainterPath transformed(const QTransform& t) const;
private:
    QVector<FluxPathNode> m_nodes; FluxVectorStyle m_style;
};
