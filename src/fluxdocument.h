#pragma once
#include <QColor>
#include <QImage>
#include <QJsonObject>
#include <QString>
#include <QVector>

struct FluxLayer {
    QString name;
    bool visible = true;
    bool locked = false;
    qreal opacity = 1.0;
    QImage image;
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
    void addLayer(const QString& name = QStringLiteral("Paint Layer"));
    void removeLayer(int index);
    void duplicateLayer(int index);
    int activeLayerIndex() const { return m_activeLayer; }
    void setActiveLayer(int index);
    QVector<FluxLayer>& layers() { return m_layers; }
    const QVector<FluxLayer>& layers() const { return m_layers; }
    QString path() const { return m_path; }
    QString name() const { return m_name; }
    int width() const { return m_width; }
    int height() const { return m_height; }
    QColor foreground() const { return m_foreground; }
    void setForeground(const QColor& color) { m_foreground = color; }
private:
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
