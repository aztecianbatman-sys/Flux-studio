#include "fluxdocument.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QPainter>
#include <QRandomGenerator>

namespace {
QString newLayerId() {
    return QStringLiteral("layer-") + QString::number(QRandomGenerator::global()->generate64(), 16);
}
QJsonObject colorObject(const QColor& c) {
    QJsonObject o; o["r"] = c.red(); o["g"] = c.green(); o["b"] = c.blue(); o["a"] = c.alpha(); return o;
}
QColor colorFromObject(const QJsonObject& o, const QColor& fallback = Qt::transparent) {
    if (o.isEmpty()) return fallback;
    return QColor(o["r"].toInt(fallback.red()), o["g"].toInt(fallback.green()), o["b"].toInt(fallback.blue()), o["a"].toInt(fallback.alpha()));
}
}

FluxDocument::FluxDocument() { create(QStringLiteral("Untitled"), 1920, 1080); }

void FluxDocument::create(const QString& name, int width, int height) {
    m_name = name.isEmpty() ? QStringLiteral("Untitled") : name;
    m_path.clear(); m_width = qMax(1, width); m_height = qMax(1, height);
    m_frame = 0; m_frameCount = 120; m_activeLayer = 0; m_layers.clear();
    addLayer(QStringLiteral("Background"));
    addLayer(QStringLiteral("Paint Layer 1"));
    m_layers[0].image = QImage(m_width, m_height, QImage::Format_ARGB32_Premultiplied); m_layers[0].image.fill(Qt::white);
    m_layers[1].image = QImage(m_width, m_height, QImage::Format_ARGB32_Premultiplied); m_layers[1].image.fill(Qt::transparent);
}

void FluxDocument::ensureFrame(int frame) {
    m_frame = qBound(0, frame, qMax(0, m_frameCount - 1));
    for (auto& layer : m_layers) {
        if (layer.frames.size() != m_frameCount) layer.frames.resize(m_frameCount);
        for (int i = 0; i < m_frameCount; ++i) if (layer.frames[i].isNull()) {
            layer.frames[i] = QImage(m_width, m_height, QImage::Format_ARGB32_Premultiplied); layer.frames[i].fill(Qt::transparent);
        }
        if (!layer.image.isNull() && layer.frames[0].isNull()) layer.frames[0] = layer.image;
    }
}
void FluxDocument::setFrame(int frame) { ensureFrame(frame); }
void FluxDocument::setFrameCount(int count) {
    const int old = m_frameCount; m_frameCount = qBound(1, count, 10000);
    for (auto& layer : m_layers) {
        if (layer.frames.size() < m_frameCount) layer.frames.resize(m_frameCount);
        for (int i = old; i < m_frameCount; ++i) { layer.frames[i] = QImage(m_width, m_height, QImage::Format_ARGB32_Premultiplied); layer.frames[i].fill(Qt::transparent); }
    }
    if (m_frame >= m_frameCount) m_frame = m_frameCount - 1;
}

