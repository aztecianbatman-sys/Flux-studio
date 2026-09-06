#include "fluxlayertree.h"
#include <QDropEvent>

void FluxLayerTree::dropEvent(QDropEvent* event) {
    QTreeWidget::dropEvent(event);
    emit hierarchyDropped();
}
