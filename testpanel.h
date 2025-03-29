#ifndef TESTPANEL_H
#define TESTPANEL_H

#include <QDialog>
#include <QSlider>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <QLineEdit>
#include <QTabWidget>
#include <QFormLayout>
#include "controllers/pumpcontroller.h"
#include "views/pinlockscreen.h"


class TestPanel : public QDialog
{
    Q_OBJECT

public:
    explicit TestPanel(PumpController *controller, QWidget *parent = nullptr);
    ~TestPanel();

private slots:
    // Existing slots
    void onBatteryLevelChanged(int value);
    void onInsulinLevelChanged(double value);
    void onGlucoseLevelChanged(double value);
    void onTrendDirectionChanged(int index);
    void onAlertButtonClicked();
    void onEmergencyLowButtonClicked();
    void onEmergencyHighButtonClicked();
    void onOcclusionButtonClicked();
    
    // New slots for enhanced functionality
    // Profile management
    void onCreateProfileButtonClicked();
    void onSwitchProfileButtonClicked();
    
    // Bolus testing
    void onDeliverBolusButtonClicked();
    void onExtendedBolusButtonClicked();
    void onCancelBolusButtonClicked();
    
    // Insulin delivery control
    void onStartStopInsulinButtonClicked();
    void onControlIQAdjustmentClicked();
    
    // CGM testing
    void onCGMDisconnectButtonClicked();
    
    // Additional error states
    void onBatteryDrainButtonClicked();
    void onPinLockTestButtonClicked();

private:
    PumpController *pumpController;

    // UI Elements for tab widget
    QTabWidget *tabWidget;
    
    // Tab 1: Basic Controls
    QWidget *basicControlsTab;
    
    // Battery controls
    QLabel *batteryLabel;
    QSlider *batterySlider;
    QSpinBox *batterySpinBox;
    
    // Insulin controls
    QLabel *insulinLabel;
    QSlider *insulinSlider;
    QDoubleSpinBox *insulinSpinBox;
    
    // Glucose controls
    QLabel *glucoseLabel;
    QSlider *glucoseSlider;
    QDoubleSpinBox *glucoseSpinBox;
    
    // Trend controls
    QLabel *trendLabel;
    QComboBox *trendComboBox;
    
    // Tab 2: Alerts and Errors
    QWidget *alertsTab;
    QPushButton *alertButton;
    QPushButton *emergencyLowButton;
    QPushButton *emergencyHighButton;
    QPushButton *occlusionButton;
    QPushButton *cgmDisconnectButton;
    QPushButton *batteryDrainButton;
    
    // Tab 3: Insulin Delivery
    QWidget *insulinDeliveryTab;
    QDoubleSpinBox *bolusAmountSpinBox;
    QSpinBox *bolusDurationSpinBox;
    QPushButton *deliverBolusButton;
    QPushButton *extendedBolusButton;
    QPushButton *cancelBolusButton;
    QPushButton *startStopInsulinButton;
    QPushButton *controlIQAdjustButton;
    
    // Tab 4: Profile Management
    QWidget *profileTab;
    QLineEdit *profileNameInput;
    QDoubleSpinBox *basalRateInput;
    QDoubleSpinBox *carbRatioInput;
    QDoubleSpinBox *correctionFactorInput;
    QDoubleSpinBox *targetGlucoseInput;
    QPushButton *createProfileButton;
    QComboBox *profileSelectCombo;
    QPushButton *switchProfileButton;
    
    // Tab 5: Security Testing
    QWidget *securityTab;
    QPushButton *pinLockTestButton;
    PinLockScreen *pinLockScreen = nullptr;
    
    // Close button (global)
    QPushButton *closeButton;
    
    // Title
    QLabel *titleLabel;
    
    void setupUi();
    void setupBasicControlsTab();
    void setupAlertsTab();
    void setupInsulinDeliveryTab();
    void setupProfileTab();
    void setupSecurityTab();
    void connectSignals();
    void updateProfileComboBox();
};

#endif // TESTPANEL_H
