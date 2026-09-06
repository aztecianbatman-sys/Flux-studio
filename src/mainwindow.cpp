#include "mainwindow.h"
#include "canvaswidget.h"
#include "fluxdocument.h"
#include "fluxlayertree.h"
#include "fluxtimelinewidget.h"
#include "fluxcompositorwidget.h"
#include "fluxexport.h"
#include "fluxcommandpalette.h"
#include "fluxwheel.h"
#include "fluxperformance.h"
#include "fluxworkflow.h"
#include "fluxrecovery.h"
#include "brusheditor.h"
#include "fluxbrush.h"

#include <QAction>
#include <QActionGroup>
#include <QColorDialog>
#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDir>
#include <QDockWidget>
#include <QFileDialog>
#include <QFileInfo>
#include <QFormLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QPushButton>
#include <QSettings>
#include <QSignalBlocker>
#include <QSlider>
#include <QStackedWidget>
#include <QStandardPaths>
#include <QStatusBar>
#include <QTimer>
#include <QToolBar>
#include <QToolButton>
#include <QTreeWidget>
#include <QVBoxLayout>

namespace {
QString esc(const QString& s){return s.toHtmlEscaped();}
QLabel* label(const QString& text,const QString& role=QString(),QWidget* parent=nullptr){auto*l=new QLabel(text,parent);if(!role.isEmpty())l->setProperty("role",role);return l;}
QToolButton* toolButton(const QString& glyph,const QString& tip,QWidget* parent=nullptr){auto*b=new QToolButton(parent);b->setText(glyph);b->setToolTip(tip);b->setCursor(Qt::PointingHandCursor);b->setAutoRaise(true);b->setProperty("railButton",true);return b;}
}

FluxMainWindow::FluxMainWindow(QWidget* parent):QMainWindow(parent),m_document(new FluxDocument),m_playTimer(new QTimer(this)){
    setWindowTitle("Flux Studio — Home");
    resize(1680,1040);
    setMinimumSize(1180,760);
    setDockNestingEnabled(true);
    m_canvas=new FluxCanvas(this);
    m_canvas->setDocument(m_document);
    m_stack=new QStackedWidget(this);
    m_stack->setObjectName("mainStack");
    m_home=new QWidget(m_stack);
    m_stack->addWidget(m_home);
    m_stack->addWidget(m_canvas);
    setCentralWidget(m_stack);

    connect(m_canvas,&FluxCanvas::documentChanged,this,&FluxMainWindow::markModified);
    connect(m_canvas,&FluxCanvas::cursorInfoChanged,this,[this](const QString&s){if(m_cursorLabel)m_cursorLabel->setText(s);});
    connect(m_canvas,&FluxCanvas::zoomChanged,this,&FluxMainWindow::updateZoomLabel);
    connect(m_canvas,&FluxCanvas::wheelRequested,this,[this](const QPoint&p){if(m_wheel)m_wheel->openAt(p);});
    connect(m_playTimer,&QTimer::timeout,this,[this]{int f=m_document->frame()+1;if(f>=m_document->frameCount())f=m_playing?0:m_document->frameCount()-1;m_document->setFrame(f);m_canvas->update();});
    m_playTimer->setInterval(1000/24);

    buildHome();
    buildMenus();
    buildTopBar();
    buildToolRail();
    buildDocks();
    m_wheel=new FluxWheel(this);
    connect(m_wheel,&FluxWheel::commandTriggered,this,[this](int i){switch(i){case 0:m_canvas->setTool("Brush");break;case 1:m_canvas->setTool("Pencil");break;case 2:m_canvas->setTool("Eraser");break;case 3:m_canvas->setTool("Ink");break;case 4:chooseColor();break;case 5:m_document->setFrame(qMax(0,m_document->frame()-1));m_canvas->update();break;case 6:m_document->setFrame(qMin(m_document->frameCount()-1,m_document->frame()+1));m_canvas->update();break;default:fitCanvas();break;}});

    m_autosavePath=QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)+QStringLiteral("/recovery.flux");
    auto*timer=new QTimer(this);timer->setInterval(30000);connect(timer,&QTimer::timeout,this,&FluxMainWindow::autosave);timer->start();
    restoreLastSession();
    m_stack->setCurrentWidget(m_home);
    for(auto*d:findChildren<QDockWidget*>())d->hide();
    if(m_topBar)m_topBar->hide(); if(m_toolRail)m_toolRail->hide();
    polish();
}
FluxMainWindow::~FluxMainWindow(){autosave();if(m_document)delete m_document;}

