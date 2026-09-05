#pragma once

#include <QDialog>

class QCheckBox;
class QDoubleSpinBox;
class QSpinBox;
class QLabel;
class QPushButton;
class BrushEngine;

class BrushEditorDialog final : public QDialog {
    Q_OBJECT
public:
    explicit BrushEditorDialog(BrushEngine* engine, QWidget* parent=nullptr);

private slots:
    void loadTexture();
    void loadPreset();
    void savePreset();
    void sync();

private:
    BrushEngine* m_engine{};
    QSpinBox* m_size{};
    QDoubleSpinBox* m_opacity{};
    QDoubleSpinBox* m_flow{};
    QDoubleSpinBox* m_spacing{};
    QDoubleSpinBox* m_jitter{};
    QDoubleSpinBox* m_scatter{};
    QDoubleSpinBox* m_wetness{};
    QDoubleSpinBox* m_texture{};
    QDoubleSpinBox* m_stabilizer{};
    QCheckBox* m_pressureSize{};
    QCheckBox* m_pressureOpacity{};
    QCheckBox* m_tiltSize{};
    QLabel* m_textureLabel{};
};
