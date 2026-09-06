#include "fluxrecovery.h"
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QSaveFile>
#include <QStandardPaths>
#include <QDateTime>

namespace { QString lockPath(const QString&id){return QDir(FluxRecoveryManager::root()).filePath(id+QStringLiteral(".running"));} QString snapDir(const QString&id){return QDir(FluxRecoveryManager::root()).filePath(id);}}
QString FluxRecoveryManager::root(){return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)+QStringLiteral("/recovery");}
bool FluxRecoveryManager::markRunning(const QString&projectId,QString*error){if(!QDir().mkpath(root())){if(error)*error="Could not create recovery directory";return false;}QSaveFile f(lockPath(projectId));if(!f.open(QIODevice::WriteOnly)||!f.commit()){if(error)*error=f.errorString();return false;}return true;}
void FluxRecoveryManager::markClean(const QString&projectId){QFile::remove(lockPath(projectId));}
bool FluxRecoveryManager::hadPreviousCrash(const QString&projectId){return QFileInfo::exists(lockPath(projectId));}
bool FluxRecoveryManager::writeSnapshot(const QString&projectId,const QByteArray&projectData,int sequence,QString*error){if(!QDir().mkpath(snapDir(projectId))){if(error)*error="Could not create snapshot directory";return false;}const QString path=QDir(snapDir(projectId)).filePath(QStringLiteral("snapshot_%1_%2.flux").arg(QDateTime::currentDateTimeUtc().toString("yyyyMMdd-HHmmss"),QString::number(sequence)));QSaveFile f(path);if(!f.open(QIODevice::WriteOnly)||f.write(projectData)!=projectData.size()||!f.commit()){if(error)*error=f.errorString();return false;}return true;}
QStringList FluxRecoveryManager::listSnapshots(const QString&projectId){return QDir(snapDir(projectId)).entryList(QStringList()<<"*.flux",QDir::Files,QDir::Time);}
bool FluxRecoveryManager::trimSnapshots(const QString&projectId,int keep,QString*error){const QString dir=snapDir(projectId);const QStringList files=listSnapshots(projectId);for(int i=qMax(0,keep);i<files.size();++i)if(!QFile::remove(QDir(dir).filePath(files[i]))){if(error)*error=QStringLiteral("Could not remove old snapshot");return false;}return true;}
