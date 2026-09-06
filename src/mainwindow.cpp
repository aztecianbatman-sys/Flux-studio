#include "mainwindow.h"
#include "canvaswidget.h"
#include "fluxdocument.h"
#include "fluxlayertree.h"
#include "fluxtimelinewidget.h"
#include "fluxcompositorwidget.h"
#include "fluxexport.h"
#include "fluxcommandpalette.h"
#include "fluxwheel.h"
#include "fluxworkflow.h"
#include "fluxrecovery.h"
#include "fluxprojectpackage.h"
#include "fluxproductiondock.h"
#include "brusheditor.h"
#include "fluxcolorwheel.h"
#include "fluxadvancedsuite.h"
#include "fluxadvancedsuite.h"
#include "fluxadvancedsuite.h"
#include "fluxadvancedsuite.h"
#include "fluxadvancedsuite.h"

#include <QAction>
#include <QActionGroup>
#include <QAbstractItemView>
#include <QColorDialog>
#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDockWidget>
#include <QFileDialog>
#include <QFileInfo>
#include <QFormLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QHash>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMenuBar>
#include <QMessageBox>
#include <QPixmap>
#include <QPushButton>
#include <QSettings>
#include <QSignalBlocker>
#include <QSlider>
#include <QSpinBox>
#include <QStackedWidget>
#include <QStandardPaths>
#include <QStatusBar>
#include <QStyle>
#include <QTabWidget>
#include <QTimer>
#include <QToolBar>
#include <QToolButton>
#include <QTreeWidget>
#include <QVBoxLayout>
#include <functional>

namespace {
QLabel* uiLabel(const QString& text,const QString& role={},QWidget* parent=nullptr){auto*l=new QLabel(text,parent);if(!role.isEmpty())l->setProperty("role",role);l->setWordWrap(true);return l;}
QPushButton* card(const QString&title,const QString&desc,QWidget*parent){auto*b=new QPushButton(parent);b->setObjectName("homeCard");b->setText(title+QStringLiteral("\n")+desc);b->setCursor(Qt::PointingHandCursor);b->setMinimumHeight(104);b->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Preferred);return b;}
QToolButton* iconButton(const QIcon&i,const QString&t,QWidget*p){auto*b=new QToolButton(p);b->setIcon(i);b->setToolTip(t);b->setCursor(Qt::PointingHandCursor);b->setIconSize(QSize(18,18));b->setFixedSize(34,34);b->setAutoRaise(true);return b;}
}

FluxMainWindow::FluxMainWindow(QWidget* parent):QMainWindow(parent),m_document(new FluxDocument),m_playTimer(new QTimer(this)){
    setWindowTitle(QStringLiteral("Flux Studio"));resize(1680,1040);setMinimumSize(1100,720);setDockNestingEnabled(true);
    m_canvas=new FluxCanvas(this);m_canvas->setDocument(m_document);
    m_stack=new QStackedWidget(this);m_home=new QWidget(m_stack);m_stack->addWidget(m_home);m_stack->addWidget(m_canvas);setCentralWidget(m_stack);
    connect(m_canvas,&FluxCanvas::documentChanged,this,&FluxMainWindow::markModified);
    connect(m_canvas,&FluxCanvas::cursorInfoChanged,this,[this](const QString&s){if(m_cursorLabel)m_cursorLabel->setText(s);});
    connect(m_canvas,&FluxCanvas::zoomChanged,this,&FluxMainWindow::updateZoomLabel);
    connect(m_canvas,&FluxCanvas::wheelRequested,this,[this](const QPoint&p){if(m_wheel)m_wheel->openAt(p);});
    connect(m_playTimer,&QTimer::timeout,this,[this]{const int f=m_document->frame()+1;const int next=f>=m_document->frameCount()?(m_playing?0:m_document->frameCount()-1):f;m_document->setFrame(next);m_canvas->update();});
    m_playTimer->setInterval(1000/24);
    buildHome();buildMenus();buildTopBar();buildToolRail();m_wheel=new FluxWheel(this);
    connect(m_wheel,&FluxWheel::commandTriggered,this,[this](int i){switch(i){case 0:m_canvas->setTool("Brush");break;case 1:m_canvas->setTool("Pencil");break;case 2:m_canvas->setTool("Eraser");break;case 3:m_canvas->setTool("Ink");break;case 4:chooseColor();break;case 5:setFrameFromTimeline(m_document->frame()-1);break;case 6:setFrameFromTimeline(m_document->frame()+1);break;default:fitCanvas();break;}m_canvas->update();});
    m_autosavePath=QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)+QStringLiteral("/recovery.flux");
    auto*timer=new QTimer(this);timer->setInterval(30000);connect(timer,&QTimer::timeout,this,&FluxMainWindow::autosave);timer->start();
    m_recoveryId=QStringLiteral("untitled");FluxRecoveryManager::markRunning(m_recoveryId,nullptr);
    restoreLastSession();m_stack->setCurrentWidget(m_home);if(m_topBar)m_topBar->hide();if(m_toolRail)m_toolRail->hide();polish();
}
FluxMainWindow::~FluxMainWindow(){autosave();FluxRecoveryManager::markClean(m_recoveryId);delete m_document;}

