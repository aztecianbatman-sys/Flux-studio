#include "fluxrenderqueue.h"

int FluxRenderQueue::enqueue(const FluxRenderJob&job){FluxQueuedRender q;q.job=job;m_jobs.push_back(q);return m_jobs.size()-1;}
void FluxRenderQueue::clear(){m_jobs.clear();m_cancelled=false;}
bool FluxRenderQueue::process(const std::function<QImage(int)>&renderer,QString*error){
    m_cancelled=false;
    for(auto&q:m_jobs){
        if(m_cancelled){q.state="Cancelled";break;}
        q.state=QStringLiteral("Rendering");q.progress=0;q.error.clear();
        const int start=q.job.settings.startFrame;const int end=qMax(start,q.job.settings.endFrame);const int total=end-start+1;
        if(q.job.format.compare("png",Qt::CaseInsensitive)==0||q.job.format.compare("jpeg",Qt::CaseInsensitive)==0||q.job.format.compare("webp",Qt::CaseInsensitive)==0||q.job.format.compare("svg",Qt::CaseInsensitive)==0){
            QImage image=renderer(start);const QString ext=q.job.format.toLower();bool ok=false;
            if(ext=="svg")ok=FluxExportEngine::exportSvgRaster(image,q.job.output,q.job.settings,&q.error);else ok=FluxExportEngine::exportImage(image,q.job.output,q.job.settings,&q.error);
            q.progress=1.0;q.state=ok?"Complete":"Failed";if(!ok){if(error)*error=q.error;return false;}continue;
        }
        const bool ok=FluxExportEngine::exportAnimated([&](int frame){if(m_cancelled)return QImage();q.progress=qBound(0.0,double(frame-start+1)/double(qMax(1,total)),1.0);return renderer(frame);},q.job,&q.error);
        if(!ok){q.state=m_cancelled?"Cancelled":"Failed";if(error)*error=q.error;return false;}
        q.progress=1.0;q.state="Complete";
    }
    return !m_cancelled;
}