void FluxMainWindow::buildHome(){
    auto*root=new QVBoxLayout(m_home);root->setContentsMargins(58,48,58,44);root->setSpacing(26);
    auto*head=new QHBoxLayout;
    auto*brand=new QVBoxLayout;brand->setSpacing(1);brand->addWidget(label("FLUX","brand"));brand->addWidget(label("STUDIO  /  0.7","brandSub"));head->addLayout(brand);head->addStretch();head->addWidget(label("LOCAL CREATIVE WORKSTATION","meta"),0,Qt::AlignTop|Qt::AlignRight);root->addLayout(head);
    auto*hero=new QVBoxLayout;hero->setSpacing(8);hero->addWidget(label("YOUR WORKSPACE","eyebrow"));hero->addWidget(label("Start making. Stay in the flow.","headline"));hero->addWidget(label("Professional raster drawing, 2D animation, compositing and export — with a workspace that gets out of the way until you need it.","subhead"));root->addLayout(hero);
    auto*cards=new QGridLayout;cards->setHorizontalSpacing(14);cards->setVerticalSpacing(14);
    auto*create=new QPushButton("NEW PROJECT\n\nChoose canvas, frame rate and working format");create->setObjectName("homePrimary");create->setMinimumHeight(150);create->setCursor(Qt::PointingHandCursor);cards->addWidget(create,0,0,1,2);
    auto*open=new QPushButton("OPEN PROJECT\n\nLoad a .flux document");open->setObjectName("homeCard");open->setMinimumHeight(150);open->setCursor(Qt::PointingHandCursor);cards->addWidget(open,0,2,1,1);
    auto*brush=new QPushButton("BRUSH LAB\n\nBuild, import and edit brush presets");brush->setObjectName("homeCard");brush->setMinimumHeight(110);brush->setCursor(Qt::PointingHandCursor);cards->addWidget(brush,1,0);
    auto*compose=new QPushButton("COMPOSITOR\n\nImage → color → blur → glow → output");compose->setObjectName("homeCard");compose->setMinimumHeight(110);compose->setCursor(Qt::PointingHandCursor);cards->addWidget(compose,1,1);
    auto*animate=new QPushButton("ANIMATION\n\nDope sheet, keyframes, holds and markers");animate->setObjectName("homeCard");animate->setMinimumHeight(110);animate->setCursor(Qt::PointingHandCursor);cards->addWidget(animate,1,2);root->addLayout(cards);
    auto*recentFrame=new QFrame(m_home);recentFrame->setObjectName("homeRecent");auto*recentLay=new QVBoxLayout(recentFrame);recentLay->setContentsMargins(18,16,18,16);recentLay->setSpacing(10);
    auto*rh=new QHBoxLayout;rh->addWidget(label("RECENT PROJECTS","section"));rh->addStretch();auto*refresh=new QPushButton("Refresh");refresh->setObjectName("tinyButton");rh->addWidget(refresh);recentLay->addLayout(rh);
    auto*recent=new QListWidget(recentFrame);recent->setObjectName("homeRecentList");recent->setSelectionMode(QAbstractItemView::SingleSelection);recentLay->addWidget(recent,1);root->addWidget(recentFrame,1);
    auto*foot=new QHBoxLayout;foot->addWidget(label("DRAW  →  ANIMATE  →  COMPOSE  →  EXPORT","footer"));foot->addStretch();foot->addWidget(label("Ctrl+N  New   •   Ctrl+O  Open   •   Ctrl+K  Commands","hint"));root->addLayout(foot);

    auto populate=[recent,this](){recent->clear();const auto paths=FluxWorkflow::recentProjects(12);if(paths.isEmpty()){auto*i=new QListWidgetItem("No recent projects yet — create your first document.");i->setFlags(Qt::NoItemFlags);recent->addItem(i);return;}for(const auto&p:paths){if(!QFileInfo::exists(p))continue;auto*i=new QListWidgetItem(QFileInfo(p).completeBaseName());i->setData(Qt::UserRole,p);i->setToolTip(p);recent->addItem(i);}if(recent->count()==0){auto*i=new QListWidgetItem("No valid recent projects.");i->setFlags(Qt::NoItemFlags);recent->addItem(i);}};
    populate();
    connect(create,&QPushButton::clicked,this,&FluxMainWindow::newProject);connect(open,&QPushButton::clicked,this,&FluxMainWindow::openProject);connect(refresh,&QPushButton::clicked,populate);connect(recent,&QListWidget::itemDoubleClicked,this,[this](QListWidgetItem*i){const auto p=i?i->data(Qt::UserRole).toString():QString();if(!p.isEmpty())loadProjectPath(p);});
    connect(brush,&QPushButton::clicked,this,&FluxMainWindow::openBrushEditor);connect(compose,&QPushButton::clicked,this,[this]{for(auto*d:findChildren<QDockWidget*>())if(d->windowTitle()=="Compose")d->show();enterWorkspace();});connect(animate,&QPushButton::clicked,this,[this]{enterWorkspace();});
}

void FluxMainWindow::buildMenus(){
    auto*file=menuBar()->addMenu("File");file->addAction("New Project",this,&FluxMainWindow::newProject,QKeySequence("Ctrl+N"));file->addAction("Open…",this,&FluxMainWindow::openProject,QKeySequence("Ctrl+O"));file->addSeparator();file->addAction("Save",this,&FluxMainWindow::saveProject,QKeySequence::Save);file->addAction("Save As…",this,&FluxMainWindow::saveProjectAs,QKeySequence("Ctrl+Shift+S"));file->addSeparator();file->addAction("Export Image…",this,&FluxMainWindow::exportImage);file->addAction("Render Animation…",this,[this]{const QString f=QFileDialog::getSaveFileName(this,"Render Animation",{},"MP4 (*.mp4);;WebM (*.webm);;GIF (*.gif)");if(f.isEmpty())return;FluxRenderJob j;j.name=QFileInfo(f).completeBaseName();j.output=f;j.format=QFileInfo(f).suffix().toLower();j.settings={m_document->width(),m_document->height(),m_fps?m_fps->value():24,0,m_document->frameCount()-1,12000,"",false,"sRGB"};QString e;if(!FluxExportEngine::exportAnimated([this](int f){m_document->setFrame(f);return m_document->composite();},j,&e))QMessageBox::critical(this,"Render failed",e);else setStatus("Rendered "+f);});file->addSeparator();file->addAction("Home",this,&FluxMainWindow::returnHome,QKeySequence("Ctrl+Shift+H"));file->addAction("Quit",this,&QWidget::close,QKeySequence::Quit);
    auto*edit=menuBar()->addMenu("Edit");edit->addAction("Undo",this,&FluxMainWindow::undo,QKeySequence::Undo);edit->addAction("Redo",this,&FluxMainWindow::redo,QKeySequence::Redo);edit->addSeparator();edit->addAction("Select All",this,&FluxMainWindow::selectAll,QKeySequence("Ctrl+A"));edit->addAction("Deselect",this,&FluxMainWindow::deselect,QKeySequence("Ctrl+Shift+A"));edit->addAction("Brush Editor…",this,&FluxMainWindow::openBrushEditor);
    auto*view=menuBar()->addMenu("View");view->addAction("Fit Canvas",this,&FluxMainWindow::fitCanvas);view->addAction("Mirror Horizontal",this,&FluxMainWindow::mirrorHorizontal);view->addAction("Mirror Vertical",this,&FluxMainWindow::mirrorVertical);view->addAction("Rotate 90°",this,&FluxMainWindow::rotateCanvas);view->addAction("Command Palette",this,&FluxMainWindow::showCommandPalette,QKeySequence("Ctrl+K"));
    auto*layer=menuBar()->addMenu("Layer");layer->addAction("Add Paint Layer",this,&FluxMainWindow::addLayer);layer->addAction("Add Group",this,&FluxMainWindow::addGroup);layer->addAction("Add Mask",this,&FluxMainWindow::addMask);layer->addAction("Add Vector Mask",this,[this]{m_document->addMask(m_document->activeLayerIndex(),true);refreshLayers();markModified();});layer->addAction("Add Adjustment",this,[this]{m_document->addAdjustment("Adjustment "+QString::number(m_document->layers().size()));refreshLayers();markModified();});layer->addSeparator();layer->addAction("Duplicate",this,&FluxMainWindow::duplicateLayer);layer->addAction("Merge Down",this,&FluxMainWindow::mergeDown);layer->addAction("Flatten Visible",this,&FluxMainWindow::flattenVisible);layer->addAction("Delete",this,&FluxMainWindow::removeLayer);
    auto*anim=menuBar()->addMenu("Animation");anim->addAction("Previous Frame",this,[this]{m_document->setFrame(qMax(0,m_document->frame()-1));m_canvas->update();},QKeySequence("Left"));anim->addAction("Next Frame",this,[this]{m_document->setFrame(qMin(m_document->frameCount()-1,m_document->frame()+1));m_canvas->update();},QKeySequence("Right"));anim->addAction("Play / Pause",this,&FluxMainWindow::togglePlayback,QKeySequence("Space"));anim->addAction("Duplicate Frame",this,[this]{int f=m_document->frame();m_document->setFrameCount(m_document->frameCount()+1);m_document->ensureFrame(f+1);m_document->setFrame(f+1);markModified();m_canvas->update();});anim->addAction("Insert Frame",this,[this]{m_document->setFrameCount(m_document->frameCount()+1);markModified();m_canvas->update();});anim->addAction("Delete Frame",this,[this]{if(m_document->frameCount()>1)m_document->setFrameCount(m_document->frameCount()-1);markModified();m_canvas->update();});anim->addAction("Toggle Onion Skin",this,&FluxMainWindow::toggleOnionSkin);
    auto*window=menuBar()->addMenu("Window");window->addAction("Layers",this,[this]{for(auto*d:findChildren<QDockWidget*>())if(d->windowTitle()=="Layers")d->show();});window->addAction("Inspector",this,[this]{for(auto*d:findChildren<QDockWidget*>())if(d->windowTitle()=="Inspector")d->show();});window->addAction("Timeline",this,[this]{for(auto*d:findChildren<QDockWidget*>())if(d->windowTitle()=="Timeline")d->show();});window->addAction("Compose",this,[this]{for(auto*d:findChildren<QDockWidget*>())if(d->windowTitle()=="Compose")d->show();});window->addSeparator();window->addAction("Reset Workspace",this,[this]{QSettings("Flux","Flux Studio").remove("geometry");QSettings("Flux","Flux Studio").remove("state");statusBar()->showMessage("Workspace reset",2000);});
}

