#include "fluxselection.h"

#include <QQueue>
#include <QPainter>
#include <QtMath>

void FluxSelectionEngine::combine(const QRegion& incoming, Mode mode) {
    switch(mode){
    case Mode::Replace: m_region=incoming; break;
    case Mode::Add: m_region+=incoming; break;
    case Mode::Subtract: m_region-=incoming; break;
    case Mode::Intersect: m_region&=incoming; break;
    }
}

void FluxSelectionEngine::clear(){m_region=QRegion();}
void FluxSelectionEngine::rectangle(const QRect& rect,Mode mode){combine(QRegion(rect.normalized()),mode);}
void FluxSelectionEngine::lasso(const QPolygon& polygon,Mode mode){combine(QRegion(polygon),mode);}

void FluxSelectionEngine::contiguous(const QImage& image,const QPoint& seed,int tolerance,Mode mode){
    if(image.isNull()||!image.rect().contains(seed)) return;
    const QColor target=image.pixelColor(seed); const int tol=qBound(0,tolerance,255);
    QImage visited(image.size(),QImage::Format_Grayscale8); visited.fill(0); QRegion found;
    QQueue<QPoint> queue; queue.enqueue(seed); visited.setPixel(seed,255);
    auto closeEnough=[&](const QColor& c){return qAbs(c.red()-target.red())<=tol && qAbs(c.green()-target.green())<=tol && qAbs(c.blue()-target.blue())<=tol && qAbs(c.alpha()-target.alpha())<=tol;};
    while(!queue.isEmpty()){
        const QPoint p=queue.dequeue(); if(!closeEnough(image.pixelColor(p))) continue; found|=QRegion(QRect(p,QSize(1,1)));
        const QPoint n[4]={{p.x()+1,p.y()},{p.x()-1,p.y()},{p.x(),p.y()+1},{p.x(),p.y()-1}};
        for(const QPoint& q:n) if(image.rect().contains(q) && visited.pixel(q.x(),q.y())==0){visited.setPixel(q,255);queue.enqueue(q);}
    }
    combine(found,mode);
}

void FluxSelectionEngine::move(const QPoint& delta){m_region.translate(delta);}

void FluxSelectionEngine::transform(const QTransform& transform,const QSize& canvasSize){
    QRegion out; for(const QRect& r:m_region){QPolygon poly=transform.map(QPolygon(r));out+=QRegion(poly);}
    out &= QRegion(QRect(QPoint(0,0),canvasSize)); m_region=out;
}

QImage FluxSelectionEngine::mask(const QSize& size) const {
    QImage out(size,QImage::Format_Grayscale8); out.fill(0); QPainter p(&out); p.setPen(Qt::NoPen); p.setBrush(Qt::white); for(const QRect& r:m_region)p.drawRect(r); return out;
}

void FluxTransform::reset(const QRectF& bounds){m_bounds=bounds;m_matrix.reset();m_warp.clear();}
void FluxTransform::translate(const QPointF& delta){m_matrix.translate(delta.x(),delta.y());}
void FluxTransform::scale(qreal sx,qreal sy,const QPointF& anchor){m_matrix.translate(anchor.x(),anchor.y());m_matrix.scale(sx,sy);m_matrix.translate(-anchor.x(),-anchor.y());}
void FluxTransform::rotate(qreal degrees,const QPointF& anchor){m_matrix.translate(anchor.x(),anchor.y());m_matrix.rotate(degrees);m_matrix.translate(-anchor.x(),-anchor.y());}
void FluxTransform::shear(qreal sh,qreal sv){m_matrix.shear(sh,sv);}
void FluxTransform::setWarp(const QPolygonF& quad){m_warp=quad;}
QPolygonF FluxTransform::handles() const {
    const QRectF r=m_bounds; QPolygonF points{r.topLeft(),r.topRight(),r.bottomRight(),r.bottomLeft()}; return m_warp.isEmpty()?m_matrix.map(points):m_warp;
}
