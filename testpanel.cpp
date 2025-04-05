#include "testpanel.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QMessageBox>
#include <QScrollArea>
#include <QScreen>
#include <QApplication>
#include <QInputDialog>

TestPanel::TestPanel(PumpController *controller, QWidget *parent)
    : QDialog(parent),
      pumpController(controller)
{
    setWindowTitle("Pump Simulator Test Panel");
    
    // Make the test panel resizable
    setWindowFlags(windowFlags() | Qt::WindowMaximizeButtonHint | Qt::WindowMinimizeButtonHint);
    
    // Set up UI with layouts
    setupUi();
    
    // Connect signals
    connectSignals();
    
    // Initialize with current values
    if (pumpController) {
        batterySpinBox->setValue(pumpController->getBatteryLevel());
        insulinSpinBox->setValue(pumpController->getInsulinRemaining());
        glucoseSpinBox->setValue(pumpController->getCurrentGlucose());
        
        // Set trend combobox based on current trend
        int trendIndex = 2; // Default to Stable
        switch (pumpController->getGlucoseTrend()) {
            case GlucoseModel::RisingQuickly: trendIndex = 0; break;
            case GlucoseModel::Rising: trendIndex = 1; break;
            case GlucoseModel::Stable: trendIndex = 2; break;
            case GlucoseModel::Falling: trendIndex = 3; break;
            case GlucoseModel::FallingQuickly: trendIndex = 4; break;
            default: trendIndex = 2; break;
        }
        trendComboBox->setCurrentIndex(trendIndex);
        
        // Update profile combobox
        updateProfileComboBox();
        
        // Set insulin button text based on current status
        if (pumpController->isPumpRunning()) {
            startStopInsulinButton->setText("Stop Insulin");
        } else {
            startStopInsulinButton->setText("Start Insulin");
        }
    }
    
    // Set a reasonable initial size
    QScreen *screen = QGuiApplication::primaryScreen();
    if (screen) {
        QRect screenGeometry = screen->availableGeometry();
        int width = qMin(800, screenGeometry.width() - 100);
        int height = qMin(600, screenGeometry.height() - 100);
        resize(width, height);
    } else {
        resize(700, 500);
    }
}

TestPanel::~TestPanel()
{
    // Nothing to clean up
}

void TestPanel::setupUi()
{
    // Main layout
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(16, 16, 16, 16);
    mainLayout->setSpacing(16);
    
    // Title
    titleLabel = new QLabel("Pump Simulator Testing Panel", this);
    titleLabel->setStyleSheet("font-size: 18px; font-weight: bold;");
    mainLayout->addWidget(titleLabel);
    
    // Tab widget to organize test controls
    tabWidget = new QTabWidget(this);
    mainLayout->addWidget(tabWidget);
    
    // Create tabs
    basicControlsTab = new QWidget();
    alertsTab = new QWidget();
    insulinDeliveryTab = new QWidget();
    profileTab = new QWidget();
    securityTab = new QWidget();
    
    // Set up each tab
    setupBasicControlsTab();
    setupAlertsTab();
    setupInsulinDeliveryTab();
    setupProfileTab();
    setupSecurityTab();
    
    // Add tabs to tab widget
    tabWidget->addTab(basicControlsTab, "Basic Controls");
    tabWidget->addTab(alertsTab, "Alerts and Errors");
    tabWidget->addTab(insulinDeliveryTab, "Insulin Delivery");
    tabWidget->addTab(profileTab, "Profile Management");
    tabWidget->addTab(securityTab, "Security");
    
    // Close button at the bottom
    QHBoxLayout *closeButtonLayout = new QHBoxLayout();
    closeButtonLayout->addStretch();
    
    closeButton = new QPushButton("Close", this);
    closeButton->setStyleSheet("padding: 8px 16px;");
    closeButton->setFixedWidth(100);
    
    closeButtonLayout->addWidget(closeButton);
    mainLayout->addLayout(closeButtonLayout);
    
    // Set layout for dialog
    setLayout(mainLayout);
}