void FluxDocument::addLayer(const QString& name, FluxLayerType type, int parentIndex) {
    FluxLayer layer; layer.id = newLayerId(); layer.name = name; layer.type = type; layer.parentIndex = parentIndex;
    layer.image = QImage(m_width, m_height, QImage::Format_ARGB32_Premultiplied); layer.image.fill(Qt::transparent);
    if (type == FluxLayerType::Mask || type == FluxLayerType::VectorMask) { layer.mask = QImage(m_width, m_height, QImage::Format_Grayscale8); layer.mask.fill(Qt::white); }
    m_layers.push_back(layer); m_activeLayer = m_layers.size() - 1;
}
void FluxDocument::addGroup(const QString& name, int parentIndex) { addLayer(name, FluxLayerType::Group, parentIndex); }
void FluxDocument::addMask(int layerIndex, bool vectorMask) { if (layerIndex < 0 || layerIndex >= m_layers.size()) return; addLayer(vectorMask ? QStringLiteral("Vector Mask") : QStringLiteral("Layer Mask"), vectorMask ? FluxLayerType::VectorMask : FluxLayerType::Mask, layerIndex); }
void FluxDocument::removeLayer(int index) {
    if (m_layers.size() <= 1 || index < 0 || index >= m_layers.size()) return;
    for (auto& l : m_layers) if (l.parentIndex == index) l.parentIndex = m_layers[index].parentIndex;
    for (auto& l : m_layers) if (l.parentIndex > index) --l.parentIndex;
    m_layers.removeAt(index); m_activeLayer = qBound(0, m_activeLayer, m_layers.size() - 1);
}
void FluxDocument::duplicateLayer(int index) {
    if (index < 0 || index >= m_layers.size()) return;
    FluxLayer copy = m_layers[index]; copy.id = newLayerId(); copy.name += QStringLiteral(" Copy");
    m_layers.insert(index + 1, copy);
    for (int i = 0; i < m_layers.size(); ++i) if (i != index + 1 && m_layers[i].parentIndex > index) ++m_layers[i].parentIndex;
    if (copy.parentIndex > index) ++copy.parentIndex;
    m_activeLayer = index + 1;
}
void FluxDocument::mergeDown(int index) {
    if (index <= 0 || index >= m_layers.size()) return;
    FluxLayer& top = m_layers[index]; FluxLayer& below = m_layers[index - 1];
    if (top.type != FluxLayerType::Paint || below.type != FluxLayerType::Paint) return;
    QImage merged = layerFrame(below); QPainter p(&merged); p.setOpacity(top.opacity); p.drawImage(0, 0, layerFrame(top)); p.end();
    below.frames.clear(); below.image = merged; below.opacity = 1.0; removeLayer(index);
}
void FluxDocument::flattenVisible() {
    QImage merged = composite(); m_layers.clear(); FluxLayer layer; layer.id = newLayerId(); layer.name = QStringLiteral("Flattened"); layer.image = merged; m_layers.push_back(layer); m_activeLayer = 0;
}
void FluxDocument::moveLayer(int from, int to) {
    if (from < 0 || from >= m_layers.size() || to < 0 || to >= m_layers.size() || from == to) return;
    FluxLayer item = m_layers.takeAt(from); m_layers.insert(to, item); for (auto& l : m_layers) { if (l.parentIndex == from) l.parentIndex = to; else if (from < to && l.parentIndex > from && l.parentIndex <= to) --l.parentIndex; else if (to < from && l.parentIndex >= to && l.parentIndex < from) ++l.parentIndex; } m_activeLayer = to;
}
void FluxDocument::setActiveLayer(int index) { m_activeLayer = qBound(0, index, m_layers.size() - 1); }
void FluxDocument::setLayerOpacity(int index, qreal opacity) { if (index >= 0 && index < m_layers.size()) m_layers[index].opacity = qBound(0.0, opacity, 1.0); }
void FluxDocument::setLayerBlendMode(int index, FluxBlendMode mode) { if (index >= 0 && index < m_layers.size()) m_layers[index].blendMode = mode; }
void FluxDocument::setLayerVisible(int index, bool visible) { if (index >= 0 && index < m_layers.size()) m_layers[index].visible = visible; }
void FluxDocument::setLayerLocked(int index, bool locked) { if (index >= 0 && index < m_layers.size()) m_layers[index].locked = locked; }
void FluxDocument::setLayerLabelColor(int index, const QColor& color) { if (index >= 0 && index < m_layers.size()) m_layers[index].labelColor = color; }
void FluxDocument::setLayerClipping(int index, bool enabled) { if (index >= 0 && index < m_layers.size()) m_layers[index].clipping = enabled; }
void FluxDocument::setLayerAlphaInherited(int index, bool enabled) { if (index >= 0 && index < m_layers.size()) m_layers[index].alphaInherited = enabled; }
void FluxDocument::setLayerStyle(int index, const FluxLayerStyle& style) { if (index >= 0 && index < m_layers.size()) m_layers[index].style = style; }
void FluxDocument::setLayerAdjustment(int index, const QVariantMap& adjustment) { if (index >= 0 && index < m_layers.size()) { m_layers[index].type = FluxLayerType::Adjustment; m_layers[index].adjustment = adjustment; } }
void FluxDocument::setSolo(int index, bool enabled) { if (index >= 0 && index < m_layers.size()) m_layers[index].solo = enabled; }
void FluxDocument::setIsolate(int index, bool enabled) { if (index >= 0 && index < m_layers.size()) m_layers[index].isolate = enabled; }
bool FluxDocument::hasSolo() const { for (const auto& l : m_layers) if (l.solo) return true; return false; }
bool FluxDocument::hasIsolate() const { for (const auto& l : m_layers) if (l.isolate) return true; return false; }

