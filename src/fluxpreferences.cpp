#include "fluxpreferences.h"
#include <QSettings>

static QSettings settings(){ return QSettings(QStringLiteral("Flux"), QStringLiteral("Flux Studio")); }

FluxPreferences FluxPreferencesStore::load(){
    QSettings s=settings(); FluxPreferences p;
#define V(key, field) p.field=s.value(QStringLiteral(key),p.field).decltype(p.field)();
    p.theme=s.value("preferences/theme",p.theme).toString();
    p.density=s.value("preferences/density",p.density).toString();
    p.uiScale=s.value("preferences/uiScale",p.uiScale).toInt();
    p.highContrast=s.value("preferences/highContrast",p.highContrast).toBool();
    p.colorBlindAssist=s.value("preferences/colorBlindAssist",p.colorBlindAssist).toBool();
    p.canvasCheckerboard=s.value("preferences/canvasCheckerboard",p.canvasCheckerboard).toBool();
    p.smoothZoom=s.value("preferences/smoothZoom",p.smoothZoom).toBool();
    p.touchpadGestures=s.value("preferences/touchpadGestures",p.touchpadGestures).toBool();
    p.autosave=s.value("preferences/autosave",p.autosave).toBool();
    p.autosaveSeconds=s.value("preferences/autosaveSeconds",p.autosaveSeconds).toInt();
    p.backupCount=s.value("preferences/backupCount",p.backupCount).toInt();
    p.memoryLimitMiB=s.value("preferences/memoryLimitMiB",p.memoryLimitMiB).toInt();
    p.defaultColorSpace=s.value("preferences/defaultColorSpace",p.defaultColorSpace).toString();
#undef V
    return p;
}
void FluxPreferencesStore::save(const FluxPreferences&p){
    QSettings s=settings();
    s.setValue("preferences/theme",p.theme); s.setValue("preferences/density",p.density); s.setValue("preferences/uiScale",p.uiScale);
    s.setValue("preferences/highContrast",p.highContrast); s.setValue("preferences/colorBlindAssist",p.colorBlindAssist);
    s.setValue("preferences/canvasCheckerboard",p.canvasCheckerboard); s.setValue("preferences/smoothZoom",p.smoothZoom);
    s.setValue("preferences/touchpadGestures",p.touchpadGestures); s.setValue("preferences/autosave",p.autosave);
    s.setValue("preferences/autosaveSeconds",p.autosaveSeconds); s.setValue("preferences/backupCount",p.backupCount);
    s.setValue("preferences/memoryLimitMiB",p.memoryLimitMiB); s.setValue("preferences/defaultColorSpace",p.defaultColorSpace);
}
QList<FluxShortcut> FluxPreferencesStore::shortcuts(){
    QSettings s=settings(); QList<FluxShortcut> out;
    const QList<FluxShortcut> defaults={{"new","New Project","Ctrl+N"},{"open","Open Project","Ctrl+O"},{"save","Save Project","Ctrl+S"},{"saveAs","Save As","Ctrl+Shift+S"},{"undo","Undo","Ctrl+Z"},{"redo","Redo","Ctrl+Y"},{"palette","Command Palette","Ctrl+K"},{"home","Home","Ctrl+Shift+H"},{"play","Play/Pause","Space"},{"previous","Previous Frame","Left"},{"next","Next Frame","Right"}};
    for(const auto&d:defaults) out.push_back({d.commandId,d.title,s.value(QStringLiteral("shortcuts/%1").arg(d.commandId),d.sequence).toString()}); return out;
}
bool FluxPreferencesStore::hasConflict(const QString&sequence,const QString&except){ if(sequence.trimmed().isEmpty()) return false; for(const auto&s:shortcuts()) if(s.commandId!=except&&s.sequence.compare(sequence,Qt::CaseInsensitive)==0) return true; return false; }
bool FluxPreferencesStore::setShortcut(const QString&id,const QString&sequence){ if(hasConflict(sequence,id)) return false; QSettings s=settings(); s.setValue(QStringLiteral("shortcuts/%1").arg(id),sequence); return true; }
void FluxPreferencesStore::restoreDefaults(){ QSettings s=settings(); s.remove("preferences"); s.remove("shortcuts"); }
