#include "fluxstudio_nextwindow.h"
#include "canvaswidget.h"
#include "fluxdocument.h"
#include "fluxwheel.h"
#include "fluxcolorwheel.h"
#include <QApplication>
#include <QAction>
#include <QActionGroup>
#include <QColorDialog>
#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDockWidget>
#include <QFileDialog>
#include <QFormLayout>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMenuBar>
#include <QMessageBox>
#include <QPainter>
#include <QPushButton>
#include <QSlider>
#include <QSpinBox>
#include <QStackedWidget>
#include <QStatusBar>
#include <QStyle>
#include <QTabWidget>
#include <QToolBar>
#include <QToolButton>
#include <QTimer>
#include <QVBoxLayout>
#include <algorithm>

namespace {
QLabel* label(const QString& text,const char* role=nullptr,QWidget* parent=nullptr){
    auto* w=new QLabel(text,parent);
    if(role) w->setProperty("role",role);
    return w;
}
QPushButton* homeCard(const QString& title,const QString& desc,QWidget* parent){
    auto* b=new QPushButton(title+QStringLiteral("\n")+desc,parent);
    b->setObjectName("fluxHomeCard");
    b->setCursor(Qt::PointingHandCursor);
    b->setMinimumHeight(112);
    return b;
}
}

FluxNextWindow::FluxNextWindow(QWidget* parent):QMainWindow(parent),m_document(new FluxDocument),m_playTimer(new QTimer(this)){
    resize(1720,1080);
    setMinimumSize(1200,760);
    setDockNestingEnabled(true);
    setWindowTitle(QStringLiteral("Flux Studio"));

    qApp->setStyleSheet(R"STYLE(
*{font-family:"Segoe UI";font-size:13px;color:#e6e9ee}
QMainWindow{background:#313438}
QMenuBar{background:#2a2c30;border:0;padding:1px 5px}
QMenuBar::item{padding:5px 9px;border-radius:3px}
QMenuBar::item:selected{background:#3b3f45}
QMenu{background:#2d3035;border:1px solid #464a50;padding:4px}
QMenu::item{padding:6px 18px;border-radius:3px}
QMenu::item:selected{background:#4a5058}
QToolBar{background:#2c2f34;border:0;border-bottom:1px solid #44484f;padding:3px;spacing:2px}
QToolBar#FluxTopBar QToolButton{min-width:30px;min-height:28px;padding:2px 5px;border-radius:3px}
QToolBar#FluxTopBar QToolButton:hover{background:#3d424a}
QToolBar#FluxToolRail{background:#2b2e33;border:0;border-right:1px solid #454950;padding:4px}
QToolBar#FluxToolRail QToolButton{min-width:38px;min-height:34px;margin:1px 0;border-radius:3px;font-size:15px}
QToolBar#FluxToolRail QToolButton:hover{background:#3a3f46}
QToolBar#FluxToolRail QToolButton:checked{background:#4a5058;border:1px solid #69717b;color:#fff}
QDockWidget{background:#2d3035;border:1px solid #474b52}
QDockWidget::title{background:#35383e;padding:6px 8px;border-bottom:1px solid #494d54;font-weight:600}
QTabWidget::pane{background:#292c31;border:0}
QTabBar::tab{background:#33363c;color:#aeb5be;padding:6px 10px;border:0}
QTabBar::tab:selected{background:#454a52;color:#fff}
QListWidget,QTreeWidget{background:#25282d;border:1px solid #44484f;outline:0}
QListWidget::item,QTreeWidget::item{padding:4px 5px}
QListWidget::item:selected,QTreeWidget::item:selected{background:#4b515a}
QComboBox,QSpinBox,QLineEdit{background:#24272c;border:1px solid #4b5057;border-radius:3px;padding:4px 6px;min-height:22px}
QPushButton,QToolButton{background:#34373c;border:1px solid #4a4f56;border-radius:3px;padding:5px 8px}
QPushButton:hover,QToolButton:hover{background:#3e434a}
QPushButton:pressed,QPushButton:checked{background:#4a5058}
QSlider::groove:horizontal{height:4px;background:#50565d;border-radius:2px}
QSlider::handle:horizontal{width:12px;margin:-4px 0;border-radius:6px;background:#cdd3da}
QStatusBar{background:#282b30;border-top:1px solid #474b52}
QLabel[role=brand]{font-size:26px;font-weight:800;letter-spacing:4px;color:#f4f5f7}
QLabel[role=eyebrow]{font-size:10px;font-weight:700;letter-spacing:1.8px;color:#9aa2ad}
QLabel[role=headline]{font-size:38px;font-weight:700;color:#f2f4f7}
QLabel[role=subhead]{font-size:14px;color:#aab2bc}
QLabel[role=panelTitle]{font-size:10px;letter-spacing:1.5px;font-weight:800;color:#d8dce2}
QPushButton#fluxHomeCard{text-align:left;background:#24272c;border:1px solid #41464e;border-radius:8px;padding:18px;font-weight:700}
QPushButton#fluxHomeCard:hover{background:#2c3036;border-color:#656d77}
)STYLE");

    m_canvas=new FluxCanvas(this);
    m_canvas->setDocument(m_document);
    m_stack=new QStackedWidget(this);
    m_home=new QWidget;
    m_studio=new QWidget;
    m_stack->addWidget(m_home);
    m_stack->addWidget(m_studio);
    setCentralWidget(m_stack);

    connect(m_canvas,&FluxCanvas::documentChanged,this,&FluxNextWindow::markDirty);
    connect(m_canvas,&FluxCanvas::cursorInfoChanged,this,[this](const QString& s){if(m_statusLabel)m_statusLabel->setText(s);});
    connect(m_canvas,&FluxCanvas::zoomChanged,this,[this](double z){if(m_zoomLabel)m_zoomLabel->setText(QString::number(qRound(z*100))+QStringLiteral("%"));});
    connect(m_canvas,&FluxCanvas::wheelRequested,this,[this](const QPoint& p){if(!m_wheel){m_wheel=new FluxWheel(this);}m_wheel->openAt(p);});

    m_playTimer->setInterval(42);
    connect(m_playTimer,&QTimer::timeout,this,&FluxNextWindow::nextFrame);

    buildHome();
    buildStudio();
    buildMenus();
    buildTopBar();
    buildToolRail();
    buildLayerDock();
    buildInspectorDock();
    buildTimelineDock();
    buildStatusBar();

    newProject();
    showHome();
}

FluxNextWindow::~FluxNextWindow()=default;

void FluxNextWindow::buildHome(){
    auto* root=new QVBoxLayout(m_home);
    root->setContentsMargins(64,48,64,36);
    root->setSpacing(18);
    root->addWidget(label(QStringLiteral("FLUX"),"brand"));
    root->addWidget(label(QStringLiteral("STUDIO / 1.0 PREVIEW"),"eyebrow"));
    root->addWidget(label(QStringLiteral("Draw. Animate. Finish."),"headline"));
    root->addWidget(label(QStringLiteral("A focused native workspace for illustration, 2D animation, compositing and final delivery."),"subhead"));
    auto* grid=new QGridLayout;
    grid->setHorizontalSpacing(12);grid->setVerticalSpacing(12);
    auto* n=homeCard(QStringLiteral("NEW PROJECT"),QStringLiteral("Create a production document."),m_home);
    auto* o=homeCard(QStringLiteral("OPEN PROJECT"),QStringLiteral("Load a .flux document."),m_home);
    auto* c=homeCard(QStringLiteral("CONTINUE"),QStringLiteral("Enter the professional studio."),m_home);
    auto* e=homeCard(QStringLiteral("EXPORT"),QStringLiteral("Deliver the current artwork."),m_home);
    grid->addWidget(n,0,0,1,2);grid->addWidget(o,0,2);grid->addWidget(c,1,0);grid->addWidget(e,1,1,1,2);
    root->addLayout(grid);
    root->addStretch();
    root->addWidget(label(QStringLiteral("DRAW  →  ANIMATE  →  COMPOSE  →  EXPORT"),"eyebrow"));
    connect(n,&QPushButton::clicked,this,[this]{newProject();enterStudio();});
    connect(o,&QPushButton::clicked,this,&FluxNextWindow::openProject);
    connect(c,&QPushButton::clicked,this,&FluxNextWindow::enterStudio);
    connect(e,&QPushButton::clicked,this,&FluxNextWindow::exportProjectImage);
}

void FluxNextWindow::buildStudio(){
    auto* root=new QVBoxLayout(m_studio);
    root->setContentsMargins(0,0,0,0);
    root->setSpacing(0);
    auto* top=new QWidget(m_studio);
    top->setObjectName("studioHeader");
    auto* h=new QHBoxLayout(top);
    h->setContentsMargins(8,4,8,4);
    h->setSpacing(8);
    m_projectLabel=label(QStringLiteral("Untitled  •  1920 × 1080"),"eyebrow",top);
    h->addWidget(m_projectLabel);
    h->addStretch();
    h->addWidget(label(QStringLiteral("RIGHT-CLICK  •  QUICK WHEEL"),"eyebrow",top));
    root->addWidget(top);
    root->addWidget(m_canvas,1);
}

void FluxNextWindow::buildMenus(){
    auto* file=menuBar()->addMenu(QStringLiteral("File"));
    file->addAction(QStringLiteral("New"),QKeySequence(QStringLiteral("Ctrl+N")),this,&FluxNextWindow::newProject);
    file->addAction(QStringLiteral("Open"),QKeySequence(QStringLiteral("Ctrl+O")),this,&FluxNextWindow::openProject);
    file->addAction(QStringLiteral("Save"),QKeySequence::Save,this,&FluxNextWindow::saveProject);
    file->addAction(QStringLiteral("Save As"),QKeySequence(QStringLiteral("Ctrl+Shift+S")),this,&FluxNextWindow::saveProjectAs);
    file->addAction(QStringLiteral("Export"),this,&FluxNextWindow::exportProjectImage);
    file->addSeparator();
    file->addAction(QStringLiteral("Home"),QKeySequence(QStringLiteral("Ctrl+Shift+H")),this,&FluxNextWindow::showHome);
    file->addAction(QStringLiteral("Quit"),QKeySequence::Quit,this,&QWidget::close);

    auto* edit=menuBar()->addMenu(QStringLiteral("Edit"));
    edit->addAction(QStringLiteral("Undo"),QKeySequence::Undo,m_canvas,&FluxCanvas::undo);
    edit->addAction(QStringLiteral("Redo"),QKeySequence::Redo,m_canvas,&FluxCanvas::redo);
    edit->addSeparator();
    edit->addAction(QStringLiteral("Select All"),QKeySequence(QStringLiteral("Ctrl+A")),m_canvas,&FluxCanvas::selectAll);
    edit->addAction(QStringLiteral("Deselect"),QKeySequence(QStringLiteral("Ctrl+Shift+A")),m_canvas,&FluxCanvas::clearSelection);

    auto* view=menuBar()->addMenu(QStringLiteral("View"));
    auto* grid=view->addAction(QStringLiteral("Grid"));grid->setCheckable(true);connect(grid,&QAction::toggled,this,&FluxNextWindow::toggleGrid);
    auto* rulers=view->addAction(QStringLiteral("Rulers"));rulers->setCheckable(true);connect(rulers,&QAction::toggled,this,&FluxNextWindow::toggleRulers);
    auto* onion=view->addAction(QStringLiteral("Onion Skin"));onion->setCheckable(true);connect(onion,&QAction::toggled,this,&FluxNextWindow::toggleOnion);
    auto* sh=view->addAction(QStringLiteral("Symmetry Horizontal"));sh->setCheckable(true);connect(sh,&QAction::toggled,this,&FluxNextWindow::toggleSymmetryH);
    auto* sv=view->addAction(QStringLiteral("Symmetry Vertical"));sv->setCheckable(true);connect(sv,&QAction::toggled,this,&FluxNextWindow::toggleSymmetryV);
    view->addSeparator();view->addAction(QStringLiteral("Fit Canvas"),this,[this]{m_canvas->fitCanvas();});
    view->addAction(QStringLiteral("Mirror Canvas"),this,[this]{m_mirrorH=!m_mirrorH;m_canvas->setMirrorHorizontal(m_mirrorH);});
    view->addAction(QStringLiteral("Command Palette"),QKeySequence(QStringLiteral("Ctrl+K")),this,&FluxNextWindow::showAbout);

    auto* image=menuBar()->addMenu(QStringLiteral("Image"));
    image->addAction(QStringLiteral("Crop to Canvas"),this,[this]{m_canvas->fitCanvas();});
    image->addAction(QStringLiteral("Rotate 90°"),this,[this]{m_canvasRotation+=90;if(m_canvasRotation>=360)m_canvasRotation=0;m_canvas->setCanvasRotation(m_canvasRotation);});
    image->addAction(QStringLiteral("Reset Rotation"),this,[this]{m_canvasRotation=0;m_canvasRotation=0;m_canvasRotation=0;m_canvas->setCanvasRotation(0);});

    auto* layer=menuBar()->addMenu(QStringLiteral("Layer"));
    layer->addAction(QStringLiteral("Add Paint Layer"),this,&FluxNextWindow::addLayer);
    layer->addAction(QStringLiteral("Add Group"),this,&FluxNextWindow::addGroup);
    layer->addAction(QStringLiteral("Duplicate"),this,&FluxNextWindow::duplicateLayer);
    layer->addAction(QStringLiteral("Delete"),this,&FluxNextWindow::deleteLayer);

    auto* animation=menuBar()->addMenu(QStringLiteral("Animation"));
    animation->addAction(QStringLiteral("Previous Frame"),QKeySequence(Qt::Key_Left),this,&FluxNextWindow::previousFrame);
    animation->addAction(QStringLiteral("Next Frame"),QKeySequence(Qt::Key_Right),this,&FluxNextWindow::nextFrame);
    animation->addAction(QStringLiteral("Play / Pause"),QKeySequence(Qt::Key_Space),this,&FluxNextWindow::togglePlayback);
    animation->addAction(QStringLiteral("Insert Frame"),this,[this]{m_document->setFrameCount(m_document->frameCount()+1);syncDocumentToUi();markDirty();});

    auto* settings=menuBar()->addMenu(QStringLiteral("Settings"));
    settings->addAction(QStringLiteral("Reset Workspace Layout"),this,[this]{
        for(auto* d:findChildren<QDockWidget*>()) d->show();
        statusBar()->showMessage(QStringLiteral("Workspace panels restored"),2500);
    });
    settings->addAction(QStringLiteral("About Flux Studio"),this,&FluxNextWindow::showAbout);
}

void FluxNextWindow::buildTopBar(){
    auto* bar=new QToolBar(QStringLiteral("Main Toolbar"),this);
    bar->setObjectName(QStringLiteral("FluxTopBar"));
    bar->setMovable(false);bar->setFloatable(false);
    addToolBar(bar);
    auto* home=bar->addAction(style()->standardIcon(QStyle::SP_DirHomeIcon),QStringLiteral("Home"));connect(home,&QAction::triggered,this,&FluxNextWindow::showHome);
    auto* n=bar->addAction(style()->standardIcon(QStyle::SP_FileIcon),QStringLiteral("New"));connect(n,&QAction::triggered,this,&FluxNextWindow::newProject);
    auto* o=bar->addAction(style()->standardIcon(QStyle::SP_DialogOpenButton),QStringLiteral("Open"));connect(o,&QAction::triggered,this,&FluxNextWindow::openProject);
    auto* s=bar->addAction(style()->standardIcon(QStyle::SP_DialogSaveButton),QStringLiteral("Save"));connect(s,&QAction::triggered,this,&FluxNextWindow::saveProject);
    bar->addSeparator();
    auto* undo=bar->addAction(style()->standardIcon(QStyle::SP_ArrowBack),QStringLiteral("Undo"));connect(undo,&QAction::triggered,m_canvas,&FluxCanvas::undo);
    auto* redo=bar->addAction(style()->standardIcon(QStyle::SP_ArrowForward),QStringLiteral("Redo"));connect(redo,&QAction::triggered,m_canvas,&FluxCanvas::redo);
    bar->addSeparator();
    bar->addWidget(label(QStringLiteral("TOOL"),"eyebrow",bar));
    auto* tool=new QComboBox(bar);
    tool->addItems({QStringLiteral("Brush"),QStringLiteral("Pencil"),QStringLiteral("Ink"),QStringLiteral("Eraser"),QStringLiteral("Line"),QStringLiteral("Rectangle"),QStringLiteral("Ellipse"),QStringLiteral("Polygon"),QStringLiteral("Star"),QStringLiteral("Bezier"),QStringLiteral("Gradient"),QStringLiteral("Text"),QStringLiteral("Fill"),QStringLiteral("Color Picker"),QStringLiteral("Transform"),QStringLiteral("Pan"),QStringLiteral("Zoom")});
    tool->setMinimumWidth(125);bar->addWidget(tool);connect(tool,&QComboBox::currentTextChanged,this,&FluxNextWindow::setTool);
    bar->addWidget(label(QStringLiteral("OPACITY"),"eyebrow",bar));
    auto* opacity=new QSlider(Qt::Horizontal,bar);opacity->setRange(0,100);opacity->setValue(100);opacity->setFixedWidth(96);bar->addWidget(opacity);
    bar->addWidget(label(QStringLiteral("SIZE"),"eyebrow",bar));
    m_brushSlider=new QSlider(Qt::Horizontal,bar);m_brushSlider->setRange(1,1000);m_brushSlider->setValue(24);m_brushSlider->setFixedWidth(110);bar->addWidget(m_brushSlider);connect(m_brushSlider,&QSlider::valueChanged,this,&FluxNextWindow::setBrushSize);
    m_brushLabel=label(QStringLiteral("24 px"),"eyebrow",bar);bar->addWidget(m_brushLabel);
    bar->addSeparator();
    auto* color=bar->addAction(QStringLiteral("Foreground"));connect(color,&QAction::triggered,this,&FluxNextWindow::chooseColor);
    auto* eraser=bar->addAction(QStringLiteral("E"));eraser->setToolTip(QStringLiteral("Eraser"));connect(eraser,&QAction::triggered,this,[this]{setTool(QStringLiteral("Eraser"));});
    auto* play=bar->addAction(style()->standardIcon(QStyle::SP_MediaPlay),QStringLiteral("Play"));connect(play,&QAction::triggered,this,&FluxNextWindow::togglePlayback);
    auto* fit=bar->addAction(QStringLiteral("Fit"));connect(fit,&QAction::triggered,this,[this]{m_canvas->fitCanvas();});
    m_zoomLabel=label(QStringLiteral("100%"),"eyebrow",bar);bar->addWidget(m_zoomLabel);
}

void FluxNextWindow::buildToolRail(){
    auto* rail=new QToolBar(QStringLiteral("Tools"),this);
    rail->setObjectName(QStringLiteral("FluxToolRail"));
    rail->setOrientation(Qt::Vertical);rail->setMovable(false);rail->setFloatable(false);
    addToolBar(Qt::LeftToolBarArea,rail);
    auto* group=new QActionGroup(this);group->setExclusive(true);
    const QStringList glyphs={QStringLiteral("B"),QStringLiteral("P"),QStringLiteral("I"),QStringLiteral("E"),QStringLiteral("╱"),QStringLiteral("□"),QStringLiteral("○"),QStringLiteral("⬡"),QStringLiteral("★"),QStringLiteral("⌁"),QStringLiteral("∕"),QStringLiteral("T"),QStringLiteral("▾"),QStringLiteral("◉"),QStringLiteral("✥"),QStringLiteral("✋"),QStringLiteral("⌕")};
    const QStringList tools={QStringLiteral("Brush"),QStringLiteral("Pencil"),QStringLiteral("Ink"),QStringLiteral("Eraser"),QStringLiteral("Line"),QStringLiteral("Rectangle"),QStringLiteral("Ellipse"),QStringLiteral("Polygon"),QStringLiteral("Star"),QStringLiteral("Bezier"),QStringLiteral("Gradient"),QStringLiteral("Text"),QStringLiteral("Fill"),QStringLiteral("Color Picker"),QStringLiteral("Transform"),QStringLiteral("Pan"),QStringLiteral("Zoom")};
    for(int i=0;i<tools.size();++i){auto* a=rail->addAction(glyphs.value(i));a->setCheckable(true);a->setToolTip(tools[i]);group->addAction(a);connect(a,&QAction::triggered,this,[this,t=tools[i]]{setTool(t);});if(i==0)a->setChecked(true);}    rail->addSeparator();
    auto* wheel=rail->addAction(QStringLiteral("◎"));wheel->setToolTip(QStringLiteral("Quick Wheel"));connect(wheel,&QAction::triggered,this,[this]{if(!m_wheel)m_wheel=new FluxWheel(this);m_wheel->openAt(mapToGlobal(rect().center()));});
}

void FluxNextWindow::buildStatusBar(){
    m_statusLabel=label(QStringLiteral("Ready  •  Flux Core  •  8-bit sRGB"),"eyebrow",this);
    statusBar()->addWidget(m_statusLabel,1);
    statusBar()->addPermanentWidget(label(QStringLiteral("RGBA/Alpha  •  1920 × 1080"),"eyebrow",this));
}

void FluxNextWindow::newProject(){
    QDialog d(this);d.setWindowTitle(QStringLiteral("New Flux Project"));d.resize(420,0);
    auto* f=new QFormLayout(&d);
    auto* name=new QLineEdit(QStringLiteral("Untitled"),&d);
    auto* w=new QSpinBox(&d);w->setRange(64,16384);w->setValue(1920);
    auto* h=new QSpinBox(&d);h->setRange(64,16384);h->setValue(1080);
    auto* fps=new QSpinBox(&d);fps->setRange(1,240);fps->setValue(24);
    auto* frames=new QSpinBox(&d);frames->setRange(1,10000);frames->setValue(120);
    auto* preset=new QComboBox(&d);preset->addItems({QStringLiteral("1920 × 1080 / 24 fps"),QStringLiteral("2048 × 2048 / 24 fps"),QStringLiteral("1280 × 720 / 30 fps"),QStringLiteral("1080 × 1920 / 30 fps"),QStringLiteral("Custom")});
    f->addRow(QStringLiteral("Preset"),preset);f->addRow(QStringLiteral("Name"),name);f->addRow(QStringLiteral("Width"),w);f->addRow(QStringLiteral("Height"),h);f->addRow(QStringLiteral("FPS"),fps);f->addRow(QStringLiteral("Frames"),frames);
    auto* bb=new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel,&d);f->addRow(bb);
    connect(preset,qOverload<int>(&QComboBox::currentIndexChanged),[=](int i){
        if(i==0){w->setValue(1920);h->setValue(1080);fps->setValue(24);frames->setValue(120);}
        else if(i==1){w->setValue(2048);h->setValue(2048);fps->setValue(24);frames->setValue(96);}
        else if(i==2){w->setValue(1280);h->setValue(720);fps->setValue(30);frames->setValue(120);}
        else if(i==3){w->setValue(1080);h->setValue(1920);fps->setValue(30);frames->setValue(120);}
        const bool custom=i==4;w->setEnabled(custom);h->setEnabled(custom);fps->setEnabled(custom);frames->setEnabled(custom);
    });
    connect(bb,&QDialogButtonBox::accepted,&d,&QDialog::accept);connect(bb,&QDialogButtonBox::rejected,&d,&QDialog::reject);
    if(d.exec()!=QDialog::Accepted)return;
    m_document->create(name->text(),w->value(),h->value());
    m_document->setFrameCount(frames->value());
    m_playTimer->setInterval(qMax(1,1000/fps->value()));
    m_dirty=false;
    syncDocumentToUi();
    refreshLayerList();
    m_canvas->fitCanvas();
    updateWindowTitle();
}

void FluxNextWindow::openProject(){
    const auto p=QFileDialog::getOpenFileName(this,QStringLiteral("Open Flux Project"),{},QStringLiteral("Flux Projects (*.flux)"));
    if(p.isEmpty())return;QString e;
    if(!m_document->load(p,&e)){QMessageBox::critical(this,QStringLiteral("Open failed"),e);return;}
    m_dirty=false;syncDocumentToUi();refreshLayerList();enterStudio();statusBar()->showMessage(QStringLiteral("Opened %1").arg(p),2500);
}

void FluxNextWindow::saveProject(){
    if(m_document->path().isEmpty()){saveProjectAs();return;}QString e;
    if(!m_document->save(m_document->path(),&e))QMessageBox::critical(this,QStringLiteral("Save failed"),e);else{m_dirty=false;updateWindowTitle();statusBar()->showMessage(QStringLiteral("Saved"),2500);}
}

void FluxNextWindow::saveProjectAs(){
    const auto p=QFileDialog::getSaveFileName(this,QStringLiteral("Save Flux Project"),QStringLiteral("Untitled.flux"),QStringLiteral("Flux Projects (*.flux)"));
    if(p.isEmpty())return;QString e;
    if(!m_document->save(p,&e))QMessageBox::critical(this,QStringLiteral("Save failed"),e);else{m_dirty=false;updateWindowTitle();statusBar()->showMessage(QStringLiteral("Saved %1").arg(p),2500);}
}

void FluxNextWindow::exportProjectImage(){
    const auto p=QFileDialog::getSaveFileName(this,QStringLiteral("Export Image"),QStringLiteral("Flux-Export.png"),QStringLiteral("PNG (*.png);;JPEG (*.jpg *.jpeg);;WebP (*.webp)"));
    if(p.isEmpty())return;QString e;
    if(!m_document->exportImage(p,&e))QMessageBox::critical(this,QStringLiteral("Export failed"),e);else statusBar()->showMessage(QStringLiteral("Exported %1").arg(p),2500);
}

void FluxNextWindow::enterStudio(){
    m_stack->setCurrentWidget(m_studio);
    for(auto* d:findChildren<QDockWidget*>())d->show();
    for(auto* b:findChildren<QToolBar*>())b->show();
    m_canvas->setFocus();m_canvas->fitCanvas();
}

void FluxNextWindow::showHome(){
    m_stack->setCurrentWidget(m_home);
    for(auto* d:findChildren<QDockWidget*>())d->hide();
    for(auto* b:findChildren<QToolBar*>())b->hide();
}

void FluxNextWindow::selectLayer(QListWidgetItem* i){if(i)m_document->setActiveLayer(i->data(Qt::UserRole).toInt());m_canvas->update();}
void FluxNextWindow::refreshLayerList(){if(!m_layers)return;m_layers->clear();for(int i=m_document->layers().size()-1;i>=0;--i){auto& l=m_document->layers()[i];auto* it=new QListWidgetItem(l.name,m_layers);it->setData(Qt::UserRole,i);if(i==m_document->activeLayerIndex())it->setSelected(true);}}
void FluxNextWindow::addLayer(){m_document->addLayer(QStringLiteral("Paint Layer %1").arg(m_document->layers().size()+1));refreshLayerList();markDirty();}
void FluxNextWindow::addGroup(){m_document->addGroup(QStringLiteral("Group %1").arg(m_document->layers().size()+1));refreshLayerList();markDirty();}
void FluxNextWindow::duplicateLayer(){if(!m_document->layers().isEmpty()){m_document->duplicateLayer(m_document->activeLayerIndex());refreshLayerList();markDirty();}}
void FluxNextWindow::deleteLayer(){if(m_document->layers().size()>1){m_document->removeLayer(m_document->activeLayerIndex());refreshLayerList();markDirty();}}
void FluxNextWindow::setFrame(int f){f=std::clamp(f,0,m_document->frameCount()-1);m_document->setFrame(f);m_canvas->update();syncDocumentToUi();}
void FluxNextWindow::previousFrame(){setFrame(m_document->frame()?m_document->frame()-1:m_document->frameCount()-1);}
void FluxNextWindow::nextFrame(){setFrame((m_document->frame()+1)%qMax(1,m_document->frameCount()));}
void FluxNextWindow::togglePlayback(){m_playing=!m_playing;if(m_playing)m_playTimer->start();else m_playTimer->stop();}
void FluxNextWindow::updateZoom(int value){if(value<=0){m_canvas->fitCanvas();return;}m_canvas->fitCanvas();if(m_zoomLabel)m_zoomLabel->setText(QString::number(value)+QStringLiteral("%"));}
void FluxNextWindow::setBrushSize(int v){m_canvas->setBrushSize(v);if(m_brushLabel)m_brushLabel->setText(QString::number(v)+QStringLiteral(" px"));}
void FluxNextWindow::chooseColor(){auto c=QColorDialog::getColor(m_document->foreground(),this,QStringLiteral("Flux Color"));if(c.isValid()){m_document->setForeground(c);m_canvas->setBrushColor(c);}}
void FluxNextWindow::toggleGrid(bool b){m_canvas->setGridEnabled(b);m_canvas->update();}
void FluxNextWindow::toggleRulers(bool b){m_canvas->setRulersEnabled(b);m_canvas->update();}
void FluxNextWindow::toggleOnion(bool b){m_canvas->toggleOnionSkin(b);m_canvas->update();}
void FluxNextWindow::toggleSymmetryH(bool b){m_canvas->setSymmetry(b,m_canvas->symmetryVertical());m_canvas->update();}
void FluxNextWindow::toggleSymmetryV(bool b){m_canvas->setSymmetry(m_canvas->symmetryHorizontal(),b);m_canvas->update();}
void FluxNextWindow::setTool(const QString& s){m_canvas->setTool(s);statusBar()->showMessage(QStringLiteral("Tool: %1").arg(s),2500);}
void FluxNextWindow::showAbout(){QMessageBox::about(this,QStringLiteral("Flux Studio"),QStringLiteral("Flux Studio 1.0 Preview\nNative C++20 / Qt 6\nDRAW → ANIMATE → COMPOSE → EXPORT"));}
void FluxNextWindow::markDirty(){m_dirty=true;updateWindowTitle();}
void FluxNextWindow::syncDocumentToUi(){if(m_projectLabel)m_projectLabel->setText(QStringLiteral("%1  •  %2 × %3  •  %4 fps").arg(m_document->name()).arg(m_document->width()).arg(m_document->height()).arg(qMax(1,1000/m_playTimer->interval())));if(m_frames){QSignalBlocker block(m_frames->model());m_frames->clear();for(int i=0;i<m_document->frameCount();++i)m_frames->addItem(QString::number(i+1));m_frames->setCurrentRow(m_document->frame());}}
void FluxNextWindow::updateWindowTitle(){setWindowTitle(QStringLiteral("%1%2 — Flux Studio 1.0 Preview").arg(m_document->name(),m_dirty?QStringLiteral(" *"):QString()));}