QImage& FluxDocument::activeImage() { auto& l = activeLayer(); if (l.frames.size() == m_frameCount) return l.frames[m_frame]; return l.image; }
const QImage& FluxDocument::activeImage() const { const auto& l = activeLayer(); return l.frames.size() == m_frameCount ? l.frames[m_frame] : l.image; }
FluxLayer& FluxDocument::activeLayer() { return m_layers[m_activeLayer]; }
const FluxLayer& FluxDocument::activeLayer() const { return m_layers[m_activeLayer]; }
QImage FluxDocument::layerFrame(const FluxLayer& layer) const { if (layer.frames.size() == m_frameCount && !layer.frames[m_frame].isNull()) return layer.frames[m_frame]; return layer.image; }

bool FluxDocument::layerVisibleForComposite(int index) const {
    const auto& l = m_layers[index];
    if (!l.visible || l.opacity <= 0.0) return false;
    if (hasSolo() && !l.solo) return false;
    if (hasIsolate() && !l.isolate) return false;
    return true;
}
void FluxDocument::applyLayerStyle(QPainter& painter, const FluxLayer& layer, const QImage& image) const {
    if (!layer.style.enabled) { painter.drawImage(0, 0, image); return; }
    if (layer.style.shadowOpacity > 0.0) { painter.save(); painter.setOpacity(layer.style.shadowOpacity); painter.drawImage(layer.style.shadowX, layer.style.shadowY, image); painter.restore(); }
    painter.drawImage(0, 0, image);
    if (layer.style.outlineOpacity > 0.0 && layer.style.outlineWidth > 0.0) { painter.save(); painter.setOpacity(layer.style.outlineOpacity); painter.setPen(QPen(layer.style.outlineColor, layer.style.outlineWidth)); painter.drawRect(image.rect().adjusted(1,1,-2,-2)); painter.restore(); }
}

QImage FluxDocument::composite() const {
    QImage out(m_width, m_height, QImage::Format_ARGB32_Premultiplied); out.fill(Qt::transparent); QPainter p(&out);
    QImage previousAlpha;
    for (int i = 0; i < m_layers.size(); ++i) {
        const auto& layer = m_layers[i]; if (!layerVisibleForComposite(i) || layer.type == FluxLayerType::Group || layer.type == FluxLayerType::Mask || layer.type == FluxLayerType::VectorMask) continue;
        QImage image = layerFrame(layer); if (image.isNull()) continue;
        if (!layer.adjustment.isEmpty() && layer.type == FluxLayerType::Adjustment) { image = image.convertToFormat(QImage::Format_ARGB32_Premultiplied); }
        if (layer.alphaInherited && !previousAlpha.isNull()) {
            QPainter maskPainter(&image); maskPainter.setCompositionMode(QPainter::CompositionMode_DestinationIn); maskPainter.drawImage(0,0,previousAlpha); maskPainter.end();
        }
        p.save(); p.setOpacity(layer.opacity);
        switch(layer.blendMode) {
        case FluxBlendMode::Multiply: p.setCompositionMode(QPainter::CompositionMode_Multiply); break;
        case FluxBlendMode::Screen: p.setCompositionMode(QPainter::CompositionMode_Screen); break;
        case FluxBlendMode::Overlay: p.setCompositionMode(QPainter::CompositionMode_Overlay); break;
        case FluxBlendMode::Add: p.setCompositionMode(QPainter::CompositionMode_Plus); break;
        case FluxBlendMode::Subtract: p.setCompositionMode(QPainter::CompositionMode_Difference); break;
        default: p.setCompositionMode(QPainter::CompositionMode_SourceOver); break;
        }
        if (layer.clipping && !previousAlpha.isNull()) { QImage clipped = image; QPainter cp(&clipped); cp.setCompositionMode(QPainter::CompositionMode_DestinationIn); cp.drawImage(0,0,previousAlpha); cp.end(); applyLayerStyle(p, layer, clipped); }
        else applyLayerStyle(p, layer, image);
        p.restore(); previousAlpha = image.alphaChannel();
    }
    return out;
}

