#include "fluxwheel.h"
#include <QMouseEvent>
#include <QPainter>
#include <QtMath>

FluxWheel::FluxWheel(QWidget* parent) : QWidget(parent) { setAttribute(Qt::WA_TransparentForMouseEvents, false); hide(); setFixedSize(360,360); }
void FluxWheel::openAt(const QPoint& globalPos) { m_center=mapFromGlobal(globalPos); move(globalPos-QPoint(width()/2,height()/2)); m_open=true; show(); raise(); update(); }
void FluxWheel::mousePressEvent(QMouseEvent* e) { if((e->position()-QPointF(width()/2,height()/2)).manhattanLength()<55 || e->button()==Qt::RightButton) { hide(); m_open=false; } }
void FluxWheel::paintEvent(QPaintEvent*) {
    if(!m_open) return; QPainter p(this); p.setRenderHint(QPainter::Antialiasing,true); QPointF c(width()/2.0,height()/2.0);
    p.setPen(QPen(QColor(255,255,255,26),1)); p.setBrush(QColor(16,18,22,242)); p.drawEllipse(c,158,158);
    p.setBrush(QColor(29,33,40,250)); p.drawEllipse(c,55,55);
    p.setPen(QColor("#f3f4f6")); p.setFont(QFont("Segoe UI",11,QFont::DemiBold)); p.drawText(QRectF(c.x()-46,c.y()-12,92,24),Qt::AlignCenter,"Flux Wheel");
    const QStringList items={"Brush","Pencil","Eraser","Ink","Pick Color","Previous","Next","Canvas"};
    for(int i=0;i<items.size();++i){ double a=(-90.0+i*45.0)*M_PI/180.0; QPointF pos=c+QPointF(std::cos(a)*112,std::sin(a)*112); p.setBrush(QColor(34,38,46,255)); p.setPen(QPen(QColor(255,255,255,22),1)); p.drawEllipse(pos,32,32); p.setPen(QColor("#d9dde5")); p.setFont(QFont("Segoe UI",9)); p.drawText(QRectF(pos.x()-42,pos.y()+40,84,18),Qt::AlignCenter,items[i]); }
    p.setPen(QColor(150,157,170)); p.setFont(QFont("Segoe UI",8)); p.drawText(QRectF(0,260,width(),30),Qt::AlignCenter,"Right-click / hold • customizable");
}
