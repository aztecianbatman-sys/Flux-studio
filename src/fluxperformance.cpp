#include "fluxperformance.h"
#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <QImage>
#include <QProcess>
#ifdef Q_OS_WIN
#include <windows.h>
#include <psapi.h>
#endif

namespace {
qint64 processMemory(){
#ifdef Q_OS_WIN
    PROCESS_MEMORY_COUNTERS_EX pmc{};pmc.cb=sizeof(pmc);if(GetProcessMemoryInfo(GetCurrentProcess(),reinterpret_cast<PROCESS_MEMORY_COUNTERS*>(&pmc),sizeof(pmc)))return qint64(pmc.WorkingSetSize);
#elif defined(Q_OS_LINUX)
    QFile f(QStringLiteral("/proc/self/statm"));if(f.open(QIODevice::ReadOnly)){const auto parts=QString::fromUtf8(f.readAll()).split(' ');if(!parts.isEmpty())return parts.value(1).toLongLong()*qint64(QSysInfo::WordSize/8)*4096;}
#endif
    return 0;
}
}

FluxPerformanceSnapshot FluxPerformance::probe(){
    FluxPerformanceSnapshot s;s.tileSize=256;s.memoryBytes=processMemory();
    if(auto*ctx=QOpenGLContext::currentContext()){s.gpuAvailable=true;s.gpuRenderer=QString::fromLatin1(reinterpret_cast<const char*>(ctx->functions()->glGetString(GL_RENDERER)));}
    return s;
}
