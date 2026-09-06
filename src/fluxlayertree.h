#pragma once

#include <QTreeWidget>

class FluxLayerTree final : public QTreeWidget {
    Q_OBJECT
public:
    explicit FluxLayerTree(QWidget* parent = nullptr) : QTreeWidget(parent) {}

signals:
    void hierarchyDropped();

protected:
    void dropEvent(QDropEvent* event) override;
};
