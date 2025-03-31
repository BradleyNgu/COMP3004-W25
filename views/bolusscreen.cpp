#include "bolusscreen.h"
#include "ui_bolusscreen.h"
#include <QMessageBox>
#include <QStyleOption>
#include <QPainter>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>

BolusScreen::BolusScreen(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::BolusScreen),
    currentGlucose(0.0),
    currentBasalRate(0.0),
    carbRatio(10.0),         // Default: 10g carbs per 1U insulin
    correctionFactor(2.0),   // Default: 1U insulin reduces glucose by 2 mmol/L
    targetGlucose(5.5)       // Default target glucose: 5.5 mmol/L
{
    ui->setupUi(this);
    
    // Setup custom UI with layouts
    setupUi();
    
    // Set initial values
    extendedCheckBox->setChecked(false);
    updateExtendedBolusVisibility();
    
    // Connect signals to slots
    connectSignals();
}

BolusScreen::~BolusScreen()
{
    delete ui;
}

void BolusScreen::setupUi()
{
    // Set dark background
    setStyleSheet("background-color: #222222;");
    
    // Main layout for entire screen
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(16, 16, 16, 16);
    mainLayout->setSpacing(12);
    
    // Create main title
    titleLabel = new QLabel("Bolus Calculator");
    titleLabel->setStyleSheet("color: white; font-size: 24px; font-weight: bold;");
    titleLabel->setAlignment(Qt::AlignCenter);
    
    // Create form layout for input fields
    QFormLayout *formLayout = new QFormLayout();
    formLayout->setSpacing(16);
    formLayout->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
    
    // Glucose entry field
    QLabel *glucoseLabel = new QLabel("Current Glucose:");
    glucoseLabel->setStyleSheet("color: white; font-size: 16px;");
    
    glucoseSpinBox = new QDoubleSpinBox();
    glucoseSpinBox->setRange(1.0, 30.0);
    glucoseSpinBox->setSingleStep(0.1);
    glucoseSpinBox->setValue(5.5);
    glucoseSpinBox->setSuffix(" mmol/L");
    glucoseSpinBox->setStyleSheet(
        "QDoubleSpinBox { background-color: #333333; color: white; border: 1px solid #666666; border-radius: 5px; padding: 4px; }"
        "QDoubleSpinBox::up-button, QDoubleSpinBox::down-button { width: 20px; }"
    );
    formLayout->addRow(glucoseLabel, glucoseSpinBox);
    
    // Carbs entry field
    QLabel *carbsLabel = new QLabel("Carbohydrates:");
    carbsLabel->setStyleSheet("color: white; font-size: 16px;");
    
    carbsSpinBox = new QDoubleSpinBox();
    carbsSpinBox->setRange(0.0, 200.0);
    carbsSpinBox->setSingleStep(1.0);
    carbsSpinBox->setValue(0.0);
    carbsSpinBox->setSuffix(" g");
    carbsSpinBox->setStyleSheet(
        "QDoubleSpinBox { background-color: #333333; color: white; border: 1px solid #666666; border-radius: 5px; padding: 4px; }"
        "QDoubleSpinBox::up-button, QDoubleSpinBox::down-button { width: 20px; }"
    );
    formLayout->addRow(carbsLabel, carbsSpinBox);
    
    // Suggested bolus display
    QLabel *suggestedLabel = new QLabel("Suggested Bolus:");
    suggestedLabel->setStyleSheet("color: white; font-size: 16px;");
    
    suggestedBolusValueLabel = new QLabel("0.0 u");
    suggestedBolusValueLabel->setStyleSheet("color: #00B2FF; font-size: 18px; font-weight: bold;");
    formLayout->addRow(suggestedLabel, suggestedBolusValueLabel);
    
    // Bolus amount entry
    QLabel *bolusLabel = new QLabel("Bolus Amount:");
    bolusLabel->setStyleSheet("color: white; font-size: 16px;");
    
    bolusSpinBox = new QDoubleSpinBox();
    bolusSpinBox->setRange(0.0, 25.0);
    bolusSpinBox->setSingleStep(0.1);
    bolusSpinBox->setValue(0.0);
    bolusSpinBox->setSuffix(" u");
    bolusSpinBox->setStyleSheet(
        "QDoubleSpinBox { background-color: #333333; color: white; border: 1px solid #666666; border-radius: 5px; padding: 4px; }"
        "QDoubleSpinBox::up-button, QDoubleSpinBox::down-button { width: 20px; }"
    );
    formLayout->addRow(bolusLabel, bolusSpinBox);
    
    // Extended bolus checkbox
    extendedCheckBox = new QCheckBox("Extended Bolus");
    extendedCheckBox->setStyleSheet("color: white; font-size: 16px;");
    
    // Extended bolus duration (initially hidden)
    QHBoxLayout *durationLayout = new QHBoxLayout();
    
    QLabel *durationLabel = new QLabel("Duration:");
    durationLabel->setStyleSheet("color: white; font-size: 16px;");
    
    durationSpinBox = new QSpinBox();
    durationSpinBox->setRange(30, 480);
    durationSpinBox->setSingleStep(15);
    durationSpinBox->setValue(120);
    durationSpinBox->setSuffix(" min");
    durationSpinBox->setStyleSheet(
        "QSpinBox { background-color: #333333; color: white; border: 1px solid #666666; border-radius: 5px; padding: 4px; }"
        "QSpinBox::up-button, QSpinBox::down-button { width: 20px; }"
    );
    
    durationLayout->addWidget(durationLabel);
    durationLayout->addWidget(durationSpinBox);
    
    // Container for extended bolus options
    extendedOptionsWidget = new QWidget();
    extendedOptionsWidget->setLayout(durationLayout);
    
    // Add form layout to main layout
    mainLayout->addWidget(titleLabel);
    mainLayout->addLayout(formLayout);
    mainLayout->addWidget(extendedCheckBox);
    mainLayout->addWidget(extendedOptionsWidget);
    
    // Add stretch to push buttons to bottom
    mainLayout->addStretch(1);
    
    // Create deliver bolus button
    deliverButton = new QPushButton("DELIVER BOLUS");
    deliverButton->setStyleSheet(
        "QPushButton { background-color: #00B2FF; color: white; border-radius: 8px; font-size: 18px; font-weight: bold; padding: 12px; }"
        "QPushButton:pressed { background-color: #0080FF; }"
    );
    deliverButton->setMinimumHeight(50);
    mainLayout->addWidget(deliverButton);
    
    // Create back button
    backButton = new QPushButton("Back");
    backButton->setStyleSheet(
        "QPushButton { background-color: #444444; color: white; border-radius: 5px; padding: 8px; }"
        "QPushButton:pressed { background-color: #666666; }"
    );
    backButton->setFixedWidth(80);
    
    // Add back button in its own layout
    QHBoxLayout *backButtonLayout = new QHBoxLayout();
    backButtonLayout->addWidget(backButton);
    backButtonLayout->addStretch();
    mainLayout->addLayout(backButtonLayout);
    
    // Set up responsive layout
    this->setLayout(mainLayout);
}

