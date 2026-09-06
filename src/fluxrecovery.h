#pragma once
#include <QByteArray>
#include <QString>
#include <QStringList>

class FluxRecoveryManager final {
public:
    static QString root();
    static bool markRunning(const QString&projectId,QString*error=nullptr);
    static void markClean(const QString&projectId);
    static bool hadPreviousCrash(const QString&projectId);
    static bool writeSnapshot(const QString&projectId,const QByteArray&projectData,int sequence,QString*error=nullptr);
    static QStringList listSnapshots(const QString&projectId);
    static bool trimSnapshots(const QString&projectId,int keep=20,QString*error=nullptr);
};