void FluxMainWindow::buildTopBar(){
    m_topBar=addToolBar("Flux Command Bar");m_topBar->setMovable(false);m_topBar->setFloatable(false);m_topBar->setObjectName("FluxTopBar");
    auto*home=m_topBar->addAction("FLUX");home->setToolTip("Home");connect(home,&QAction::triggered,this,&FluxMainWindow::returnHome);
    m_topBar->addSeparator();auto*save=m_topBar->addAction("Save");save->setShortcut(QKeySequence::Save);connect(save,&QAction::triggered,this,&FluxMainWindow::saveProject);auto*undoAct=m_topBar->addAction("Undo");connect(undoAct,&QAction::triggered,this,&FluxMainWindow::undo);auto*redoAct=m_topBar->addAction("Redo");connect(redoAct,&QAction::triggered,this,&FluxMainWindow::redo);
    m_topBar->addSeparator();m_topBar->addWidget(label("TOOL","topLabel"));auto*tool=new QComboBox;m_toolActions=QVector<QAction*>();tool->addItems({"Brush","Pencil","Ink","Airbrush","Marker","Eraser","Rectangle Select","Lasso Select","Contiguous Select","Transform","Fill","Color Picker","Zoom","Pan"});connect(tool,&QComboBox::currentTextChanged,this,[this](const QString&s){m_canvas->setTool(s);});m_topBar->addWidget(tool);
    m_topBar->addWidget(label("SIZE","topLabel"));m_brushSlider=new QSlider(Qt::Horizontal);m_brushSlider->setRange(1,1000);m_brushSlider->setValue(24);m_brushSlider->setFixedWidth(160);m_topBar->addWidget(m_brushSlider);connect(m_brushSlider,&QSlider::valueChanged,this,&FluxMainWindow::updateBrushSize);m_brushSizeLabel=label("24 px","topValue");m_topBar->addWidget(m_brushSizeLabel);
    m_topBar->addSeparator();auto*color=m_topBar->addAction("Color");connect(color,&QAction::triggered,this,&FluxMainWindow::chooseColor);m_colorSwatch=label("●","swatch");m_topBar->addWidget(m_colorSwatch);m_topBar->addSeparator();m_topBar->addAction("Fit",this,&FluxMainWindow::fitCanvas);m_topBar->addAction("▶",this,&FluxMainWindow::togglePlayback);m_topBar->addSeparator();m_zoomLabel=label("100%","topValue");m_topBar->addWidget(m_zoomLabel);m_topBar->addSeparator();m_cursorLabel=label("0, 0","cursor");m_topBar->addWidget(m_cursorLabel);m_topBar->addSeparator();auto*cmd=m_topBar->addAction("⌘");cmd->setToolTip("Command Palette (Ctrl+K)");connect(cmd,&QAction::triggered,this,&FluxMainWindow::showCommandPalette);
}

