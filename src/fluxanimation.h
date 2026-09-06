#pragma once
#include <QJsonObject>
#include <QPointF>
#include <QVector>
#include <QString>
#include <QVariant>

struct FluxKeyframe { int frame=0; QVariant value; bool hold=false; QString interpolation=QStringLiteral("Bezier"); };
struct FluxAnimationTrack { QString id; QString name; QString property=QStringLiteral("Opacity"); bool muted=false; bool locked=false; QVector<FluxKeyframe> keys; };
struct FluxAnimationMarker { int frame=0; QString name; QString color; };
struct FluxShot { QString id; QString name; int start=0; int end=119; QString scene; };
struct FluxScene { QString id; QString name; QVector<QString> shotIds; };

class FluxAnimationSystem final {
public:
    void setFrameRange(int start,int end); int startFrame() const{return m_start;} int endFrame() const{return m_end;}
    void setFps(int fps); int fps() const{return m_fps;}
    FluxAnimationTrack& addTrack(const QString& name,const QString&property=QStringLiteral("Opacity"));
    void removeTrack(int index); QVector<FluxAnimationTrack>& tracks(){return m_tracks;} const QVector<FluxAnimationTrack>& tracks() const{return m_tracks;}
    void insertFrame(int frame,int count=1); void deleteFrame(int frame,int count=1); void duplicateFrame(int frame);
    void addKey(int track,int frame,const QVariant&value,bool hold=false,const QString&interp=QStringLiteral("Bezier"));
    QVariant evaluate(int track,int frame) const;
    void addMarker(int frame,const QString&name,const QString&color=QStringLiteral("#8aa8ff"));
    QVector<FluxAnimationMarker>& markers(){return m_markers;} const QVector<FluxAnimationMarker>& markers()const{return m_markers;}
    FluxScene& addScene(const QString&name); FluxShot& addShot(const QString&name,int start,int end,const QString&sceneId=QString());
    QVector<FluxScene>& scenes(){return m_scenes;} QVector<FluxShot>& shots(){return m_shots;}
    QJsonObject toJson() const; void fromJson(const QJsonObject&);
private:
    QVariant interpolate(const FluxKeyframe&a,const FluxKeyframe&b,int frame) const;
    int m_start=0,m_end=119,m_fps=24; QVector<FluxAnimationTrack> m_tracks; QVector<FluxAnimationMarker> m_markers; QVector<FluxScene> m_scenes; QVector<FluxShot> m_shots;
};