void FluxMainWindow::buildHome(){
    auto*root=new QVBoxLayout(m_home);root->setContentsMargins(68,48,68,34);root->setSpacing(24);
    auto*header=new QHBoxLayout;header->setSpacing(16);
    auto*logo=new QLabel(m_home);logo->setFixedSize(62,62);logo->setScaledContents(true);logo->setPixmap(QPixmap(QStringLiteral(":/branding/flux-logo.svg")));header->addWidget(logo);
    auto*brand=new QVBoxLayout;brand->setSpacing(1);brand->addWidget(uiLabel("FLUX","brand"));brand->addWidget(uiLabel("STUDIO / 0.9","brandSub"));header->addLayout(brand);header->addStretch();header->addWidget(uiLabel("NATIVE  •  LOCAL  •  PROFESSIONAL","meta"),0,Qt::AlignTop|Qt::AlignRight);root->addLayout(header);
    auto*hero=new QVBoxLayout;hero->setSpacing(5);hero->addWidget(uiLabel("PROJECT HUB","eyebrow"));hero->addWidget(uiLabel("Make. Animate. Finish.","headline"));hero->addWidget(uiLabel("A focused workspace for illustration, 2D animation, compositing and final delivery.","subhead"));root->addLayout(hero);
    auto*actions=new QGridLayout;actions->setHorizontalSpacing(14);actions->setVerticalSpacing(14);
    auto*fresh=card("NEW PROJECT","Start a clean document with a production preset.",m_home);fresh->setObjectName("homePrimary");actions->addWidget(fresh,0,0,1,2);
    auto*open=card("OPEN PROJECT","Open a packaged or legacy .flux file.",m_home);actions->addWidget(open,0,2);
    auto*resume=card("RECOVER / RESUME","Review the latest recoverable workspace snapshot.",m_home);actions->addWidget(resume,1,0);
    auto*brush=card("BRUSH LAB","Tune pressure, texture, spacing and presets.",m_home);actions->addWidget(brush,1,1);
    auto*comp=card("COMPOSITOR","Build effects and finishing chains visually.",m_home);actions->addWidget(comp,1,2);root->addLayout(actions);
    auto*recentBox=new QGroupBox(QStringLiteral("RECENT PROJECTS"),m_home);recentBox->setObjectName("homeRecentBox");auto*recentLayout=new QVBoxLayout(recentBox);recentLayout->setContentsMargins(14,18,14,14);auto*recent=new QListWidget(recentBox);recent->setObjectName("homeRecentList");recent->setMinimumHeight(150);recentLayout->addWidget(recent);root->addWidget(recentBox,1);
    auto populate=[recent](){recent->clear();for(const auto&p:FluxWorkflow::recentProjects(12)){if(!QFileInfo::exists(p))continue;auto*i=new QListWidgetItem(QFileInfo(p).completeBaseName());i->setData(Qt::UserRole,p);i->setToolTip(p);recent->addItem(i);}if(recent->count()==0){auto*i=new QListWidgetItem(QStringLiteral("No recent projects yet — start a new one above."));i->setFlags(Qt::NoItemFlags);recent->addItem(i);}};populate();
    connect(fresh,&QPushButton::clicked,this,&FluxMainWindow::newProject);connect(open,&QPushButton::clicked,this,&FluxMainWindow::openProject);connect(resume,&QPushButton::clicked,this,&FluxMainWindow::showRecoveryBrowser);connect(recent,&QListWidget::itemDoubleClicked,this,[this](QListWidgetItem*i){if(i)loadProjectPath(i->data(Qt::UserRole).toString());});connect(brush,&QPushButton::clicked,this,[this]{enterWorkspace();openBrushEditor();});connect(comp,&QPushButton::clicked,this,[this]{enterWorkspace();});
    auto*foot=new QHBoxLayout;foot->addWidget(uiLabel("DRAW  →  ANIMATE  →  COMPOSE  →  EXPORT","footer"));foot->addStretch();foot->addWidget(uiLabel("Ctrl+N   Ctrl+O   Ctrl+K   Ctrl+Shift+H","hint"));root->addLayout(foot);
}

void FluxMainWindow::buildMenus(){
    auto*file=menuBar()->addMenu("File");file->addAction("New Project",QKeySequence("Ctrl+N"),this,&FluxMainWindow::newProject);file->addAction("Open…",QKeySequence("Ctrl+O"),this,&FluxMainWindow::openProject);file->addSeparator();file->addAction("Save",QKeySequence::Save,this,&FluxMainWindow::saveProject);file->addAction("Save As…",QKeySequence("Ctrl+Shift+S"),this,&FluxMainWindow::saveProjectAs);file->addSeparator();file->addAction("Export Image…",this,&FluxMainWindow::exportImage);file->addAction("Home",QKeySequence("Ctrl+Shift+H"),this,&FluxMainWindow::returnHome);file->addAction("Quit",QKeySequence::Quit,this,&QWidget::close);
    auto*edit=menuBar()->addMenu("Edit");edit->addAction("Undo",QKeySequence::Undo,this,&FluxMainWindow::undo);edit->addAction("Redo",QKeySequence::Redo,this,&FluxMainWindow::redo);edit->addAction("Select All",QKeySequence("Ctrl+A"),this,&FluxMainWindow::selectAll);edit->addAction("Deselect",QKeySequence("Ctrl+Shift+A"),this,&FluxMainWindow::deselect);edit->addSeparator();edit->addAction("Brush Editor…",this,&FluxMainWindow::openBrushEditor);
    auto*view=menuBar()->addMenu("View");view->addAction("Fit Canvas",this,&FluxMainWindow::fitCanvas);view->addAction("Mirror Horizontal",this,&FluxMainWindow::mirrorHorizontal);view->addAction("Mirror Vertical",this,&FluxMainWindow::mirrorVertical);view->addAction("Rotate 90°",this,&FluxMainWindow::rotateCanvas);view->addAction("Toggle Onion Skin",this,&FluxMainWindow::toggleOnionSkin);view->addAction("Command Palette",QKeySequence("Ctrl+K"),this,&FluxMainWindow::showCommandPalette);
    auto*layer=menuBar()->addMenu("Layer");layer->addAction("Add Paint Layer",this,&FluxMainWindow::addLayer);layer->addAction("Add Group",this,&FluxMainWindow::addGroup);layer->addAction("Add Mask",this,&FluxMainWindow::addMask);layer->addAction("Add Vector Mask",this,[this]{m_document->addMask(m_document->activeLayerIndex(),true);refreshLayers();markModified();});layer->addAction("Add Adjustment",this,[this]{m_document->addAdjustment("Adjustment "+QString::number(m_document->layers().size()));refreshLayers();markModified();});layer->addSeparator();layer->addAction("Duplicate",this,&FluxMainWindow::duplicateLayer);layer->addAction("Merge Down",this,&FluxMainWindow::mergeDown);layer->addAction("Flatten Visible",this,&FluxMainWindow::flattenVisible);layer->addAction("Delete",this,&FluxMainWindow::removeLayer);
    auto*animation=menuBar()->addMenu("Animation");animation->addAction("Previous Frame",QKeySequence("Left"),this,[this]{setFrameFromTimeline(m_document->frame()-1);});animation->addAction("Next Frame",QKeySequence("Right"),this,[this]{setFrameFromTimeline(m_document->frame()+1);});animation->addAction("Play / Pause",QKeySequence("Space"),this,&FluxMainWindow::togglePlayback);animation->addAction("Insert Frame",this,[this]{m_document->setFrameCount(m_document->frameCount()+1);markModified();});animation->addAction("Delete Frame",this,[this]{if(m_document->frameCount()>1)m_document->setFrameCount(m_document->frameCount()-1);markModified();});
    auto*window=menuBar()->addMenu("Window");window->addAction("Studio Panels",this,[this]{enterWorkspace();});window->addAction("Advanced Production Suite",this,[this]{enterWorkspace();if(auto*d=findChild<QDockWidget*>("AdvancedSuiteDock"))d->show();});window->addAction("Timeline",this,[this]{enterWorkspace();if(auto*d=findChild<QDockWidget*>("TimelineDock"))d->show();});
}

