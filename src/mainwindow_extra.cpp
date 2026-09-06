#include "mainwindow.h"
#include <QFileInfo>
#include <QLabel>
#include <QStatusBar>

void FluxMainWindow::polish(){
    setStyleSheet(R"STYLE(
*{font-family:"Segoe UI";font-size:13px;color:#e7ebf3}
QMainWindow{background:#20242a}
QMenuBar{background:#252a31;border:0;border-bottom:1px solid #353b44;padding:2px 7px}
QMenuBar::item{padding:6px 10px;border-radius:5px}QMenuBar::item:selected{background:#363d47}
QMenu{background:#252a31;border:1px solid #414852;padding:5px}QMenu::item{padding:7px 22px 7px 10px;border-radius:5px}QMenu::item:selected{background:#3b4450}
QToolBar{background:#292f36;border:0;border-bottom:1px solid #3b424b;padding:4px;spacing:3px}
QToolBar#FluxTopBar QToolButton{min-width:32px;min-height:30px;padding:3px 6px;border-radius:5px}QToolBar#FluxTopBar QToolButton:hover{background:#3a424d}
QToolBar#FluxToolRail{background:#282d34;border-right:1px solid #3b424a;padding:5px 4px}QToolBar#FluxToolRail QToolButton{min-width:38px;min-height:34px;margin:1px 0;border-radius:5px;font-weight:700;color:#d9dfe7}QToolBar#FluxToolRail QToolButton:hover{background:#3a424d}QToolBar#FluxToolRail QToolButton:checked{background:#4a5664;color:#fff;border:1px solid #657383}
QDockWidget{background:#262b32;border:1px solid #3c434c}QDockWidget::title{background:#2d333b;padding:7px 10px;border-bottom:1px solid #3d454f;font-weight:700}
QTabWidget::pane{border:1px solid #3c434c;background:#262b32}QTabBar::tab{background:#2d333b;padding:7px 13px;border:0;color:#aeb7c3}QTabBar::tab:selected{background:#3b4550;color:#fff}
QTreeWidget,QListWidget{background:#20252c;border:1px solid #39414b;border-radius:5px;outline:0}QTreeWidget::item,QListWidget::item{padding:5px;border-radius:4px}QTreeWidget::item:selected,QListWidget::item:selected{background:#3d4957}
QPushButton{background:#303740;border:1px solid #48525d;border-radius:5px;padding:7px 10px}QPushButton:hover{background:#3a444f;border-color:#697582}QPushButton:pressed,QPushButton:checked{background:#465363}
QComboBox,QSpinBox,QLineEdit{background:#20262d;border:1px solid #414a55;border-radius:5px;padding:5px 7px;min-height:24px}QComboBox::drop-down{border:0;width:22px}
QSlider::groove:horizontal{height:4px;background:#4b5560;border-radius:2px}QSlider::handle:horizontal{width:12px;margin:-4px 0;border-radius:6px;background:#dce2e9}
QGroupBox{border:1px solid #39414a;border-radius:6px;margin-top:8px;padding-top:8px}QGroupBox::title{subcontrol-origin:margin;left:9px;padding:0 4px;color:#a7b0bc;font-size:10px;font-weight:700;letter-spacing:1px}
QStatusBar{background:#252a31;border-top:1px solid #3a414a}
QLabel[role="brand"]{font-size:30px;font-weight:800;letter-spacing:7px;color:#f5f7fb}QLabel[role="brandSub"]{font-size:10px;font-weight:700;letter-spacing:3px;color:#768292}
QLabel[role="meta"],QLabel[role="hint"]{font-size:10px;letter-spacing:1.2px;color:#7f8b9a;font-weight:700}QLabel[role="eyebrow"]{font-size:11px;letter-spacing:2.1px;color:#8a97a8;font-weight:800}QLabel[role="headline"]{font-size:38px;font-weight:700;color:#f5f7fa}QLabel[role="subhead"]{font-size:14px;color:#9aa5b3}QLabel[role="footer"]{font-size:10px;letter-spacing:1.2px;color:#687587;font-weight:700}QLabel[role="topLabel"]{font-size:10px;color:#7d8998;font-weight:700}QLabel[role="topValue"]{font-size:11px;color:#c1c8d2;font-weight:700}QLabel[role="cursor"]{font-size:11px;color:#8491a1;min-width:70px}QLabel[role="panelTitle"]{font-size:11px;letter-spacing:1.5px;font-weight:800}
QPushButton#homePrimary{background:#394b5d;border:1px solid #657e97;text-align:left;padding:19px;border-radius:10px;font-size:13px;font-weight:700}QPushButton#homePrimary:hover{background:#43586d}QPushButton#homeCard{background:#292f37;border:1px solid #3d4650;text-align:left;padding:17px;border-radius:10px;font-size:12px;font-weight:700}QPushButton#homeCard:hover{background:#343c46;border-color:#5d6977}QGroupBox#homeRecentBox{background:#272d34}QListWidget#homeRecentList{background:#20252b;border:0}
)STYLE");
    if(!m_statusLabel){m_statusLabel=new QLabel(QStringLiteral("Ready  •  Flux Core  •  Local workspace"));statusBar()->addWidget(m_statusLabel,1);}
}

void FluxMainWindow::showRecoveryBrowser(){enterWorkspace();if(m_statusLabel)m_statusLabel->setText(QStringLiteral("Recovery browser is available in Production panels."));}
void FluxMainWindow::markModified(){const QString name=m_filePath.isEmpty()?QStringLiteral("Untitled"):QFileInfo(m_filePath).completeBaseName();setWindowTitle(QStringLiteral("Flux Studio — %1 *").arg(name));if(m_statusLabel)m_statusLabel->setText(QStringLiteral("Modified  •  Unsaved changes"));}
