#include "fluxbrush.h"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QPainter>
#include <QRandomGenerator>

QJsonObject BrushPreset::toJson() const {
    QJsonObject o;
    o["name"] = name; o["size"] = size; o["opacity"] = opacity; o["flow"] = flow;
    o["spacing"] = spacing; o["jitter"] = jitter; o["scatter"] = scatter;
    o["textureStrength"] = textureStrength; o["wetness"] = wetness; o["stabilization"] = stabilization;
    o["color"] = color.name(QColor::HexArgb);
    QJsonObject d;
    d["pressureSize"] = dynamics.pressureSize; d["pressureOpacity"] = dynamics.pressureOpacity;
    d["tiltSize"] = dynamics.tiltSize; d["velocityOpacity"] = dynamics.velocityOpacity;
    d["minPressure"] = dynamics.minPressure; d["sizeScale"] = dynamics.sizeScale; d["opacityScale"] = dynamics.opacityScale;
    o["dynamics"] = d;
    return o;
}

BrushPreset BrushPreset::fromJson(const QJsonObject& o) {
    BrushPreset p;
    p.name=o.value("name").toString(p.name); p.size=o.value("size").toInt(p.size);
    p.opacity=o.value("opacity").toDouble(p.opacity); p.flow=o.value("flow").toDouble(p.flow);
    p.spacing=o.value("spacing").toDouble(p.spacing); p.jitter=o.value("jitter").toDouble(p.jitter);
    p.scatter=o.value("scatter").toDouble(p.scatter); p.textureStrength=o.value("textureStrength").toDouble(p.textureStrength);
    p.wetness=o.value("wetness").toDouble(p.wetness); p.stabilization=o.value("stabilization").toDouble(p.stabilization);
    p.color=QColor(o.value("color").toString()); if(!p.color.isValid()) p.color=Qt::black;
    const auto d=o.value("dynamics").toObject();
    p.dynamics.pressureSize=d.value("pressureSize").toBool(p.dynamics.pressureSize);
    p.dynamics.pressureOpacity=d.value("pressureOpacity").toBool(p.dynamics.pressureOpacity);
    p.dynamics.tiltSize=d.value("tiltSize").toBool(p.dynamics.tiltSize);
    p.dynamics.velocityOpacity=d.value("velocityOpacity").toBool(p.dynamics.velocityOpacity);
    p.dynamics.minPressure=d.value("minPressure").toDouble(p.dynamics.minPressure);
    p.dynamics.sizeScale=d.value("sizeScale").toDouble(p.dynamics.sizeScale);
    p.dynamics.opacityScale=d.value("opacityScale").toDouble(p.dynamics.opacityScale);
    return p;
}

bool BrushPreset::save(const QString& filePath, QString* error) const {
    QFile file(filePath);
    if(!file.open(QIODevice::WriteOnly|QIODevice::Truncate)){ if(error)*error=file.errorString(); return false; }
    file.write(QJsonDocument(toJson()).toJson(QJsonDocument::Indented));
    return true;
}

BrushPreset BrushPreset::load(const QString& filePath, QString* error) {
    QFile file(filePath);
    if(!file.open(QIODevice::ReadOnly)){ if(error)*error=file.errorString(); return {}; }
    QJsonParseError parse{}; const auto doc=QJsonDocument::fromJson(file.readAll(),&parse);
    if(doc.isNull()||!doc.isObject()){ if(error)*error=parse.errorString(); return {}; }
    return fromJson(doc.object());
}

BrushEngine::BrushEngine() {
    m_preset = BrushPreset{};
}

void BrushEngine::setPreset(const BrushPreset& preset) { m_preset = preset; }
void BrushEngine::setColor(const QColor& color) { m_preset.color = color; }

void BrushEngine::beginStroke(const BrushInput& input) {
    m_history.clear(); m_history.push_back(input); m_lastStamp=input.position; m_active=true;
}