void FluxMainWindow::buildTopBar(){
    m_topBar=addToolBar("Flux Toolbar");m_topBar->setObjectName("FluxTopBar");m_topBar->setMovable(false);m_topBar->setFloatable(false);m_topBar->setIconSize(QSize(18,18));m_topBar->setToolButtonStyle(Qt::ToolButtonIconOnly);
    auto*home=m_topBar->addAction(style()->standardIcon(QStyle::SP_DirHomeIcon),"Home");connect(home,&QAction::triggered,this,&FluxMainWindow::returnHome);
    auto*newAct=m_topBar->addAction(style()->standardIcon(QStyle::SP_FileIcon),"New");connect(newAct,&QAction::triggered,this,&FluxMainWindow::newProject);
    auto*openAct=m_topBar->addAction(style()->standardIcon(QStyle::SP_DialogOpenButton),"Open");connect(openAct,&QAction::triggered,this,&FluxMainWindow::openProject);
    auto*saveAct=m_topBar->addAction(style()->standardIcon(QStyle::SP_DialogSaveButton),"Save");connect(saveAct,&QAction::triggered,this,&FluxMainWindow::saveProject);
    m_topBar->addSeparator();auto*undoAct=m_topBar->addAction(style()->standardIcon(QStyle::SP_ArrowBack),"Undo");connect(undoAct,&QAction::triggered,this,&FluxMainWindow::undo);auto*redoAct=m_topBar->addAction(style()->standardIcon(QStyle::SP_ArrowForward),"Redo");connect(redoAct,&QAction::triggered,this,&FluxMainWindow::redo);
    m_topBar->addSeparator();m_topBar->addWidget(uiLabel("TOOL","topLabel"));auto*tool=new QComboBox;tool->addItems({"Brush","Pencil","Ink","Airbrush","Marker","Eraser","Line","Rectangle","Ellipse","Polygon","Star","Bezier","Gradient","Text","Fill","Color Picker","Transform","Pan","Zoom"});tool->setMinimumWidth(130);m_topBar->addWidget(tool);connect(tool,&QComboBox::currentTextChanged,this,[this](const QString&s){m_canvas->setTool(s);});
    m_topBar->addWidget(uiLabel("SIZE","topLabel"));m_brushSlider=new QSlider(Qt::Horizontal);m_brushSlider->setRange(1,1000);m_brushSlider->setValue(24);m_brushSlider->setFixedWidth(110);m_topBar->addWidget(m_brushSlider);connect(m_brushSlider,&QSlider::valueChanged,this,&FluxMainWindow::updateBrushSize);m_brushSizeLabel=uiLabel("24 px","topValue");m_topBar->addWidget(m_brushSizeLabel);
    m_topBar->addSeparator();m_colorSwatch=uiLabel("●","swatch");m_topBar->addWidget(m_colorSwatch);auto*play=m_topBar->addAction(style()->standardIcon(QStyle::SP_MediaPlay),"Play");connect(play,&QAction::triggered,this,&FluxMainWindow::togglePlayback);auto*fit=m_topBar->addAction("Fit");connect(fit,&QAction::triggered,this,&FluxMainWindow::fitCanvas);m_zoomLabel=uiLabel("100%","topValue");m_topBar->addWidget(m_zoomLabel);m_cursorLabel=uiLabel("0, 0","cursor");m_topBar->addWidget(m_cursorLabel);auto*cmd=m_topBar->addAction("⌘");cmd->setToolTip(QStringLiteral("Command Palette (Ctrl+K)"));connect(cmd,&QAction::triggered,this,&FluxMainWindow::showCommandPalette);
}

void FluxMainWindow::buildToolRail(){
    m_toolRail=new QToolBar("Flux Tools",this);m_toolRail->setObjectName("FluxToolRail");m_toolRail->setMovable(false);m_toolRail->setFloatable(false);m_toolRail->setOrientation(Qt::Vertical);m_toolRail->setIconSize(QSize(18,18));m_toolRail->setToolButtonStyle(Qt::ToolButtonTextOnly);addToolBar(Qt::LeftToolBarArea,m_toolRail);
    auto*group=new QActionGroup(this);group->setExclusive(true);const QStringList names={"Brush","Pencil","Ink","Eraser","Line","Rectangle","Ellipse","Polygon","Star","Bezier","Gradient","Text","Fill","Color Picker","Transform","Pan","Zoom"};const QStringList glyphs={"B","P","I","E","╱","▢","○","⬡","★","⌁","▤","T","▾","◉","✥","✋","⌕"};
    for(int i=0;i<names.size();++i){auto*a=m_toolRail->addAction(glyphs[i]);a->setCheckable(true);a->setData(names[i]);a->setToolTip(names[i]);group->addAction(a);connect(a,&QAction::triggered,this,&FluxMainWindow::setToolFromAction);if(i==0)a->setChecked(true);}m_toolRail->addSeparator();m_toolRail->addAction("R",this,[this]{const auto f=QFileDialog::getOpenFileName(this,"Reference Image",{},"Images (*.png *.jpg *.jpeg *.webp)");if(!f.isEmpty())m_canvas->loadReference(f);});m_toolRail->addAction("O",this,&FluxMainWindow::toggleOnionSkin);m_toolRail->addAction("Y",this,[this]{m_canvas->setSymmetry(true,true);});
}

