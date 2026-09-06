#include "fluxwheel.h"
#include <QMouseEvent>
#include <QPainter>
#include <QSettings>
#include <QtMath>

FluxWheel::FluxWheel(QWidget* parent) : QWidget(parent) {
    setAttribute(Qt::WA_TranslucentBackground);
    setMouseTracking(true);
    hide();
    setFixedSize(380,380);
}

void FluxWheel::openAt(const QPoint& globalPos) {
    if (auto* w=window()) {
        const QPoint local=w->mapFromGlobal(globalPos);
        move(local-QPoint(width()/2,height()/2));
    }
    m_center=QPoint(width()/2,height()/2);
    m_open=true;
    show();
    raise();
    setFocus();
    update();
}

void FluxWheel::mousePressEvent(QMouseEvent* e) {
    const QPointF d=e->position()-QPointF(width()/2.0,height()/2.0);
    const double r=std::hypot(d.x(),d.y());
    if(r<62 || r>178 || e->button()==Qt::RightButton){hide();m_open=false;return;}
    const double angle=qAtan2(d.y(),d.x())*180.0/M_PI+90.0;
    const int index=qBound(0,int(qFloor((angle<0?angle+360:angle)/45.0)),7);
    emit commandTriggered(index);
    hide();m_open=false;
}

void FluxWheel::mouseMoveEvent(QMouseEvent* e) {
    if(!m_open)return;
    const QPointF d=e->position()-QPointF(width()/2.0,height()/2.0);
    const double r=std::hypot(d.x(),d.y());
    if(r<62||r>178)m_hover=-1;
    else {const double angle=qAtan2(d.y(),d.x())*180.0/M_PI+90.0;m_hover=qBound(0,int(qFloor(((angle<0?angle+360:angle)/45.0))),7);}
    update();
}

void FluxWheel::paintEvent(QPaintEvent*) {
    if(!m_open)return;
    QSettings s("Flux","Flux Studio");
    const int radius=qBound(128,s.value("wheel/radius",165).toInt(),245);
    QPainter p(this);p.setRenderHint(QPainter::Antialiasing,true);const QPointF c(width()/2.0,height()/2.0);
    p.setPen(QPen(QColor(255,255,255,35),1));p.setBrush(QColor(13,17,22,246));p.drawEllipse(c,radius,radius);
    p.setBrush(QColor(25,31,40,255));p.drawEllipse(c,60,60);
    p.setPen(QColor("#f3f5f8"));p.setFont(QFont("Segoe UI",11,QFont::DemiBold));p.drawText(QRectF(c.x()-62,c.y()-12,124,24),Qt::AlignCenter,"FLUX WHEEL");
    const QStringList items={s.value("wheel/0","Brush").toString(),s.value("wheel/1","Pencil").toString(),s.value("wheel/2","Eraser").toString(),s.value("wheel/3","Ink").toString(),s.value("wheel/4","Pick Color").toString(),s.value("wheel/5","Previous Frame").toString(),s.value("wheel/6","Next Frame").toString(),s.value("wheel/7","Fit Canvas").toString()};
    for(int i=0;i<items.size();++i){const double a=(-90.0+i*45.0)*M_PI/180.0;const QPointF pos=c+QPointF(std::cos(a)*(radius-48),std::sin(a)*(radius-48));p.setBrush(i==m_hover?QColor(66,80,99,255):QColor(31,38,48,255));p.setPen(QPen(QColor(255,255,255,i==m_hover?70:26),1));p.drawEllipse(pos,34,34);p.setPen(QColor("#dde2ea"));p.setFont(QFont("Segoe UI",9,QFont::DemiBold));p.drawText(QRectF(pos.x()-58,pos.y()+39,116,22),Qt::AlignCenter,items[i]);}
}