void TestPanel::setupBasicControlsTab()
{
    QVBoxLayout *layout = new QVBoxLayout(basicControlsTab);
    layout->setContentsMargins(8, 8, 8, 8);
    layout->setSpacing(16);
    
    // Battery control group
    QGroupBox *batteryGroup = new QGroupBox("Battery Control", basicControlsTab);
    QHBoxLayout *batteryLayout = new QHBoxLayout(batteryGroup);
    batteryLayout->setContentsMargins(12, 16, 12, 16);
    batteryLayout->setSpacing(12);
    
    batteryLabel = new QLabel("Battery Level:", basicControlsTab);
    batterySlider = new QSlider(Qt::Horizontal, basicControlsTab);
    batterySlider->setRange(0, 100);
    batterySlider->setValue(100);
    batterySlider->setTickPosition(QSlider::TicksBelow);
    batterySlider->setTickInterval(10);
    
    batterySpinBox = new QSpinBox(basicControlsTab);
    batterySpinBox->setRange(0, 100);
    batterySpinBox->setValue(100);
    batterySpinBox->setSuffix("%");
    batterySpinBox->setFixedWidth(80);
    
    batteryLayout->addWidget(batteryLabel, 1);
    batteryLayout->addWidget(batterySlider, 4);
    batteryLayout->addWidget(batterySpinBox, 1);
    
    layout->addWidget(batteryGroup);
    
    // Insulin control group
    QGroupBox *insulinGroup = new QGroupBox("Insulin Control", basicControlsTab);
    QHBoxLayout *insulinLayout = new QHBoxLayout(insulinGroup);
    insulinLayout->setContentsMargins(12, 16, 12, 16);
    insulinLayout->setSpacing(12);
    
    insulinLabel = new QLabel("Insulin Remaining:", basicControlsTab);
    insulinSlider = new QSlider(Qt::Horizontal, basicControlsTab);
    insulinSlider->setRange(0, 300);
    insulinSlider->setValue(300);
    insulinSlider->setTickPosition(QSlider::TicksBelow);
    insulinSlider->setTickInterval(50);
    
    insulinSpinBox = new QDoubleSpinBox(basicControlsTab);
    insulinSpinBox->setRange(0, 300);
    insulinSpinBox->setValue(300);
    insulinSpinBox->setSuffix(" u");
    insulinSpinBox->setFixedWidth(80);
    
    insulinLayout->addWidget(insulinLabel, 1);
    insulinLayout->addWidget(insulinSlider, 4);
    insulinLayout->addWidget(insulinSpinBox, 1);
    
    layout->addWidget(insulinGroup);
    
    // Glucose control group
    QGroupBox *glucoseGroup = new QGroupBox("Glucose Control", basicControlsTab);
    QHBoxLayout *glucoseLayout = new QHBoxLayout(glucoseGroup);
    glucoseLayout->setContentsMargins(12, 16, 12, 16);
    glucoseLayout->setSpacing(12);
    
    glucoseLabel = new QLabel("Glucose Level:", basicControlsTab);
    glucoseSlider = new QSlider(Qt::Horizontal, basicControlsTab);
    glucoseSlider->setRange(0, 250);  // 0-25.0 mmol/L * 10
    glucoseSlider->setValue(55);      // 5.5 mmol/L
    glucoseSlider->setTickPosition(QSlider::TicksBelow);
    glucoseSlider->setTickInterval(20);
    
    glucoseSpinBox = new QDoubleSpinBox(basicControlsTab);
    glucoseSpinBox->setRange(0, 25.0);
    glucoseSpinBox->setValue(5.5);
    glucoseSpinBox->setSuffix(" mmol/L");
    glucoseSpinBox->setDecimals(1);
    glucoseSpinBox->setFixedWidth(100);
    
    glucoseLayout->addWidget(glucoseLabel, 1);
    glucoseLayout->addWidget(glucoseSlider, 4);
    glucoseLayout->addWidget(glucoseSpinBox, 1);
    
    layout->addWidget(glucoseGroup);
    
    // Trend direction control
    QGroupBox *trendGroup = new QGroupBox("Glucose Trend", basicControlsTab);
    QHBoxLayout *trendLayout = new QHBoxLayout(trendGroup);
    trendLayout->setContentsMargins(12, 16, 12, 16);
    trendLayout->setSpacing(12);
    
    trendLabel = new QLabel("Trend Direction:", basicControlsTab);
    trendComboBox = new QComboBox(basicControlsTab);
    trendComboBox->addItem("Rising Quickly (↑↑)");
    trendComboBox->addItem("Rising (↑)");
    trendComboBox->addItem("Stable (→)");
    trendComboBox->addItem("Falling (↓)");
    trendComboBox->addItem("Falling Quickly (↓↓)");
    trendComboBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    
    trendLayout->addWidget(trendLabel, 1);
    trendLayout->addWidget(trendComboBox, 4);
    
    layout->addWidget(trendGroup);
    
    // Add stretch to push everything to the top
    layout->addStretch(1);
}

