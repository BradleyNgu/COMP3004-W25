#include "controliqscreen.h"
#include "ui_controliqscreen.h"
#include <QStyleOption>
#include <QPainter>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QCheckBox>
#include <QComboBox>
#include <QSlider>
#include <QMessageBox>
#include <QGroupBox>

ControlIQScreen::ControlIQScreen(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ControlIQScreen),
    pumpController(nullptr),
    algorithm(nullptr)
{
    ui->setupUi(this);
    
    // Setup custom UI with layouts
    setupUi();
    
    // Connect signals to slots
    connectSignals();
}

ControlIQScreen::~ControlIQScreen()
{
    delete ui;
}

void ControlIQScreen::setPumpController(PumpController *controller)
{
    pumpController = controller;
    
    // Update UI with current Control-IQ settings
    if (pumpController) {
        updateUIFromSettings();
    }
}

void ControlIQScreen::setControlIQAlgorithm(ControlIQAlgorithm *controlIQAlgorithm)
{
    algorithm = controlIQAlgorithm;
    
    // Update UI with current Control-IQ settings
    if (algorithm) {
        updateUIFromSettings();
    }
}

void ControlIQScreen::setupUi()
{
    // Set dark background
    setStyleSheet("background-color: #222222;");
    
    // Main layout for entire screen
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(16, 16, 16, 16);
    mainLayout->setSpacing(12);
    
    // Title
    titleLabel = new QLabel("Control-IQ Settings");
    titleLabel->setStyleSheet("color: white; font-size: 24px; font-weight: bold;");
    titleLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(titleLabel);
    
    // Status group box
    QGroupBox *statusGroup = new QGroupBox("Control-IQ Status");
    statusGroup->setStyleSheet("QGroupBox { color: white; font-weight: bold; border: 1px solid #444444; border-radius: 5px; padding: 10px; }");
    
    QVBoxLayout *statusLayout = new QVBoxLayout(statusGroup);
    statusLayout->setContentsMargins(8, 16, 8, 8);
    
    // Enable Control-IQ checkbox
    enableControlIQCheckBox = new QCheckBox("Enable Control-IQ Technology");
    enableControlIQCheckBox->setStyleSheet("QCheckBox { color: white; font-size: 16px; }");
    enableControlIQCheckBox->setChecked(true);
    
    // Status label
    statusLabel = new QLabel("Status: Active");
    statusLabel->setStyleSheet("color: #4CD964; font-size: 16px; font-weight: bold;");
    
    statusLayout->addWidget(enableControlIQCheckBox);
    statusLayout->addWidget(statusLabel);
    
    mainLayout->addWidget(statusGroup);
    
    // Activity Settings
    QGroupBox *activityGroup = new QGroupBox("Activity Settings");
    activityGroup->setStyleSheet("QGroupBox { color: white; font-weight: bold; border: 1px solid #444444; border-radius: 5px; padding: 10px; }");
    
    QVBoxLayout *activityLayout = new QVBoxLayout(activityGroup);
    activityLayout->setContentsMargins(8, 16, 8, 8);
    
    // Sleep Activity
    sleepModeCheckBox = new QCheckBox("Sleep Activity");
    sleepModeCheckBox->setStyleSheet("QCheckBox { color: white; font-size: 16px; }");
    
    // Sleep Schedule
    QHBoxLayout *sleepScheduleLayout = new QHBoxLayout();
    sleepScheduleLayout->setContentsMargins(20, 0, 0, 0);
    
    QLabel *fromLabel = new QLabel("From:");
    fromLabel->setStyleSheet("color: white;");
    
    sleepStartTime = new QTimeEdit();
    sleepStartTime->setDisplayFormat("hh:mm AP");
    sleepStartTime->setTime(QTime(22, 0)); // 10:00 PM
    sleepStartTime->setStyleSheet(
        "QTimeEdit { background-color: #333333; color: white; border: 1px solid #666666; border-radius: 3px; padding: 4px; }"
        "QTimeEdit::up-button, QTimeEdit::down-button { width: 16px; }"
    );
    
    QLabel *toLabel = new QLabel("To:");
    toLabel->setStyleSheet("color: white;");
    
    sleepEndTime = new QTimeEdit();
    sleepEndTime->setDisplayFormat("hh:mm AP");
    sleepEndTime->setTime(QTime(7, 0)); // 7:00 AM
    sleepEndTime->setStyleSheet(
        "QTimeEdit { background-color: #333333; color: white; border: 1px solid #666666; border-radius: 3px; padding: 4px; }"
        "QTimeEdit::up-button, QTimeEdit::down-button { width: 16px; }"
    );
    
    sleepScheduleLayout->addWidget(fromLabel);
    sleepScheduleLayout->addWidget(sleepStartTime);
    sleepScheduleLayout->addWidget(toLabel);
    sleepScheduleLayout->addWidget(sleepEndTime);
    sleepScheduleLayout->addStretch(1);
    
    // Exercise Activity
    exerciseModeCheckBox = new QCheckBox("Exercise Activity");
    exerciseModeCheckBox->setStyleSheet("QCheckBox { color: white; font-size: 16px; }");
    
    // Exercise Duration
    QHBoxLayout *exerciseDurationLayout = new QHBoxLayout();
    exerciseDurationLayout->setContentsMargins(20, 0, 0, 0);
    
    QLabel *durationLabel = new QLabel("Duration:");
    durationLabel->setStyleSheet("color: white;");
    
    durationComboBox = new QComboBox();
    durationComboBox->addItems({"1 hour", "2 hours", "3 hours", "4 hours", "Indefinite"});
    durationComboBox->setStyleSheet(
        "QComboBox { background-color: #333333; color: white; border: 1px solid #666666; border-radius: 3px; padding: 4px; }"
        "QComboBox::drop-down { border: 0px; width: 20px; }"
        "QComboBox QAbstractItemView { background-color: #333333; color: white; selection-background-color: #00B2FF; }"
    );
    
    exerciseDurationLayout->addWidget(durationLabel);
    exerciseDurationLayout->addWidget(durationComboBox);
    exerciseDurationLayout->addStretch(1);
    
    activityLayout->addWidget(sleepModeCheckBox);
    activityLayout->addLayout(sleepScheduleLayout);
    activityLayout->addWidget(exerciseModeCheckBox);
    activityLayout->addLayout(exerciseDurationLayout);
    
    mainLayout->addWidget(activityGroup);
    
    // Target Range Settings
    QGroupBox *targetsGroup = new QGroupBox("Target Range Settings");
    targetsGroup->setStyleSheet("QGroupBox { color: white; font-weight: bold; border: 1px solid #444444; border-radius: 5px; padding: 10px; }");
    
    QFormLayout *targetsLayout = new QFormLayout(targetsGroup);
    targetsLayout->setContentsMargins(8, 16, 8, 8);
    targetsLayout->setVerticalSpacing(12);
    targetsLayout->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
    
    // Target glucose range
    QLabel *targetRangeLabel = new QLabel("Target Glucose Range:");
    targetRangeLabel->setStyleSheet("color: white;");
    
    QHBoxLayout *rangeLayout = new QHBoxLayout();
    
    targetLowSpinBox = new QDoubleSpinBox();
    targetLowSpinBox->setRange(3.9, 8.3);
    targetLowSpinBox->setSingleStep(0.1);
    targetLowSpinBox->setValue(5.5);
    targetLowSpinBox->setSuffix(" mmol/L");
    targetLowSpinBox->setStyleSheet(
        "QDoubleSpinBox { background-color: #333333; color: white; border: 1px solid #666666; border-radius: 3px; padding: 4px; }"
        "QDoubleSpinBox::up-button, QDoubleSpinBox::down-button { width: 16px; }"
    );
    
    QLabel *toRangeLabel = new QLabel("to");
    toRangeLabel->setStyleSheet("color: white;");
    
    targetHighSpinBox = new QDoubleSpinBox();
    targetHighSpinBox->setRange(6.1, 10.0);
    targetHighSpinBox->setSingleStep(0.1);
    targetHighSpinBox->setValue(6.5);
    targetHighSpinBox->setSuffix(" mmol/L");
    targetHighSpinBox->setStyleSheet(
        "QDoubleSpinBox { background-color: #333333; color: white; border: 1px solid #666666; border-radius: 3px; padding: 4px; }"
        "QDoubleSpinBox::up-button, QDoubleSpinBox::down-button { width: 16px; }"
    );
    
    rangeLayout->addWidget(targetLowSpinBox);
    rangeLayout->addWidget(toRangeLabel);
    rangeLayout->addWidget(targetHighSpinBox);
    
    targetsLayout->addRow(targetRangeLabel, rangeLayout);
    
    // Max insulin adjustment
    QLabel *maxAdjustmentLabel = new QLabel("Maximum Basal Rate:");
    maxAdjustmentLabel->setStyleSheet("color: white;");
    
    maxBasalRateSpinBox = new QDoubleSpinBox();
    maxBasalRateSpinBox->setRange(1.0, 5.0);
    maxBasalRateSpinBox->setSingleStep(0.1);
    maxBasalRateSpinBox->setValue(3.0);
    maxBasalRateSpinBox->setSuffix(" u/hr");
    maxBasalRateSpinBox->setStyleSheet(
        "QDoubleSpinBox { background-color: #333333; color: white; border: 1px solid #666666; border-radius: 3px; padding: 4px; }"
        "QDoubleSpinBox::up-button, QDoubleSpinBox::down-button { width: 16px; }"
    );
    
    targetsLayout->addRow(maxAdjustmentLabel, maxBasalRateSpinBox);
    
    // Algorithm aggressiveness
    QLabel *aggressivenessLabel = new QLabel("Algorithm Aggressiveness:");
    aggressivenessLabel->setStyleSheet("color: white;");
    
    QHBoxLayout *aggressivenessLayout = new QHBoxLayout();
    
    aggressivenessSlider = new QSlider(Qt::Horizontal);
    aggressivenessSlider->setRange(50, 200);
    aggressivenessSlider->setValue(100);
    aggressivenessSlider->setTickPosition(QSlider::TicksBelow);
    aggressivenessSlider->setTickInterval(25);
    aggressivenessSlider->setStyleSheet(
        "QSlider::groove:horizontal { height: 8px; background: #444444; margin: 2px 0; }"
        "QSlider::handle:horizontal { background: #00B2FF; border: 1px solid #00B2FF; width: 18px; margin: -6px 0; border-radius: 9px; }"
        "QSlider::add-page:horizontal { background: #444444; }"
        "QSlider::sub-page:horizontal { background: #00B2FF; }"
    );
    
    aggressivenessValueLabel = new QLabel("1.0x");
    aggressivenessValueLabel->setStyleSheet("color: white;");
    
    aggressivenessLayout->addWidget(aggressivenessSlider);
    aggressivenessLayout->addWidget(aggressivenessValueLabel);
    
    targetsLayout->addRow(aggressivenessLabel, aggressivenessLayout);
    
    // Hypo prevention
    QLabel *hypoPreventionLabel = new QLabel("Hypo Prevention:");
    hypoPreventionLabel->setStyleSheet("color: white;");
    
    hypoPreventionCheckBox = new QCheckBox("Enabled");
    hypoPreventionCheckBox->setStyleSheet("QCheckBox { color: white; }");
    hypoPreventionCheckBox->setChecked(true);
    
    targetsLayout->addRow(hypoPreventionLabel, hypoPreventionCheckBox);
    
    mainLayout->addWidget(targetsGroup);
    
    // Add stretch to push buttons to bottom
    mainLayout->addStretch(1);
    
    // Buttons
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    
    // Save Settings button
    saveButton = new QPushButton("Save Settings");
    saveButton->setStyleSheet(
        "QPushButton { background-color: #00B2FF; color: white; font-weight: bold; border-radius: 5px; padding: 10px 20px; }"
        "QPushButton:pressed { background-color: #0080FF; }"
    );
    
    // Back button
    backButton = new QPushButton("Back");
    backButton->setStyleSheet(
        "QPushButton { background-color: #444444; color: white; border-radius: 5px; padding: 10px 20px; }"
        "QPushButton:pressed { background-color: #666666; }"
    );
    
    buttonLayout->addWidget(backButton);
    buttonLayout->addStretch();
    buttonLayout->addWidget(saveButton);
    
    mainLayout->addLayout(buttonLayout);
    
    // Connect signals for immediate UI feedback
    connect(enableControlIQCheckBox, &QCheckBox::toggled, this, &ControlIQScreen::onEnableControlIQToggled);
    connect(sleepModeCheckBox, &QCheckBox::toggled, this, [this](bool checked) {
        sleepStartTime->setEnabled(checked);
        sleepEndTime->setEnabled(checked);
    });
    connect(exerciseModeCheckBox, &QCheckBox::toggled, this, [this](bool checked) {
        durationComboBox->setEnabled(checked);
    });
    connect(aggressivenessSlider, &QSlider::valueChanged, this, [this](int value) {
        double factor = value / 100.0;
        aggressivenessValueLabel->setText(QString::number(factor, 'f', 1) + "x");
    });
    
    // Initial state
    sleepStartTime->setEnabled(sleepModeCheckBox->isChecked());
    sleepEndTime->setEnabled(sleepModeCheckBox->isChecked());
    durationComboBox->setEnabled(exerciseModeCheckBox->isChecked());
}

