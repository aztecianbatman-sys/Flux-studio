#include "mainwindow.h"
#include "canvaswidget.h"
#include "fluxdocument.h"
#include "brusheditor.h"
#include "fluxbrush.h"

#include <QAction>
#include <QActionGroup>
#include <QColorDialog>
#include <QDockWidget>
#include <QFileDialog>
#include <QFileInfo>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QMessageBox>
#include <QMenuBar>
#include <QPushButton>
#include <QSettings>
#include <QStandardPaths>
#include <QStyleFactory>
#include <QTimer>
#include <QToolBar>
#include <QToolButton>
#include <QTreeWidget>
#include <QVBoxLayout>

namespace { QLabel* panelTitle(const QString& t){auto*l=new QLabel(t);l->setObjectName("panelTitle");return l;} }

FluxMainWindow::FluxMainWindow(QWidget* parent):QMainWindow(parent),m_document(new FluxDocument),m_playTimer(new QTimer(this)){
    setWindowTitle("Flux Studio — Untitled");resize(1600,980);setDockNestingEnabled(true);m_document->ensureFrame(0);
    m_canvas=new FluxCanvas(this);m_canvas->setDocument(m_document);setCentralWidget(m_canvas);
    connect(m_canvas,&FluxCanvas::documentChanged,this,&FluxMainWindow::markModified);connect(m_canvas,&FluxCanvas::cursorInfoChanged,this,[this](const QString&s){if(m_cursorLabel)m_cursorLabel->setText(s);});
    connect(m_playTimer,&QTimer::timeout,this,[this]{int f=m_document->frame()+1;if(f>=m_document->frameCount())f=m_playing?0:m_document->frameCount()-1;m_document->setFrame(f);if(m_frames)m_frames->setCurrentRow(f);m_canvas->update();});
    m_playTimer->setInterval(1000/24);buildMenus();buildToolbar();buildDocks();restoreLastSession();autosave();m_playTimer->stop();polish();
    m_autosavePath=QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)+QStringLiteral("/recovery.flux");QTimer* timer=new QTimer(this);timer->setInterval(30000);connect(timer,&QTimer::timeout,this,&FluxMainWindow::autosave);timer->start();
}
FluxMainWindow::~FluxMainWindow(){autosave();delete m_document;}

