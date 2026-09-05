#include "fluxcanvasengine.h"
#include "fluxdocument.h"

#include <QPainter>
#include <QtMath>

void FluxCanvasEngine::setDocument(FluxDocument* document) { if(m_document==document) return; m_document=document; invalidate(); }
void FluxCanvasEngine::setZoom(qreal zoom) { m_zoom=qBound(0.05,zoom,32.0); }
void FluxCanvasEngine::setRotation(qreal degrees) { m_rotation=degrees; }
void FluxCanvasEngine::setMirror(bool horizontal,bool vertical) { m_mirrorH=horizontal; m_mirrorV=vertical; }
void FluxCanvasEngine::panBy(const QPointF& delta) { m_pan += delta; }
void FluxCanvasEngine::resetView(){m_zoom=1.0;m_rotation=0.0;m_pan={0,0};m_mirrorH=false;m_mirrorV=false;}

void FluxCanvasEngine::fitToViewport(const QSize& viewport) {
    if(!m_document || viewport.isEmpty()) return;
    const qreal sx=viewport.width()/qreal(m_document->width());
    const qreal sy=viewport.height()/qreal(m_document->height());
    m_zoom=qMin(sx,sy)*0.9;
    m_pan={0,0};
}

QPointF FluxCanvasEngine::widgetToCanvas(const QPointF& point,const QSize& viewport) const {
    QPointF p=point-QPointF(viewport.width()/2.0,viewport.height()/2.0)-m_pan;
    if(qAbs(m_rotation)>0.001){const qreal a=qDegreesToRadians(-m_rotation),x=p.x()*qCos(a)-p.y()*qSin(a),y=p.x()*qSin(a)+p.y()*qCos(a);p={x,y};}
    if(m_mirrorH)p.rx()=-p.x(); if(m_mirrorV)p.ry()=-p.y();
    return p/m_zoom+QPointF(m_document?m_document->width()/2.0:0,m_document?m_document->height()/2.0:0);
}

QPointF FluxCanvasEngine::canvasToWidget(const QPointF& point,const QSize& viewport) const {
    QPointF p=(point-QPointF(m_document?m_document->width()/2.0:0,m_document?m_document->height()/2.0:0))*m_zoom;
    if(m_mirrorH)p.rx()=-p.x(); if(m_mirrorV)p.ry()=-p.y();
    if(qAbs(m_rotation)>0.001){const qreal a=qDegreesToRadians(m_rotation),x=p.x()*qCos(a)-p.y()*qSin(a),y=p.x()*qSin(a)+p.y()*qCos(a);p={x,y};}
    return p+QPointF(viewport.width()/2.0,viewport.height()/2.0)+m_pan;
}

QRectF FluxCanvasEngine::visibleCanvasRect(const QSize& viewport) const {
    return QRectF(widgetToCanvas({0,0},viewport), widgetToCanvas({viewport.width(),viewport.height()},viewport)).normalized();
}

QImage FluxCanvasEngine::renderTile(const QPoint& grid) const {
    QImage tile(TileSize,TileSize,QImage::Format_ARGB32_Premultiplied); tile.fill(Qt::transparent);
    if(!m_document)return tile;
    const QPoint origin(grid.x()*TileSize,grid.y()*TileSize); const QImage composite=m_document->composite();
    QPainter p(&tile); p.drawImage(-origin,composite); return tile;
}

QImage FluxCanvasEngine::tileFor(const QPoint& grid) {
    for(const auto& t:m_tiles) if(t.grid==grid && t.valid) return t.image;
    Tile tile; tile.grid=grid; tile.image=renderTile(grid); tile.valid=true; m_tiles.push_back(tile);
    return tile.image;
}

void FluxCanvasEngine::invalidate(){for(auto& t:m_tiles)t.valid=false;}

void FluxCanvasEngine::draw(QPainter& painter,const QSize& viewport) {
    painter.fillRect(QRect(QPoint(0,0),viewport),QColor("#101114"));
    if(!m_document)return;
    painter.save(); painter.setRenderHint(QPainter::Antialiasing,!m_pixelPerfect); painter.translate(viewport.width()/2.0,viewport.height()/2.0); painter.translate(m_pan); painter.rotate(m_rotation); painter.scale((m_mirrorH?-1:1)*m_zoom,(m_mirrorV?-1:1)*m_zoom);
    const QPointF center(m_document->width()/2.0,m_document->height()/2.0); painter.translate(-center);
    const int minX=qMax(0,int(m_document->width()?0:0)); Q_UNUSED(minX);
    for(int y=0;y<m_document->height();y+=TileSize) for(int x=0;x<m_document->width();x+=TileSize) {
        const int gx=x/TileSize, gy=y/TileSize; painter.drawImage(QPoint(x,y),tileFor({gx,gy}));
    }
    painter.restore();
}
