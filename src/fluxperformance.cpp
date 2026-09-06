#include "fluxperformance.h"
#include <QFile>
#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <QSysInfo>
#ifdef Q_OS_WIN
#include <windows.h>
#include <psapi.h>
#endif

namespace {
qint64 processMemory(){
#ifdef Q_OS_WIN
    PROCESS_MEMORY_COUNTERS_EX pmc{};pmc.cb=sizeof(pmc);if(GetProcessMemoryInfo(GetCurrentProcess(),reinterpret_cast<PROCESS_MEMORY_COUNTERS*>(&pmc),sizeof(pmc)))return qint64(pmc.WorkingSetSize);
#elif defined(Q_OS_LINUX)
    QFile f(QStringLiteral("/proc/self/statm"));if(f.open(QIODevice::ReadOnly)){const auto parts=QString::fromUtf8(f.readAll()).split(' ');if(parts.size()>1)return parts.at(1).toLongLong()*4096;}
#endif
    return 0;
}
}

FluxPerformanceSnapshot FluxPerformance::probe(){
    FluxPerformanceSnapshot s;s.tileSize=256;s.memoryBytes=processMemory();
    if(auto*ctx=QOpenGLContext::currentContext()){s.gpuAvailable=true;s.gpuRenderer=QString::fromLatin1(reinterpret_cast<const char*>(ctx->functions()->glGetString(GL_RENDERER)));}
    return s;
}
