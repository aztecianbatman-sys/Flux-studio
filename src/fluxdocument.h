#pragma once

#include <QColor>
#include <QImage>
#include <QJsonObject>
#include <QString>
#include <QVector>
#include <QVariantMap>

struct FluxLayerStyle {
    bool enabled = false;
    qreal shadowOpacity = 0.0;
    qreal shadowRadius = 0.0;
    qreal shadowX = 0.0;
    qreal shadowY = 0.0;
    qreal outlineOpacity = 0.0;
    qreal outlineWidth = 0.0;
    QColor outlineColor = QColor(Qt::black);
};

enum class FluxLayerType { Paint, Group, Mask, VectorMask, Adjustment };
enum class FluxBlendMode { Normal, Multiply, Screen, Overlay, Add, Subtract };

struct FluxLayer {
    QString id;
    QString name;
    FluxLayerType type = FluxLayerType::Paint;
    int parentIndex = -1;
    bool expanded = true;
    bool visible = true;
    bool locked = false;
    bool alphaInherited = false;
    bool clipping = false;
    bool solo = false;
    bool isolate = false;
    qreal opacity = 1.0;
    QColor labelColor;
    FluxBlendMode blendMode = FluxBlendMode::Normal;
    FluxLayerStyle style;
    QImage image;
    QImage mask;
    QVariantMap adjustment;
    QVector<QImage> frames;
};

class FluxDocument final {
public:
    FluxDocument();
    void create(const QString& name, int width, int height);
    bool save(const QString& filePath, QString* error = nullptr) const;
    bool load(const QString& filePath, QString* error = nullptr);
    bool exportImage(const QString& filePath, QString* error = nullptr) const;
    QImage composite() const;
    QImage& activeImage();
    const QImage& activeImage() const;
    FluxLayer& activeLayer();
    const FluxLayer& activeLayer() const;
    void ensureFrame(int frame);
    void setFrame(int frame);
    int frame() const { return m_frame; }
    int frameCount() const { return m_frameCount; }
    void setFrameCount(int count);

    void addLayer(const QString& name = QStringLiteral("Paint Layer"), FluxLayerType type = FluxLayerType::Paint, int parentIndex = -1);
    void addGroup(const QString& name = QStringLiteral("Group"), int parentIndex = -1);
    void addMask(int layerIndex, bool vectorMask = false);
    void removeLayer(int index);
    void duplicateLayer(int index);
    void mergeDown(int index);
    void flattenVisible();
    void moveLayer(int from, int to);
    int activeLayerIndex() const { return m_activeLayer; }
    void setActiveLayer(int index);
    QVector<FluxLayer>& layers() { return m_layers; }
    const QVector<FluxLayer>& layers() const { return m_layers; }

    void setLayerOpacity(int index, qreal opacity);
    void setLayerBlendMode(int index, FluxBlendMode mode);
    void setLayerVisible(int index, bool visible);
    void setLayerLocked(int index, bool locked);
    void setLayerLabelColor(int index, const QColor& color);
    void setLayerClipping(int index, bool enabled);
    void setLayerAlphaInherited(int index, bool enabled);
    void setLayerStyle(int index, const FluxLayerStyle& style);
    void setLayerAdjustment(int index, const QVariantMap& adjustment);
    void setSolo(int index, bool enabled);
    void setIsolate(int index, bool enabled);
    bool hasSolo() const;
    bool hasIsolate() const;

    QString path() const { return m_path; }
    QString name() const { return m_name; }
    int width() const { return m_width; }
    int height() const { return m_height; }
    QColor foreground() const { return m_foreground; }
    void setForeground(const QColor& color) { m_foreground = color; }

    static QString blendModeName(FluxBlendMode mode);
    static FluxBlendMode blendModeFromName(const QString& name);
    static QString layerTypeName(FluxLayerType type);

private:
    bool layerVisibleForComposite(int index) const;
    QImage layerFrame(const FluxLayer& layer) const;
    void applyLayerStyle(QPainter& painter, const FluxLayer& layer, const QImage& image) const;

    QString m_name = QStringLiteral("Untitled");
    QString m_path;
    int m_width = 1920;
    int m_height = 1080;
    int m_frame = 0;
    int m_frameCount = 120;
    int m_activeLayer = 0;
    QColor m_foreground = QColor(Qt::black);
    QVector<FluxLayer> m_layers;
};