void ControlIQScreen::connectSignals()
{
    connect(saveButton, &QPushButton::clicked, this, &ControlIQScreen::saveSettings);
    connect(backButton, &QPushButton::clicked, this, &ControlIQScreen::backButtonClicked);
}

void ControlIQScreen::updateUIFromSettings()
{
    if (!algorithm) return;
    
    // Enable checkbox
    bool controlIQEnabled = pumpController ? pumpController->isControlIQEnabled() : false;
    enableControlIQCheckBox->setChecked(controlIQEnabled);
    onEnableControlIQToggled(controlIQEnabled);
    
    // Target range
    // Note: In a real implementation, we would get these values from the algorithm
    targetLowSpinBox->setValue(5.5);
    targetHighSpinBox->setValue(6.5);
    
    // Max basal rate
    maxBasalRateSpinBox->setValue(3.0);
    
    // Aggressiveness (convert from 0.5-2.0 range to slider 50-200 range)
    double aggressiveness = 1.0; // Default
    int sliderValue = qRound(aggressiveness * 100.0);
    aggressivenessSlider->setValue(sliderValue);
    
    // Activity settings
    sleepModeCheckBox->setChecked(algorithm->isSleepModeActive());
    exerciseModeCheckBox->setChecked(algorithm->isExerciseModeActive());
    
    // Hypo prevention
    hypoPreventionCheckBox->setChecked(algorithm->isHypoPreventionActive());
}