QString FluxDocument::blendModeName(FluxBlendMode mode) { switch(mode){case FluxBlendMode::Multiply:return "Multiply";case FluxBlendMode::Screen:return "Screen";case FluxBlendMode::Overlay:return "Overlay";case FluxBlendMode::Add:return "Add";case FluxBlendMode::Subtract:return "Subtract";default:return "Normal";} }
FluxBlendMode FluxDocument::blendModeFromName(const QString& name) { if(name=="Multiply")return FluxBlendMode::Multiply;if(name=="Screen")return FluxBlendMode::Screen;if(name=="Overlay")return FluxBlendMode::Overlay;if(name=="Add")return FluxBlendMode::Add;if(name=="Subtract")return FluxBlendMode::Subtract;return FluxBlendMode::Normal; }
QString FluxDocument::layerTypeName(FluxLayerType type) { switch(type){case FluxLayerType::Group:return "Group";case FluxLayerType::Mask:return "Mask";case FluxLayerType::VectorMask:return "Vector Mask";case FluxLayerType::Adjustment:return "Adjustment";default:return "Paint";} }

bool FluxDocument::save(const QString& filePath, QString* error) const {
    QFileInfo info(filePath); if (!info.dir().exists() && !QDir().mkpath(info.dir().absolutePath())) { if(error)*error="Could not create project directory"; return false; }
    const QString assetDir = info.absolutePath()+QDir::separator()+info.completeBaseName()+QStringLiteral(".fluxdata"); if(!QDir().mkpath(assetDir)){if(error)*error="Could not create data directory";return false;}
    QJsonObject root; root["version"]=3; root["name"]=m_name; root["width"]=m_width; root["height"]=m_height; root["frameCount"]=m_frameCount; root["activeFrame"]=m_frame; root["activeLayer"]=m_activeLayer; root["foreground"]=colorObject(m_foreground);
    QJsonArray layers;
    for(int i=0;i<m_layers.size();++i){const auto&l=m_layers[i];QJsonObject o;o["id"]=l.id;o["name"]=l.name;o["type"]=FluxDocument::layerTypeName(l.type);o["parent"]=l.parentIndex;o["expanded"]=l.expanded;o["visible"]=l.visible;o["locked"]=l.locked;o["alphaInherited"]=l.alphaInherited;o["clipping"]=l.clipping;o["solo"]=l.solo;o["isolate"]=l.isolate;o["opacity"]=l.opacity;o["labelColor"]=colorObject(l.labelColor);o["blendMode"]=blendModeName(l.blendMode);o["adjustment"]=QJsonObject::fromVariantMap(l.adjustment);QJsonObject st;st["enabled"]=l.style.enabled;st["shadowOpacity"]=l.style.shadowOpacity;st["shadowRadius"]=l.style.shadowRadius;st["shadowX"]=l.style.shadowX;st["shadowY"]=l.style.shadowY;st["outlineOpacity"]=l.style.outlineOpacity;st["outlineWidth"]=l.style.outlineWidth;st["outlineColor"]=colorObject(l.style.outlineColor);o["style"]=st;
        QJsonArray frames;for(int f=0;f<m_frameCount;++f){QImage image;if(l.frames.size()==m_frameCount)image=l.frames[f];else if(f==0)image=l.image;if(image.isNull()){image=QImage(m_width,m_height,QImage::Format_ARGB32_Premultiplied);image.fill(Qt::transparent);}QString fn=QString("layer_%1_f%2.png").arg(i,4,10,QChar('0')).arg(f,6,10,QChar('0'));if(!image.save(assetDir+QDir::separator()+fn,"PNG")){if(error)*error="Failed to write layer frame";return false;}frames.append(QFileInfo(assetDir).fileName()+QStringLiteral("/")+fn);}o["frames"]=frames;layers.append(o);}
    root["layers"]=layers;QFile file(filePath);if(!file.open(QIODevice::WriteOnly|QIODevice::Truncate)){if(error)*error=file.errorString();return false;}file.write(QJsonDocument(root).toJson(QJsonDocument::Indented));return true;
}