void FluxMainWindow::buildToolRail(){
    m_toolRail=addToolBar(Qt::LeftToolBarArea,"Flux Tools");m_toolRail->setObjectName("FluxToolRail");m_toolRail->setMovable(false);m_toolRail->setFloatable(false);m_toolRail->setOrientation(Qt::Vertical);m_toolRail->setIconSize(QSize(20,20));
    QActionGroup*group=new QActionGroup(this);group->setExclusive(true);const QStringList tools={"B","P","I","A","M","E","S","L","C","T","F","K","Z","H"};const QStringList names={"Brush","Pencil","Ink","Airbrush","Marker","Eraser","Rectangle Select","Lasso Select","Contiguous Select","Transform","Fill","Color Picker","Zoom","Pan"};for(int i=0;i<names.size();++i){auto*a=m_toolRail->addAction(tools[i]);a->setCheckable(true);a->setToolTip(names[i]);a->setData(names[i]);group->addAction(a);connect(a,&QAction::triggered,this,&FluxMainWindow::setToolFromAction);if(i==0)a->setChecked(true);}m_toolRail->addSeparator();m_toolRail->addAction("R",this,[this]{m_canvas->loadReference(QFileDialog::getOpenFileName(this,"Reference Image",{},"Images (*.png *.jpg *.jpeg *.webp)"));});m_toolRail->addAction("G",this,[this]{m_canvas->setGridEnabled(true);});m_toolRail->addAction("Y",this,[this]{m_canvas->setSymmetry(true,true);});
}

void FluxMainWindow::buildDocks(){
    auto*layers=new QDockWidget("Layers",this);layers->setObjectName("LayersDock");layers->setWidget(makeLayersPanel());addDockWidget(Qt::RightDockWidgetArea,layers);
    auto*inspector=new QDockWidget("Inspector",this);inspector->setObjectName("InspectorDock");inspector->setWidget(makeInspectorPanel());addDockWidget(Qt::RightDockWidgetArea,inspector);tabifyDockWidget(layers,inspector);
    auto*compose=new QDockWidget("Compose",this);compose->setObjectName("ComposeDock");compose->setWidget(new FluxCompositorWidget);addDockWidget(Qt::RightDockWidgetArea,compose);tabifyDockWidget(layers,compose);inspector->raise();
    auto*timeline=new QDockWidget("Timeline",this);timeline->setObjectName("TimelineDock");timeline->setWidget(makeTimelinePanel());timeline->setMinimumHeight(300);addDockWidget(Qt::BottomDockWidgetArea,timeline);
}

QWidget* FluxMainWindow::makeLayersPanel(){
    auto*root=new QWidget;auto*lay=new QVBoxLayout(root);lay->setContentsMargins(10,10,10,10);lay->setSpacing(8);lay->addWidget(label("LAYERS","panelTitle"));m_layers=new FluxLayerTree;m_layers->setHeaderLabels({"Layer","%"});m_layers->setSelectionMode(QAbstractItemView::ExtendedSelection);m_layers->setDragDropMode(QAbstractItemView::InternalMove);m_layers->setDefaultDropAction(Qt::MoveAction);m_layers->setAnimated(false);m_layers->header()->setSectionResizeMode(0,QHeaderView::Stretch);m_layers->header()->setSectionResizeMode(1,QHeaderView::ResizeToContents);lay->addWidget(m_layers,1);
    connect(m_layers,&QTreeWidget::itemSelectionChanged,this,&FluxMainWindow::layerSelectionChanged);connect(m_layers,&QTreeWidget::itemChanged,this,&FluxMainWindow::layerItemChanged);connect(m_layers,&FluxLayerTree::hierarchyDropped,this,&FluxMainWindow::syncLayerTreeToDocument);
    auto*addRow=new QHBoxLayout;for(const auto&spec:QStringList{"+ Paint","+ Group","+ Mask"}){auto*b=new QPushButton(spec);addRow->addWidget(b);if(spec=="+ Paint")connect(b,&QPushButton::clicked,this,&FluxMainWindow::addLayer);else if(spec=="+ Group")connect(b,&QPushButton::clicked,this,&FluxMainWindow::addGroup);else connect(b,&QPushButton::clicked,this,&FluxMainWindow::addMask);}lay->addLayout(addRow);
    auto*ops=new QHBoxLayout;for(const auto&spec:QStringList{"Dup","Merge","Flat","Del"}){auto*b=new QPushButton(spec);ops->addWidget(b);if(spec=="Dup")connect(b,&QPushButton::clicked,this,&FluxMainWindow::duplicateLayer);if(spec=="Merge")connect(b,&QPushButton::clicked,this,&FluxMainWindow::mergeDown);if(spec=="Flat")connect(b,&QPushButton::clicked,this,&FluxMainWindow::flattenVisible);if(spec=="Del")connect(b,&QPushButton::clicked,this,&FluxMainWindow::removeLayer);}lay->addLayout(ops);return root;
}

QWidget* FluxMainWindow::makeInspectorPanel(){
    auto*root=new QWidget;auto*lay=new QVBoxLayout(root);lay->setContentsMargins(12,12,12,12);lay->setSpacing(10);lay->addWidget(label("INSPECTOR","panelTitle"));
    auto*layerBox=new QGroupBox("Layer");auto*lf=new QFormLayout(layerBox);m_blendMode=new QComboBox;m_blendMode->addItems({"Normal","Multiply","Screen","Overlay","Add","Subtract"});auto*opacity=new QSlider(Qt::Horizontal);opacity->setRange(0,100);opacity->setValue(100);connect(m_blendMode,qOverload<int>(&QComboBox::currentIndexChanged),this,&FluxMainWindow::setLayerBlendMode);connect(opacity,&QSlider::valueChanged,this,&FluxMainWindow::setLayerOpacity);lf->addRow("Blend",m_blendMode);lf->addRow("Opacity",opacity);lay->addWidget(layerBox);
    auto*brush=new QGroupBox("Brush dynamics");auto*bf=new QFormLayout(brush);auto*flow=new QSlider(Qt::Horizontal);flow->setRange(0,100);flow->setValue(100);auto*stab=new QSlider(Qt::Horizontal);stab->setRange(0,100);stab->setValue(12);bf->addRow("Flow",flow);bf->addRow("Stabilizer",stab);connect(stab,&QSlider::valueChanged,this,[this](int v){m_canvas->setStabilization(v/100.0);});lay->addWidget(brush);
    auto*view=new QGroupBox("Canvas");auto*vf=new QGridLayout(view);auto*onion=new QPushButton("Onion");onion->setCheckable(true);onion->setChecked(true);auto*grid=new QPushButton("Grid");grid->setCheckable(true);grid->setChecked(true);auto*rulers=new QPushButton("Rulers");rulers->setCheckable(true);rulers->setChecked(true);auto*perspective=new QPushButton("Perspective");perspective->setCheckable(true);vf->addWidget(onion,0,0);vf->addWidget(grid,0,1);vf->addWidget(rulers,1,0);vf->addWidget(perspective,1,1);connect(onion,&QPushButton::toggled,this,[this](bool v){m_canvas->toggleOnionSkin(v);});connect(grid,&QPushButton::toggled,this,[this](bool v){m_canvas->setGridEnabled(v);});connect(rulers,&QPushButton::toggled,this,[this](bool v){m_canvas->setRulersEnabled(v);});connect(perspective,&QPushButton::toggled,this,[this](bool v){m_canvas->setPerspectiveGuide(v);});lay->addWidget(view);
    auto*advanced=new QGroupBox("Production");auto*af=new QVBoxLayout(advanced);auto*loadRef=new QPushButton("Load reference image…");auto*sym=new QPushButton("Symmetry H + V");auto*pixel=new QPushButton("Pixel perfect");pixel->setCheckable(true);auto*tablet=new QLabel("Tablet: automatic pressure / tilt");af->addWidget(loadRef);af->addWidget(sym);af->addWidget(pixel);af->addWidget(tablet);connect(loadRef,&QPushButton::clicked,this,[this]{const auto f=QFileDialog::getOpenFileName(this,"Reference Image",{},"Images (*.png *.jpg *.jpeg *.webp)");if(!f.isEmpty())m_canvas->loadReference(f);});connect(sym,&QPushButton::clicked,this,[this]{m_canvas->setSymmetry(true,true);});connect(pixel,&QPushButton::toggled,this,[this](bool v){m_canvas->setPixelPerfect(v);});lay->addWidget(advanced);lay->addStretch();return root;
}