void BolusScreen::connectSignals()
{
    // Connect spinbox value changes
    connect(glucoseSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), 
            this, &BolusScreen::on_glucoseSpinBox_valueChanged);
    connect(carbsSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), 
            this, &BolusScreen::on_carbsSpinBox_valueChanged);
    connect(bolusSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), 
            this, &BolusScreen::on_bolusSpinBox_valueChanged);
    
    // Connect checkbox state change
    connect(extendedCheckBox, &QCheckBox::stateChanged, 
            this, &BolusScreen::on_extendedCheckBox_stateChanged);
    
    // Connect duration spinbox
    connect(durationSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), 
            this, &BolusScreen::on_durationSpinBox_valueChanged);
    
    // Connect buttons
    connect(deliverButton, &QPushButton::clicked, 
            this, &BolusScreen::on_deliverButton_clicked);
    connect(backButton, &QPushButton::clicked, 
            this, &BolusScreen::on_backButton_clicked);
}

void BolusScreen::updateExtendedBolusVisibility()
{
    extendedOptionsWidget->setVisible(extendedCheckBox->isChecked());
}

void BolusScreen::updateCurrentValues(PumpController *controller)
{
    if (!controller) return;
    
    // Get current glucose from controller
    currentGlucose = controller->getCurrentGlucose();
    glucoseSpinBox->setValue(currentGlucose);
    
    // Get current basal rate
    currentBasalRate = controller->getCurrentBasalRate();
    
    // Get profile settings from controller
    Profile activeProfile = controller->getActiveProfile();
    carbRatio = activeProfile.carbRatio;
    correctionFactor = activeProfile.correctionFactor;
    targetGlucose = activeProfile.targetGlucose;
    
    // Reset carbs and bolus
    carbsSpinBox->setValue(0.0);
    bolusSpinBox->setValue(0.0);
    
    // Calculate initial suggested bolus
    calculateSuggestedBolus();
}

