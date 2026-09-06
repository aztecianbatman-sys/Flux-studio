#pragma once
#include <QByteArray>
#include <QString>

class FluxDocument;

class FluxProjectPackage final {
public:
    static bool save(const QString&filePath,const FluxDocument&document,QString*error=nullptr);
    static bool load(const QString&filePath,FluxDocument&document,QString*error=nullptr);
    static bool isPackage(const QString&filePath);
};
