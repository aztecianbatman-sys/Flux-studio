#pragma once
#include <QString>
#include <QVector>

struct FluxPressureCurve { qreal input=0; qreal output=0; };
struct FluxTabletProfile { QString deviceId; QString name; QString driver; QString primaryAction="Brush"; QString barrelAction="Pan"; QString eraserAction="Eraser"; QVector<FluxPressureCurve> pressureCurve{{0,0},{0.25,0.12},{0.5,0.5},{0.75,0.82},{1,1}}; bool windowsInk=true; bool enabled=true; };
class FluxTabletProfiles final {
public:
    static QVector<FluxTabletProfile> profiles();
    static void setProfile(const FluxTabletProfile&profile);
    static FluxTabletProfile profileFor(const QString&deviceId);
    static qreal mapPressure(const FluxTabletProfile&profile,qreal pressure);
};