void TestPanel::setupAlertsTab()
{
    QVBoxLayout *layout = new QVBoxLayout(alertsTab);
    layout->setContentsMargins(8, 8, 8, 8);
    layout->setSpacing(16);
    
    // Basic Alerts Group
    QGroupBox *basicAlertGroup = new QGroupBox("Basic Alerts", alertsTab);
    QVBoxLayout *basicAlertLayout = new QVBoxLayout(basicAlertGroup);
    
    alertButton = new QPushButton("Generate Test Alert", alertsTab);
    alertButton->setStyleSheet("padding: 8px;");
    
    basicAlertLayout->addWidget(alertButton);
    layout->addWidget(basicAlertGroup);
    
    // Emergency Alerts Group
    QGroupBox *emergencyGroup = new QGroupBox("Emergency Alerts", alertsTab);
    QVBoxLayout *emergencyLayout = new QVBoxLayout(emergencyGroup);
    
    emergencyLowButton = new QPushButton("Emergency Low Glucose", alertsTab);
    emergencyLowButton->setStyleSheet("background-color: #FF3B30; color: white; padding: 8px;");
    
    emergencyHighButton = new QPushButton("Emergency High Glucose", alertsTab);
    emergencyHighButton->setStyleSheet("background-color: #FF9500; color: white; padding: 8px;");
    
    emergencyLayout->addWidget(emergencyLowButton);
    emergencyLayout->addWidget(emergencyHighButton);
    layout->addWidget(emergencyGroup);
    
    // Device Alerts Group
    QGroupBox *deviceGroup = new QGroupBox("Device Alerts", alertsTab);
    QVBoxLayout *deviceLayout = new QVBoxLayout(deviceGroup);
    
    occlusionButton = new QPushButton("Simulate Occlusion", alertsTab);
    occlusionButton->setStyleSheet("background-color: #FF9500; color: white; padding: 8px;");
    
    cgmDisconnectButton = new QPushButton("Simulate CGM Disconnection", alertsTab);
    cgmDisconnectButton->setStyleSheet("padding: 8px;");
    
    batteryDrainButton = new QPushButton("Simulate Battery Drain (Critical)", alertsTab);
    batteryDrainButton->setStyleSheet("padding: 8px;");
    
    deviceLayout->addWidget(occlusionButton);
    deviceLayout->addWidget(cgmDisconnectButton);
    deviceLayout->addWidget(batteryDrainButton);
    layout->addWidget(deviceGroup);
    
    // Add stretch to push everything to the top
    layout->addStretch(1);
}

