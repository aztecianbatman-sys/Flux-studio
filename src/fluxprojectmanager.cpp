#include "fluxprojectmanager.h"
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QSaveFile>
#include <QStandardPaths>
#include <QDateTime>

FluxProjectValidation FluxProjectManager::validate(const QString&projectPath){
    FluxProjectValidation v; QFile f(projectPath); if(!f.open(QIODevice::ReadOnly)){v.errors<<f.errorString();return v;}
    QJsonParseError e{}; const QJsonDocument d=QJsonDocument::fromJson(f.readAll(),&e); if(e.error!=QJsonParseError::NoError||!d.isObject()){v.errors<<QStringLiteral("Invalid project manifest: ")+e.errorString();return v;}
    const QJsonObject o=d.object(); const int version=o.value("version").toInt(0); if(version<=0)v.errors<<"Missing project version"; if(version>CurrentVersion)v.errors<<"Project was created by a newer Flux Studio version";
    if(!o.value("name").isString())v.warnings<<"Project name is missing"; if(!o.value("layers").isArray()&&!o.value("documents").isObject())v.warnings<<"No layer/document manifest found";
    v.valid=v.errors.isEmpty(); return v;
}
bool FluxProjectManager::migrate(QJsonObject&project,int fromVersion,QString*error){
    if(fromVersion>CurrentVersion){if(error)*error="Cannot migrate a newer project";return false;}
    int v=fromVersion; while(v<CurrentVersion){
        if(v==1&&!project.contains("assets"))project["assets"]=QJsonArray();
        else if(v==2&&!project.contains("recovery"))project["recovery"]=QJsonObject();
        else if(v==3&&!project.contains("format"))project["format"]="flux-package";
        else if(v==4&&!project.contains("documents"))project["documents"]=QJsonObject();
        else if(v==5&&!project.contains("embeddedAssets"))project["embeddedAssets"]=true;
        ++v;
    }
    project["version"]=CurrentVersion; return true;
}
bool FluxProjectManager::writeAtomic(const QString&path,const QByteArray&data,QString*error){QFileInfo i(path);if(!i.dir().exists()&&!QDir().mkpath(i.dir().absolutePath())){if(error)*error="Could not create project directory";return false;}QSaveFile f(path);if(!f.open(QIODevice::WriteOnly)){if(error)*error=f.errorString();return false;}if(f.write(data)!=data.size()||!f.commit()){if(error)*error=f.errorString();return false;}return true;}
bool FluxProjectManager::writeManifest(const QString&rootPath,const QJsonObject&project,QString*error){QDir root(rootPath);const QStringList dirs={"document","layers","frames","audio","video","assets","brushes","previews","thumbnails","recovery","backups"};for(const auto&d:dirs)if(!root.mkpath(d)){if(error)*error=QStringLiteral("Could not create %1").arg(d);return false;}return writeAtomic(root.filePath("project.json"),QJsonDocument(project).toJson(QJsonDocument::Indented),error);}
bool FluxProjectManager::createBackup(const QString&projectPath,int keep,QString*error){QFileInfo info(projectPath);const QString backupDir=info.dir().filePath(info.completeBaseName()+".backups");if(!QDir().mkpath(backupDir)){if(error)*error="Could not create backup directory";return false;}const QString stamp=QDateTime::currentDateTimeUtc().toString("yyyyMMdd-HHmmss");const QString dest=QDir(backupDir).filePath(info.fileName()+"."+stamp+".bak");if(!QFile::copy(projectPath,dest)){if(error)*error="Could not create backup";return false;}QStringList files=QDir(backupDir).entryList(QDir::Files,QDir::Time);for(int i=qMax(0,keep);i<files.size();++i)QFile::remove(QDir(backupDir).filePath(files[i]));return true;}
QString FluxProjectManager::recoveryRoot(){return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)+QStringLiteral("/recovery");}