void FluxMainWindow::buildDocks(){
    auto*studio=new QDockWidget("Studio Panels",this);studio->setObjectName("StudioDock");studio->setMinimumWidth(310);studio->setMaximumWidth(390);auto*tabs=new QTabWidget(studio);tabs->setDocumentMode(true);tabs->addTab(makeColorPanel(),"Color");tabs->addTab(makeBrushPanel(),"Brushes");tabs->addTab(makeLayersPanel(),"Layers");tabs->addTab(makeInspectorPanel(),"Tool Options");tabs->addTab(new FluxCompositorWidget,"Compose");m_production=new FluxProductionDock(m_document,m_canvas,studio);tabs->addTab(m_production,"Production");studio->setWidget(tabs);addDockWidget(Qt::RightDockWidgetArea,studio);
    connect(m_production,&FluxProductionDock::requestNewProject,this,&FluxMainWindow::newProject);connect(m_production,&FluxProductionDock::requestOpenProject,this,&FluxMainWindow::openProject);connect(m_production,&FluxProductionDock::requestSaveProject,this,&FluxMainWindow::saveProject);
    auto*timeline=new QDockWidget("Timeline",this);timeline->setObjectName("TimelineDock");timeline->setMinimumHeight(220);timeline->setWidget(makeTimelinePanel());addDockWidget(Qt::BottomDockWidgetArea,timeline);
    auto*advanced=new QDockWidget("Advanced Production Suite",this);advanced->setObjectName("AdvancedSuiteDock");advanced->setWidget(new FluxAdvancedSuite(m_document,m_canvas,advanced));addDockWidget(Qt::LeftDockWidgetArea,advanced);advanced->hide();
    studio->raise();
}

QWidget*FluxMainWindow::makeColorPanel(){
    auto*w=new QWidget;auto*l=new QVBoxLayout(w);l->setContentsMargins(12,12,12,12);l->setSpacing(10);l->addWidget(uiLabel("COLOR SELECTOR","panelTitle"));
    m_colorWheel=new FluxColorWheel(w);l->addWidget(m_colorWheel,0,Qt::AlignHCenter);auto*hex=new QLineEdit("#2A80FF",w);hex->setPlaceholderText("#RRGGBB");l->addWidget(hex);
    auto*apply=new QPushButton("Apply Color",w);l->addWidget(apply);
    auto*swatches=new QGridLayout;swatches->setSpacing(7);const QStringList colors={"#111318","#F2F4F7","#FF3B30","#FF9500","#FFD60A","#34C759","#30D5C8","#0A84FF","#5856D6","#AF52DE","#FF2D55","#8E8E93"};for(int i=0;i<colors.size();++i){auto*b=new QPushButton(w);b->setFixedSize(30,30);b->setCursor(Qt::PointingHandCursor);b->setStyleSheet(QString("QPushButton{background:%1;border:1px solid rgba(255,255,255,.25);border-radius:6px;}QPushButton:hover{border:2px solid #ffffff;}").arg(colors[i]));swatches->addWidget(b,i/6,i%6);connect(b,&QPushButton::clicked,this,[this,hex,colors,i]{hex->setText(colors[i]);m_document->setForeground(QColor(colors[i]));m_canvas->setBrushColor(QColor(colors[i]));if(m_colorWheel)m_colorWheel->setColor(QColor(colors[i]));if(m_colorSwatch)m_colorSwatch->setStyleSheet(QString("QLabel{color:%1;font-size:24px}").arg(colors[i]));});}l->addLayout(swatches);l->addWidget(uiLabel("Recent / production palette","hint"));l->addStretch();
    auto applyColor=[this,hex]{const QColor c(hex->text().trimmed());if(!c.isValid())return;m_document->setForeground(c);m_canvas->setBrushColor(c);if(m_colorWheel)m_colorWheel->setColor(c);if(m_colorSwatch)m_colorSwatch->setStyleSheet(QString("QLabel{color:%1;font-size:24px}").arg(c.name()));};connect(apply,&QPushButton::clicked,this,applyColor);connect(hex,&QLineEdit::returnPressed,this,applyColor);connect(m_colorWheel,&FluxColorWheel::colorChanged,this,[this,hex](const QColor&c){hex->setText(c.name().toUpper());m_document->setForeground(c);m_canvas->setBrushColor(c);if(m_colorSwatch)m_colorSwatch->setStyleSheet(QString("QLabel{color:%1;font-size:24px}").arg(c.name()));});
    m_colorWheel->setColor(m_document->foreground());return w;
}

QWidget*FluxMainWindow::makeBrushPanel(){auto*w=new QWidget;auto*l=new QVBoxLayout(w);l->setContentsMargins(12,12,12,12);l->addWidget(uiLabel("BRUSH PRESETS","panelTitle"));auto*search=new QLineEdit;search->setPlaceholderText("Search brushes…");l->addWidget(search);auto*list=new QListWidget;list->setMinimumHeight(260);const QStringList names={"Flux Ink","Pencil","Dry Marker","Soft Airbrush","Hard Eraser","Texture Scatter","Wet Mix","Sketch","Chalk","Pixel","Line Control","Clean Fill"};for(const auto&n:names){auto*i=new QListWidgetItem(QStringLiteral("●   ")+n);i->setSizeHint(QSize(0,34));list->addItem(i);}l->addWidget(list,1);auto*row=new QHBoxLayout;auto*editor=new QPushButton("Brush Editor…");auto*fav=new QPushButton("★ Favorite");row->addWidget(editor);row->addWidget(fav);l->addLayout(row);connect(editor,&QPushButton::clicked,this,&FluxMainWindow::openBrushEditor);connect(fav,&QPushButton::clicked,this,[list]{if(auto*i=list->currentItem())if(!i->text().startsWith("★"))i->setText(QStringLiteral("★ ")+i->text());});connect(search,&QLineEdit::textChanged,this,[list](const QString&s){for(int i=0;i<list->count();++i)list->item(i)->setHidden(!list->item(i)->text().contains(s,Qt::CaseInsensitive));});return w;}