void BolusScreen::calculateSuggestedBolus()
{
    // Get carb bolus
    double carbBolus = calculateCarbBolus(carbsSpinBox->value());
    
    // Get correction bolus
    double correctionBolus = calculateCorrectionBolus(glucoseSpinBox->value());
    
    // Total suggested bolus
    double suggestedBolus = carbBolus + correctionBolus;
    
    // Ensure non-negative bolus
    if (suggestedBolus < 0.0) {
        suggestedBolus = 0.0;
    }
    
    // Update UI
    suggestedBolusValueLabel->setText(QString::number(suggestedBolus, 'f', 1) + " u");
    
    // Set the suggested value in the bolus spin box
    if (bolusSpinBox->value() == 0.0 || 
        bolusSpinBox->value() == suggestedBolusValueLabel->text().split(" ").first().toDouble()) {
        bolusSpinBox->setValue(suggestedBolus);
    }
}

double BolusScreen::calculateCarbBolus(double carbs)
{
    // Calculate insulin needed for carbs
    if (carbRatio <= 0.0) return 0.0;
    return carbs / carbRatio;
}

double BolusScreen::calculateCorrectionBolus(double glucose)
{
    // Calculate insulin needed to reach target glucose
    if (glucose <= targetGlucose || correctionFactor <= 0.0) {
        return 0.0; // No correction needed if below target
    }
    
    return (glucose - targetGlucose) / correctionFactor;
}

bool BolusScreen::validateBolusSettings()
{
    // Check if bolus amount is valid
    double bolus = bolusSpinBox->value();
    if (bolus <= 0.0) {
        QMessageBox::warning(this, "Invalid Bolus", "Bolus amount must be greater than 0.");
        return false;
    }
    
    // Check if bolus is within safe limits
    if (bolus > 25.0) {
        QMessageBox::warning(this, "Unsafe Bolus", "Bolus amount exceeds the maximum safe limit of 25 units.");
        return false;
    }
    
    // Check if bolus is significantly higher than suggested
    double suggestedBolus = suggestedBolusValueLabel->text().split(" ").first().toDouble();
    if (bolus > suggestedBolus * 2.0 && bolus > suggestedBolus + 5.0) {
        QMessageBox::StandardButton reply = QMessageBox::question(
            this, 
            "Bolus Confirmation", 
            "The bolus amount is significantly higher than suggested. Are you sure you want to proceed?",
            QMessageBox::Yes | QMessageBox::No
        );
        
        if (reply == QMessageBox::No) {
            return false;
        }
    }
    
    // For extended bolus, check duration
    if (extendedCheckBox->isChecked()) {
        int duration = durationSpinBox->value();
        if (duration < 30) {
            QMessageBox::warning(this, "Invalid Duration", "Extended bolus duration must be at least 30 minutes.");
            return false;
        }
    }
    
    return true;
}

void BolusScreen::on_extendedCheckBox_stateChanged(int state)
{
    // Show or hide duration controls based on extended bolus checkbox
    Q_UNUSED(state);
    updateExtendedBolusVisibility();
}

void BolusScreen::on_backButton_clicked()
{
    emit backButtonClicked();
}

void BolusScreen::on_deliverButton_clicked()
{
    if (!validateBolusSettings()) {
        return;
    }
    
    double bolusAmount = bolusSpinBox->value();
    bool extended = extendedCheckBox->isChecked();
    int duration = extended ? durationSpinBox->value() : 0;
    
    // Confirm bolus delivery
    QMessageBox::StandardButton reply = QMessageBox::question(
        this, 
        "Bolus Confirmation", 
        QString("Deliver %1 units%2?")
            .arg(bolusAmount, 0, 'f', 1)
            .arg(extended ? QString(" over %1 minutes").arg(duration) : ""),
        QMessageBox::Yes | QMessageBox::No
    );
    
    if (reply == QMessageBox::Yes) {
        emit bolusRequested(bolusAmount, extended, duration);
        emit backButtonClicked(); // Return to home screen after delivery
    }
}

void BolusScreen::on_glucoseSpinBox_valueChanged(double value)
{
    Q_UNUSED(value);
    calculateSuggestedBolus();
}

void BolusScreen::on_carbsSpinBox_valueChanged(double value)
{
    Q_UNUSED(value);
    calculateSuggestedBolus();
}

void BolusScreen::on_bolusSpinBox_valueChanged(double value)
{
    Q_UNUSED(value);
    // Nothing special to do here, just track the value
}

void BolusScreen::on_durationSpinBox_valueChanged(int value)
{
    Q_UNUSED(value);
    // Nothing special to do here, just track the value
}

void BolusScreen::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(value);
    // Draw the widget background and style
    QStyleOption opt;
    opt.init(this);
    QPainter painter(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &painter, this);
}
