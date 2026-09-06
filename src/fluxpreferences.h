#pragma once
#include <QColor>
#include <QHash>
#include <QString>
#include <QStringList>

struct FluxPreferences {
    QString theme = QStringLiteral("Flux Dark");
    QString density = QStringLiteral("Comfortable");
    int uiScale = 100;
    bool highContrast = false;
    bool colorBlindAssist = false;
    bool canvasCheckerboard = true;
    bool smoothZoom = true;
    bool touchpadGestures = true;
    bool autosave = true;
    int autosaveSeconds = 30;
    int backupCount = 10;
    int memoryLimitMiB = 4096;
    QString defaultColorSpace = QStringLiteral("sRGB");
};

struct FluxShortcut { QString commandId; QString title; QString sequence; };

class FluxPreferencesStore final {
public:
    static FluxPreferences load();
    static void save(const FluxPreferences& prefs);
    static QList<FluxShortcut> shortcuts();
    static bool setShortcut(const QString& commandId, const QString& sequence);
    static bool hasConflict(const QString& sequence, const QString& exceptCommandId = {});
    static void restoreDefaults();
};