void TestPanel::setupInsulinDeliveryTab()
{
    QVBoxLayout *layout = new QVBoxLayout(insulinDeliveryTab);
    layout->setContentsMargins(8, 8, 8, 8);
    layout->setSpacing(16);
    
    // Bolus Settings Group
    QGroupBox *bolusGroup = new QGroupBox("Bolus Settings", insulinDeliveryTab);
    QFormLayout *bolusLayout = new QFormLayout(bolusGroup);
    
    bolusAmountSpinBox = new QDoubleSpinBox(insulinDeliveryTab);
    bolusAmountSpinBox->setRange(0.1, 25.0);
    bolusAmountSpinBox->setValue(1.0);
    bolusAmountSpinBox->setSuffix(" u");
    bolusAmountSpinBox->setSingleStep(0.1);
    
    bolusDurationSpinBox = new QSpinBox(insulinDeliveryTab);
    bolusDurationSpinBox->setRange(0, 480);
    bolusDurationSpinBox->setValue(0);
    bolusDurationSpinBox->setSuffix(" min");
    bolusDurationSpinBox->setSingleStep(15);
    bolusDurationSpinBox->setSpecialValueText("Standard");
    
    bolusLayout->addRow("Bolus Amount:", bolusAmountSpinBox);
    bolusLayout->addRow("Duration (for extended):", bolusDurationSpinBox);
    
    layout->addWidget(bolusGroup);
    
    // Bolus Control Group
    QGroupBox *bolusControlGroup = new QGroupBox("Bolus Control", insulinDeliveryTab);
    QHBoxLayout *bolusControlLayout = new QHBoxLayout(bolusControlGroup);
    
    deliverBolusButton = new QPushButton("Deliver Standard Bolus", insulinDeliveryTab);
    deliverBolusButton->setStyleSheet("background-color: #007AFF; color: white; padding: 8px;");
    
    extendedBolusButton = new QPushButton("Deliver Extended Bolus", insulinDeliveryTab);
    extendedBolusButton->setStyleSheet("background-color: #5856D6; color: white; padding: 8px;");
    
    cancelBolusButton = new QPushButton("Cancel Active Bolus", insulinDeliveryTab);
    cancelBolusButton->setStyleSheet("background-color: #FF3B30; color: white; padding: 8px;");
    
    bolusControlLayout->addWidget(deliverBolusButton);
    bolusControlLayout->addWidget(extendedBolusButton);
    bolusControlLayout->addWidget(cancelBolusButton);
    
    layout->addWidget(bolusControlGroup);
    
    // Basal Control Group
    QGroupBox *basalGroup = new QGroupBox("Basal Control", insulinDeliveryTab);
    QVBoxLayout *basalLayout = new QVBoxLayout(basalGroup);
    
    startStopInsulinButton = new QPushButton("Start/Stop Insulin", insulinDeliveryTab);
    startStopInsulinButton->setStyleSheet("padding: 8px;");
    
    controlIQAdjustButton = new QPushButton("Simulate Control-IQ Adjustment", insulinDeliveryTab);
    controlIQAdjustButton->setStyleSheet("padding: 8px;");
    
    basalLayout->addWidget(startStopInsulinButton);
    basalLayout->addWidget(controlIQAdjustButton);
    
    layout->addWidget(basalGroup);
    
    // Add stretch to push everything to the top
    layout->addStretch(1);
}

void TestPanel::setupProfileTab()
{
    QVBoxLayout *layout = new QVBoxLayout(profileTab);
    layout->setContentsMargins(8, 8, 8, 8);
    layout->setSpacing(16);
    
    // Create Profile Group
    QGroupBox *createProfileGroup = new QGroupBox("Create Test Profile", profileTab);
    QFormLayout *createProfileLayout = new QFormLayout(createProfileGroup);
    
    profileNameInput = new QLineEdit(profileTab);
    profileNameInput->setText("TestProfile");
    
    basalRateInput = new QDoubleSpinBox(profileTab);
    basalRateInput->setRange(0.1, 5.0);
    basalRateInput->setValue(1.0);
    basalRateInput->setSuffix(" u/hr");
    basalRateInput->setSingleStep(0.1);
    
    carbRatioInput = new QDoubleSpinBox(profileTab);
    carbRatioInput->setRange(1.0, 50.0);
    carbRatioInput->setValue(10.0);
    carbRatioInput->setSuffix(" g/u");
    carbRatioInput->setSingleStep(0.5);
    
    correctionFactorInput = new QDoubleSpinBox(profileTab);
    correctionFactorInput->setRange(0.1, 10.0);
    correctionFactorInput->setValue(2.0);
    correctionFactorInput->setSuffix(" mmol/L/u");
    correctionFactorInput->setSingleStep(0.1);
    
    targetGlucoseInput = new QDoubleSpinBox(profileTab);
    targetGlucoseInput->setRange(3.0, 10.0);
    targetGlucoseInput->setValue(5.5);
    targetGlucoseInput->setSuffix(" mmol/L");
    targetGlucoseInput->setSingleStep(0.1);
    
    createProfileLayout->addRow("Profile Name:", profileNameInput);
    createProfileLayout->addRow("Basal Rate:", basalRateInput);
    createProfileLayout->addRow("Carb Ratio:", carbRatioInput);
    createProfileLayout->addRow("Correction Factor:", correctionFactorInput);
    createProfileLayout->addRow("Target Glucose:", targetGlucoseInput);
    
    createProfileButton = new QPushButton("Create Profile", profileTab);
    createProfileButton->setStyleSheet("background-color: #4CD964; color: white; padding: 8px;");
    createProfileLayout->addRow(createProfileButton);
    
    layout->addWidget(createProfileGroup);
    
    // Switch Profile Group
    QGroupBox *switchProfileGroup = new QGroupBox("Switch Active Profile", profileTab);
    QHBoxLayout *switchProfileLayout = new QHBoxLayout(switchProfileGroup);
    
    profileSelectCombo = new QComboBox(profileTab);
    
    switchProfileButton = new QPushButton("Switch to Selected Profile", profileTab);
    switchProfileButton->setStyleSheet("background-color: #007AFF; color: white; padding: 8px;");
    
    switchProfileLayout->addWidget(profileSelectCombo);
    switchProfileLayout->addWidget(switchProfileButton);
    
    layout->addWidget(switchProfileGroup);
    
    // Add stretch to push everything to the top
    layout->addStretch(1);
}

