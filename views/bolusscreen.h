#ifndef BOLUSSCREEN_H
#define BOLUSSCREEN_H

#include <QWidget>
#include <QDoubleSpinBox>
#include <QLabel>
#include <QPushButton>
#include <QCheckBox>
#include <QSpinBox>
#include "../controllers/pumpcontroller.h"

namespace Ui {
class BolusScreen;
}

class BolusScreen : public QWidget
{
    Q_OBJECT

public:
    explicit BolusScreen(QWidget *parent = nullptr);
    ~BolusScreen();
    
    void updateCurrentValues(PumpController *controller);
    
signals:
    void backButtonClicked();
    void bolusRequested(double units, bool extended, int duration);
    
protected:
    void paintEvent(QPaintEvent *event) override;
    
private slots:
    void on_backButton_clicked();
    void on_deliverButton_clicked();
    void on_glucoseSpinBox_valueChanged(double value);
    void on_carbsSpinBox_valueChanged(double value);
    void on_bolusSpinBox_valueChanged(double value);
    void on_extendedCheckBox_stateChanged(int state);
    void on_durationSpinBox_valueChanged(int value);
    
private:
    Ui::BolusScreen *ui;
    
    double currentGlucose;
    double currentBasalRate;
    double carbRatio;         // Grams of carbs per unit of insulin
    double correctionFactor;  // mmol/L per unit of insulin
    double targetGlucose;     // Target glucose in mmol/L
    
    void setupUi();
    void connectSignals();
    void calculateSuggestedBolus();
    double calculateCarbBolus(double carbs);
    double calculateCorrectionBolus(double glucose);
    bool validateBolusSettings();
    void updateExtendedBolusVisibility();  // Added this missing declaration
    
    // UI elements
    QLabel *titleLabel;
    QLabel *glucoseLabel;
    QDoubleSpinBox *glucoseSpinBox;
    QLabel *carbsLabel;
    QDoubleSpinBox *carbsSpinBox;
    QLabel *suggestedBolusLabel;
    QLabel *suggestedBolusValueLabel;
    QLabel *bolusLabel;
    QDoubleSpinBox *bolusSpinBox;
    QCheckBox *extendedCheckBox;
    QLabel *durationLabel;
    QSpinBox *durationSpinBox;
    QPushButton *deliverButton;
    QPushButton *backButton;
    QWidget *extendedOptionsWidget;  // Added this missing declaration
};

#endif // BOLUSSCREEN_H