QWidget*FluxMainWindow::makeLayersPanel(){auto*root=new QWidget;auto*lay=new QVBoxLayout(root);lay->setContentsMargins(12,12,12,10);lay->addWidget(uiLabel("LAYERS","panelTitle"));m_layers=new FluxLayerTree;m_layers->setHeaderLabels({"Layer","%"});m_layers->setSelectionMode(QAbstractItemView::ExtendedSelection);m_layers->setDragDropMode(QAbstractItemView::InternalMove);m_layers->setDefaultDropAction(Qt::MoveAction);m_layers->header()->setSectionResizeMode(0,QHeaderView::Stretch);m_layers->header()->setSectionResizeMode(1,QHeaderView::ResizeToContents);lay->addWidget(m_layers,1);connect(m_layers,&QTreeWidget::itemSelectionChanged,this,&FluxMainWindow::layerSelectionChanged);connect(m_layers,&QTreeWidget::itemChanged,this,&FluxMainWindow::layerItemChanged);connect(m_layers,&FluxLayerTree::hierarchyDropped,this,&FluxMainWindow::syncLayerTreeToDocument);auto*row=new QHBoxLayout;auto*paint=new QPushButton("+ Paint");auto*group=new QPushButton("+ Group");auto*mask=new QPushButton("+ Mask");row->addWidget(paint);row->addWidget(group);row->addWidget(mask);connect(paint,&QPushButton::clicked,this,&FluxMainWindow::addLayer);connect(group,&QPushButton::clicked,this,&FluxMainWindow::addGroup);connect(mask,&QPushButton::clicked,this,&FluxMainWindow::addMask);lay->addLayout(row);auto*ops=new QHBoxLayout;for(const auto&spec:QStringList{"Dup","Merge","Flat","Del"}){auto*b=new QPushButton(spec);ops->addWidget(b);if(spec=="Dup")connect(b,&QPushButton::clicked,this,&FluxMainWindow::duplicateLayer);else if(spec=="Merge")connect(b,&QPushButton::clicked,this,&FluxMainWindow::mergeDown);else if(spec=="Flat")connect(b,&QPushButton::clicked,this,&FluxMainWindow::flattenVisible);else connect(b,&QPushButton::clicked,this,&FluxMainWindow::removeLayer);}lay->addLayout(ops);return root;}

QWidget*FluxMainWindow::makeInspectorPanel(){auto*root=new QWidget;auto*lay=new QVBoxLayout(root);lay->setContentsMargins(12,12,12,12);lay->addWidget(uiLabel("TOOL OPTIONS","panelTitle"));auto*box=new QGroupBox("Layer");auto*f=new QFormLayout(box);m_blendMode=new QComboBox;m_blendMode->addItems({"Normal","Multiply","Screen","Overlay","Add","Subtract"});auto*opacity=new QSlider(Qt::Horizontal);opacity->setRange(0,100);opacity->setValue(100);f->addRow("Blend",m_blendMode);f->addRow("Opacity",opacity);connect(m_blendMode,qOverload<int>(&QComboBox::currentIndexChanged),this,&FluxMainWindow::setLayerBlendMode);connect(opacity,&QSlider::valueChanged,this,&FluxMainWindow::setLayerOpacity);lay->addWidget(box);
    auto*brush=new QGroupBox("Brush dynamics");auto*bf=new QFormLayout(brush);auto*flow=new QSlider(Qt::Horizontal);flow->setRange(0,100);flow->setValue(100);auto*stab=new QSlider(Qt::Horizontal);stab->setRange(0,100);stab->setValue(12);bf->addRow("Flow",flow);bf->addRow("Stabilizer",stab);connect(stab,&QSlider::valueChanged,this,[this](int v){m_canvas->setStabilization(v/100.0);});lay->addWidget(brush);
    auto*view=new QGroupBox("Canvas View");auto*vf=new QGridLayout(view);auto*onion=new QPushButton("Onion");onion->setCheckable(true);onion->setChecked(true);auto*grid=new QPushButton("Grid");grid->setCheckable(true);grid->setChecked(false);auto*rulers=new QPushButton("Rulers");rulers->setCheckable(true);rulers->setChecked(false);auto*perspective=new QPushButton("Perspective");perspective->setCheckable(true);vf->addWidget(onion,0,0);vf->addWidget(grid,0,1);vf->addWidget(rulers,1,0);vf->addWidget(perspective,1,1);connect(onion,&QPushButton::toggled,this,[this](bool v){m_canvas->toggleOnionSkin(v);});connect(grid,&QPushButton::toggled,this,[this](bool v){m_canvas->setGridEnabled(v);});connect(rulers,&QPushButton::toggled,this,[this](bool v){m_canvas->setRulersEnabled(v);});connect(perspective,&QPushButton::toggled,this,[this](bool v){m_canvas->setPerspectiveGuide(v);});lay->addWidget(view);lay->addStretch();return root;}