QWidget* FluxMainWindow::makeTimelinePanel(){
    auto*root=new QWidget;auto*lay=new QVBoxLayout(root);lay->setContentsMargins(10,8,10,8);auto*head=new QHBoxLayout;head->addWidget(label("TIMELINE","panelTitle"));head->addSpacing(18);head->addWidget(label("FPS"));m_fps=new QSpinBox;m_fps->setRange(1,240);m_fps->setValue(24);head->addWidget(m_fps);auto*play=new QPushButton("▶");play->setCheckable(true);head->addWidget(play);auto*frameLabel=new QLabel("Frame 1 / 120");head->addWidget(frameLabel);head->addStretch();auto*graph=new QPushButton("Graph");graph->setCheckable(true);head->addWidget(graph);lay->addLayout(head);
    auto*timeline=new FluxTimelineWidget(m_document,root);connect(play,&QPushButton::toggled,this,[this,play](bool on){m_playing=on;if(on)m_playTimer->start();else m_playTimer->stop();play->setText(on?"■":"▶");});connect(m_fps,qOverload<int>(&QSpinBox::valueChanged),this,[this](int v){m_playTimer->setInterval(qMax(1,1000/v));});connect(timeline,&FluxTimelineWidget::frameChanged,this,[this,frameLabel](int f){m_document->setFrame(f);frameLabel->setText(QString("Frame %1 / %2").arg(f+1).arg(m_document->frameCount()));m_canvas->update();});connect(timeline,&FluxTimelineWidget::documentEdited,this,[this]{markModified();m_canvas->update();});lay->addWidget(timeline,1);return root;
}

void FluxMainWindow::newProject(){
    QDialog dlg(this);dlg.setWindowTitle("New Flux Project");dlg.setModal(true);auto*lay=new QVBoxLayout(&dlg);lay->addWidget(label("NEW PROJECT","dialogEyebrow"));lay->addWidget(label("Set up the document before entering the workspace.","dialogTitle"));auto*form=new QFormLayout;auto*preset=new QComboBox;preset->addItems({"1920 × 1080  /  24 fps","2048 × 2048  / 24 fps","1280 × 720  / 30 fps","1080 × 1920  / 30 fps","Custom"});auto*w=new QSpinBox;w->setRange(1,16384);w->setValue(1920);auto*h=new QSpinBox;h->setRange(1,16384);h->setValue(1080);auto*fps=new QSpinBox;fps->setRange(1,240);fps->setValue(24);form->addRow("Preset",preset);form->addRow("Width",w);form->addRow("Height",h);form->addRow("FPS",fps);lay->addLayout(form);auto*buttons=new QDialogButtonBox(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);lay->addWidget(buttons);connect(preset,qOverload<int>(&QComboBox::currentIndexChanged),[=](int i){if(i==0){w->setValue(1920);h->setValue(1080);fps->setValue(24);}else if(i==1){w->setValue(2048);h->setValue(2048);fps->setValue(24);}else if(i==2){w->setValue(1280);h->setValue(720);fps->setValue(30);}else if(i==3){w->setValue(1080);h->setValue(1920);fps->setValue(30);}const bool custom=i==4;w->setEnabled(custom);h->setEnabled(custom);fps->setEnabled(custom);});connect(buttons,&QDialogButtonBox::accepted,&dlg,&QDialog::accept);connect(buttons,&QDialogButtonBox::rejected,&dlg,&QDialog::reject);if(dlg.exec()!=QDialog::Accepted)return;m_document->create("Untitled",w->value(),h->value());m_filePath.clear();m_fps->setValue(fps->value());enterWorkspace();setStatus(QString("New %1 × %2 project").arg(w->value()).arg(h->value()));}

