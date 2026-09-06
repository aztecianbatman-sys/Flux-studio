#include "fluxclipboard.h"
#include <QApplication>
#include <QClipboard>
#include <QBuffer>
#include <QByteArray>
#include <QJsonDocument>
#include <QJsonObject>

void FluxClipboard::setImage(const QImage& image,const QString& source){
    if(auto*cb=QApplication::clipboard()){ auto*md=new QMimeData; md->setImageData(image); md->setData("application/x-flux-image",encodeLayerPng(image)); md->setText(source); cb->setMimeData(md); }
}
QImage FluxClipboard::image(){ if(auto*cb=QApplication::clipboard()) return qvariant_cast<QImage>(cb->mimeData()->imageData()); return {}; }
bool FluxClipboard::hasImage(){ return QApplication::clipboard() && QApplication::clipboard()->mimeData()->hasImage(); }
QByteArray FluxClipboard::encodeLayerPng(const QImage& image){ QByteArray b; QBuffer buf(&b); buf.open(QIODevice::WriteOnly); image.save(&buf,"PNG"); return b; }
QImage FluxClipboard::decodeLayerPng(const QByteArray& bytes){ QImage image; image.loadFromData(bytes,"PNG"); return image; }
QString FluxClipboard::makeLayerPayload(const QString&name,const QImage&image){ QJsonObject o; o["format"]="flux-layer-1"; o["name"]=name; o["png"]=QString::fromLatin1(encodeLayerPng(image).toBase64()); return QString::fromUtf8(QJsonDocument(o).toJson(QJsonDocument::Compact)); }
bool FluxClipboard::parseLayerPayload(const QString&payload,QString*name,QImage*image){ QJsonParseError e{}; const auto d=QJsonDocument::fromJson(payload.toUtf8(),&e); if(e.error!=QJsonParseError::NoError||!d.isObject()) return false; const auto o=d.object(); if(o.value("format").toString()!="flux-layer-1") return false; if(name)*name=o.value("name").toString("Pasted Layer"); if(image)*image=decodeLayerPng(QByteArray::fromBase64(o.value("png").toString().toLatin1())); return image? !image->isNull():true; }
