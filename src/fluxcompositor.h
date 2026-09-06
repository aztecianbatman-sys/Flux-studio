#pragma once
#include <QImage>
#include <QJsonObject>
#include <QPointF>
#include <QVector>
#include <QString>

struct FluxEffect { QString type; QVariantMap params; bool enabled=true; };
struct FluxCompNode { int id=0; QString name; QString type; QVector<int> inputs; QVariantMap params; };

class FluxCompositor final {
public:
    FluxCompositor();
    int addNode(const QString&type,const QString&name);
    void removeNode(int id); void connectNodes(int from,int to); void disconnectNodes(int from,int to);
    FluxCompNode* node(int id); const QVector<FluxCompNode>& nodes()const{return m_nodes;}
    QImage render(const QImage&source)const;
    QImage applyEffects(QImage image,const QVector<FluxEffect>&effects)const;
    QJsonObject toJson()const; void fromJson(const QJsonObject&);
private:
    QImage applyNode(const FluxCompNode&node,const QImage&image)const;
    QVector<FluxCompNode> m_nodes; int m_nextId=1;
};