void FluxMainWindow::buildMenus(){
    auto* file=menuBar()->addMenu("File");file->addAction("New Project",this,&FluxMainWindow::newProject,QKeySequence("Ctrl+N"));file->addAction("Open…",this,&FluxMainWindow::openProject,QKeySequence::Open);file->addSeparator();file->addAction("Save",this,&FluxMainWindow::saveProject,QKeySequence::Save);file->addAction("Save As…",this,&FluxMainWindow::saveProjectAs,QKeySequence("Ctrl+Shift+S"));file->addSeparator();file->addAction("Export Image…",this,&FluxMainWindow::exportImage);file->addSeparator();file->addAction("Quit",this,&QWidget::close,QKeySequence::Quit);
    auto* edit=menuBar()->addMenu("Edit");edit->addAction("Undo",this,&FluxMainWindow::undo,QKeySequence::Undo);edit->addAction("Redo",this,&FluxMainWindow::redo,QKeySequence::Redo);edit->addSeparator();edit->addAction("Select All",this,&FluxMainWindow::selectAll,QKeySequence("Ctrl+A"));edit->addAction("Deselect",this,&FluxMainWindow::deselect,QKeySequence("Ctrl+Shift+A"));edit->addSeparator();edit->addAction("Brush Editor…",this,&FluxMainWindow::openBrushEditor);
    auto* canvas=menuBar()->addMenu("Canvas");canvas->addAction("Fit Canvas",this,&FluxMainWindow::fitCanvas);canvas->addAction("Mirror Horizontal",this,&FluxMainWindow::mirrorHorizontal);canvas->addAction("Mirror Vertical",this,&FluxMainWindow::mirrorVertical);canvas->addAction("Rotate 90°",this,&FluxMainWindow::rotateCanvas);canvas->addAction("Pixel Perfect",this,[this](bool){m_canvas->setPixelPerfect(true);},Qt::Key_P);
    auto* select=menuBar()->addMenu("Select");select->addAction("Rectangle Select",this,[this]{m_canvas->setTool("Rectangle Select");});select->addAction("Lasso Select",this,[this]{m_canvas->setTool("Lasso Select");});select->addAction("Contiguous Select",this,[this]{m_canvas->setTool("Contiguous Select");});select->addAction("Transform",this,[this]{m_canvas->setTool("Transform");});
    auto* anim=menuBar()->addMenu("Animation");anim->addAction("Previous Frame",this,[this]{m_document->setFrame(qMax(0,m_document->frame()-1));refreshTimeline();m_canvas->update();},QKeySequence("Left"));anim->addAction("Next Frame",this,[this]{m_document->setFrame(qMin(m_document->frameCount()-1,m_document->frame()+1));refreshTimeline();m_canvas->update();},QKeySequence("Right"));anim->addAction("Duplicate Frame",this,[this]{m_document->ensureFrame(m_document->frame());for(auto& l:m_document->layers()){if(l.frames.size()<m_document->frameCount())l.frames.resize(m_document->frameCount());int n=m_document->frame();int next=qMin(m_document->frameCount()-1,n+1);l.frames[next]=l.frames[n];}m_document->setFrame(qMin(m_document->frameCount()-1,m_document->frame()+1));refreshTimeline();m_canvas->update();});anim->addAction("Toggle Onion Skin",this,&FluxMainWindow::toggleOnionSkin);
    menuBar()->addMenu("Layer")->addAction("Add Paint Layer",this,&FluxMainWindow::addLayer);auto* window=menuBar()->addMenu("Window");window->addAction("Brush Editor…",this,&FluxMainWindow::openBrushEditor);window->addAction("Reset Workspace",this,[this]{QSettings s("Flux","Flux Studio");s.clear();statusBar()->showMessage("Workspace reset; restart to apply",3000);});menuBar()->addMenu("Filter");menuBar()->addMenu("Help");
}

void FluxMainWindow::buildToolbar(){auto*tb=addToolBar("Flux Tools");tb->setMovable(false);tb->setIconSize(QSize(22,22));
    const QStringList tools={"Brush","Pencil","Ink","Airbrush","Marker","Eraser","Rectangle Select","Lasso Select","Contiguous Select","Transform","Fill","Color Picker","Zoom","Pan"};
    QActionGroup* group=new QActionGroup(this);group->setExclusive(true);
    for(const auto&t:tools){auto*a=tb->addAction(t);a->setCheckable(true);a->setData(t);group->addAction(a);connect(a,&QAction::triggered,this,&FluxMainWindow::setToolFromAction);if(t=="Brush")a->setChecked(true);}
    tb->addSeparator();auto* color=tb->addAction("Color");connect(color,&QAction::triggered,this,&FluxMainWindow::chooseColor);m_colorSwatch=new QLabel; m_colorSwatch->setFixedSize(24,24);m_colorSwatch->setStyleSheet("background:#111;border:1px solid #444;border-radius:5px");tb->addWidget(m_colorSwatch);
    tb->addSeparator();tb->addWidget(new QLabel(" Size "));m_brushSlider=new QSlider(Qt::Horizontal);m_brushSlider->setRange(1,1000);m_brushSlider->setValue(24);m_brushSlider->setFixedWidth(160);tb->addWidget(m_brushSlider);connect(m_brushSlider,&QSlider::valueChanged,this,&FluxMainWindow::updateBrushSize);m_brushSizeLabel=new QLabel("24 px");tb->addWidget(m_brushSizeLabel);
}

void FluxMainWindow::buildDocks(){
    auto*layers=new QDockWidget("Layers",this);layers->setObjectName("LayersDock");layers->setWidget(makeLayersPanel());addDockWidget(Qt::RightDockWidgetArea,layers);
    auto*inspector=new QDockWidget("Inspector",this);inspector->setObjectName("InspectorDock");inspector->setWidget(makeInspectorPanel());tabifyDockWidget(layers,inspector);inspector->raise();
    auto*timeline=new QDockWidget("Timeline",this);timeline->setObjectName("TimelineDock");timeline->setWidget(makeTimelinePanel());addDockWidget(Qt::BottomDockWidgetArea,timeline);timeline->setMinimumHeight(250);
}

