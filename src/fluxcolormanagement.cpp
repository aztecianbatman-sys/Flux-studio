#include "fluxcolormanagement.h"
#include <QJsonDocument>
#include <QJsonObject>

QStringList FluxColorManagement::colorSpaces(){ return {"sRGB","Display P3","Rec.709","Linear sRGB"}; }
QStringList FluxColorManagement::ranges(){ return {"Full","Limited"}; }
FluxExportValidation FluxColorManagement::validate(const FluxColorConfig& c,const QString& format,const QImage& image){
    FluxExportValidation v; if(image.isNull()){v.errors<<"Render image is empty.";return v;}
    if(!colorSpaces().contains(c.colorSpace))v.errors<<"Unsupported color space: "+c.colorSpace;
    if(!ranges().contains(c.range))v.errors<<"Unsupported color range: "+c.range;
    const QString f=format.toLower(); if((f=="jpg"||f=="jpeg")&&image.hasAlphaChannel())v.warnings<<"JPEG cannot preserve alpha; transparent pixels will be flattened.";
    if((f=="gif"||f=="webm")&&c.linearWorkflow)v.warnings<<"Linear workflow is converted to display-referred output for this format.";
    v.valid=v.errors.isEmpty(); return v;
}
QImage FluxColorManagement::convertForExport(const QImage& image,const FluxColorConfig& c){
    if(image.isNull())return {};
    QImage out=image.convertToFormat(c.premultipliedAlpha?QImage::Format_ARGB32_Premultiplied:QImage::Format_RGBA8888);
    if(c.range=="Limited"){out=out.convertToFormat(QImage::Format_ARGB32_Premultiplied);for(int y=0;y<out.height();++y){auto*r=reinterpret_cast<QRgb*>(out.scanLine(y));for(int x=0;x<out.width();++x){QColor q=QColor::fromRgba(r[x]);auto map=[](int v){return qBound(0,int(16.0+v*219.0/255.0+0.5),255);};r[x]=QColor(map(q.red()),map(q.green()),map(q.blue()),q.alpha()).rgba();}}}
    return out;
}
QByteArray FluxColorManagement::metadata(const FluxColorConfig& c){ QJsonObject o; o["colorSpace"]=c.colorSpace; o["range"]=c.range; o["linearWorkflow"]=c.linearWorkflow; o["premultipliedAlpha"]=c.premultipliedAlpha; return QJsonDocument(o).toJson(QJsonDocument::Compact); }
