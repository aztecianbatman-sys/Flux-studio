#include "fluxvector.h"

void FluxVectorPath::clear(){m_nodes.clear();}
int FluxVectorPath::addNode(const QPointF&p){m_nodes.push_back({p,QPointF(-20,0),QPointF(20,0),false});return m_nodes.size()-1;}
void FluxVectorPath::moveNode(int i,const QPointF&d){if(i<0||i>=m_nodes.size())return;m_nodes[i].position+=d;}
void FluxVectorPath::setNode(int i,const QPointF&p){if(i<0||i>=m_nodes.size())return;m_nodes[i].position=p;}
void FluxVectorPath::setTangent(int i,const QPointF&in,const QPointF&out){if(i<0||i>=m_nodes.size())return;m_nodes[i].inTangent=in;m_nodes[i].outTangent=out;}
QPainterPath FluxVectorPath::painterPath()const{
    QPainterPath p;if(m_nodes.isEmpty())return p;p.moveTo(m_nodes.first().position);
    for(int i=1;i<m_nodes.size();++i){const auto&a=m_nodes[i-1],b=m_nodes[i];p.cubicTo(a.position+a.outTangent,b.position+b.inTangent,b.position);}if(m_style.closed&&m_nodes.size()>2){const auto&a=m_nodes.last(),b=m_nodes.first();p.cubicTo(a.position+a.outTangent,b.position+b.inTangent,b.position);p.closeSubpath();}return p;
}
QPolygonF FluxVectorPath::boundsPolygon()const{return painterPath().toFillPolygon();}
QPainterPath FluxVectorPath::transformed(const QTransform&t)const{return t.map(painterPath());}
