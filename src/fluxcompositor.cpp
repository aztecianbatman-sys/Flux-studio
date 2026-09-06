#include "fluxcompositor.h"
#include <QColor>
#include <QJsonArray>
#include <QPainter>
#include <QtMath>
#include <algorithm>
#include <functional>

namespace {
QImage blurImage(const QImage&i,int radius){if(radius<=0)return i;QImage s=i;QPainter p(&s);p.setOpacity(0.15);const int step=qMax(1,radius/3);for(int dx=-radius;dx<=radius;dx+=step)for(int dy=-radius;dy<=radius;dy+=step)if(dx||dy)p.drawImage(dx,dy,i);p.end();return s;}
QImage applyColor(QImage out,qreal brightness,qreal contrast,qreal saturation){out=out.convertToFormat(QImage::Format_ARGB32_Premultiplied);for(int y=0;y<out.height();++y){auto*row=reinterpret_cast<QRgb*>(out.scanLine(y));for(int x=0;x<out.width();++x){QColor c=QColor::fromRgba(row[x]);const int r=qBound(0,int((c.red()-128)*contrast+128+brightness),255);const int g=qBound(0,int((c.green()-128)*contrast+128+brightness),255);const int b=qBound(0,int((c.blue()-128)*contrast+128+brightness),255);QColor h(r,g,b,c.alpha());float hh=0.0f,ss=0.0f,ll=0.0f,aa=0.0f;h.getHslF(&hh,&ss,&ll,&aa);ss=static_cast<float>(qBound(0.0,static_cast<qreal>(ss)*saturation,1.0));h.setHslF(hh,ss,ll,aa);row[x]=h.rgba();}}return out;}
}

FluxCompositor::FluxCompositor(){const int image=addNode("IMAGE","Image");const int color=addNode("COLOR","Color");const int blur=addNode("BLUR","Blur");const int glow=addNode("GLOW","Glow");const int transform=addNode("TRANSFORM","Transform");const int output=addNode("OUTPUT","Output");connectNodes(image,color);connectNodes(color,blur);connectNodes(blur,glow);connectNodes(glow,transform);connectNodes(transform,output);}
int FluxCompositor::addNode(const QString&type,const QString&name){FluxCompNode n;n.id=m_nextId++;n.type=type;n.name=name;m_nodes.push_back(n);return n.id;}
void FluxCompositor::removeNode(int id){m_nodes.erase(std::remove_if(m_nodes.begin(),m_nodes.end(),[id](const auto&n){return n.id==id;}),m_nodes.end());for(auto&n:m_nodes)n.inputs.removeAll(id);}
void FluxCompositor::connectNodes(int from,int to){if(auto*n=node(to);n&&from!=to&&!n->inputs.contains(from))n->inputs.push_back(from);}
void FluxCompositor::disconnectNodes(int from,int to){if(auto*n=node(to))n->inputs.removeAll(from);}
FluxCompNode*FluxCompositor::node(int id){for(auto&n:m_nodes)if(n.id==id)return &n;return nullptr;}

