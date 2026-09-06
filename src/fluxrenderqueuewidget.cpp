#include "fluxrenderqueuewidget.h"
#include <QHBoxLayout>
#include <QListWidget>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>

FluxRenderQueueWidget::FluxRenderQueueWidget(QWidget* parent):QWidget(parent){
    auto* root=new QVBoxLayout(this); m_list=new QListWidget; root->addWidget(m_list,1);
    auto* row=new QHBoxLayout; auto*up=new QPushButton("↑");auto*down=new QPushButton("↓");auto*remove=new QPushButton("Remove");auto*clear=new QPushButton("Clear");m_render=new QPushButton("Render Queue");
    for(auto*b:{up,down,remove,clear,m_render})row->addWidget(b);root->addLayout(row);
    connect(up,&QPushButton::clicked,this,&FluxRenderQueueWidget::moveSelectedUp);connect(down,&QPushButton::clicked,this,&FluxRenderQueueWidget::moveSelectedDown);connect(remove,&QPushButton::clicked,this,&FluxRenderQueueWidget::removeSelected);connect(clear,&QPushButton::clicked,this,&FluxRenderQueueWidget::clearQueue);connect(m_render,&QPushButton::clicked,this,&FluxRenderQueueWidget::renderAll);
}
void FluxRenderQueueWidget::addJob(const FluxRenderJob&job){m_queue.enqueue(job);refresh();}
void FluxRenderQueueWidget::removeSelected(){const int r=m_list->currentRow();if(r<0)return;auto jobs=m_queue.jobs();jobs.removeAt(r);m_queue.setJobs(jobs);refresh();}
void FluxRenderQueueWidget::clearQueue(){m_queue.clear();refresh();}
void FluxRenderQueueWidget::moveSelectedUp(){const int r=m_list->currentRow();if(r<=0)return;auto jobs=m_queue.jobs();qSwap(jobs[r],jobs[r-1]);m_queue.setJobs(jobs);refresh();m_list->setCurrentRow(r-1);}
void FluxRenderQueueWidget::moveSelectedDown(){const int r=m_list->currentRow();if(r<0||r>=m_list->count()-1)return;auto jobs=m_queue.jobs();qSwap(jobs[r],jobs[r+1]);m_queue.setJobs(jobs);refresh();m_list->setCurrentRow(r+1);}
void FluxRenderQueueWidget::renderAll(){emit renderRequested();}
void FluxRenderQueueWidget::refresh(){m_list->clear();for(const auto&j:m_queue.jobs())m_list->addItem(QString("%1  •  %2  •  %3").arg(j.name,j.format,j.output));}
