#pragma once
#include <QString>
#include <QStringList>
class FluxWorkflow final {
public:
    static QStringList recentProjects(int max=12);
    static void addRecentProject(const QString&path,int max=12);
    static QStringList workspaceNames();
    static bool saveWorkspace(const QString&name,const QByteArray&state);
    static QByteArray loadWorkspace(const QString&name);
};
