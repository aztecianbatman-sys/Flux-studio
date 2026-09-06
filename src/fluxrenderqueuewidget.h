#pragma once
#include <QWidget>
#include "fluxrenderqueue.h"

class QListWidget; class QProgressBar; class QPushButton; class QLabel;
class FluxRenderQueueWidget final : public QWidget {
    Q_OBJECT
public:
    explicit FluxRenderQueueWidget(QWidget* parent=nullptr);
    FluxRenderQueue& queue(){return m_queue;}
    const FluxRenderQueue& queue() const{return m_queue;}
    void addJob(const FluxRenderJob& job);
    void refreshQueue();
    void setRunning(bool running);
    void setProgress(double progress, const QString& status);
signals:
    void renderRequested();
    void cancelRequested();
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
    QPushButton* m_cancel{};
    QProgressBar* m_progress{};
    QLabel* m_status{};
};
