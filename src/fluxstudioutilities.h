#pragma once
#include <QWidget>

class FluxDocument;
class QListWidget;
class QLabel;
class QComboBox;
class QSpinBox;
class QCheckBox;

class FluxStudioUtilities final : public QWidget {
    Q_OBJECT
public:
    explicit FluxStudioUtilities(FluxDocument* document, QWidget* parent=nullptr);
private:
    QWidget* buildColorLab();
    QWidget* buildAssets();
    QWidget* buildPreferences();
    QWidget* buildDiagnostics();
    void extractPalette(QListWidget* list, QLabel* info);
    FluxDocument* m_document{};
    QComboBox* m_colorSpace{};
    QComboBox* m_range{};
    QCheckBox* m_linear{};
    QCheckBox* m_premultiplied{};
    QSpinBox* m_uiScale{};
    QCheckBox* m_highContrast{};
    QCheckBox* m_colorBlind{};
    QCheckBox* m_checkerboard{};
    QCheckBox* m_smoothZoom{};
    QCheckBox* m_gestures{};
    QCheckBox* m_autosave{};
    QSpinBox* m_autosaveSeconds{};
    QSpinBox* m_backupCount{};
    QSpinBox* m_memoryLimit{};
};
