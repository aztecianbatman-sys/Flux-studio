#include "mainwindow.h"
#include "fluxworkflow.h"
#include <QFileInfo>
#include <QLabel>
#include <QStatusBar>
#include <QSvgRenderer>
#include <QPainter>
#include <QScrollArea>
#include <QFrame>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QListWidget>
#include <QDockWidget>
#include <QStackedWidget>
#include <QTimer>
#include <QSizePolicy>
#include <functional>

namespace {
QPixmap artPixmap(const QString& path,const QSize& size){
    QPixmap pix(size); pix.fill(Qt::transparent);
    QSvgRenderer renderer(path);
    QPainter p(&pix);
    renderer.render(&p,QRectF(QPointF(0,0),QSizeF(size)));
    return pix;
}
void clearLayout(QLayout* layout){
    if(!layout) return;
    while(auto* item=layout->takeAt(0)){
        if(auto* child=item->layout()) clearLayout(child);
        if(auto* w=item->widget()) w->deleteLater();
        delete item;
    }
    delete layout;
}
QFrame* featureCard(const QString& title,const QString& desc,const QString& art,const std::function<void()>& action,QWidget* parent){
    auto* frame=new QFrame(parent); frame->setObjectName("visualFeatureCard");
    auto* l=new QHBoxLayout(frame); l->setContentsMargins(10,10,10,10); l->setSpacing(12);
    auto* thumb=new QLabel(frame); thumb->setFixedSize(150,88); thumb->setScaledContents(true); thumb->setPixmap(artPixmap(art,thumb->size())); l->addWidget(thumb);
    auto* copy=new QVBoxLayout; copy->setSpacing(3); auto* t=new QLabel(title,frame); t->setStyleSheet("font-weight:800;font-size:13px;color:#eef2f8;"); copy->addWidget(t);
    auto* d=new QLabel(desc,frame); d->setWordWrap(true); d->setStyleSheet("color:#9da9b8;font-size:12px;"); copy->addWidget(d); copy->addStretch();
    auto* open=new QPushButton("OPEN",frame); open->setFixedWidth(78); open->setCursor(Qt::PointingHandCursor); copy->addWidget(open,0,Qt::AlignLeft);
    QObject::connect(open,&QPushButton::clicked,frame,[action]{action();}); l->addLayout(copy,1); return frame;
}
QWidget* dockHeader(const QString& title,const QString& art,QWidget* parent){
    auto* w=new QWidget(parent); w->setMinimumHeight(58); w->setMaximumHeight(68);
    auto* l=new QHBoxLayout(w); l->setContentsMargins(8,6,8,6); l->setSpacing(9);
    auto* img=new QLabel(w); img->setFixedSize(76,46); img->setScaledContents(true); img->setPixmap(artPixmap(art,img->size()));
    auto* text=new QLabel(title,w); text->setObjectName("dockVisualTitle"); l->addWidget(img); l->addWidget(text); l->addStretch(); return w;
}
}