void TestPanel::setupSecurityTab()
{
    QVBoxLayout *layout = new QVBoxLayout(securityTab);
    layout->setContentsMargins(8, 8, 8, 8);
    layout->setSpacing(16);
    
    // PIN Lock Testing
    QGroupBox *pinGroup = new QGroupBox("Security Testing", securityTab);
    QVBoxLayout *pinLayout = new QVBoxLayout(pinGroup);
    
    pinLockTestButton = new QPushButton("Test PIN Lock Screen", securityTab);
    pinLockTestButton->setStyleSheet("padding: 8px;");
    
    pinLayout->addWidget(pinLockTestButton);
    layout->addWidget(pinGroup);
    
    // Add stretch to push everything to the top
    layout->addStretch(1);
}

void TestPanel::connectSignals()
{
    // Basic Controls Tab
    // Battery controls
    connect(batterySlider, &QSlider::valueChanged, batterySpinBox, &QSpinBox::setValue);
    connect(batterySpinBox, QOverload<int>::of(&QSpinBox::valueChanged), batterySlider, &QSlider::setValue);
    connect(batterySpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &TestPanel::onBatteryLevelChanged);
    
    // Insulin controls
    connect(insulinSlider, &QSlider::valueChanged, [this](int value) {
        insulinSpinBox->setValue(value);
    });
    connect(insulinSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double value) {
        insulinSlider->setValue(static_cast<int>(value));
    });
    connect(insulinSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &TestPanel::onInsulinLevelChanged);
    
    // Glucose controls
    connect(glucoseSlider, &QSlider::valueChanged, [this](int value) {
        glucoseSpinBox->setValue(value / 10.0);
    });
    connect(glucoseSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double value) {
        glucoseSlider->setValue(static_cast<int>(value * 10));
    });
    connect(glucoseSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &TestPanel::onGlucoseLevelChanged);
    
    // Trend direction control
    connect(trendComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &TestPanel::onTrendDirectionChanged);
    
    // Alerts Tab
    connect(alertButton, &QPushButton::clicked, this, &TestPanel::onAlertButtonClicked);
    connect(emergencyLowButton, &QPushButton::clicked, this, &TestPanel::onEmergencyLowButtonClicked);
    connect(emergencyHighButton, &QPushButton::clicked, this, &TestPanel::onEmergencyHighButtonClicked);
    connect(occlusionButton, &QPushButton::clicked, this, &TestPanel::onOcclusionButtonClicked);
    connect(cgmDisconnectButton, &QPushButton::clicked, this, &TestPanel::onCGMDisconnectButtonClicked);
    connect(batteryDrainButton, &QPushButton::clicked, this, &TestPanel::onBatteryDrainButtonClicked);
    
    // Insulin Delivery Tab
    connect(deliverBolusButton, &QPushButton::clicked, this, &TestPanel::onDeliverBolusButtonClicked);
    connect(extendedBolusButton, &QPushButton::clicked, this, &TestPanel::onExtendedBolusButtonClicked);
    connect(cancelBolusButton, &QPushButton::clicked, this, &TestPanel::onCancelBolusButtonClicked);
    connect(startStopInsulinButton, &QPushButton::clicked, this, &TestPanel::onStartStopInsulinButtonClicked);
    connect(controlIQAdjustButton, &QPushButton::clicked, this, &TestPanel::onControlIQAdjustmentClicked);
    
    // Profile Tab
    connect(createProfileButton, &QPushButton::clicked, this, &TestPanel::onCreateProfileButtonClicked);
    connect(switchProfileButton, &QPushButton::clicked, this, &TestPanel::onSwitchProfileButtonClicked);
    
    // Security Tab
    connect(pinLockTestButton, &QPushButton::clicked, this, &TestPanel::onPinLockTestButtonClicked);
    
    // Close button
    connect(closeButton, &QPushButton::clicked, this, &QDialog::accept);
}

