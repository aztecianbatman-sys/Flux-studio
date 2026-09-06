#include "fluxproductiondock.h"
#include "fluxdocument.h"
#include "canvaswidget.h"
#include "fluxmedia.h"
#include "fluxexport.h"
#include "fluxperformance.h"
#include "fluxworkflow.h"
#include "fluxrecovery.h"
#include "fluxtabletprofiles.h"
#include "fluxprojectpackage.h"
#include "fluxrenderqueuewidget.h"
#include <QCheckBox>
#include <QComboBox>
#include <QCoreApplication>
#include <QDateTime>
#include <QFileDialog>
#include <QFileInfo>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMessageBox>
#include <QPushButton>
#include <QSettings>
#include <QSlider>
#include <QSpinBox>
#include <QTabWidget>
#include <QTimer>
#include <QVBoxLayout>

namespace { QLabel* text(const QString&s){auto*l=new QLabel(s);l->setWordWrap(true);return l;} QPushButton* action(const QString&s){auto*b=new QPushButton(s);b->setMinimumHeight(34);return b;}}

FluxProductionDock::FluxProductionDock(FluxDocument*document,FluxCanvas*canvas,QWidget*parent):QWidget(parent),m_document(document),m_canvas(canvas){
    m_tabs=new QTabWidget(this);m_tabs->addTab(buildProjectTab(),"Project");m_tabs->addTab(buildAnimationTab(),"Animation");m_tabs->addTab(buildMediaTab(),"Media");m_tabs->addTab(buildExportTab(),"Export");m_tabs->addTab(buildPerformanceTab(),"Performance");m_tabs->addTab(buildInputTab(),"Input");m_tabs->addTab(buildWorkspaceTab(),"Workspace");
    auto*root=new QVBoxLayout(this);root->setContentsMargins(8,8,8,8);root->addWidget(m_tabs);
    setStyleSheet("QGroupBox{border:1px solid #29313b;border-radius:8px;margin-top:9px;padding-top:10px}QGroupBox::title{subcontrol-origin:margin;left:9px;padding:0 4px;color:#8d99aa;font-size:11px;font-weight:700}QPushButton{background:#18212b;border:1px solid #303c4b;border-radius:7px;padding:7px 10px}QPushButton:hover{background:#24303d}QComboBox,QSpinBox,QLineEdit{background:#141b23;border:1px solid #303c4b;border-radius:6px;padding:5px}QListWidget{background:#0f151c;border:1px solid #27313d;border-radius:7px}");
}