void FluxMainWindow::openProject(){const auto f=QFileDialog::getOpenFileName(this,"Open Flux Project",{},"Flux Project (*.flux)");if(!f.isEmpty())loadProjectPath(f);}
void FluxMainWindow::loadProjectPath(const QString&path){QString e;if(!m_document->load(path,&e)){QMessageBox::critical(this,"Flux Studio",e);return;}m_filePath=path;rememberRecent(path);m_canvas->setDocument(m_document);refreshLayers();enterWorkspace();setWindowTitle("Flux Studio — "+QFileInfo(path).completeBaseName());setStatus("Opened "+QFileInfo(path).completeBaseName());}
void FluxMainWindow::saveProject(){if(m_filePath.isEmpty()){saveProjectAs();return;}QString e;if(!m_document->save(m_filePath,&e)){QMessageBox::critical(this,"Save failed",e);return;}rememberRecent(m_filePath);setWindowTitle("Flux Studio — "+QFileInfo(m_filePath).completeBaseName());setStatus("Saved");}
void FluxMainWindow::saveProjectAs(){auto f=QFileDialog::getSaveFileName(this,"Save Flux Project",{},"Flux Project (*.flux)");if(f.isEmpty())return;if(!f.endsWith(".flux",Qt::CaseInsensitive))f+=".flux";m_filePath=f;saveProject();}
void FluxMainWindow::exportImage(){const auto f=QFileDialog::getSaveFileName(this,"Export Image",{},"PNG (*.png);;JPEG (*.jpg *.jpeg);;WebP (*.webp);;SVG (*.svg)");if(f.isEmpty())return;QString e;bool ok=false;if(f.endsWith(".svg",Qt::CaseInsensitive))ok=FluxExportEngine::exportSvgRaster(m_document->composite(),f,FluxRenderSettings{},&e);else ok=m_document->exportImage(f,&e);if(!ok)QMessageBox::critical(this,"Export failed",e);else setStatus("Exported "+QFileInfo(f).fileName());}

void FluxMainWindow::enterWorkspace(){m_stack->setCurrentWidget(m_canvas);if(m_topBar)m_topBar->show();if(m_toolRail)m_toolRail->show();for(auto*d:findChildren<QDockWidget*>())d->show();m_canvas->setFocus();m_canvas->fitCanvas();refreshLayers();}
void FluxMainWindow::returnHome(){m_playing=false;m_playTimer->stop();m_stack->setCurrentWidget(m_home);if(m_topBar)m_topBar->hide();if(m_toolRail)m_toolRail->hide();for(auto*d:findChildren<QDockWidget*>())d->hide();setWindowTitle(m_filePath.isEmpty()?"Flux Studio — Home":"Flux Studio — "+QFileInfo(m_filePath).completeBaseName());}
void FluxMainWindow::rememberRecent(const QString&path){FluxWorkflow::addRecentProject(path,12);updateHomeRecent();}
void FluxMainWindow::updateHomeRecent(){/* home list is refreshed on next Home visit */}

void FluxMainWindow::addLayer(){int parent=-1;if(m_layers&&m_layers->currentItem()){const int i=m_layers->currentItem()->data(0,Qt::UserRole).toInt();if(i>=0&&i<m_document->layers().size()&&m_document->layers()[i].type==FluxLayerType::Group)parent=i;}m_document->addLayer("Paint Layer "+QString::number(m_document->layers().size()),FluxLayerType::Paint,parent);refreshLayers();markModified();m_canvas->update();}
void FluxMainWindow::addGroup(){m_document->addGroup("Group "+QString::number(m_document->layers().size()));refreshLayers();markModified();}
void FluxMainWindow::addMask(){m_document->addMask(m_document->activeLayerIndex(),false);refreshLayers();markModified();m_canvas->update();}
void FluxMainWindow::duplicateLayer(){m_document->duplicateLayer(m_document->activeLayerIndex());refreshLayers();markModified();m_canvas->update();}
void FluxMainWindow::removeLayer(){m_document->removeLayer(m_document->activeLayerIndex());refreshLayers();markModified();m_canvas->update();}
void FluxMainWindow::mergeDown(){m_document->mergeDown(m_document->activeLayerIndex());refreshLayers();markModified();m_canvas->update();}
void FluxMainWindow::flattenVisible(){m_document->flattenVisible();refreshLayers();markModified();m_canvas->update();}
void FluxMainWindow::layerSelectionChanged(){if(m_syncingLayers||!m_layers||!m_layers->currentItem())return;const int i=m_layers->currentItem()->data(0,Qt::UserRole).toInt();m_document->setActiveLayer(i);const auto&l=m_document->activeLayer();if(m_blendMode){QSignalBlocker b(m_blendMode);m_blendMode->setCurrentText(FluxDocument::blendModeName(l.blendMode));}}
void FluxMainWindow::layerItemChanged(QTreeWidgetItem*item,int column){if(m_syncingLayers||!item||column!=0)return;const int i=item->data(0,Qt::UserRole).toInt();if(i>=0&&i<m_document->layers().size()){m_document->setLayerVisible(i,item->checkState(0)==Qt::Checked);markModified();m_canvas->update();}}
void FluxMainWindow::setLayerOpacity(int v){m_document->setLayerOpacity(m_document->activeLayerIndex(),v/100.0);markModified();m_canvas->update();}
void FluxMainWindow::setLayerBlendMode(int i){if(m_blendMode)m_document->setLayerBlendMode(m_document->activeLayerIndex(),FluxDocument::blendModeFromName(m_blendMode->itemText(i)));markModified();m_canvas->update();}
void FluxMainWindow::setLayerLocked(bool e){m_document->setLayerLocked(m_document->activeLayerIndex(),e);refreshLayers();}
void FluxMainWindow::setLayerClipping(bool e){m_document->setLayerClipping(m_document->activeLayerIndex(),e);markModified();}
void FluxMainWindow::setLayerAlphaInheritance(bool e){m_document->setLayerAlphaInherited(m_document->activeLayerIndex(),e);markModified();}
void FluxMainWindow::setLayerSolo(bool e){m_document->setSolo(m_document->activeLayerIndex(),e);m_canvas->update();}
void FluxMainWindow::setLayerIsolate(bool e){m_document->setIsolate(m_document->activeLayerIndex(),e);m_canvas->update();}
void FluxMainWindow::setLayerLabelColor(){auto c=QColorDialog::getColor(m_document->activeLayer().labelColor.isValid()?m_document->activeLayer().labelColor:Qt::white,this,"Layer Label");if(c.isValid()){m_document->setLayerLabelColor(m_document->activeLayerIndex(),c);refreshLayers();markModified();}}
void FluxMainWindow::setLayerStyle(){auto s=m_document->activeLayer().style;s.enabled=!s.enabled;if(s.enabled){s.outlineOpacity=.8;s.outlineWidth=2;s.outlineColor=m_document->foreground();}m_document->setLayerStyle(m_document->activeLayerIndex(),s);m_canvas->update();markModified();}
void FluxMainWindow::syncLayerTreeToDocument(){if(!m_layers||m_syncingLayers)return;QVector<int>order,parents;std::function<void(QTreeWidgetItem*,int)>walk=[&](QTreeWidgetItem*p,int parentNew){const int count=p?p->childCount():m_layers->topLevelItemCount();for(int k=0;k<count;++k){auto*it=p?p->child(k):m_layers->topLevelItem(k);order.push_back(it->data(0,Qt::UserRole).toInt());parents.push_back(parentNew);walk(it,order.size()-1);}};walk(nullptr,-1);if(order.size()!=m_document->layers().size())return;const auto old=m_document->layers();QVector<FluxLayer>reordered;QHash<int,int>map;for(int j=0;j<order.size();++j){map[order[j]]=j;reordered.push_back(old[order[j]]);}for(int j=0;j<reordered.size();++j){reordered[j].parentIndex=parents[j]<0?-1:map.value(order[parents[j]],-1);if(reordered[j].maskTarget>=0)reordered[j].maskTarget=map.value(reordered[j].maskTarget,-1);}const int active=m_document->activeLayerIndex();m_document->layers()=reordered;m_document->setActiveLayer(map.value(active,0));markModified();m_canvas->update();}

