#include <QApplication>
#include <QDockWidget>
#include <QFileInfo>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QMetaObject>
#include <QPushButton>
#include <QSettings>
#include <QToolBar>
#include <QVBoxLayout>
#include <QWidget>

#include "mainwindow.h"
#include "canvaswidget.h"

namespace {

QLabel* makeLabel(const QString& text, const QString& role, QWidget* parent = nullptr) {
    auto* label = new QLabel(text, parent);
    label->setProperty("role", role);
    label->setWordWrap(true);
    return label;
}

QPushButton* makeActionCard(const QString& eyebrow,
                            const QString& title,
                            const QString& description,
                            QWidget* parent = nullptr) {
    auto* button = new QPushButton(parent);
    button->setObjectName("startActionCard");
    button->setCursor(Qt::PointingHandCursor);
    button->setMinimumHeight(118);
    button->setText(QString("%1\n%2\n%3").arg(eyebrow, title, description));
    button->setStyleSheet(R"(
        QPushButton#startActionCard {
            text-align: left;
            padding: 20px;
            border-radius: 16px;
            border: 1px solid #303745;
            background: #171b22;
            color: #edf1f7;
            font-size: 13px;
        }
        QPushButton#startActionCard:hover {
            background: #1c222c;
            border-color: #59677d;
        }
        QPushButton#startActionCard:pressed {
            background: #202733;
        }
    )");
    return button;
}

