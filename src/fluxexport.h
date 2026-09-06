#pragma once
#include <QImage>
#include <QProcess>
#include <QString>
#include <QStringList>
#include <QVector>
#include <functional>

struct FluxRenderSettings { int width=0; int height=0; int fps=24; int startFrame=0; int endFrame=0; int bitrateKbps=12000; QString codec; bool transparent=false; QString colorSpace=QStringLiteral("sRGB"); };
struct FluxRenderJob { QString name; QString output; QString format; FluxRenderSettings settings; };

class FluxExportEngine final {
public:
    static bool exportImage(const QImage&image,const QString&path,const FluxRenderSettings&settings,QString*error=nullptr);
    static bool exportSvgRaster(const QImage&image,const QString&path,const FluxRenderSettings&settings,QString*error=nullptr);
    static bool exportSequence(const std::function<QImage(int)>&renderer,const FluxRenderSettings&settings,const QString&directory,QString*error=nullptr);
    static bool exportSpriteSheet(const std::function<QImage(int)>&renderer,const FluxRenderSettings&settings,const QString&path,int columns,QString*error=nullptr);
    static bool exportAnimated(const std::function<QImage(int)>&renderer,const FluxRenderJob&job,QString*error=nullptr);
    static bool encodeWithFfmpeg(const QStringList&args,QString*error=nullptr);
};
