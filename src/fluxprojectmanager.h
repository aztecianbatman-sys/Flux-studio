#pragma once
#include <QJsonObject>
#include <QString>
#include <QStringList>

struct FluxAssetRef { QString id; QString relativePath; QString sourcePath; bool embedded=false; };
struct FluxProjectValidation { bool valid=false; QStringList errors; QStringList warnings; };

class FluxProjectManager final {
public:
    static constexpr int CurrentVersion = 6;
    static FluxProjectValidation validate(const QString&projectPath);
    static bool migrate(QJsonObject&project,int fromVersion,QString*error=nullptr);
    static bool writeManifest(const QString&rootPath,const QJsonObject&project,QString*error=nullptr);
    static bool createBackup(const QString&projectPath,int keep=5,QString*error=nullptr);
    static QString recoveryRoot();
    static bool writeAtomic(const QString&path,const QByteArray&data,QString*error=nullptr);
};
