#include "fluxworkflow.h"
#include <QSettings>
#include <algorithm>

QStringList FluxWorkflow::recentProjects(int max){QSettings s("Flux","Flux Studio");QStringList list=s.value("recentProjects").toStringList();QStringList out;for(const auto&p:list)if(!p.isEmpty())out<<p;if(out.size()>max)out=out.mid(0,max);return out;}
void FluxWorkflow::addRecentProject(const QString&path,int max){if(path.isEmpty())return;auto list=recentProjects(max);list.removeAll(path);list.prepend(path);while(list.size()>max)list.removeLast();QSettings("Flux","Flux Studio").setValue("recentProjects",list);}
QStringList FluxWorkflow::workspaceNames(){QSettings s("Flux","Flux Studio");s.beginGroup("workspaces");const auto groups=s.childGroups();s.endGroup();return groups;}
bool FluxWorkflow::saveWorkspace(const QString&name,const QByteArray&state){if(name.isEmpty())return false;QSettings s("Flux","Flux Studio");s.beginGroup("workspaces");s.setValue(name,state);s.endGroup();return true;}
QByteArray FluxWorkflow::loadWorkspace(const QString&name){QSettings s("Flux","Flux Studio");s.beginGroup("workspaces");auto v=s.value(name);s.endGroup();return v.toByteArray();}