QImage FluxCompositor::applyNode(const FluxCompNode&n,const QImage&input)const{
    QImage out=input;
    if(n.type=="BLUR")return blurImage(input,qBound(0,n.params.value("radius",6).toInt(),96));
    if(n.type=="GLOW"){QImage soft=blurImage(input,qBound(1,n.params.value("radius",14).toInt(),96));QPainter p(&out);p.setOpacity(qBound(0.0,n.params.value("intensity",0.55).toDouble(),2.0));p.drawImage(0,0,soft);p.end();return out;}
    if(n.type=="COLOR"||n.type=="ADJUSTMENT")return applyColor(out,n.params.value("brightness",0).toDouble(),n.params.value("contrast",1).toDouble(),n.params.value("saturation",1).toDouble());
    if(n.type=="LEVELS"){const qreal black=qBound(0.0,n.params.value("black",0).toDouble(),1.0);const qreal white=qBound(black+0.0001,n.params.value("white",1).toDouble(),1.0);const qreal gamma=qBound(0.05,n.params.value("gamma",1).toDouble(),8.0);out=out.convertToFormat(QImage::Format_ARGB32_Premultiplied);for(int y=0;y<out.height();++y){auto*row=reinterpret_cast<QRgb*>(out.scanLine(y));for(int x=0;x<out.width();++x){QColor c=QColor::fromRgba(row[x]);auto map=[&](int v){qreal z=v/255.0;z=qBound(0.0,(z-black)/(white-black),1.0);z=qPow(z,1.0/gamma);return qBound(0,int(z*255.0+0.5),255);};row[x]=QColor(map(c.red()),map(c.green()),map(c.blue()),c.alpha()).rgba();}}return out;}
    if(n.type=="HUE_SATURATION"){const qreal hueOffset=n.params.value("hue",0).toDouble();const qreal satMul=n.params.value("saturation",1).toDouble();const qreal light=n.params.value("lightness",0).toDouble();out=out.convertToFormat(QImage::Format_ARGB32_Premultiplied);for(int y=0;y<out.height();++y){auto*row=reinterpret_cast<QRgb*>(out.scanLine(y));for(int x=0;x<out.width();++x){QColor c=QColor::fromRgba(row[x]);float h=0,s=0,l=0,a=0;c.getHslF(&h,&s,&l,&a);h=std::fmod(h+hueOffset/360.0f+1.0f,1.0f);s=static_cast<float>(qBound(0.0,static_cast<qreal>(s)*satMul,1.0));l=static_cast<float>(qBound(0.0,static_cast<qreal>(l)+light,1.0));c.setHslF(h,s,l,a);row[x]=c.rgba();}}return out;}
    if(n.type=="SHADOW"){QImage soft=blurImage(input,qBound(1,n.params.value("radius",8).toInt(),64));QColor color=n.params.value("color",QString("#000000")).value<QColor>();if(!color.isValid())color=Qt::black;QPainter p(&out);p.setOpacity(qBound(0.0,n.params.value("opacity",0.4).toDouble(),1.0));p.drawImage(n.params.value("x",5).toInt(),n.params.value("y",5).toInt(),soft);p.setOpacity(1);p.drawImage(0,0,input);p.end();return out;}
    if(n.type=="BLEND"){return out;}
    if(n.type=="TRANSFORM"){QTransform t;t.translate(n.params.value("x",0).toDouble(),n.params.value("y",0).toDouble());t.rotate(n.params.value("rotation",0).toDouble());t.scale(n.params.value("scaleX",1).toDouble(),n.params.value("scaleY",1).toDouble());return input.transformed(t,Qt::SmoothTransformation);}
    return out;
}
QImage FluxCompositor::applyEffects(QImage image,const QVector<FluxEffect>&effects)const{for(const auto&e:effects)if(e.enabled){FluxCompNode n;n.type=e.type;n.params=e.params;image=applyNode(n,image);}return image;}
QImage FluxCompositor::render(const QImage&source)const{QImage image=source;int outputId=-1;for(const auto&n:m_nodes)if(n.type=="OUTPUT")outputId=n.id;if(outputId<0)return source;QVector<int>order;std::function<void(int)>visit=[&](int id){const auto it=std::find_if(m_nodes.cbegin(),m_nodes.cend(),[&](const auto&n){return n.id==id;});if(it==m_nodes.cend())return;for(const int in:it->inputs)visit(in);if(!order.contains(id))order.push_back(id);};visit(outputId);for(const int id:order){const auto it=std::find_if(m_nodes.cbegin(),m_nodes.cend(),[&](const auto&n){return n.id==id;});if(it!=m_nodes.cend()&&it->type!="IMAGE"&&it->type!="OUTPUT")image=applyNode(*it,image);}return image;}
QJsonObject FluxCompositor::toJson()const{QJsonObject root;QJsonArray a;for(const auto&n:m_nodes){QJsonObject o;o["id"]=n.id;o["name"]=n.name;o["type"]=n.type;o["params"]=QJsonObject::fromVariantMap(n.params);QJsonArray ins;for(int id:n.inputs)ins.append(id);o["inputs"]=ins;a.append(o);}root["nodes"]=a;return root;}
void FluxCompositor::fromJson(const QJsonObject&o){m_nodes.clear();m_nextId=1;for(const auto&v:o["nodes"].toArray()){const auto x=v.toObject();FluxCompNode n;n.id=x["id"].toInt();n.name=x["name"].toString();n.type=x["type"].toString();n.params=x["params"].toObject().toVariantMap();for(const auto&i:x["inputs"].toArray())n.inputs.push_back(i.toInt());m_nodes.push_back(n);m_nextId=qMax(m_nextId,n.id+1);}}
