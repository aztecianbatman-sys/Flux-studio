#include "fluxrenderqueuewidget.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QProgressBar>
#include <QPushButton>
#include <QVBoxLayout>

FluxRenderQueueWidget::FluxRenderQueueWidget(QWidget* parent):QWidget(parent){
    auto* root=new QVBoxLayout(this);
    m_list=new QListWidget;
    root->addWidget(m_list,1);
    auto* statusRow=new QHBoxLayout;
    m_status=new QLabel("Queue idle");
    m_progress=new QProgressBar;
    m_progress->setRange(0,1000);
    m_progress->setValue(0);
    statusRow->addWidget(m_status,1);
    statusRow->addWidget(m_progress);
    root->addLayout(statusRow);
    auto* row=new QHBoxLayout;
    auto*up=new QPushButton("↑"); auto*down=new QPushButton("↓");
    auto*remove=new QPushButton("Remove"); auto*clear=new QPushButton("Clear");
    m_render=new QPushButton("Render Queue"); m_cancel=new QPushButton("Cancel");
    m_cancel->setEnabled(false);
    for(auto*b:{up,down,remove,clear,m_render,m_cancel})row->addWidget(b);
    root->addLayout(row);
    connect(up,&QPushButton::clicked,this,&FluxRenderQueueWidget::moveSelectedUp);
    connect(down,&QPushButton::clicked,this,&FluxRenderQueueWidget::moveSelectedDown);
    connect(remove,&QPushButton::clicked,this,&FluxRenderQueueWidget::removeSelected);
    connect(clear,&QPushButton::clicked,this,&FluxRenderQueueWidget::clearQueue);
    connect(m_render,&QPushButton::clicked,this,&FluxRenderQueueWidget::renderAll);
    connect(m_cancel,&QPushButton::clicked,this,&FluxRenderQueueWidget::cancelRequested);
}
void FluxRenderQueueWidget::addJob(const FluxRenderJob&job){m_queue.enqueue(job);refresh();}
void FluxRenderQueueWidget::removeSelected(){const int r=m_list->currentRow();if(r<0)return;auto jobs=m_queue.jobs();jobs.removeAt(r);m_queue.setJobs(jobs);refresh();}
void FluxRenderQueueWidget::clearQueue(){m_queue.clear();refresh();}
void FluxRenderQueueWidget::moveSelectedUp(){const int r=m_list->currentRow();if(r<=0)return;auto jobs=m_queue.jobs();qSwap(jobs[r],jobs[r-1]);m_queue.setJobs(jobs);refresh();m_list->setCurrentRow(r-1);}
void FluxRenderQueueWidget::moveSelectedDown(){const int r=m_list->currentRow();if(r<0||r>=m_list->count()-1)return;auto jobs=m_queue.jobs();qSwap(jobs[r],jobs[r+1]);m_queue.setJobs(jobs);refresh();m_list->setCurrentRow(r+1);}
void FluxRenderQueueWidget::renderAll(){emit renderRequested();}
void FluxRenderQueueWidget::refreshQueue(){refresh();}
void FluxRenderQueueWidget::setRunning(bool running){m_render->setEnabled(!running);m_cancel->setEnabled(running);}
void FluxRenderQueueWidget::setProgress(double progress,const QString&status){m_progress->setValue(qBound(0,static_cast<int>(progress*1000.0),1000));m_status->setText(status);}
void FluxRenderQueueWidget::refresh(){m_list->clear();for(const auto&j:m_queue.jobs()){auto*item=new QListWidgetItem(QString("%1  •  %2  •  %3  •  %4").arg(j.job.name,j.job.format,j.job.output,j.state));item->setToolTip(j.error.isEmpty()?j.state:j.error);m_list->addItem(item);}}
