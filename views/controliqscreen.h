#ifndef CONTROLIQSCREEN_H
#define CONTROLIQSCREEN_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QCheckBox>
#include <QComboBox>
#include <QTimeEdit>
#include <QDoubleSpinBox>
#include <QSlider>
#include "../controllers/pumpcontroller.h"
#include "../utils/controliqalgorithm.h"

namespace Ui {
class ControlIQScreen;
}

class ControlIQScreen : public QWidget
{
    Q_OBJECT

public:
    explicit ControlIQScreen(QWidget *parent = nullptr);
    ~ControlIQScreen();
    
    void setPumpController(PumpController *controller);
    void setControlIQAlgorithm(ControlIQAlgorithm *controlIQAlgorithm);
    void updateUIFromSettings();
    
signals:
    void backButtonClicked();
    void homeButtonClicked();
    
protected:
    void paintEvent(QPaintEvent *event) override;
    
private slots:
    void onEnableControlIQToggled(bool checked);
    void saveSettings();
    void onHomeButtonClicked();
    
private:
    Ui::ControlIQScreen *ui;
    PumpController *pumpController;
    ControlIQAlgorithm *algorithm;
    
    // UI elements
    QLabel *titleLabel;
    QCheckBox *enableControlIQCheckBox;
    QLabel *statusLabel;
    QCheckBox *sleepModeCheckBox;
    QTimeEdit *sleepStartTime;
    QTimeEdit *sleepEndTime;
    QCheckBox *exerciseModeCheckBox;
    QComboBox *durationComboBox;
    QDoubleSpinBox *targetLowSpinBox;
    QDoubleSpinBox *targetHighSpinBox;
    QDoubleSpinBox *maxBasalRateSpinBox;
    QSlider *aggressivenessSlider;
    QLabel *aggressivenessValueLabel;
    QCheckBox *hypoPreventionCheckBox;
    QPushButton *saveButton;
    QPushButton *backButton;
    QPushButton *homeButton;
    
    void setupUi();
    void connectSignals();
};

#endif // CONTROLIQSCREEN_H
