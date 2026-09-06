#include "fluxperformance.h"
#include <QOpenGLContext>
#include <QOpenGLFunctions>
FluxPerformanceSnapshot FluxPerformance::probe(){FluxPerformanceSnapshot s; s.tileSize=256; if(auto*ctx=QOpenGLContext::currentContext()){s.gpuAvailable=true; s.gpuRenderer=QString::fromLatin1(reinterpret_cast<const char*>(ctx->functions()->glGetString(GL_RENDERER)));} return s;}
