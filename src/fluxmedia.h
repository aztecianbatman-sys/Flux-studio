#pragma once
#include <QByteArray>
#include <QImage>
#include <QString>
#include <QVector>
#include <functional>

struct FluxAudioInfo { QString path; double duration=0; int sampleRate=48000; int channels=2; QVector<float> waveform; };
struct FluxVideoInfo { QString path; double duration=0; int fps=24; int width=0; int height=0; int frames=0; };

class FluxMediaEngine final {
public:
    static bool inspectAudio(const QString&path,FluxAudioInfo*info,QString*error=nullptr);
    static bool inspectVideo(const QString&path,FluxVideoInfo*info,QString*error=nullptr);
    static QVector<float> buildWaveform(const QString&path,int samples=2000,QString*error=nullptr);
    static QImage extractVideoFrame(const QString&path,int frame,int fps,const QSize&size=QSize(),QString*error=nullptr);
    static bool decodeAudioPcm(const QString&path,QByteArray*out,int sampleRate=48000,int channels=2,QString*error=nullptr);
private:
    static bool runFfmpeg(const QStringList&args,QByteArray*stdoutData,QByteArray*stderrData,QString*error);
};