QWidget* FluxMainWindow::makeLayersPanel(){auto*root=new QWidget;auto*lay=new QVBoxLayout(root);lay->setContentsMargins(10,10,10,10);lay->addWidget(panelTitle("LAYERS"));m_layers=new QTreeWidget;m_layers->setHeaderLabels({"Layer","Opacity"});m_layers->header()->setStretchLastSection(false);m_layers->header()->setSectionResizeMode(0,QHeaderView::Stretch);m_layers->header()->setSectionResizeMode(1,QHeaderView::ResizeToContents);lay->addWidget(m_layers,1);connect(m_layers,&QTreeWidget::itemSelectionChanged,this,&FluxMainWindow::layerSelectionChanged);auto*row=new QHBoxLayout;for(auto text:{"+ Layer","Duplicate","Delete"}){auto*b=new QPushButton(text);row->addWidget(b);if(text==QString("+ Layer"))connect(b,&QPushButton::clicked,this,&FluxMainWindow::addLayer);else if(text==QString("Duplicate"))connect(b,&QPushButton::clicked,this,&FluxMainWindow::duplicateLayer);else connect(b,&QPushButton::clicked,this,&FluxMainWindow::removeLayer);}lay->addLayout(row);return root;}

QWidget* FluxMainWindow::makeInspectorPanel(){auto*root=new QWidget;auto*lay=new QVBoxLayout(root);lay->setContentsMargins(12,12,12,12);lay->addWidget(panelTitle("INSPECTOR"));auto*brush=new QGroupBox("Brush");auto*f=new QFormLayout(brush);auto*flow=new QSlider(Qt::Horizontal);flow->setRange(0,100);flow->setValue(100);auto*stab=new QSlider(Qt::Horizontal);stab->setRange(0,100);stab->setValue(12);connect(stab,&QSlider::valueChanged,this,[this](int v){m_canvas->setStabilization(v/100.0);});f->addRow("Flow",flow);f->addRow("Stabilizer",stab);lay->addWidget(brush);auto*canvas=new QGroupBox("Canvas View");auto*cf=new QFormLayout(canvas);auto*onion=new QPushButton("Onion Skin");onion->setCheckable(true);onion->setChecked(true);connect(onion,&QPushButton::toggled,this,[this](bool v){m_canvas->toggleOnionSkin(v);});auto*fit=new QPushButton("Fit");connect(fit,&QPushButton::clicked,this,&FluxMainWindow::fitCanvas);cf->addRow(onion,fit);lay->addWidget(canvas);lay->addStretch();return root;}

QWidget* FluxMainWindow::makeTimelinePanel(){auto*root=new QWidget;auto*lay=new QVBoxLayout(root);lay->setContentsMargins(10,8,10,8);auto*head=new QHBoxLayout;head->addWidget(panelTitle("TIMELINE"));head->addWidget(new QLabel("FPS"));m_fps=new QSpinBox;m_fps->setRange(1,240);m_fps->setValue(24);head->addWidget(m_fps);connect(m_fps,qOverload<int>(&QSpinBox::valueChanged),this,[this](int v){m_playTimer->setInterval(qMax(1,1000/v));});head->addStretch();auto*play=new QPushButton("▶ Play");connect(play,&QPushButton::clicked,this,&FluxMainWindow::togglePlayback);head->addWidget(play);lay->addLayout(head);m_frames=new QListWidget;for(int i=0;i<m_document->frameCount();++i)m_frames->addItem(QString("%1  %2").arg(i+1,3,10,QChar('0')).arg(i%3==0?"◆":"·"));m_frames->setFlow(QListView::LeftToRight);m_frames->setWrapping(false);m_frames->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);connect(m_frames,&QListWidget::currentRowChanged,this,&FluxMainWindow::setFrameFromTimeline);lay->addWidget(m_frames,1);return root;}

