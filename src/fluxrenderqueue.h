#pragma once
#include "fluxexport.h"
#include <QVector>
#include <QString>
#include <functional>

struct FluxQueuedRender { FluxRenderJob job; QString state=QStringLiteral("Queued"); double progress=0.0; QString error; };
class FluxRenderQueue final {
public:
    int enqueue(const FluxRenderJob&job);
    void clear(); int count()const{return m_jobs.size();}
    const QVector<FluxQueuedRender>& jobs()const{return m_jobs;}
    bool process(const std::function<QImage(int)>&renderer,QString*error=nullptr);
    void cancel(){m_cancelled=true;}
private:
    QVector<FluxQueuedRender> m_jobs; bool m_cancelled=false;
};