QWidget*FluxMainWindow::makeTimelinePanel(){auto*root=new QWidget;auto*lay=new QVBoxLayout(root);auto*head=new QHBoxLayout;head->addWidget(uiLabel("TIMELINE","panelTitle"));head->addWidget(uiLabel("FPS"));m_fps=new QSpinBox;m_fps->setRange(1,240);m_fps->setValue(24);head->addWidget(m_fps);auto*play=new QPushButton("▶");play->setCheckable(true);head->addWidget(play);auto*frameLabel=new QLabel("Frame 1 / 1");head->addWidget(frameLabel);head->addStretch();lay->addLayout(head);auto*timeline=new FluxTimelineWidget(m_document,root);connect(play,&QPushButton::toggled,this,[this,play](bool on){m_playing=on;if(on)m_playTimer->start();else m_playTimer->stop();play->setText(on?"■":"▶");});connect(m_fps,qOverload<int>(&QSpinBox::valueChanged),this,[this](int v){m_playTimer->setInterval(qMax(1,1000/v));});connect(timeline,&FluxTimelineWidget::frameChanged,this,[this,frameLabel](int f){m_document->setFrame(f);frameLabel->setText(QString("Frame %1 / %2").arg(f+1).arg(m_document->frameCount()));m_canvas->update();});connect(timeline,&FluxTimelineWidget::documentEdited,this,[this]{markModified();m_canvas->update();});lay->addWidget(timeline,1);return root;}

void FluxMainWindow::newProject(){QDialog dlg(this);dlg.setWindowTitle("New Flux Project");auto*lay=new QVBoxLayout(&dlg);lay->addWidget(uiLabel("NEW PROJECT","dialogEyebrow"));lay->addWidget(uiLabel("Choose the production format before entering the workspace.","dialogTitle"));auto*form=new QFormLayout;auto*preset=new QComboBox;preset->addItems({"1920 × 1080 / 24 fps","2048 × 2048 / 24 fps","1280 × 720 / 30 fps","1080 × 1920 / 30 fps","Custom"});auto*w=new QSpinBox;w->setRange(1,16384);w->setValue(1920);auto*h=new QSpinBox;h->setRange(1,16384);h->setValue(1080);auto*fps=new QSpinBox;fps->setRange(1,240);fps->setValue(24);form->addRow("Preset",preset);form->addRow("Width",w);form->addRow("Height",h);form->addRow("FPS",fps);lay->addLayout(form);auto*b=new QDialogButtonBox(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);lay->addWidget(b);connect(preset,qOverload<int>(&QComboBox::currentIndexChanged),[=](int i){if(i==0){w->setValue(1920);h->setValue(1080);fps->setValue(24);}else if(i==1){w->setValue(2048);h->setValue(2048);fps->setValue(24);}else if(i==2){w->setValue(1280);h->setValue(720);fps->setValue(30);}else if(i==3){w->setValue(1080);h->setValue(1920);fps->setValue(30);}const bool custom=i==4;w->setEnabled(custom);h->setEnabled(custom);fps->setEnabled(custom);});connect(b,&QDialogButtonBox::accepted,&dlg,&QDialog::accept);connect(b,&QDialogButtonBox::rejected,&dlg,&QDialog::reject);if(dlg.exec()!=QDialog::Accepted)return;m_document->create("Untitled",w->value(),h->value());m_document->activeImage().fill(Qt::white);m_filePath.clear();m_recoveryId="untitled";if(m_fps)m_fps->setValue(fps->value());m_canvas->setDocument(m_document);enterWorkspace();markModified();}
void FluxMainWindow::openProject(){const auto f=QFileDialog::getOpenFileName(this,"Open Flux Project",{},"Flux Project (*.flux)");if(!f.isEmpty())loadProjectPath(f);}
void FluxMainWindow::loadProjectPath(const QString&path){QString e;const bool package=FluxProjectPackage::isPackage(path);const bool ok=package?FluxProjectPackage::load(path,*m_document,&e):m_document->load(path,&e);if(!ok){QMessageBox::critical(this,"Flux Studio",e);return;}m_filePath=path;m_recoveryId=QFileInfo(path).completeBaseName();FluxWorkflow::addRecentProject(path,12);m_canvas->setDocument(m_document);enterWorkspace();setWindowTitle("Flux Studio — "+QFileInfo(path).completeBaseName());setStatus("Opened "+QFileInfo(path).completeBaseName());}
void FluxMainWindow::saveProject(){if(m_filePath.isEmpty()){saveProjectAs();return;}QString e;if(!FluxProjectPackage::save(m_filePath,*m_document,&e)){QMessageBox::critical(this,"Save failed",e);return;}FluxWorkflow::addRecentProject(m_filePath,12);setWindowTitle("Flux Studio — "+QFileInfo(m_filePath).completeBaseName());setStatus("Saved");}
void FluxMainWindow::saveProjectAs(){auto f=QFileDialog::getSaveFileName(this,"Save Flux Project",{},"Flux Project (*.flux)");if(f.isEmpty())return;if(!f.endsWith(".flux",Qt::CaseInsensitive))f+=".flux";m_filePath=f;saveProject();}
void FluxMainWindow::exportImage(){const auto f=QFileDialog::getSaveFileName(this,"Export Image",{},"PNG (*.png);;JPEG (*.jpg);;WebP (*.webp);;SVG (*.svg)");if(f.isEmpty())return;QString e;const bool ok=f.endsWith(".svg",Qt::CaseInsensitive)?FluxExportEngine::exportSvgRaster(m_document->composite(),f,FluxRenderSettings{},&e):m_document->exportImage(f,&e);if(!ok)QMessageBox::critical(this,"Export failed",e);else setStatus("Exported "+QFileInfo(f).fileName());}