void FluxMainWindow::refreshLayers(){if(!m_layers)return;m_layers->clear();for(int i=m_document->layers().size()-1;i>=0;--i){const auto&l=m_document->layers()[i];auto*item=new QTreeWidgetItem(m_layers,{(l.visible?"● ":"○ ")+l.name,QString::number(int(l.opacity*100))+"%"});item->setData(0,Qt::UserRole,i);item->setCheckState(0,l.visible?Qt::Checked:Qt::Unchecked);}m_layers->setCurrentItem(m_layers->topLevelItem(m_document->layers().size()-1-m_document->activeLayerIndex()));}
void FluxMainWindow::refreshTimeline(){if(!m_frames)return;QSignalBlocker b(m_frames);m_frames->setCurrentRow(m_document->frame());}
void FluxMainWindow::layerSelectionChanged(){if(!m_layers||m_layers->currentItem()==nullptr)return;const int i=m_layers->currentItem()->data(0,Qt::UserRole).toInt();m_document->setActiveLayer(i);m_canvas->update();}
void FluxMainWindow::addLayer(){m_document->addLayer("Paint Layer "+QString::number(m_document->layers().size()));refreshLayers();markModified();m_canvas->update();}
void FluxMainWindow::duplicateLayer(){int i=m_document->activeLayerIndex();m_document->duplicateLayer(i);refreshLayers();markModified();m_canvas->update();}
void FluxMainWindow::removeLayer(){int i=m_document->activeLayerIndex();m_document->removeLayer(i);refreshLayers();markModified();m_canvas->update();}
void FluxMainWindow::setFrameFromTimeline(int row){if(row<0)return;m_document->setFrame(row);refreshTimeline();m_canvas->update();}
void FluxMainWindow::togglePlayback(){m_playing=!m_playing;if(m_playing)m_playTimer->start();else m_playTimer->stop();}
void FluxMainWindow::updateBrushSize(int value){m_canvas->setBrushSize(value);if(m_brushSizeLabel)m_brushSizeLabel->setText(QString::number(value)+" px");}
void FluxMainWindow::chooseColor(){const QColor c=QColorDialog::getColor(m_document->foreground(),this,"Flux Color");if(c.isValid()){m_document->setForeground(c);m_canvas->setBrushColor(c);if(m_colorSwatch)m_colorSwatch->setStyleSheet(QString("background:%1;border:1px solid #444;border-radius:5px").arg(c.name()));}}
void FluxMainWindow::setToolFromAction(){auto*a=qobject_cast<QAction*>(sender());if(a)m_canvas->setTool(a->data().toString());}
void FluxMainWindow::openBrushEditor(){BrushEditorDialog dlg(m_canvas->brushEngine(),this);if(dlg.exec()==QDialog::Accepted){m_canvas->setBrushSize(m_canvas->brushEngine()->preset().size);m_canvas->update();}}
void FluxMainWindow::mirrorHorizontal(){m_mirrorH=!m_mirrorH;m_canvas->setMirrorHorizontal(m_mirrorH);}
void FluxMainWindow::mirrorVertical(){m_mirrorV=!m_mirrorV;m_canvas->setMirrorVertical(m_mirrorV);}
void FluxMainWindow::rotateCanvas(){m_canvasRotation+=90;if(m_canvasRotation>=360)m_canvasRotation=0;m_canvas->setCanvasRotation(m_canvasRotation);}
void FluxMainWindow::fitCanvas(){m_canvas->fitCanvas();}
void FluxMainWindow::toggleOnionSkin(){m_canvas->toggleOnionSkin(!m_canvas->onionSkin());}
void FluxMainWindow::selectAll(){m_canvas->selectAll();}
void FluxMainWindow::deselect(){m_canvas->clearSelection();}
void FluxMainWindow::undo(){m_canvas->undo();}
void FluxMainWindow::redo(){m_canvas->redo();}

