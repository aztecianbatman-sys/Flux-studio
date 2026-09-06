#pragma once
#include <QWidget>
#include "fluxrenderqueue.h"

class QListWidget; class QProgressBar; class QPushButton;
class FluxRenderQueueWidget final : public QWidget {
    Q_OBJECT
public:
    explicit FluxRenderQueueWidget(QWidget* parent=nullptr);
    FluxRenderQueue& queue(){return m_queue;}
    void addJob(const FluxRenderJob& job);
signals:
    void renderRequested();
private slots:
    void removeSelected();
    void clearQueue();
    void moveSelectedUp();
    void moveSelectedDown();
    void renderAll();
private:
    void refresh();
    FluxRenderQueue m_queue;
    QListWidget* m_list{};
    QPushButton* m_render{};
};
