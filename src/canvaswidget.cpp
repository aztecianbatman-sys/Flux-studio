#include "canvaswidget.h"
#include <QMouseEvent>
#include <QPainter>
#include <QWheelEvent>
#include <algorithm>
#include <cmath>

FluxCanvas::FluxCanvas(QWidget* parent) : QWidget(parent) { setAttribute(Qt::WA_OpaquePaintEvent); setFocusPolicy(Qt::StrongFocus); setMouseTracking(true); }

void FluxCanvas::setBrushSize(int px) { m_brushSize = std::clamp(px,1,300); update(); }

void FluxCanvas::paintEvent(QPaintEvent*) {
    QPainter p(this); p.setRenderHint(QPainter::Antialiasing, true); p.fillRect(rect(), QColor("#101114"));
    const int w = width(), h = height();
    QRectF paper(w*0.17, h*0.08, w*0.66, h*0.80);
    p.save(); p.setClipRect(paper);
    const int s=22; for(int y=int(paper.top());y<paper.bottom();y+=s) for(int x=int(paper.left());x<paper.right();x+=s) p.fillRect(x,y,s,s,((x/s+y/s)%2)?QColor("#e3e5e8"):QColor("#f0f1f3"));
    p.setPen(QPen(QColor("#c9ccd1"),1)); p.drawRect(paper);
    p.setPen(Qt::NoPen); p.setBrush(QColor(35,38,44,12)); p.drawRect(paper.adjusted(8,8,8,8));
    p.restore();
    p.setPen(QColor("#757b86")); p.setFont(QFont("Segoe UI",10)); p.drawText(18,28,"Canvas  •  100%  •  2480 × 1600");
    p.setPen(QColor("#a5aab4")); p.drawText(QRectF(0,0,w,h), Qt::AlignCenter, "Start drawing — or right-click to open the Flux Wheel");
}

void FluxCanvas::mousePressEvent(QMouseEvent* e) {
    if(e->button()==Qt::RightButton) { emit wheelRequested(e->globalPosition().toPoint()); return; }
    if(e->button()==Qt::LeftButton) { m_drawing=true; m_lastPoint=e->position().toPoint(); update(); }
}
void FluxCanvas::mouseMoveEvent(QMouseEvent* e) { if(m_drawing) { m_lastPoint=e->position().toPoint(); update(); } }
void FluxCanvas::mouseReleaseEvent(QMouseEvent* e) { if(e->button()==Qt::LeftButton) { m_drawing=false; emit brushSizeChanged(m_brushSize); update(); } }
void FluxCanvas::wheelEvent(QWheelEvent* e) { if(e->modifiers() & Qt::ControlModifier) { m_zoom=std::clamp(m_zoom + e->angleDelta().y()/1200.0,0.1,8.0); update(); e->accept(); return; } QWidget::wheelEvent(e); }
