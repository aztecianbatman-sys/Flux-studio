#pragma once
#include <QDialog>
#include <QVector>
#include <QString>
#include <functional>

struct FluxCommand { QString id; QString title; QString shortcut; std::function<void()> invoke; };
class QLineEdit; class QListWidget;
class FluxCommandPalette final : public QDialog {
    Q_OBJECT
public:
    explicit FluxCommandPalette(QWidget*parent=nullptr);
    void setCommands(const QVector<FluxCommand>&commands);
private:
    void rebuild();
    QLineEdit* m_search{}; QListWidget* m_list{}; QVector<FluxCommand> m_commands;
};