// Existing slot implementations
void TestPanel::onBatteryLevelChanged(int value)
{
    if (pumpController) {
        pumpController->updateBatteryLevel(value);
    }
}

void TestPanel::onInsulinLevelChanged(double value)
{
    if (pumpController) {
        pumpController->updateInsulinRemaining(value);
    }
}

void TestPanel::onGlucoseLevelChanged(double value)
{
    if (pumpController) {
        pumpController->updateGlucoseLevel(value);
    }
}

void TestPanel::onTrendDirectionChanged(int index)
{
    if (pumpController) {
        GlucoseModel::TrendDirection trend;
        
        switch (index) {
            case 0: trend = GlucoseModel::RisingQuickly; break;
            case 1: trend = GlucoseModel::Rising; break;
            case 2: trend = GlucoseModel::Stable; break;
            case 3: trend = GlucoseModel::Falling; break;
            case 4: trend = GlucoseModel::FallingQuickly; break;
            default: trend = GlucoseModel::Stable; break;
        }
        
        pumpController->updateGlucoseTrend(trend);
    }
}

void TestPanel::onAlertButtonClicked()
{
    if (pumpController) {
        pumpController->generateTestAlert("This is a test alert", PumpModel::Warning);
        QMessageBox::information(this, "Alert Generated", "A test alert has been generated. Check the alerts screen.");
    }
}

void TestPanel::onEmergencyLowButtonClicked()
{
    if (pumpController) {
        glucoseSpinBox->setValue(2.8); // Set very low glucose
        pumpController->generateTestAlert("URGENT LOW GLUCOSE: 2.8 mmol/L", PumpModel::Critical);
        QMessageBox::warning(this, "Emergency Low Glucose", "Emergency low glucose alert generated. Insulin delivery should be suspended.");
    }
}

void TestPanel::onEmergencyHighButtonClicked()
{
    if (pumpController) {
        glucoseSpinBox->setValue(18.5); // Set very high glucose
        pumpController->generateTestAlert("URGENT HIGH GLUCOSE: 18.5 mmol/L", PumpModel::Critical);
        QMessageBox::warning(this, "Emergency High Glucose", "Emergency high glucose alert generated.");
    }
}

void TestPanel::onOcclusionButtonClicked()
{
    if (pumpController) {
        // Generate the alert
        pumpController->generateTestAlert("OCCLUSION DETECTED: Check infusion set", PumpModel::Critical);
        
        // In a real implementation, you would need to access the insulin model through pumpController
        // For now, we can show a message about what would happen
        QMessageBox::warning(this, "Occlusion Simulated", 
            "Occlusion detected. Insulin delivery has been suspended. Check the alerts screen.");
    }
}

// New slot implementations
void TestPanel::onCreateProfileButtonClicked()
{
    if (pumpController) {
        // Create a new profile with the input values
        Profile newProfile;
        newProfile.name = profileNameInput->text();
        newProfile.basalRate = basalRateInput->value();
        newProfile.carbRatio = carbRatioInput->value();
        newProfile.correctionFactor = correctionFactorInput->value();
        newProfile.targetGlucose = targetGlucoseInput->value();
        
        // Add the profile
        bool success = pumpController->createProfile(newProfile);
        
        if (success) {
            QMessageBox::information(this, "Profile Created", 
                QString("Profile '%1' has been created successfully.").arg(newProfile.name));
            
            // Update the profile combo box
            updateProfileComboBox();
        } else {
            QMessageBox::warning(this, "Profile Creation Failed", 
                "Failed to create the profile. Profile name may already exist.");
        }
    }
}

