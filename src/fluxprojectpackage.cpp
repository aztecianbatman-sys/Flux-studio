#include "fluxprojectpackage.h"
#include "fluxdocument.h"

#include <QDataStream>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSaveFile>
#include <QTemporaryDir>

namespace {
constexpr quint32 Magic = 0x464C5558; // FLUX
constexpr quint32 Version = 1;

bool writeFile(QDataStream& out,const QString&name,const QByteArray&data,QString*error){
    out << name << quint64(data.size());
    if(out.status()!=QDataStream::Ok){if(error)*error="Failed to write package metadata";return false;}
    if(data.isEmpty()) return true;
    if(out.writeRawData(data.constData(),data.size())!=data.size()){if(error)*error="Failed to write package payload";return false;}
    return true;
}

bool readFile(QDataStream& in,QString*name,QByteArray*data,QString*error){
    quint64 size=0; in>>*name>>size;
    if(in.status()!=QDataStream::Ok||size>quint64(std::numeric_limits<int>::max())){if(error)*error="Invalid package entry";return false;}
    data->resize(int(size));
    if(size>0&&in.readRawData(data->data(),int(size))!=int(size)){if(error)*error="Truncated package payload";return false;}
    return true;
}
}

bool FluxProjectPackage::isPackage(const QString&filePath){
    QFile f(filePath); if(!f.open(QIODevice::ReadOnly)) return false;
    QDataStream in(&f); in>>std::hex; quint32 magic=0; in>>magic; return in.status()==QDataStream::Ok&&magic==Magic;
}

bool FluxProjectPackage::save(const QString&filePath,const FluxDocument&document,QString*error){
    QTemporaryDir tmp; if(!tmp.isValid()){if(error)*error="Could not create temporary project directory";return false;}
    const QString manifest=QDir(tmp.path()).filePath("project.flux");
    if(!document.save(manifest,error)) return false;
    const QString dataDir=manifest+QStringLiteral("data"); // compatibility path is project.fluxdata next to manifest
    Q_UNUSED(dataDir);
    const QFileInfo mi(manifest);
    const QString sidecar=mi.absolutePath()+QDir::separator()+mi.completeBaseName()+QStringLiteral(".fluxdata");
    QDir root(mi.absolutePath());
    QStringList files; files<<mi.fileName();
    const QStringList pngs=QDir(sidecar).entryList(QStringList()<<"*.png",QDir::Files,QDir::Name);
    for(const auto&p:pngs) files<<QString("%1/%2").arg(QFileInfo(sidecar).fileName(),p);

    QJsonObject meta; meta["format"]="Flux Package"; meta["version"]=Version; QJsonArray entries;
    for(const auto&rel:files) entries.append(rel); meta["entries"]=entries;
    const QByteArray metaBytes=QJsonDocument(meta).toJson(QJsonDocument::Compact);

    QSaveFile outFile(filePath); if(!outFile.open(QIODevice::WriteOnly)){if(error)*error=outFile.errorString();return false;}
    QDataStream out(&outFile); out.setVersion(QDataStream::Qt_6_5); out<<Magic<<Version<<quint32(metaBytes.size());
    if(out.writeRawData(metaBytes.constData(),metaBytes.size())!=metaBytes.size()){if(error)*error="Could not write package manifest";return false;}
    out<<quint32(files.size());
    for(const auto&rel:files){QFile f(root.filePath(rel));if(!f.open(QIODevice::ReadOnly)){if(error)*error=f.errorString();return false;}if(!writeFile(out,rel,f.readAll(),error))return false;}
    if(!outFile.commit()){if(error)*error=outFile.errorString();return false;}
    return true;
}

bool FluxProjectPackage::load(const QString&filePath,FluxDocument&document,QString*error){
    QFile file(filePath); if(!file.open(QIODevice::ReadOnly)){if(error)*error=file.errorString();return false;}
    QDataStream in(&file); in.setVersion(QDataStream::Qt_6_5); quint32 magic=0,version=0,metaSize=0;in>>magic>>version>>metaSize;
    if(in.status()!=QDataStream::Ok||magic!=Magic||version>Version||metaSize>64*1024*1024u){if(error)*error="Not a valid Flux package";return false;}
    QByteArray metaBytes(metaSize,Qt::Uninitialized); if(metaSize>0&&in.readRawData(metaBytes.data(),int(metaSize))!=int(metaSize)){if(error)*error="Truncated package manifest";return false;}
    QJsonParseError pe{};const QJsonDocument md=QJsonDocument::fromJson(metaBytes,&pe);if(pe.error!=QJsonParseError::NoError||!md.isObject()){if(error)*error="Invalid Flux package manifest";return false;}
    QTemporaryDir tmp;if(!tmp.isValid()){if(error)*error="Could not create extraction directory";return false;}
    quint32 count=0;in>>count;if(in.status()!=QDataStream::Ok||count>100000){if(error)*error="Invalid package entry count";return false;}
    const QDir root(tmp.path());
    for(quint32 i=0;i<count;++i){QString name;QByteArray data;if(!readFile(in,&name,&data,error))return false;QFileInfo info(root.filePath(name));if(info.filePath().contains("..")){if(error)*error="Unsafe package entry";return false;}QDir().mkpath(info.absolutePath());QFile f(info.filePath());if(!f.open(QIODevice::WriteOnly)){if(error)*error=f.errorString();return false;}if(f.write(data)!=data.size()){if(error)*error=f.errorString();return false;}}
    const QString manifest=root.filePath("project.flux");if(!QFileInfo::exists(manifest)){if(error)*error="Package does not contain a project manifest";return false;}
    return document.load(manifest,error);
}