void FluxMainWindow::newProject(){m_document->create("Untitled",1920,1080);m_filePath.clear();m_canvas->setDocument(m_document);refreshLayers();refreshTimeline();setWindowTitle("Flux Studio — Untitled");setStatus("New project");}
void FluxMainWindow::openProject(){const QString f=QFileDialog::getOpenFileName(this,"Open Flux Project",{},"Flux Project (*.flux)");if(f.isEmpty())return;QString err;if(!m_document->load(f,&err)){QMessageBox::critical(this,"Flux Studio",err);return;}m_filePath=f;m_canvas->setDocument(m_document);refreshLayers();refreshTimeline();setWindowTitle("Flux Studio — "+QFileInfo(f).completeBaseName());setStatus("✓  Opened");}
void FluxMainWindow::saveProject(){if(m_filePath.isEmpty())saveProjectAs();else{QString err;if(!m_document->save(m_filePath,&err))QMessageBox::critical(this,"Save failed",err);else setStatus("✓  Saved");}}
void FluxMainWindow::saveProjectAs(){const QString f=QFileDialog::getSaveFileName(this,"Save Flux Project",{},"Flux Project (*.flux)");if(f.isEmpty())return;m_filePath=f;if(!f.endsWith(".flux",Qt::CaseInsensitive))m_filePath+=".flux";saveProject();}
void FluxMainWindow::exportImage(){const QString f=QFileDialog::getSaveFileName(this,"Export Image",{},"PNG (*.png);;JPEG (*.jpg *.jpeg);;WebP (*.webp)");if(f.isEmpty())return;QString err;if(!m_document->exportImage(f,&err))QMessageBox::critical(this,"Export failed",err);else setStatus("✓  Exported");}
void FluxMainWindow::autosave(){if(!m_document||m_autosavePath.isEmpty())return;QDir dir(QFileInfo(m_autosavePath).absolutePath());if(!dir.exists())dir.mkpath(".");m_document->save(m_autosavePath,nullptr);}
void FluxMainWindow::restoreLastSession(){QSettings s("Flux","Flux Studio");restoreGeometry(s.value("geometry").toByteArray());restoreState(s.value("state").toByteArray());refreshLayers();refreshTimeline();}
void FluxMainWindow::markModified(){setWindowTitle(QString("Flux Studio — %1*").arg(m_filePath.isEmpty()?QStringLiteral("Untitled"):QFileInfo(m_filePath).completeBaseName()));setStatus("●  Unsaved");}
void FluxMainWindow::setStatus(const QString& text){if(m_statusLabel)m_statusLabel->setText(text);else statusBar()->showMessage(text);}
void FluxMainWindow::polish(){setStyleSheet(R"(QMainWindow,QWidget{background:#14161a;color:#e7e9ed;font-family:"Segoe UI";font-size:13px}QMenuBar{background:#191b20;border-bottom:1px solid #2a2d34;padding:4px 8px}QMenuBar::item{padding:7px 10px;border-radius:6px}QMenuBar::item:selected{background:#292e38}QMenu{background:#1c1f25;border:1px solid #353a44;padding:5px}QMenu::item{padding:7px 24px 7px 10px;border-radius:5px}QMenu::item:selected{background:#2b303a}QToolBar{background:#181a1f;border:0;border-bottom:1px solid #2b2e35;spacing:4px;padding:6px}QToolBar QToolButton{min-width:70px;min-height:32px;border:1px solid transparent;border-radius:7px}QToolBar QToolButton:hover{background:#252a33}QToolBar QToolButton:checked{background:#353b47;border-color:#4a5261}QDockWidget{border:1px solid #292d34}QDockWidget::title{background:#1b1e23;padding:9px 12px;border-bottom:1px solid #2b2e35}QTreeWidget,QListWidget{background:#17191e;border:1px solid #2a2e36;border-radius:8px}QTreeWidget::item,QListWidget::item{padding:6px;border-radius:5px}QTreeWidget::item:selected,QListWidget::item:selected{background:#303641}QPushButton{background:#252a33;border:1px solid #363b45;padding:7px 11px;border-radius:7px}QPushButton:hover{background:#303640}QSlider::groove:horizontal{height:4px;background:#353a44;border-radius:2px}QSlider::handle:horizontal{width:14px;margin:-5px 0;border-radius:7px;background:#e6e8ec}QSpinBox,QDoubleSpinBox{background:#1e2229;border:1px solid #363b45;border-radius:6px;padding:4px 7px}#panelTitle{font-size:11px;font-weight:700;letter-spacing:1.2px;color:#a4aab5}QStatusBar{background:#17191e;border-top:1px solid #2a2e36})");}