void TestPanel::onSwitchProfileButtonClicked()
{
    if (pumpController && profileSelectCombo->currentIndex() >= 0) {
        QString profileName = profileSelectCombo->currentText();
        
        // Switch to the selected profile
        pumpController->setActiveProfile(profileName);
        
        // Assume success since the method doesn't return a value
        QMessageBox::information(this, "Profile Activated", 
            QString("Profile '%1' has been activated.").arg(profileName));
    }
}

void TestPanel::onDeliverBolusButtonClicked()
{
    if (pumpController) {
        double amount = bolusAmountSpinBox->value();
        
        // Confirm bolus delivery
        QMessageBox::StandardButton reply = QMessageBox::question(this, "Confirm Bolus", 
            QString("Deliver standard bolus of %1 units?").arg(amount),
            QMessageBox::Yes | QMessageBox::No);
        
        if (reply == QMessageBox::Yes) {
            bool success = pumpController->deliverBolus(amount, false, 0);
            
            if (success) {
                QMessageBox::information(this, "Bolus Delivery", 
                    QString("Standard bolus of %1 units started.").arg(amount));
            } else {
                QMessageBox::warning(this, "Bolus Delivery Failed", 
                    "Failed to deliver bolus. Check if pump is running and no bolus is active.");
            }
        }
    }
}

void TestPanel::onExtendedBolusButtonClicked()
{
    if (pumpController) {
        double amount = bolusAmountSpinBox->value();
        int duration = bolusDurationSpinBox->value();
        
        if (duration <= 0) {
            QMessageBox::warning(this, "Invalid Duration", 
                "Please set a duration greater than 0 for extended bolus.");
            return;
        }
        
        // Confirm extended bolus delivery
        QMessageBox::StandardButton reply = QMessageBox::question(this, "Confirm Extended Bolus", 
            QString("Deliver extended bolus of %1 units over %2 minutes?").arg(amount).arg(duration),
            QMessageBox::Yes | QMessageBox::No);
        
        if (reply == QMessageBox::Yes) {
            bool success = pumpController->deliverBolus(amount, true, duration);
            
            if (success) {
                QMessageBox::information(this, "Extended Bolus Delivery", 
                    QString("Extended bolus of %1 units over %2 minutes started.").arg(amount).arg(duration));
            } else {
                QMessageBox::warning(this, "Extended Bolus Delivery Failed", 
                    "Failed to deliver extended bolus. Check if pump is running and no bolus is active.");
            }
        }
    }
}

void TestPanel::onCancelBolusButtonClicked()
{
    if (pumpController) {
        if (pumpController->isBolusActive()) {
            QMessageBox::StandardButton reply = QMessageBox::question(this, "Cancel Bolus", 
                "Cancel the active bolus delivery?",
                QMessageBox::Yes | QMessageBox::No);
            
            if (reply == QMessageBox::Yes) {
                bool success = pumpController->cancelBolus();
                
                if (success) {
                    QMessageBox::information(this, "Bolus Cancelled", 
                        "The active bolus has been cancelled.");
                } else {
                    QMessageBox::warning(this, "Bolus Cancellation Failed", 
                        "Failed to cancel the bolus.");
                }
            }
        } else {
            QMessageBox::information(this, "No Active Bolus", 
                "There is no active bolus to cancel.");
        }
    }
}