void ControlIQScreen::onEnableControlIQToggled(bool checked)
{
    // Update status label
    if (checked) {
        statusLabel->setText("Status: Active");
        statusLabel->setStyleSheet("color: #4CD964; font-size: 16px; font-weight: bold;");
    } else {
        statusLabel->setText("Status: Inactive");
        statusLabel->setStyleSheet("color: #FF3B30; font-size: 16px; font-weight: bold;");
    }
    
    // Enable/disable other settings
    sleepModeCheckBox->setEnabled(checked);
    sleepStartTime->setEnabled(checked && sleepModeCheckBox->isChecked());
    sleepEndTime->setEnabled(checked && sleepModeCheckBox->isChecked());
    exerciseModeCheckBox->setEnabled(checked);
    durationComboBox->setEnabled(checked && exerciseModeCheckBox->isChecked());
    targetLowSpinBox->setEnabled(checked);
    targetHighSpinBox->setEnabled(checked);
    maxBasalRateSpinBox->setEnabled(checked);
    aggressivenessSlider->setEnabled(checked);
    hypoPreventionCheckBox->setEnabled(checked);
}

void ControlIQScreen::saveSettings()
{
    if (!pumpController || !algorithm) return;
    
    // Enable/disable Control-IQ
    pumpController->enableControlIQ(enableControlIQCheckBox->isChecked());
    
    // Set target range
    algorithm->setTargetRange(targetLowSpinBox->value(), targetHighSpinBox->value());
    
    // Set max basal rate
    algorithm->setMaxBasalRate(maxBasalRateSpinBox->value());
    
    // Set aggressiveness (convert slider 50-200 range to 0.5-2.0 range)
    double aggressiveness = aggressivenessSlider->value() / 100.0;
    algorithm->setAggressiveness(aggressiveness);
    
    // Set activity settings
    algorithm->setSleepSetting(sleepModeCheckBox->isChecked());
    algorithm->setExerciseSetting(exerciseModeCheckBox->isChecked());
    
    // Set hypo prevention
    algorithm->setHypoPrevention(hypoPreventionCheckBox->isChecked());
    
    // Show success message
    QMessageBox::information(this, "Settings Saved", "Control-IQ settings have been saved successfully.");
}

void ControlIQScreen::paintEvent(QPaintEvent * /* event */)
{
    // Draw the widget background and style
    QStyleOption opt;
    opt.init(this);
    QPainter painter(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &painter, this);
}
