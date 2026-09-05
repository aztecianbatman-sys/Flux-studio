#pragma once

#include <QColor>
#include <QImage>
#include <QJsonObject>
#include <QPointF>
#include <QVector>

struct BrushInput {
    QPointF position;
    qreal pressure = 1.0;
    qreal tiltX = 0.0;
    qreal tiltY = 0.0;
    qreal rotation = 0.0;
    qreal velocity = 0.0;
};

struct BrushDynamics {
    bool pressureSize = true;
    bool pressureOpacity = true;
    bool tiltSize = false;
    bool velocityOpacity = false;
    qreal minPressure = 0.05;
    qreal sizeScale = 1.0;
    qreal opacityScale = 1.0;
};

struct BrushPreset {
    QString name = QStringLiteral("Flux Ink");
    int size = 24;
    qreal opacity = 1.0;
    qreal flow = 1.0;
    qreal spacing = 0.18;
    qreal jitter = 0.0;
    qreal scatter = 0.0;
    qreal textureStrength = 0.0;
    qreal wetness = 0.0;
    qreal stabilization = 0.12;
    QColor color = Qt::black;
    QImage texture;
    BrushDynamics dynamics;

    QJsonObject toJson() const;
    static BrushPreset fromJson(const QJsonObject& object);
    bool save(const QString& filePath, QString* error = nullptr) const;
    static BrushPreset load(const QString& filePath, QString* error = nullptr);
};

class BrushEngine final {
public:
    BrushEngine();

    void setPreset(const BrushPreset& preset);
    const BrushPreset& preset() const { return m_preset; }
    void setColor(const QColor& color);

    void beginStroke(const BrushInput& input);
    void addPoint(QImage& target, const BrushInput& input);
    void endStroke();

private:
    BrushInput dynamics(const BrushInput& input) const;
    void stamp(QImage& target, const QPointF& center, qreal size, qreal opacity, qreal rotation);
    QPointF stabilized(const QPointF& point) const;
    qreal pressureSize(qreal pressure) const;
    qreal pressureOpacity(qreal pressure) const;

    BrushPreset m_preset;
    QVector<BrushInput> m_history;
    QPointF m_lastStamp;
    bool m_active = false;
};
