#include "fluxdocument.h"
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QPainter>

FluxDocument::FluxDocument() { create(QStringLiteral("Untitled"), 1920, 1080); }

void FluxDocument::create(const QString& name, int width, int height) {
    m_name = name.isEmpty() ? QStringLiteral("Untitled") : name;
    m_path.clear(); m_width = qMax(1, width); m_height = qMax(1, height);
    m_frame = 0; m_frameCount = 120; m_activeLayer = 0; m_layers.clear();
    addLayer(QStringLiteral("Background"));
    addLayer(QStringLiteral("Paint Layer 1"));
    m_layers[0].image.fill(Qt::white);
    m_layers[1].image.fill(Qt::transparent);
}

void FluxDocument::ensureFrame(int frame) {
    frame = qBound(0, frame, qMax(0, m_frameCount - 1));
    for (auto& layer : m_layers) {
        if (layer.frames.size() < m_frameCount) layer.frames.resize(m_frameCount);
        for (int i = 0; i < layer.frames.size(); ++i) {
            if (layer.frames[i].isNull()) layer.frames[i] = i == 0 ? layer.image : QImage(m_width, m_height, QImage::Format_ARGB32_Premultiplied);
            if (i > 0 && layer.frames[i].isNull()) layer.frames[i].fill(Qt::transparent);
        }
    }
    m_frame = frame;
}

void FluxDocument::setFrame(int frame) {
    m_frame = qBound(0, frame, qMax(0, m_frameCount - 1));
    for (auto& layer : m_layers) {
        if (layer.frames.size() < m_frameCount) layer.frames.resize(m_frameCount);
        if (layer.frames[m_frame].isNull()) {
            layer.frames[m_frame] = QImage(m_width, m_height, QImage::Format_ARGB32_Premultiplied);
            layer.frames[m_frame].fill(Qt::transparent);
        }
    }
}

void FluxDocument::setFrameCount(int count) { m_frameCount = qBound(1, count, 10000); if (m_frame >= m_frameCount) m_frame = m_frameCount - 1; }

void FluxDocument::addLayer(const QString& name) {
    FluxLayer layer; layer.name = name; layer.image = QImage(m_width, m_height, QImage::Format_ARGB32_Premultiplied); layer.image.fill(Qt::transparent); m_layers.push_back(layer); m_activeLayer = m_layers.size() - 1;
}

void FluxDocument::removeLayer(int index) {
    if (m_layers.size() <= 1 || index < 0 || index >= m_layers.size()) return;
    m_layers.removeAt(index); m_activeLayer = qBound(0, m_activeLayer, m_layers.size() - 1);
}

void FluxDocument::duplicateLayer(int index) {
    if (index < 0 || index >= m_layers.size()) return;
    FluxLayer copy = m_layers[index]; copy.name += QStringLiteral(" Copy"); m_layers.insert(index + 1, copy); m_activeLayer = index + 1;
}

void FluxDocument::setActiveLayer(int index) { m_activeLayer = qBound(0, index, m_layers.size() - 1); }

QImage& FluxDocument::activeImage() {
    FluxLayer& layer = activeLayer();
    if (layer.frames.size() >= m_frameCount) return layer.frames[m_frame];
    return layer.image;
}
const QImage& FluxDocument::activeImage() const { return activeLayer().frames.size() >= m_frameCount ? activeLayer().frames[m_frame] : activeLayer().image; }
FluxLayer& FluxDocument::activeLayer() { return m_layers[m_activeLayer]; }
const FluxLayer& FluxDocument::activeLayer() const { return m_layers[m_activeLayer]; }

QImage FluxDocument::composite() const {
    QImage out(m_width, m_height, QImage::Format_ARGB32_Premultiplied); out.fill(Qt::transparent); QPainter p(&out);
    for (const auto& layer : m_layers) {
        if (!layer.visible || layer.opacity <= 0.0) continue;
        p.setOpacity(layer.opacity); const QImage& image = layer.frames.size() >= m_frameCount ? layer.frames[m_frame] : layer.image; p.drawImage(0, 0, image);
    }
    return out;
}

bool FluxDocument::save(const QString& filePath, QString* error) const {
    QFileInfo info(filePath); if (!info.dir().exists() && !QDir().mkpath(info.dir().absolutePath())) { if(error)*error=QStringLiteral("Could not create project directory"); return false; }
    const QString assetDir = info.absolutePath() + QDir::separator() + info.completeBaseName() + QStringLiteral(".fluxdata"); QDir().mkpath(assetDir);
    QJsonObject root; root["version"] = 1; root["name"] = m_name; root["width"] = m_width; root["height"] = m_height; root["fps"] = 24; root["frameCount"] = m_frameCount; root["activeLayer"] = m_activeLayer; root["activeFrame"] = m_frame;
    QJsonArray layers;
    for (int i=0;i<m_layers.size();++i) {
        const auto& layer=m_layers[i]; QJsonObject obj; obj["name"]=layer.name; obj["visible"]=layer.visible; obj["locked"]=layer.locked; obj["opacity"]=layer.opacity;
        const QString imageName=QString("layer_%1_f%2.png").arg(i,4,10,QChar('0')).arg(m_frame,6,10,QChar('0')); const QString rel=QFileInfo(assetDir).fileName()+QStringLiteral("/")+imageName;
        const QImage image=layer.frames.size() >= m_frameCount ? layer.frames[m_frame] : layer.image; if(!image.save(assetDir+QDir::separator()+imageName,"PNG")) { if(error)*error=QStringLiteral("Failed to write layer image"); return false; } obj["image"]=rel; layers.append(obj);
    }
    root["layers"]=layers; QFile file(filePath); if(!file.open(QIODevice::WriteOnly|QIODevice::Truncate)){ if(error)*error=file.errorString(); return false; }
    file.write(QJsonDocument(root).toJson(QJsonDocument::Indented)); m_path=filePath; return true;
}

bool FluxDocument::load(const QString& filePath, QString* error) {
    QFile file(filePath); if(!file.open(QIODevice::ReadOnly)){if(error)*error=file.errorString();return false;} QJsonParseError parse{}; const auto doc=QJsonDocument::fromJson(file.readAll(),&parse); if(doc.isNull()){if(error)*error=parse.errorString();return false;}
    const auto root=doc.object(); m_name=root["name"].toString(QStringLiteral("Untitled")); m_width=root["width"].toInt(1920); m_height=root["height"].toInt(1080); m_frameCount=root["frameCount"].toInt(120); m_frame=root["activeFrame"].toInt(0); m_activeLayer=0; m_layers.clear();
    const QDir base=QFileInfo(filePath).absoluteDir(); for(const auto value: root["layers"].toArray()){ const auto obj=value.toObject(); FluxLayer layer; layer.name=obj["name"].toString(QStringLiteral("Layer")); layer.visible=obj["visible"].toBool(true); layer.locked=obj["locked"].toBool(false); layer.opacity=obj["opacity"].toDouble(1.0); const QString imagePath=base.absoluteFilePath(obj["image"].toString()); layer.image.load(imagePath); if(layer.image.isNull()) layer.image=QImage(m_width,m_height,QImage::Format_ARGB32_Premultiplied); layer.frames.clear(); m_layers.push_back(layer); }
    if(m_layers.isEmpty()) addLayer(QStringLiteral("Paint Layer")); m_activeLayer=qBound(0,root["activeLayer"].toInt(0),m_layers.size()-1); m_path=filePath; return true;
}

bool FluxDocument::exportImage(const QString& filePath, QString* error) const { const QImage out=composite(); if(!out.save(filePath)){if(error)*error=QStringLiteral("Export failed");return false;} return true; }
