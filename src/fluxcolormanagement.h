#pragma once
#include <QImage>
#include <QString>
#include <QStringList>
#include <QByteArray>

struct FluxColorConfig { QString colorSpace = QStringLiteral("sRGB"); QString range = QStringLiteral("Full"); bool linearWorkflow = false; bool premultipliedAlpha = true; };
struct FluxExportValidation { bool valid=false; QStringList errors; QStringList warnings; };

class FluxColorManagement final {
public:
    static QStringList colorSpaces();
    static QStringList ranges();
    static FluxExportValidation validate(const FluxColorConfig&,const QString&format,const QImage&);
    static QImage convertForExport(const QImage&,const FluxColorConfig&);
    static QByteArray metadata(const FluxColorConfig&);
};
