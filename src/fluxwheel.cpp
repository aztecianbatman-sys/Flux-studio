#include "fluxwheel.h"
#include <QMouseEvent>
#include <QPainter>
#include <QSettings>
#include <QtMath>

FluxWheel::FluxWheel(QWidget* parent) : QWidget(parent) {
    setAttribute(Qt::WA_TransparentForMouseEvents, false);
    setMouseTracking(true); hide(); setFixedSize(360,360);
}
void FluxWheel::openAt(const QPoint& globalPos) { m_center=mapFromGlobal(globalPos); move(globalPos-QPoint(width()/2,height()/2)); m_open=true; show(); raise(); update(); }
void FluxWheel::mousePressEvent(QMouseEvent* e) {
    const QPointF d=e->position()-QPointF(width()/2.0,height()/2.0); const double r=std::hypot(d.x(),d.y());
    if(r<58 || r>170 || e->button()==Qt::RightButton){ hide(); m_open=false; return; }
    const double angle=qAtan2(d.y(),d.x())*180.0/M_PI+90.0; const int index=qBound(0,int(qFloor((angle<0?angle+360:angle)/45.0)),7);
    emit commandTriggered(index); hide(); m_open=false;
}
void FluxWheel::mouseMoveEvent(QMouseEvent* e) {
    if(!m_open)return; const QPointF d=e->position()-QPointF(width()/2.0,height()/2.0); const double r=std::hypot(d.x(),d.y());
    if(r<58||r>170){m_hover=-1;} else {const double angle=qAtan2(d.y(),d.x())*180.0/M_PI+90.0; m_hover=qBound(0,int(qFloor(((angle<0?angle+360:angle)/45.0))),7);} update();
}
void FluxWheel::paintEvent(QPaintEvent*) {
    if(!m_open) return; QSettings s("Flux","Flux Studio"); const int radius=qBound(120,s.value("wheel/radius",158).toInt(),240);
    QPainter p(this); p.setRenderHint(QPainter::Antialiasing,true); QPointF c(width()/2.0,height()/2.0);
    p.setPen(QPen(QColor(255,255,255,28),1)); p.setBrush(QColor(16,18,22,242)); p.drawEllipse(c,radius,radius);
    p.setBrush(QColor(29,33,40,250)); p.drawEllipse(c,55,55);
    p.setPen(QColor("#f3f4f6")); p.setFont(QFont("Segoe UI",11,QFont::DemiBold)); p.drawText(QRectF(c.x()-55,c.y()-12,110,24),Qt::AlignCenter,"Flux Wheel");
    const QStringList items={s.value("wheel/0","Brush").toString(),s.value("wheel/1","Pencil").toString(),s.value("wheel/2","Eraser").toString(),s.value("wheel/3","Ink").toString(),s.value("wheel/4","Pick Color").toString(),s.value("wheel/5","Previous").toString(),s.value("wheel/6","Next").toString(),s.value("wheel/7","Canvas").toString()};
    for(int i=0;i<items.size();++i){ double a=(-90.0+i*45.0)*M_PI/180.0; QPointF pos=c+QPointF(std::cos(a)*(radius-45),std::sin(a)*(radius-45)); p.setBrush(i==m_hover?QColor(66,74,88,255):QColor(34,38,46,255)); p.setPen(QPen(QColor(255,255,255,i==m_hover?52:22),1)); p.drawEllipse(pos,32,32); p.setPen(QColor("#d9dde5")); p.setFont(QFont("Segoe UI",9)); p.drawText(QRectF(pos.x()-48,pos.y()+40,96,18),Qt::AlignCenter,items[i]); }
}