QWidget*FluxProductionDock::buildProjectTab(){
    auto*w=new QWidget;auto*l=new QVBoxLayout(w);l->addWidget(text("Project storage, recovery and packaging. Flux keeps editor state separate from workspace UI."));
    auto*g=new QGroupBox("Project");auto*gl=new QVBoxLayout(g);auto*newB=action("New project…");auto*openB=action("Open project…");auto*saveB=action("Save project");auto*packageB=action("Package as single-file .flux");auto*openPackageB=action("Open single-file package…");for(auto*b:{newB,openB,saveB,packageB,openPackageB})gl->addWidget(b);
    connect(newB,&QPushButton::clicked,this,&FluxProductionDock::requestNewProject);connect(openB,&QPushButton::clicked,this,&FluxProductionDock::requestOpenProject);connect(saveB,&QPushButton::clicked,this,&FluxProductionDock::requestSaveProject);
    connect(packageB,&QPushButton::clicked,this,[this]{const auto f=QFileDialog::getSaveFileName(this,"Package Flux Project",{},"Flux Package (*.flux)");if(f.isEmpty())return;QString e;if(!FluxProjectPackage::save(f,*m_document,&e))QMessageBox::critical(this,"Package failed",e);else QMessageBox::information(this,"Flux Studio","Self-contained package written.\n"+f);});
    connect(openPackageB,&QPushButton::clicked,this,[this]{const auto f=QFileDialog::getOpenFileName(this,"Open Flux Package",{},"Flux Package (*.flux)");if(f.isEmpty())return;QString e;if(!FluxProjectPackage::load(f,*m_document,&e))QMessageBox::critical(this,"Open package failed",e);else{m_canvas->setDocument(m_document);m_canvas->fitCanvas();m_canvas->update();}});l->addWidget(g);
    auto*r=new QGroupBox("Recovery");auto*rl=new QVBoxLayout(r);auto*id=new QLineEdit;id->setPlaceholderText("Project recovery id");auto*recover=new QPushButton("Inspect snapshots");auto*list=new QListWidget;rl->addWidget(id);rl->addWidget(recover);rl->addWidget(list,1);connect(recover,&QPushButton::clicked,this,[id,list]{list->clear();const QString pid=id->text().isEmpty()?QStringLiteral("untitled"):id->text();const auto s=FluxRecoveryManager::listSnapshots(pid);for(const auto&p:s)list->addItem(p);if(s.isEmpty())list->addItem("No recovery snapshots");});l->addWidget(r,1);l->addWidget(new QLabel("Autosave • atomic writes • rotating recovery snapshots"));return w;
}
QWidget*FluxProductionDock::buildAnimationTab(){
    auto*w=new QWidget;auto*l=new QVBoxLayout(w);l->addWidget(text("Exposure-sheet timing, keyframes, holds, markers and playback controls."));auto*g=new QGroupBox("Timing");auto*f=new QFormLayout(g);m_exportFps=new QSpinBox;m_exportFps->setRange(1,240);m_exportFps->setValue(24);auto*frames=new QSpinBox;frames->setRange(1,10000);frames->setValue(m_document?m_document->frameCount():120);f->addRow("FPS",m_exportFps);f->addRow("Frame count",frames);l->addWidget(g);
    auto*ops=new QGroupBox("Animation tools");auto*ol=new QVBoxLayout(ops);auto*prev=action("Previous frame");auto*next=action("Next frame");auto*dup=action("Duplicate current frame");auto*ins=action("Insert frame");auto*del=action("Delete frame");auto*marker=action("Add marker at current frame");auto*onion=new QCheckBox("Onion skin");onion->setChecked(true);for(auto*b:{prev,next,dup,ins,del,marker})ol->addWidget(b);ol->addWidget(onion);
    connect(prev,&QPushButton::clicked,this,[this]{m_document->setFrame(qMax(0,m_document->frame()-1));m_canvas->update();});connect(next,&QPushButton::clicked,this,[this]{m_document->setFrame(qMin(m_document->frameCount()-1,m_document->frame()+1));m_canvas->update();});connect(dup,&QPushButton::clicked,this,[this]{const int f=m_document->frame();m_document->setFrameCount(m_document->frameCount()+1);for(auto&layer:m_document->layers()){if(layer.frames.size()!=m_document->frameCount())layer.frames.resize(m_document->frameCount());if(f+1<layer.frames.size())layer.frames[f+1]=layer.frames[f];}m_document->setFrame(f+1);m_canvas->update();});connect(ins,&QPushButton::clicked,this,[this]{m_document->setFrameCount(m_document->frameCount()+1);m_canvas->update();});connect(del,&QPushButton::clicked,this,[this]{if(m_document->frameCount()>1){m_document->setFrameCount(m_document->frameCount()-1);m_document->setFrame(qMin(m_document->frame(),m_document->frameCount()-1));m_canvas->update();}});connect(onion,&QCheckBox::toggled,this,[this](bool on){m_canvas->toggleOnionSkin(on);});connect(marker,&QPushButton::clicked,this,[this,marker]{marker->setText(QString("Marker • Frame %1").arg(m_document->frame()+1));});l->addWidget(ops,1);return w;
}
QWidget*FluxProductionDock::buildMediaTab(){
    auto*w=new QWidget;auto*l=new QVBoxLayout(w);l->addWidget(text("Audio/video inspection and waveform-ready media support via FFmpeg/FFprobe."));auto*bar=new QHBoxLayout;auto*audio=action("Import audio…");auto*video=action("Import video…");bar->addWidget(audio);bar->addWidget(video);l->addLayout(bar);m_mediaList=new QListWidget;l->addWidget(m_mediaList,1);m_mediaInfo=text("Select media to inspect.");l->addWidget(m_mediaInfo);
    connect(audio,&QPushButton::clicked,this,[this]{const auto f=QFileDialog::getOpenFileName(this,"Import Audio",{},"Audio (*.wav *.mp3 *.flac *.ogg *.m4a)");if(f.isEmpty())return;FluxAudioInfo info;QString e;if(!FluxMediaEngine::inspectAudio(f,&info,&e)){QMessageBox::warning(this,"Audio",e);return;}auto*i=new QListWidgetItem(QString("AUDIO • %1 • %2 s").arg(QFileInfo(f).fileName()).arg(info.duration,0,'f',2),m_mediaList);i->setData(Qt::UserRole,f);});
    connect(video,&QPushButton::clicked,this,[this]{const auto f=QFileDialog::getOpenFileName(this,"Import Video",{},"Video (*.mp4 *.webm *.mov *.mkv)");if(f.isEmpty())return;FluxVideoInfo info;QString e;if(!FluxMediaEngine::inspectVideo(f,&info,&e)){QMessageBox::warning(this,"Video",e);return;}auto*i=new QListWidgetItem(QString("VIDEO • %1 • %2×%3 • %4 fps").arg(QFileInfo(f).fileName()).arg(info.width).arg(info.height).arg(info.fps),m_mediaList);i->setData(Qt::UserRole,f);});connect(m_mediaList,&QListWidget::currentRowChanged,this,[this](int row){if(row>=0)m_mediaInfo->setText(m_mediaList->item(row)->data(Qt::UserRole).toString());});return w;
}
QWidget*FluxProductionDock::buildExportTab(){
    auto*w=new QWidget;auto*l=new QVBoxLayout(w);l->addWidget(text("Render presets, direct export and a persistent interactive queue with progress and cancellation."));auto*g=new QGroupBox("Render settings");auto*f=new QFormLayout(g);m_exportFormat=new QComboBox;m_exportFormat->addItems({"PNG","JPEG","WebP","PNG Sequence","Sprite Sheet","GIF","MP4","WebM"});m_exportWidth=new QSpinBox;m_exportWidth->setRange(1,16384);m_exportWidth->setValue(m_document?m_document->width():1920);m_exportHeight=new QSpinBox;m_exportHeight->setRange(1,16384);m_exportHeight->setValue(m_document?m_document->height():1080);m_exportFps=new QSpinBox;m_exportFps->setRange(1,240);m_exportFps->setValue(24);auto*bitrate=new QSpinBox;bitrate->setRange(100,100000);bitrate->setValue(12000);auto*transparent=new QCheckBox("Preserve transparency");f->addRow("Format",m_exportFormat);f->addRow("Width",m_exportWidth);f->addRow("Height",m_exportHeight);f->addRow("FPS",m_exportFps);f->addRow("Bitrate kbps",bitrate);f->addRow(transparent);l->addWidget(g);
    auto*buttons=new QHBoxLayout;auto*exportB=action("Export Now…");auto*queueB=action("Add to Queue…");buttons->addWidget(exportB);buttons->addWidget(queueB);l->addLayout(buttons);
    m_renderQueue=new FluxRenderQueueWidget; l->addWidget(m_renderQueue,1);
    auto makeJob=[this,transparent,bitrate] (const QString&f)->FluxRenderJob {FluxRenderJob j;j.output=f;j.format=QFileInfo(f).suffix().toLower();j.name=QFileInfo(f).completeBaseName();j.settings.width=m_exportWidth->value();j.settings.height=m_exportHeight->value();j.settings.fps=m_exportFps->value();j.settings.startFrame=0;j.settings.endFrame=m_document->frameCount()-1;j.settings.bitrateKbps=bitrate->value();j.settings.transparent=transparent->isChecked();j.settings.codec=j.format=="webm"?"libvpx-vp9":"libx264";return j;};
    connect(queueB,&QPushButton::clicked,this,[this,makeJob]{const auto f=QFileDialog::getSaveFileName(this,"Add Render Job",{},"Media (*.png *.jpg *.jpeg *.webp *.gif *.mp4 *.webm)");if(f.isEmpty())return;m_renderQueue->addJob(makeJob(f));});
    connect(exportB,&QPushButton::clicked,this,[this,makeJob]{const auto f=QFileDialog::getSaveFileName(this,"Export Render",{},"Media (*.png *.jpg *.jpeg *.webp *.gif *.mp4 *.webm)");if(f.isEmpty())return;auto j=makeJob(f);QString e;const bool still=j.format=="png"||j.format=="jpg"||j.format=="jpeg"||j.format=="webp";const bool ok=still?FluxExportEngine::exportImage(m_document->composite(),f,j.settings,&e):FluxExportEngine::exportAnimated([this](int frame){m_document->setFrame(frame);return m_document->composite();},j,&e);if(!ok)QMessageBox::critical(this,"Export failed",e);else QMessageBox::information(this,"Flux Studio","Render complete.");});
    connect(m_renderQueue,&FluxRenderQueueWidget::renderRequested,this,[this]{if(m_renderQueue->queue().count()==0){QMessageBox::information(this,"Render Queue","Queue is empty.");return;}m_renderQueue->setRunning(true);m_renderQueue->setProgress(0.0,"Rendering queue…");QString e;auto&queue=m_renderQueue->queue();const int total=queue.count();int finished=0;const bool ok=queue.process([this,&queue,total,&finished](int frame)->QImage{QCoreApplication::processEvents();if(queue.cancelled())return QImage();m_document->setFrame(frame);const auto image=m_document->composite();const double p=total>0?(double(finished)+double(frame+1)/double(qMax(1,m_document->frameCount())))/double(total):1.0;m_renderQueue->setProgress(p,QString("Rendering • %1%%").arg(int(p*100)));QCoreApplication::processEvents();return image;},&e);Q_UNUSED(ok);finished=total;m_renderQueue->setRunning(false);m_renderQueue->setProgress(ok?1.0:0.0,ok?QStringLiteral("Queue complete"):QStringLiteral("Queue stopped"));m_renderQueue->refreshQueue();if(!ok&&!e.isEmpty())QMessageBox::warning(this,"Render Queue",e);});
    connect(m_renderQueue,&FluxRenderQueueWidget::cancelRequested,this,[this]{m_renderQueue->queue().cancel();m_renderQueue->setProgress(m_renderQueue->queue().count()?0.0:0.0,"Cancellation requested…");});
    return w;
}
QWidget*FluxProductionDock::buildPerformanceTab(){
    auto*w=new QWidget;auto*l=new QVBoxLayout(w);l->addWidget(text("Renderer diagnostics and preview-quality controls."));m_proxyMode=new QCheckBox("Proxy / reduced-resolution preview");m_cacheEnabled=new QCheckBox("Tile cache");m_cacheEnabled->setChecked(true);auto*background=new QCheckBox("Background-friendly rendering");auto*hud=new QCheckBox("Live performance monitor");l->addWidget(m_proxyMode);l->addWidget(m_cacheEnabled);l->addWidget(background);l->addWidget(hud);m_performance=new QLabel;l->addWidget(m_performance);auto*t=new QTimer(this);connect(t,&QTimer::timeout,this,[this]{refreshPerformance();});t->start(1000);return w;
}
QWidget*FluxProductionDock::buildInputTab(){
    auto*w=new QWidget;auto*l=new QVBoxLayout(w);l->addWidget(text("Tablet pressure, tilt, rotation, eraser/barrel mappings and Quick Wheel settings."));auto*g=new QGroupBox("Quick Wheel");auto*f=new QFormLayout(g);m_wheelRadius=new QSlider(Qt::Horizontal);m_wheelRadius->setRange(120,240);m_wheelRadius->setValue(QSettings("Flux","Flux Studio").value("wheel/radius",158).toInt());f->addRow("Radius",m_wheelRadius);connect(m_wheelRadius,&QSlider::valueChanged,[](int v){QSettings("Flux","Flux Studio").setValue("wheel/radius",v);});l->addWidget(g);auto*t=new QGroupBox("Tablet profiles");auto*tl=new QVBoxLayout(t);auto*profiles=new QListWidget;for(const auto&p:FluxTabletProfiles::profiles())profiles->addItem(QString("%1 • %2").arg(p.name,p.driver));if(profiles->count()==0)profiles->addItem("No saved device profiles");tl->addWidget(profiles);auto*ink=new QCheckBox("Use Windows Ink when available");ink->setChecked(QSettings("Flux","Flux Studio").value("input/windowsInk",true).toBool());tl->addWidget(ink);connect(ink,&QCheckBox::toggled,[](bool on){QSettings("Flux","Flux Studio").setValue("input/windowsInk",on);});l->addWidget(t);l->addStretch();return w;
}
QWidget*FluxProductionDock::buildWorkspaceTab(){
    auto*w=new QWidget;auto*l=new QVBoxLayout(w);l->addWidget(text("Named workspaces and UI persistence."));auto*name=new QLineEdit;name->setPlaceholderText("Workspace name");auto*save=action("Save workspace");auto*load=action("Activate selected workspace");auto*list=new QListWidget;list->addItems(FluxWorkflow::workspaceNames());l->addWidget(name);l->addWidget(save);l->addWidget(load);l->addWidget(list,1);connect(save,&QPushButton::clicked,this,[this,name,list]{const auto n=name->text().trimmed();if(n.isEmpty())return;FluxWorkflow::saveWorkspace(n,QByteArray("flux-workspace-v1"));if(list->findItems(n,Qt::MatchExactly).isEmpty())list->addItem(n);});connect(load,&QPushButton::clicked,this,[list]{if(list->currentItem())QSettings("Flux","Flux Studio").setValue("workspace/active",list->currentItem()->text());});return w;
}
void FluxProductionDock::refresh(){refreshMedia();refreshPerformance();if(m_renderQueue)m_renderQueue->refreshQueue();}
void FluxProductionDock::refreshMedia(){if(m_mediaList)m_mediaList->viewport()->update();}
void FluxProductionDock::refreshPerformance(){if(!m_performance)return;const auto s=FluxPerformance::probe();m_performance->setText(QString("GPU: %1\nRenderer: %2\nTile size: %3\nFPS probe: %4\nFrame time: %5 ms\nMemory: %6 MiB").arg(s.gpuAvailable?"available":"fallback").arg(s.gpuRenderer.isEmpty()?"unknown":s.gpuRenderer).arg(s.tileSize).arg(s.fps,0,'f',1).arg(s.frameMs,0,'f',1).arg(double(s.memoryBytes)/1048576.0,0,'f',1));}
