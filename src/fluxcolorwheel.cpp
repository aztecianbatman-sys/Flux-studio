#include "fluxcolorwheel.h"
#include <QConicalGradient>
#include <QLinearGradient>
#include <QMouseEvent>
#include <QPainter>
#include <QtMath>

namespace { QColor hsvColor(qreal h,qreal s,qreal v){ return QColor::fromHsvF(qBound<qreal>(0,h,1),qBound<qreal>(0,s,1),qBound<qreal>(0,v,1)); } }

FluxColorWheel::FluxColorWheel(QWidget* parent):QWidget(parent){setMinimumSize(190,190);setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Preferred);setMouseTracking(true);}
void FluxColorWheel::setColor(const QColor& color){if(!color.isValid()||color==m_color)return;m_color=color;update();emit colorChanged(m_color);}
void FluxColorWheel::pick(const QPointF& point){const QPointF c(width()/2.0,height()/2.0);const qreal outer=qMin(width(),height())*0.43;const qreal dx=point.x()-c.x(),dy=point.y()-c.y(),r=qSqrt(dx*dx+dy*dy);if(r>outer*0.72&&r<outer*1.06){qreal h=qAtan2(dy,dx)/(2*M_PI)+0.5;if(h<0)h+=1;const auto hsv=m_color.toHsvF();setColor(hsvColor(h,hsv.saturationF(),hsv.valueF()));return;}const qreal side=outer*1.34;const QRectF area(c.x()-side/2,c.y()-side/2,side,side);if(!area.contains(point))return;const qreal x=qBound<qreal>(0,(point.x()-area.left())/area.width(),1),y=qBound<qreal>(0,(point.y()-area.top())/area.height(),1);const auto hsv=m_color.toHsvF();setColor(hsvColor(hsv.hueF()<0?0:hsv.hueF(),x,1-y));}
void FluxColorWheel::mousePressEvent(QMouseEvent*e){m_dragging=true;pick(e->position());e->accept();}
void FluxColorWheel::mouseMoveEvent(QMouseEvent*e){if(m_dragging)pick(e->position());e->accept();}
void FluxColorWheel::mouseReleaseEvent(QMouseEvent*e){m_dragging=false;e->accept();}
void FluxColorWheel::paintEvent(QPaintEvent*){QPainter p(this);p.setRenderHint(QPainter::Antialiasing,true);const QPointF c(width()/2.0,height()/2.0);const qreal outer=qMin(width(),height())*0.43;QConicalGradient ring(c,0);ring.setColorAt(0.00,QColor("#ff3b30"));ring.setColorAt(0.08,QColor("#ff9500"));ring.setColorAt(0.17,QColor("#ffd60a"));ring.setColorAt(0.28,QColor("#34c759"));ring.setColorAt(0.42,QColor("#30d5c8"));ring.setColorAt(0.54,QColor("#0a84ff"));ring.setColorAt(0.67,QColor("#5856d6"));ring.setColorAt(0.79,QColor("#af52de"));ring.setColorAt(0.90,QColor("#ff2d55"));ring.setColorAt(1,QColor("#ff3b30"));p.setPen(Qt::NoPen);p.setBrush(ring);p.drawEllipse(c,outer,outer);p.setBrush(QColor("#10151d"));p.drawEllipse(c,outer*0.72,outer*0.72);const qreal side=outer*1.34;const QRectF area(c.x()-side/2,c.y()-side/2,side,side);QLinearGradient white(area.topLeft(),area.topRight());white.setColorAt(0,Qt::white);white.setColorAt(1,m_color);p.setBrush(white);p.drawRoundedRect(area,16,16);QLinearGradient black(area.topLeft(),area.bottomLeft());black.setColorAt(0,QColor(0,0,0,0));black.setColorAt(1,Qt::black);p.setBrush(black);p.drawRoundedRect(area,16,16);const qreal h=m_color.toHsvF().hueF()<0?0:m_color.toHsvF().hueF(),angle=h*2*M_PI-M_PI/2;const QPointF hp=c+QPointF(qCos(angle),qSin(angle))*outer*0.88;p.setBrush(Qt::NoBrush);p.setPen(QPen(Qt::white,3));p.drawEllipse(hp,6,6);p.setPen(QPen(QColor("#0b1016"),1));p.drawEllipse(hp,8,8);p.setBrush(m_color);p.setPen(QPen(QColor("#edf2f8"),3));p.drawEllipse(c,17,17);}