bool FluxDocument::load(const QString&filePath, QString* error) {
    QFile file(filePath);if(!file.open(QIODevice::ReadOnly)){if(error)*error=file.errorString();return false;}QJsonParseError parse{};auto doc=QJsonDocument::fromJson(file.readAll(),&parse);if(doc.isNull()){if(error)*error=parse.errorString();return false;}auto root=doc.object();m_name=root["name"].toString("Untitled");m_width=root["width"].toInt(1920);m_height=root["height"].toInt(1080);m_frameCount=qBound(1,root["frameCount"].toInt(120),10000);m_frame=qBound(0,root["activeFrame"].toInt(0),m_frameCount-1);m_foreground=colorFromObject(root["foreground"].toObject(),Qt::black);m_layers.clear();QDir base=QFileInfo(filePath).absoluteDir();
    for(const auto value:root["layers"].toArray()){auto o=value.toObject();FluxLayer l;l.id=o["id"].toString(newLayerId());l.name=o["name"].toString("Layer");QString type=o["type"].toString("Paint");l.type= type=="Group"?FluxLayerType::Group:type=="Mask"?FluxLayerType::Mask:type=="Vector Mask"?FluxLayerType::VectorMask:type=="Adjustment"?FluxLayerType::Adjustment:FluxLayerType::Paint;l.parentIndex=o["parent"].toInt(-1);l.expanded=o["expanded"].toBool(true);l.visible=o["visible"].toBool(true);l.locked=o["locked"].toBool(false);l.alphaInherited=o["alphaInherited"].toBool(false);l.clipping=o["clipping"].toBool(false);l.solo=o["solo"].toBool(false);l.isolate=o["isolate"].toBool(false);l.opacity=o["opacity"].toDouble(1.0);l.labelColor=colorFromObject(o["labelColor"].toObject());l.blendMode=blendModeFromName(o["blendMode"].toString("Normal"));l.adjustment=o["adjustment"].toObject().toVariantMap();auto st=o["style"].toObject();l.style.enabled=st["enabled"].toBool(false);l.style.shadowOpacity=st["shadowOpacity"].toDouble(0);l.style.shadowRadius=st["shadowRadius"].toDouble(0);l.style.shadowX=st["shadowX"].toDouble(0);l.style.shadowY=st["shadowY"].toDouble(0);l.style.outlineOpacity=st["outlineOpacity"].toDouble(0);l.style.outlineWidth=st["outlineWidth"].toDouble(0);l.style.outlineColor=colorFromObject(st["outlineColor"].toObject(),Qt::black);
        auto fa=o["frames"].toArray();l.frames.resize(m_frameCount);for(int f=0;f<m_frameCount&&f<fa.size();++f){l.frames[f].load(base.absoluteFilePath(fa[f].toString()));if(l.frames[f].isNull()){l.frames[f]=QImage(m_width,m_height,QImage::Format_ARGB32_Premultiplied);l.frames[f].fill(Qt::transparent);}}if(!l.frames.isEmpty())l.image=l.frames[0];m_layers.push_back(l);}
    if(m_layers.isEmpty())addLayer("Paint Layer");m_activeLayer=qBound(0,root["activeLayer"].toInt(0),m_layers.size()-1);m_path=filePath;return true;
}
bool FluxDocument::exportImage(const QString&filePath,QString*error)const{if(!composite().save(filePath)){if(error)*error="Export failed";return false;}return true;}