void FluxMainWindow::refreshLayers(){if(!m_layers)return;m_syncingLayers=true;m_layers->clear();for(int i=0;i<m_document->layers().size();++i)if(m_document->layers()[i].parentIndex<0)addLayerTreeItem(nullptr,i);for(int i=0;i<m_layers->topLevelItemCount();++i){auto*it=m_layers->topLevelItem(i);if(it->data(0,Qt::UserRole).toInt()==m_document->activeLayerIndex())m_layers->setCurrentItem(it);}m_syncingLayers=false;}
void FluxMainWindow::addLayerTreeItem(QTreeWidgetItem*parent,int index){const auto&l=m_document->layers()[index];auto*item=parent?new QTreeWidgetItem(parent):new QTreeWidgetItem(m_layers);QString prefix=l.type==FluxLayerType::Group?"▾ ":l.type==FluxLayerType::Mask?"◐ ":l.type==FluxLayerType::VectorMask?"◇ ":l.type==FluxLayerType::Adjustment?"◈ ":"";item->setText(0,prefix+l.name);item->setText(1,QString::number(qRound(l.opacity*100)));item->setData(0,Qt::UserRole,index);item->setCheckState(0,l.visible?Qt::Checked:Qt::Unchecked);item->setToolTip(0,FluxDocument::layerTypeName(l.type));const QImage thumb=(l.frames.size()==m_document->frameCount()&&!l.frames[m_document->frame()].isNull())?l.frames[m_document->frame()]:l.image;if(!thumb.isNull())item->setIcon(0,QIcon(QPixmap::fromImage(thumb.scaled(30,30,Qt::KeepAspectRatio,Qt::FastTransformation))));if(l.labelColor.isValid())item->setBackground(0,l.labelColor);for(int i=0;i<m_document->layers().size();++i)if(m_document->layers()[i].parentIndex==index)addLayerTreeItem(item,i);}
void FluxMainWindow::refreshTimeline(){ }
void FluxMainWindow::setFrameFromTimeline(int row){if(row<0)return;m_document->setFrame(row);m_canvas->update();}
void FluxMainWindow::togglePlayback(){m_playing=!m_playing;if(m_playing)m_playTimer->start();else m_playTimer->stop();}
void FluxMainWindow::updateBrushSize(int value){m_canvas->setBrushSize(value);if(m_brushSizeLabel)m_brushSizeLabel->setText(QString::number(value)+" px");}
void FluxMainWindow::chooseColor(){const auto c=QColorDialog::getColor(m_document->foreground(),this,"Flux Color");if(c.isValid()){m_document->setForeground(c);m_canvas->setBrushColor(c);if(m_colorSwatch)m_colorSwatch->setText("●");m_colorSwatch->setStyleSheet(QString("QLabel{color:%1;font-size:22px}").arg(c.name()));}}
void FluxMainWindow::setToolFromAction(){if(auto*a=qobject_cast<QAction*>(sender()))m_canvas->setTool(a->data().toString());}
void FluxMainWindow::openBrushEditor(){BrushEditorDialog dlg(m_canvas->brushEngine(),this);if(dlg.exec()==QDialog::Accepted){m_canvas->setBrushSize(m_canvas->brushEngine()->preset().size);updateBrushSize(m_canvas->brushEngine()->preset().size);}}
void FluxMainWindow::mirrorHorizontal(){m_mirrorH=!m_mirrorH;m_canvas->setMirrorHorizontal(m_mirrorH);}
void FluxMainWindow::mirrorVertical(){m_mirrorV=!m_mirrorV;m_canvas->setMirrorVertical(m_mirrorV);}
void FluxMainWindow::rotateCanvas(){m_canvasRotation+=90;if(m_canvasRotation>=360)m_canvasRotation=0;m_canvas->setCanvasRotation(m_canvasRotation);}
void FluxMainWindow::fitCanvas(){m_canvas->fitCanvas();}
void FluxMainWindow::toggleOnionSkin(){m_canvas->toggleOnionSkin(!m_canvas->onionSkin());}
void FluxMainWindow::selectAll(){m_canvas->selectAll();}
void FluxMainWindow::deselect(){m_canvas->clearSelection();}
void FluxMainWindow::undo(){m_canvas->undo();}
void FluxMainWindow::redo(){m_canvas->redo();}
void FluxMainWindow::autosave(){if(!m_document||m_autosavePath.isEmpty())return;QDir().mkpath(QFileInfo(m_autosavePath).absolutePath());m_document->save(m_autosavePath,nullptr);}
void FluxMainWindow::restoreLastSession(){QSettings s("Flux","Flux Studio");restoreGeometry(s.value("geometry").toByteArray());restoreState(s.value("state").toByteArray());}
void FluxMainWindow::setStatus(const QString&text){if(m_statusLabel)m_statusLabel->setText(text);statusBar()->showMessage(text,3000);}
void FluxMainWindow::updateZoomLabel(double zoom){if(m_zoomLabel)m_zoomLabel->setText(QString::number(qRound(zoom*100))+"%");}
void FluxMainWindow::showCommandPalette(){FluxCommandPalette dlg(this);QVector<FluxCommand>cmds={{"new","New Project","Ctrl+N",[this]{newProject();}},{"open","Open Project","Ctrl+O",[this]{openProject();}},{"save","Save Project","Ctrl+S",[this]{saveProject();}},{"brush","Brush Editor","",[this]{openBrushEditor();}},{"fit","Fit Canvas","",[this]{fitCanvas();}},{"home","Home","Ctrl+Shift+H",[this]{returnHome();}},{"undo","Undo","Ctrl+Z",[this]{undo();}},{"redo","Redo","Ctrl+Y",[this]{redo();}},{"onion","Toggle Onion Skin","",[this]{toggleOnionSkin();}},{"reference","Load Reference","",[this]{const auto f=QFileDialog::getOpenFileName(this,"Reference",{},"Images (*.png *.jpg *.jpeg *.webp)");if(!f.isEmpty())m_canvas->loadReference(f);}}};dlg.setCommands(cmds);dlg.exec();}
void FluxMainWindow::polish(){setStyleSheet(R"STYLE(
*{font-family:"Segoe UI";font-size:13px;color:#e7ebf2}QMainWindow{background:#0c0f13}QMenuBar{background:#10141a;border-bottom:1px solid #252b34;padding:3px 7px}QMenuBar::item{padding:6px 10px;border-radius:5px}QMenuBar::item:selected{background:#1d2430}QMenu{background:#151a21;border:1px solid #303744;padding:5px}QMenu::item{padding:7px 22px 7px 10px;border-radius:5px}QMenu::item:selected{background:#253041}QToolBar{background:#11161d;border:0;border-bottom:1px solid #262d37;padding:6px;spacing:4px}QToolBar#FluxTopBar QToolButton{min-height:30px;padding:4px 9px;border-radius:6px}QToolBar#FluxTopBar QToolButton:hover{background:#202936}QToolBar#FluxToolRail{background:#0f141a;border-right:1px solid #262d37;padding:7px 5px}QToolBar#FluxToolRail QToolButton{min-width:38px;min-height:38px;margin:2px 0;border-radius:8px;font-weight:700}QToolBar#FluxToolRail QToolButton:hover{background:#1d2530}QToolBar#FluxToolRail QToolButton:checked{background:#313c4c;border:1px solid #4b5a70}QDockWidget{background:#11161d;border:1px solid #252c35}QDockWidget::title{background:#151b23;padding:9px 11px;border-bottom:1px solid #252c35;font-weight:700}QTreeWidget,QListWidget{background:#0f141a;border:1px solid #252c35;border-radius:8px;outline:0}QTreeWidget::item,QListWidget::item{padding:5px;border-radius:5px}QTreeWidget::item:selected,QListWidget::item:selected{background:#2b3544}QPushButton{background:#1a2028;border:1px solid #303846;border-radius:8px;padding:8px 12px}QPushButton:hover{background:#222b37;border-color:#4a586d}QPushButton:pressed,QPushButton:checked{background:#2b3645}QComboBox,QSpinBox{background:#171d25;border:1px solid #303846;border-radius:7px;padding:5px 8px}QSlider::groove:horizontal{height:4px;background:#303846;border-radius:2px}QSlider::handle:horizontal{width:13px;margin:-5px 0;border-radius:6px;background:#e8ebef}QGroupBox{border:1px solid #29313b;border-radius:9px;margin-top:9px;padding-top:9px}QGroupBox::title{subcontrol-origin:margin;left:10px;padding:0 5px;color:#8e9aaa;font-size:11px;font-weight:700}QStatusBar{background:#0d1117;border-top:1px solid #252c35}QStatusBar::item{border:0}QLabel[role="brand"]{font-size:36px;font-weight:800;letter-spacing:10px;color:#f5f7fa}QLabel[role="brandSub"]{font-size:10px;font-weight:700;letter-spacing:3px;color:#697789}QLabel[role="meta"],QLabel[role="hint"]{font-size:10px;letter-spacing:1.3px;color:#687487;font-weight:700}QLabel[role="eyebrow"]{font-size:11px;letter-spacing:2.5px;color:#7c8ba0;font-weight:800}QLabel[role="headline"]{font-size:36px;font-weight:700;color:#f6f7fa}QLabel[role="subhead"]{font-size:14px;color:#8d99aa;max-width:900px}QLabel[role="section"]{font-size:11px;letter-spacing:1.8px;font-weight:800;color:#9faaba}QLabel[role="footer"]{font-size:10px;letter-spacing:1.3px;color:#5e6b7d;font-weight:700}QLabel[role="topLabel"]{font-size:10px;color:#778497;font-weight:700}QLabel[role="topValue"]{font-size:11px;color:#b6bec9;font-weight:700}QLabel[role="cursor"]{font-size:11px;color:#718095;min-width:75px}QLabel[role="panelTitle"]{font-size:11px;letter-spacing:1.6px;font-weight:800;color:#8996a8}QLabel[role="dialogEyebrow"]{font-size:11px;letter-spacing:2px;color:#728198;font-weight:800}QLabel[role="dialogTitle"]{font-size:22px;font-weight:700;color:#f3f5f8}QFrame#homeRecent{background:#11161d;border:1px solid #252d37;border-radius:14px}QPushButton#homePrimary{background:#27374a;border:1px solid #4a627d;text-align:left;padding:22px;border-radius:14px;font-size:13px;font-weight:700}QPushButton#homePrimary:hover{background:#30445a}QPushButton#homeCard{background:#151b22;border:1px solid #29323d;text-align:left;padding:20px;border-radius:14px;font-size:12px;font-weight:700}QPushButton#homeCard:hover{background:#1c2530;border-color:#44546a}QPushButton#tinyButton{padding:4px 9px;font-size:11px}QListWidget#homeRecentList{background:transparent;border:0}QLabel[role="swatch"]{font-size:22px;color:#111}
)STYLE");
    auto snap=FluxPerformance::probe();m_statusLabel=new QLabel(QString("Ready  •  %1  •  GPU: %2").arg(snap.tileSize).arg(snap.gpuAvailable?"On":"Off"));statusBar()->addWidget(m_statusLabel,1);}