QWidget* createStartPage(FluxMainWindow* window, QWidget* parent) {
    auto* page = new QWidget(parent);
    page->setObjectName("fluxStartPage");
    page->setAttribute(Qt::WA_StyledBackground, true);

    auto* root = new QVBoxLayout(page);
    root->setContentsMargins(56, 42, 56, 42);
    root->setSpacing(26);

    auto* top = new QHBoxLayout;
    auto* brand = new QVBoxLayout;
    brand->setSpacing(2);
    brand->addWidget(makeLabel("FLUX", "wordmark"));
    brand->addWidget(makeLabel("STUDIO", "wordmarkSecondary"));
    top->addLayout(brand);
    top->addStretch();
    top->addWidget(makeLabel("0.6  •  CREATIVE WORKSTATION", "version"), 0, Qt::AlignTop | Qt::AlignRight);
    root->addLayout(top);

    auto* hero = new QVBoxLayout;
    hero->setSpacing(8);
    hero->addWidget(makeLabel("WELCOME TO FLUX", "eyebrow"));
    hero->addWidget(makeLabel("Create something worth looking at.", "headline"));
    hero->addWidget(makeLabel("A focused workspace for drawing, animation, compositing and export — start from here instead of being dropped into an empty canvas.", "subhead"));
    root->addLayout(hero);

    auto* actions = new QHBoxLayout;
    actions->setSpacing(16);
    auto* newCard = makeActionCard("START", "New project", "Create a fresh 1920 × 1080 Flux document.", page);
    auto* openCard = makeActionCard("CONTINUE", "Open project", "Open an existing .flux project from disk.", page);
    actions->addWidget(newCard, 1);
    actions->addWidget(openCard, 1);
    root->addLayout(actions);

    auto* recentFrame = new QFrame(page);
    recentFrame->setObjectName("recentFrame");
    auto* recentLayout = new QVBoxLayout(recentFrame);
    recentLayout->setContentsMargins(20, 18, 20, 18);
    recentLayout->setSpacing(10);

    auto* recentHeader = new QHBoxLayout;
    recentHeader->addWidget(makeLabel("RECENT PROJECTS", "sectionTitle"));
    recentHeader->addStretch();
    recentHeader->addWidget(makeLabel("LOCAL ONLY", "sectionMeta"));
    recentLayout->addLayout(recentHeader);

    auto* recentList = new QListWidget(recentFrame);
    recentList->setObjectName("startRecentList");
    const auto recent = QSettings("Flux", "Flux Studio").value("recentProjects").toStringList();
    if (recent.isEmpty()) {
        auto* item = new QListWidgetItem("No recent projects yet — your next project will appear here.");
        item->setFlags(Qt::NoItemFlags);
        recentList->addItem(item);
    } else {
        for (const auto& path : recent) {
            auto* item = new QListWidgetItem(QFileInfo(path).completeBaseName());
            item->setData(Qt::UserRole, path);
            recentList->addItem(item);
        }
    }
    recentLayout->addWidget(recentList, 1);
    root->addWidget(recentFrame, 1);

    auto* footer = new QHBoxLayout;
    footer->addWidget(makeLabel("DRAW  →  ANIMATE  →  COMPOSE  →  EXPORT", "footerBrand"));
    footer->addStretch();
    footer->addWidget(makeLabel("Ctrl+N  New Project", "shortcutHint"));
    root->addLayout(footer);

    page->setStyleSheet(R"(
        QWidget#fluxStartPage { background: #10141a; color: #edf1f7; }
        QLabel[role="wordmark"] { font-size: 31px; font-weight: 800; letter-spacing: 8px; color: #f4f6fa; }
        QLabel[role="wordmarkSecondary"] { font-size: 11px; font-weight: 700; letter-spacing: 5px; color: #818b9b; }
        QLabel[role="version"], QLabel[role="sectionMeta"] { color: #697383; font-size: 10px; font-weight: 700; letter-spacing: 1.5px; }
        QLabel[role="eyebrow"] { color: #8290a5; font-size: 12px; font-weight: 700; letter-spacing: 2px; }
        QLabel[role="headline"] { color: #f5f7fb; font-size: 34px; font-weight: 700; }
        QLabel[role="subhead"] { color: #8e99a9; font-size: 14px; }
        QFrame#recentFrame { background: #141920; border: 1px solid #252c36; border-radius: 16px; }
        QLabel[role="sectionTitle"] { color: #b4bdca; font-size: 11px; font-weight: 800; letter-spacing: 1.6px; }
        QListWidget#startRecentList { background: transparent; border: 0; color: #aab3c0; outline: none; }
        QListWidget#startRecentList::item { padding: 12px 10px; border-radius: 9px; }
        QListWidget#startRecentList::item:selected, QListWidget#startRecentList::item:hover { background: #1d2430; }
        QLabel[role="footerBrand"] { color: #647083; font-size: 10px; font-weight: 700; letter-spacing: 1.5px; }
        QLabel[role="shortcutHint"] { color: #5c6778; font-size: 11px; }
    )");

    const auto showEditor = [page, window]() {
        page->hide();
        for (auto* dock : window->findChildren<QDockWidget*>()) dock->show();
        for (auto* toolbar : window->findChildren<QToolBar*>()) toolbar->show();
    };

    QObject::connect(newCard, &QPushButton::clicked, window, [window, showEditor]() {
        QMetaObject::invokeMethod(window, "newProject", Qt::DirectConnection);
        showEditor();
    });

    QObject::connect(openCard, &QPushButton::clicked, window, [window, showEditor]() {
        QMetaObject::invokeMethod(window, "openProject", Qt::DirectConnection);
        showEditor();
    });

    QObject::connect(recentList, &QListWidget::itemDoubleClicked, window, [window, showEditor](QListWidgetItem* item) {
        if (item->data(Qt::UserRole).toString().isEmpty()) return;
        for (const auto* action : window->menuBar()->actions()) {
            if (!action->menu()) continue;
            for (auto* menuAction : action->menu()->actions()) {
                if (menuAction->text().startsWith("Open")) {
                    menuAction->trigger();
                    showEditor();
                    return;
                }
            }
        }
    });

    return page;
}

} // namespace

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    QCoreApplication::setApplicationName("Flux Studio");
    QCoreApplication::setApplicationVersion("0.6.0");
    QCoreApplication::setOrganizationName("Flux");

    FluxMainWindow window;
    window.show();

    if (auto* canvas = window.findChild<FluxCanvas*>()) {
        auto* page = createStartPage(&window, canvas);
        page->setGeometry(canvas->rect());
        page->show();
        page->raise();
        for (auto* dock : window.findChildren<QDockWidget*>()) dock->hide();
        for (auto* toolbar : window.findChildren<QToolBar*>()) toolbar->hide();
    }

    return app.exec();
}