void TestPanel::onStartStopInsulinButtonClicked()
{
    if (pumpController) {
        if (pumpController->isPumpRunning()) {
            // Stop insulin delivery
            QMessageBox::StandardButton reply = QMessageBox::question(this, "Stop Insulin", 
                "Stop all insulin delivery?",
                QMessageBox::Yes | QMessageBox::No);
            
            if (reply == QMessageBox::Yes) {
                pumpController->stopPump();
                startStopInsulinButton->setText("Start Insulin");
                QMessageBox::information(this, "Insulin Stopped", 
                    "All insulin delivery has been stopped.");
            }
        } else {
            // Start insulin delivery
            QMessageBox::StandardButton reply = QMessageBox::question(this, "Start Insulin", 
                "Start insulin delivery?",
                QMessageBox::Yes | QMessageBox::No);
            
            if (reply == QMessageBox::Yes) {
                pumpController->startPump();
                startStopInsulinButton->setText("Stop Insulin");
                QMessageBox::information(this, "Insulin Started", 
                    "Insulin delivery has been started.");
            }
        }
    }
}

void TestPanel::onControlIQAdjustmentClicked()
{
    if (pumpController) {
        bool ok;
        double adjustment = QInputDialog::getDouble(this, "Control-IQ Adjustment",
            "Enter basal rate adjustment (units/hour):\n(positive for increase, negative for decrease)",
            0.0, -2.0, 2.0, 1, &ok);
        
        if (ok) {
            // In a real implementation, you would apply this adjustment through ControlIQAlgorithm
            // For now, we can simulate the effect
            if (pumpController->isControlIQEnabled()) {
                // Trigger a control-IQ message
                QString message = adjustment > 0 ? 
                    QString("Control-IQ increased basal rate by %1 u/hr").arg(adjustment) :
                    QString("Control-IQ decreased basal rate by %1 u/hr").arg(-adjustment);
                
                pumpController->generateTestAlert(message, PumpModel::Info);
                
                QMessageBox::information(this, "Control-IQ Adjustment", 
                    QString("Control-IQ has adjusted the basal rate by %1 u/hr.").arg(adjustment));
            } else {
                QMessageBox::warning(this, "Control-IQ Disabled", 
                    "Control-IQ is not enabled. Enable it in the Control-IQ settings screen.");
            }
        }
    }
}

void TestPanel::onCGMDisconnectButtonClicked()
{
    if (pumpController) {
        // Simulate CGM disconnection
        pumpController->generateTestAlert("CGM SIGNAL LOST: Check sensor connection", PumpModel::Warning);
        
        QMessageBox::warning(this, "CGM Disconnected", 
            "CGM disconnection simulated. A warning alert has been generated.");
    }
}

void TestPanel::onBatteryDrainButtonClicked()
{
    if (pumpController) {
        // Set battery to critical level
        batterySpinBox->setValue(3);
        
        QMessageBox::critical(this, "Critical Battery", 
            "Battery level set to critical (3%). The pump will generate alerts and may shut down soon.");
    }
}

void TestPanel::onPinLockTestButtonClicked()
{
    if (pumpController) {
        // Create the PIN lock screen if it doesn't exist yet
        if (!pinLockScreen) {
            pinLockScreen = new PinLockScreen(this);
            
            // Connect its signals
            connect(pinLockScreen, &PinLockScreen::pinAccepted, this, [this]() {
                QMessageBox::information(this, "PIN Status", "PIN accepted successfully!");
                pinLockScreen->hide();
            });
            
            connect(pinLockScreen, &PinLockScreen::pinRejected, this, [this]() {
                // The PIN lock screen will display its own error message,
                // so we don't need to show another one here
            });
            
            connect(pinLockScreen, &PinLockScreen::backButtonClicked, pinLockScreen, &PinLockScreen::hide);
        }
        
        // Show the PIN lock screen as a modal dialog
        pinLockScreen->setWindowModality(Qt::ApplicationModal);
        pinLockScreen->setWindowTitle("PIN Lock");
        pinLockScreen->setMinimumSize(320, 480);
        pinLockScreen->show();
    }
}

void TestPanel::updateProfileComboBox()
{
    if (pumpController) {
        profileSelectCombo->clear();
        
        QVector<Profile> profiles = pumpController->getAllProfiles();
        for (const auto &profile : profiles) {
            profileSelectCombo->addItem(profile.name);
        }
        
        // Select the active profile
        QString activeProfile = pumpController->getActiveProfileName();
        int index = profileSelectCombo->findText(activeProfile);
        if (index >= 0) {
            profileSelectCombo->setCurrentIndex(index);
        }
    }
}
