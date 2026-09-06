#pragma once
#include <QImage>
#include <QString>
#include <QByteArray>
#include <QMimeData>

class FluxClipboard final {
public:
    static void setImage(const QImage& image, const QString& source = QStringLiteral("Flux Studio"));
    static QImage image();
    static bool hasImage();
    static QByteArray encodeLayerPng(const QImage& image);
    static QImage decodeLayerPng(const QByteArray& bytes);
    static QString makeLayerPayload(const QString& name, const QImage& image);
    static bool parseLayerPayload(const QString& payload, QString* name, QImage* image);
};