BrushInput BrushEngine::dynamics(const BrushInput& input) const {
    BrushInput out=input;
    out.pressure=qBound(m_preset.dynamics.minPressure, input.pressure, 1.0);
    return out;
}

qreal BrushEngine::pressureSize(qreal pressure) const {
    return m_preset.size * (m_preset.dynamics.pressureSize ? qMax(m_preset.dynamics.minPressure, pressure) : 1.0) * m_preset.dynamics.sizeScale;
}

qreal BrushEngine::pressureOpacity(qreal pressure) const {
    return m_preset.opacity * (m_preset.dynamics.pressureOpacity ? qMax(m_preset.dynamics.minPressure, pressure) : 1.0) * m_preset.dynamics.opacityScale;
}

QPointF BrushEngine::stabilized(const QPointF& point) const {
    if(m_history.isEmpty() || m_preset.stabilization <= 0.0) return point;
    const qreal k=qBound(0.0, 1.0-m_preset.stabilization, 1.0);
    return m_history.last().position*(1.0-k)+point*k;
}

void BrushEngine::stamp(QImage& target, const QPointF& center, qreal size, qreal opacity, qreal rotation) {
    if(size <= 0.5 || target.isNull()) return;
    QPainter painter(&target); painter.setRenderHint(QPainter::Antialiasing,true);
    painter.setOpacity(qBound(0.0, opacity*m_preset.flow, 1.0));
    painter.setCompositionMode(m_preset.wetness>0.0 ? QPainter::CompositionMode_SourceOver : QPainter::CompositionMode_SourceOver);
    painter.translate(center); painter.rotate(rotation);
    const QRectF dst(-size*0.5,-size*0.5,size,size);
    if(!m_preset.texture.isNull() && m_preset.textureStrength>0.0) {
        QImage tex=m_preset.texture.convertToFormat(QImage::Format_ARGB32_Premultiplied);
        painter.setOpacity(qBound(0.0,opacity,1.0));
        painter.drawImage(dst,tex);
    } else {
        painter.setBrush(m_preset.color); painter.setPen(Qt::NoPen); painter.drawEllipse(dst);
    }
}

void BrushEngine::addPoint(QImage& target, const BrushInput& rawInput) {
    if(!m_active) beginStroke(rawInput);
    BrushInput input=dynamics(rawInput);
    input.position=stabilized(input.position);
    const QPointF from=m_lastStamp; const QPointF to=input.position; const qreal dist=QLineF(from,to).length();
    const qreal size=pressureSize(input.pressure);
    const qreal step=qMax(1.0,size*qMax(0.03,m_preset.spacing));
    const int count=qMax(1,int(std::ceil(dist/step)));
    for(int i=1;i<=count;++i){
        const qreal t=double(i)/count; QPointF p=from+(to-from)*t;
        if(m_preset.jitter>0.0){ const qreal r=m_preset.jitter*size; p += QPointF((QRandomGenerator::global()->generateDouble()-0.5)*2*r,(QRandomGenerator::global()->generateDouble()-0.5)*2*r); }
        if(m_preset.scatter>0.0){ const qreal r=m_preset.scatter*size; const qreal a=QRandomGenerator::global()->generateDouble()*6.283185307; p += QPointF(std::cos(a)*r,std::sin(a)*r); }
        qreal opacity=pressureOpacity(input.pressure);
        if(m_preset.dynamics.velocityOpacity) opacity*=1.0-qBound(0.0,input.velocity/2500.0);
        qreal adjustedSize=size;
        if(m_preset.dynamics.tiltSize) adjustedSize*=1.0+qMin(1.0,(qAbs(input.tiltX)+qAbs(input.tiltY))/90.0);
        stamp(target,p,adjustedSize,opacity,input.rotation);
    }
    m_lastStamp=to; m_history.push_back(input);
}

void BrushEngine::endStroke() { m_active=false; }
