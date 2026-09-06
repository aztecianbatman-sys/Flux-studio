#pragma once
#include <QString>
struct FluxPerformanceSnapshot { double fps=0; double frameMs=0; qint64 memoryBytes=0; qint64 cacheBytes=0; int tileSize=256; bool gpuAvailable=false; QString gpuRenderer; };
class FluxPerformance final { public: static FluxPerformanceSnapshot probe(); };