void FluxMainWindow::enterWorkspace(){if(!m_workspaceBuilt){buildDocks();m_workspaceBuilt=true;}m_stack->setCurrentWidget(m_canvas);if(m_topBar)m_topBar->show();if(m_toolRail)m_toolRail->show();for(auto*d:findChildren<QDockWidget*>())if(d->objectName()!="AdvancedSuiteDock")d->show();m_canvas->setFocus();m_canvas->fitCanvas();refreshLayers();if(m_production)m_production->update();}
void FluxMainWindow::returnHome(){m_playing=false;m_playTimer->stop();m_stack->setCurrentWidget(m_home);if(m_topBar)m_topBar->hide();if(m_toolRail)m_toolRail->hide();for(auto*d:findChildren<QDockWidget*>())d->hide();setWindowTitle(m_filePath.isEmpty()?"Flux Studio":"Flux Studio — "+QFileInfo(m_filePath).completeBaseName());}
void FluxMainWindow::openRecentProject(){}
void FluxMainWindow::rememberRecent(const QString&path){FluxWorkflow::addRecentProject(path,12);updateHomeRecent();}
void FluxMainWindow::updateHomeRecent(){}
void FluxMainWindow::addLayer(){int parent=-1;if(m_layers&&m_layers->currentItem()){const int i=m_layers->currentItem()->data(0,Qt::UserRole).toInt();if(i>=0&&i<m_document->layers().size()&&m_document->layers()[i].type==FluxLayerType::Group)parent=i;}m_document->addLayer("Paint Layer "+QString::number(m_document->layers().size()),FluxLayerType::Paint,parent);refreshLayers();markModified();m_canvas->update();}
void FluxMainWindow::addGroup(){m_document->addGroup("Group "+QString::number(m_document->layers().size()));refreshLayers();markModified();}
void FluxMainWindow::addMask(){m_document->addMask(m_document->activeLayerIndex(),false);refreshLayers();markModified();m_canvas->update();}
void FluxMainWindow::duplicateLayer(){m_document->duplicateLayer(m_document->activeLayerIndex());refreshLayers();markModified();m_canvas->update();}
void FluxMainWindow::removeLayer(){m_document->removeLayer(m_document->activeLayerIndex());refreshLayers();markModified();m_canvas->update();}
void FluxMainWindow::mergeDown(){m_document->mergeDown(m_document->activeLayerIndex());refreshLayers();markModified();m_canvas->update();}
void FluxMainWindow::flattenVisible(){m_document->flattenVisible();refreshLayers();markModified();m_canvas->update();}
void FluxMainWindow::layerSelectionChanged(){if(m_syncingLayers||!m_layers||!m_layers->currentItem())return;const int i=m_layers->currentItem()->data(0,Qt::UserRole).toInt();m_document->setActiveLayer(i);const auto&l=m_document->activeLayer();if(m_blendMode){QSignalBlocker b(m_blendMode);m_blendMode->setCurrentText(FluxDocument::blendModeName(l.blendMode));}m_canvas->update();}
void FluxMainWindow::layerItemChanged(QTreeWidgetItem*item,int column){if(m_syncingLayers||!item||column!=0)return;const int i=item->data(0,Qt::UserRole).toInt();if(i>=0&&i<m_document->layers().size()){m_document->setLayerVisible(i,item->checkState(0)==Qt::Checked);markModified();m_canvas->update();}}
void FluxMainWindow::setLayerOpacity(int v){m_document->setLayerOpacity(m_document->activeLayerIndex(),v/100.0);markModified();m_canvas->update();}
void FluxMainWindow::setLayerBlendMode(int i){if(m_blendMode)m_document->setLayerBlendMode(m_document->activeLayerIndex(),FluxDocument::blendModeFromName(m_blendMode->itemText(i)));markModified();m_canvas->update();}
void FluxMainWindow::setLayerLocked(bool e){m_document->setLayerLocked(m_document->activeLayerIndex(),e);refreshLayers();markModified();}
void FluxMainWindow::setLayerClipping(bool e){m_document->setLayerClipping(m_document->activeLayerIndex(),e);markModified();m_canvas->update();}
void FluxMainWindow::setLayerAlphaInheritance(bool e){m_document->setLayerAlphaInherited(m_document->activeLayerIndex(),e);markModified();m_canvas->update();}
void FluxMainWindow::setLayerSolo(bool e){m_document->setSolo(m_document->activeLayerIndex(),e);m_canvas->update();}
void FluxMainWindow::setLayerIsolate(bool e){m_document->setIsolate(m_document->activeLayerIndex(),e);m_canvas->update();}
void FluxMainWindow::setLayerLabelColor(){const auto c=QColorDialog::getColor(m_document->activeLayer().labelColor.isValid()?m_document->activeLayer().labelColor:Qt::white,this,"Layer Label Color");if(c.isValid()){m_document->setLayerLabelColor(m_document->activeLayerIndex(),c);refreshLayers();markModified();}}
void FluxMainWindow::setLayerStyle(){auto s=m_document->activeLayer().style;s.enabled=!s.enabled;if(s.enabled){s.outlineOpacity=.8;s.outlineWidth=2;s.outlineColor=m_document->foreground();}m_document->setLayerStyle(m_document->activeLayerIndex(),s);markModified();m_canvas->update();}
void FluxMainWindow::syncLayerTreeToDocument(){if(!m_layers||m_syncingLayers)return;QVector<int>order,parents;std::function<void(QTreeWidgetItem*,int)>walk=[&](QTreeWidgetItem*p,int parentNew){const int count=p?p->childCount():m_layers->topLevelItemCount();for(int k=0;k<count;++k){auto*it=p?p->child(k):m_layers->topLevelItem(k);order.push_back(it->data(0,Qt::UserRole).toInt());parents.push_back(parentNew);walk(it,order.size()-1);}};walk(nullptr,-1);if(order.size()!=m_document->layers().size())return;const auto old=m_document->layers();QVector<FluxLayer>reordered;QHash<int,int>map;for(int j=0;j<order.size();++j){map[order[j]]=j;reordered.push_back(old[order[j]]);}for(int j=0;j<reordered.size();++j){reordered[j].parentIndex=parents[j]<0?-1:map.value(order[parents[j]],-1);if(reordered[j].maskTarget>=0)reordered[j].maskTarget=map.value(reordered[j].maskTarget,-1);}const int active=m_document->activeLayerIndex();m_document->layers()=reordered;m_document->setActiveLayer(map.value(active,0));markModified();m_canvas->update();}
void FluxMainWindow::refreshLayers(){if(!m_layers)return;m_syncingLayers=true;m_layers->clear();for(int i=0;i<m_document->layers().size();++i)if(m_document->layers()[i].parentIndex<0)addLayerTreeItem(nullptr,i);m_layers->setCurrentItem(nullptr);QList<QTreeWidgetItem*> stack;for(int i=0;i<m_layers->topLevelItemCount();++i)stack.push_back(m_layers->topLevelItem(i));while(!stack.isEmpty()){auto*it=stack.takeLast();if(it->data(0,Qt::UserRole).toInt()==m_document->activeLayerIndex()){m_layers->setCurrentItem(it);break;}for(int j=0;j<it->childCount();++j)stack.push_back(it->child(j));}m_syncingLayers=false;}
void FluxMainWindow::addLayerTreeItem(QTreeWidgetItem*parent,int index){const auto&l=m_document->layers()[index];auto*item=parent?new QTreeWidgetItem(parent):new QTreeWidgetItem(m_layers);const QString prefix=l.type==FluxLayerType::Group?"▾ ":l.type==FluxLayerType::Mask?"◐ ":l.type==FluxLayerType::VectorMask?"◇ ":l.type==FluxLayerType::Adjustment?"◈ ":"";item->setText(0,prefix+l.name);item->setText(1,QString::number(qRound(l.opacity*100)));item->setData(0,Qt::UserRole,index);item->setCheckState(0,l.visible?Qt::Checked:Qt::Unchecked);item->setToolTip(0,FluxDocument::layerTypeName(l.type));const QImage thumb=(l.frames.size()==m_document->frameCount()&&!l.frames[m_document->frame()].isNull())?l.frames[m_document->frame()]:l.image;if(!thumb.isNull())item->setIcon(0,QIcon(QPixmap::fromImage(thumb.scaled(30,30,Qt::KeepAspectRatio,Qt::FastTransformation))));for(int i=0;i<m_document->layers().size();++i)if(m_document->layers()[i].parentIndex==index)addLayerTreeItem(item,i);}
void FluxMainWindow::refreshTimeline(){}
void FluxMainWindow::setFrameFromTimeline(int row){if(row<0)return;m_document->setFrame(qBound(0,row,m_document->frameCount()-1));m_canvas->update();}
void FluxMainWindow::togglePlayback(){m_playing=!m_playing;if(m_playing)m_playTimer->start();else m_playTimer->stop();}
void FluxMainWindow::updateBrushSize(int value){m_canvas->setBrushSize(value);if(m_brushSizeLabel)m_brushSizeLabel->setText(QString::number(value)+" px");}
void FluxMainWindow::chooseColor(){const auto c=QColorDialog::getColor(m_document->foreground(),this,"Flux Color");if(c.isValid()){m_document->setForeground(c);m_canvas->setBrushColor(c);if(m_colorWheel)m_colorWheel->setColor(c);if(m_colorSwatch)m_colorSwatch->setStyleSheet(QString("QLabel{color:%1;font-size:24px}").arg(c.name()));}}
void FluxMainWindow::setToolFromAction(){if(auto*a=qobject_cast<QAction*>(sender()))m_canvas->setTool(a->data().toString());}
void FluxMainWindow::openBrushEditor(){BrushEditorDialog dlg(m_canvas->brushEngine(),this);if(dlg.exec()==QDialog::Accepted){const int s=m_canvas->brushEngine()->preset().size;updateBrushSize(s);}}
void FluxMainWindow::mirrorHorizontal(){m_mirrorH=!m_mirrorH;m_canvas->setMirrorHorizontal(m_mirrorH);}
void FluxMainWindow::mirrorVertical(){m_mirrorV=!m_mirrorV;m_canvas->setMirrorVertical(m_mirrorV);}
void FluxMainWindow::rotateCanvas(){m_canvasRotation+=90;if(m_canvasRotation>=360)m_canvasRotation=0;m_canvas->setCanvasRotation(m_canvasRotation);}
void FluxMainWindow::fitCanvas(){m_canvas->fitCanvas();}
void FluxMainWindow::toggleOnionSkin(){m_canvas->toggleOnionSkin(!m_canvas->onionSkin());}
void FluxMainWindow::selectAll(){m_canvas->selectAll();}
void FluxMainWindow::deselect(){m_canvas->clearSelection();}
void FluxMainWindow::undo(){m_canvas->undo();}
void FluxMainWindow::redo(){m_canvas->redo();}
void FluxMainWindow::autosave(){if(!m_document||m_autosavePath.isEmpty())return;FluxProjectPackage::save(m_autosavePath,*m_document,nullptr);}
void FluxMainWindow::restoreLastSession(){QSettings s("Flux","Flux Studio");restoreGeometry(s.value("geometry").toByteArray());}
void FluxMainWindow::setStatus(const QString&text){if(m_statusLabel)m_statusLabel->setText(text);statusBar()->showMessage(text,3000);}
void FluxMainWindow::updateZoomLabel(double zoom){if(m_zoomLabel)m_zoomLabel->setText(QString::number(qRound(zoom*100))+"%");}
void FluxMainWindow::showCommandPalette(){enterWorkspace();FluxCommandPalette dlg(this);QVector<FluxCommand>cmds={{"new","New Project","Ctrl+N",[this]{newProject();}},{"open","Open Project","Ctrl+O",[this]{openProject();}},{"save","Save Project","Ctrl+S",[this]{saveProject();}},{"saveas","Save As","Ctrl+Shift+S",[this]{saveProjectAs();}},{"home","Home","Ctrl+Shift+H",[this]{returnHome();}},{"brush","Brush Editor","",[this]{openBrushEditor();}},{"fit","Fit Canvas","",[this]{fitCanvas();}},{"undo","Undo","Ctrl+Z",[this]{undo();}},{"redo","Redo","Ctrl+Y",[this]{redo();}},{"onion","Onion Skin","",[this]{toggleOnionSkin();}},{"production","Production Center","",[this]{enterWorkspace();}}};dlg.setCommands(cmds);dlg.exec();}