void FluxMainWindow::polish(){
    setStyleSheet(R"STYLE(
*{font-family:"Segoe UI";font-size:13px;color:#e6e9ee}
QMainWindow{background:#1c1f24}
QMenuBar{background:#20242a;border:0;border-bottom:1px solid #353a42;padding:2px 8px}
QMenuBar::item{padding:6px 10px;border-radius:4px}QMenuBar::item:selected{background:#333942}
QMenu{background:#23272d;border:1px solid #424850;padding:5px}QMenu::item{padding:7px 22px 7px 10px;border-radius:4px}QMenu::item:selected{background:#39414b}
QToolBar{background:#252a31;border:0;border-bottom:1px solid #3b4149;padding:4px;spacing:3px}
QToolBar#FluxTopBar QToolButton{min-width:32px;min-height:30px;padding:3px 6px;border-radius:4px}QToolBar#FluxTopBar QToolButton:hover{background:#343a43}
QToolBar#FluxToolRail{background:#24282e;border-right:1px solid #3b4148;padding:4px}QToolBar#FluxToolRail QToolButton{min-width:38px;min-height:34px;margin:1px 0;border-radius:4px;font-weight:700;color:#d7dce3}QToolBar#FluxToolRail QToolButton:hover{background:#363c45}QToolBar#FluxToolRail QToolButton:checked{background:#47515e;color:#fff;border:1px solid #697381}
QDockWidget{background:#282c31;border:1px solid #3d434a}QDockWidget::title{background:#2c3137;padding:0;border-bottom:1px solid #3e454d;font-weight:700}
QTabWidget::pane{border:1px solid #3d434a;background:#282c31}QTabBar::tab{background:#2d3238;padding:7px 13px;border:0;color:#abb3be}QTabBar::tab:selected{background:#3a424b;color:#fff}
QTreeWidget,QListWidget{background:#20242a;border:1px solid #3a4149;border-radius:4px;outline:0}QTreeWidget::item,QListWidget::item{padding:5px;border-radius:4px}QTreeWidget::item:selected,QListWidget::item:selected{background:#3d4957}
QPushButton{background:#303740;border:1px solid #48515a;border-radius:5px;padding:7px 10px}QPushButton:hover{background:#3a434d;border-color:#65707d}QPushButton:pressed,QPushButton:checked{background:#465363}
QComboBox,QSpinBox,QLineEdit{background:#20252b;border:1px solid #414953;border-radius:4px;padding:5px 7px;min-height:24px}QComboBox::drop-down{border:0;width:22px}
QSlider::groove:horizontal{height:4px;background:#4a535d;border-radius:2px}QSlider::handle:horizontal{width:12px;margin:-4px 0;border-radius:6px;background:#dce2e9}
QGroupBox{border:1px solid #394149;border-radius:6px;margin-top:8px;padding-top:8px}QGroupBox::title{subcontrol-origin:margin;left:9px;padding:0 4px;color:#9ca8b5;font-size:10px;font-weight:700;letter-spacing:1px}
QStatusBar{background:#20242a;border-top:1px solid #3a4149}
QLabel[role="brand"]{font-size:30px;font-weight:800;letter-spacing:7px;color:#f5f7fb}QLabel[role="brandSub"]{font-size:10px;font-weight:700;letter-spacing:3px;color:#7f8996}
QLabel[role="meta"],QLabel[role="hint"]{font-size:10px;letter-spacing:1.2px;color:#788492;font-weight:700}QLabel[role="eyebrow"]{font-size:11px;letter-spacing:2.1px;color:#8b97a5;font-weight:800}QLabel[role="headline"]{font-size:38px;font-weight:700;color:#f4f6fa}QLabel[role="subhead"]{font-size:14px;color:#9ba5b2}QLabel[role="footer"]{font-size:10px;letter-spacing:1.2px;color:#697686;font-weight:700}QLabel[role="topLabel"]{font-size:10px;color:#7d8795;font-weight:700}QLabel[role="topValue"]{font-size:11px;color:#c1c8d2;font-weight:700}QLabel[role="cursor"]{font-size:11px;color:#84909d;min-width:70px}
QLabel#dockVisualTitle{font-size:12px;font-weight:800;letter-spacing:1px;color:#e8edf4}
QFrame#visualHero{background:#101722;border:1px solid #3a4655;border-radius:12px}
QFrame#visualFeatureCard{background:#242a31;border:1px solid #39424d;border-radius:9px}
QPushButton#visualPrimary{background:#3b536a;border:1px solid #708aa3;font-weight:800;font-size:13px;padding:9px 16px}
QPushButton#visualPrimary:hover{background:#48647f}
QLabel#visualSection{font-size:11px;font-weight:800;letter-spacing:2px;color:#8491a0}
)STYLE");
    if(!m_statusLabel){m_statusLabel=new QLabel(QStringLiteral("Ready  •  Flux Core  •  Local workspace"));statusBar()->addWidget(m_statusLabel,1);}
    if(m_home){
        auto* old=m_home->layout(); if(old) clearLayout(old);
        auto*root=new QVBoxLayout(m_home); root->setContentsMargins(28,24,28,22); root->setSpacing(14);
        auto*scroll=new QScrollArea(m_home); scroll->setWidgetResizable(true); scroll->setFrameShape(QFrame::NoFrame); scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        auto*page=new QWidget(scroll); auto*pageLayout=new QVBoxLayout(page); pageLayout->setContentsMargins(18,10,18,18); pageLayout->setSpacing(14);
        auto*hero=new QFrame(page); hero->setObjectName("visualHero"); hero->setMinimumHeight(238); hero->setMaximumHeight(270);
        auto*hl=new QHBoxLayout(hero); hl->setContentsMargins(22,20,22,20); hl->setSpacing(20);
        auto*copy=new QVBoxLayout; copy->setSpacing(4);
        auto*ey=new QLabel("FLUX STUDIO  /  0.9",hero); ey->setObjectName("visualSection"); copy->addWidget(ey);
        auto*title=new QLabel("A real creative workstation.",hero); title->setStyleSheet("font-size:32px;font-weight:800;color:#f4f7fb;"); copy->addWidget(title);
        auto*sub=new QLabel("The canvas, layers, animation, brushes and compositor now share one focused production surface.",hero); sub->setWordWrap(true); sub->setStyleSheet("font-size:14px;color:#a4afbd;"); copy->addWidget(sub);
        auto*buttons=new QHBoxLayout; auto*newB=new QPushButton("NEW PROJECT",hero); newB->setObjectName("visualPrimary"); auto*openB=new QPushButton("OPEN PROJECT",hero); buttons->addWidget(newB); buttons->addWidget(openB); buttons->addStretch(); copy->addSpacing(10); copy->addLayout(buttons); copy->addStretch(); hl->addLayout(copy,1);
        auto*heroImg=new QLabel(hero); heroImg->setMinimumWidth(430); heroImg->setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Expanding); heroImg->setScaledContents(true); heroImg->setPixmap(artPixmap(":/art/home-hero.svg",QSize(720,300))); hl->addWidget(heroImg,1); pageLayout->addWidget(hero);
        auto*label=new QLabel("WORKSPACES",page); label->setObjectName("visualSection"); pageLayout->addWidget(label);
        auto*grid=new QGridLayout; grid->setHorizontalSpacing(12); grid->setVerticalSpacing(12);
        grid->addWidget(featureCard("PAINTING", "Large canvas + tool rail + layers + color", ":/art/brushes.svg", [this]{enterWorkspace();}, page),0,0);
        grid->addWidget(featureCard("ANIMATION", "Timeline, frame transport and onion skin", ":/art/timeline.svg", [this]{enterWorkspace();}, page),0,1);
        grid->addWidget(featureCard("COMPOSITOR", "Node based effects and finishing surface", ":/art/compositor.svg", [this]{enterWorkspace(); if(auto*d=findChild<QDockWidget*>("AdvancedSuiteDock")) d->show();}, page),1,0);
        grid->addWidget(featureCard("LAYERS", "Hierarchical paint, masks and adjustments", ":/art/layers.svg", [this]{enterWorkspace();}, page),1,1); pageLayout->addLayout(grid);
        auto*recentBox=new QFrame(page); recentBox->setStyleSheet("QFrame{background:#20252b;border:1px solid #353d47;border-radius:9px;}");
        auto*rl=new QVBoxLayout(recentBox); rl->setContentsMargins(14,12,14,12); rl->setSpacing(8); auto*rt=new QLabel("RECENT PROJECTS",recentBox); rt->setObjectName("visualSection"); rl->addWidget(rt);
        auto*recent=new QListWidget(recentBox); recent->setMinimumHeight(86); recent->setMaximumHeight(120); rl->addWidget(recent);
        for(const auto&p:FluxWorkflow::recentProjects(8)){ if(QFileInfo::exists(p)){auto*i=new QListWidgetItem(QFileInfo(p).completeBaseName()); i->setData(Qt::UserRole,p); i->setToolTip(p); recent->addItem(i);} }
        if(recent->count()==0){auto*i=new QListWidgetItem("No recent projects yet — start from New Project.");i->setFlags(Qt::NoItemFlags);recent->addItem(i);}
        QObject::connect(recent,&QListWidget::itemDoubleClicked,m_home,[this](QListWidgetItem*i){if(i)loadProjectPath(i->data(Qt::UserRole).toString());}); pageLayout->addWidget(recentBox); pageLayout->addStretch(); scroll->setWidget(page); root->addWidget(scroll);
        QObject::connect(newB,&QPushButton::clicked,this,&FluxMainWindow::newProject); QObject::connect(openB,&QPushButton::clicked,this,&FluxMainWindow::openProject);
    }
    auto restyleDocks=[this](){
        const QList<QStringList> specs={{"ColorSelectorDock","COLOR / SELECTOR",":/art/color-wheel.svg"},{"LayersDock","LAYERS",":/art/layers.svg"},{"BrushPresetsDock","BRUSH LAB",":/art/brushes.svg"},{"TimelineDock","ANIMATION TIMELINE",":/art/timeline.svg"},{"ProductionDock","PRODUCTION CENTER",":/art/home-hero.svg"},{"AdvancedSuiteDock","COMPOSITOR / ADVANCED",":/art/compositor.svg"}};
        for(const auto&s:specs){if(auto*d=findChild<QDockWidget*>(s[0]))d->setTitleBarWidget(dockHeader(s[1],s[2],d));}
        if(auto*d=findChild<QDockWidget*>("AdvancedSuiteDock"))d->hide();
        if(auto*d=findChild<QDockWidget*>("ColorSelectorDock")){d->setMinimumWidth(276);d->setMaximumWidth(360);} if(auto*d=findChild<QDockWidget*>("LayersDock")){d->setMinimumWidth(276);d->setMaximumWidth(360);} if(auto*d=findChild<QDockWidget*>("BrushPresetsDock")){d->setMinimumWidth(276);d->setMaximumWidth(360);} if(auto*d=findChild<QDockWidget*>("ToolOptionsDock")){d->setMinimumWidth(250);d->setMaximumWidth(360);} if(auto*d=findChild<QDockWidget*>("TimelineDock")){d->setMinimumHeight(220);d->setMaximumHeight(320);} if(auto*d=findChild<QDockWidget*>("ProductionDock")){d->setMinimumHeight(220);d->setMaximumHeight(320);}
    };
    QObject::connect(m_stack,&QStackedWidget::currentChanged,this,[restyleDocks](int){QTimer::singleShot(0,[restyleDocks]{restyleDocks();});});
    QTimer::singleShot(0,[restyleDocks]{restyleDocks();});
}

void FluxMainWindow::showRecoveryBrowser(){enterWorkspace();if(m_statusLabel)m_statusLabel->setText(QStringLiteral("Recovery browser is available in Production panels."));}
void FluxMainWindow::markModified(){const QString name=m_filePath.isEmpty()?QStringLiteral("Untitled"):QFileInfo(m_filePath).completeBaseName();setWindowTitle(QStringLiteral("Flux Studio — %1 *").arg(name));if(m_statusLabel)m_statusLabel->setText(QStringLiteral("Modified  •  Unsaved changes"));}
