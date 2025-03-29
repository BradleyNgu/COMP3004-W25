#include "alertsscreen.h"
#include "ui_alertsscreen.h"
#include <QStyleOption>
#include <QPainter>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QScrollArea>
#include <QMessageBox>
#include <QSettings>
#include <QDateTime>
#include <QDir>

AlertsScreen::AlertsScreen(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::AlertsScreen),
    alertController(nullptr),
    historyScreen(nullptr),
    dataStorage(nullptr)
{
    ui->setupUi(this);
    
    // Setup custom UI with layouts
    setupUi();
    
    // Connect signals to slots
    connectSignals();
    
    // Load any saved settings
    loadSettings();
}

AlertsScreen::~AlertsScreen()
{
    delete ui;
}

void AlertsScreen::setAlertController(AlertController *controller)
{
    alertController = controller;
    
    if (alertController) {
        // Update UI with current settings
        enableAlertsCheckBox->setChecked(alertController->areAlertsEnabled());
        
        // Connect controller signals
        connect(alertController, &AlertController::alertAdded, this, &AlertsScreen::updateActiveAlerts);
        connect(alertController, &AlertController::alertAcknowledged, this, &AlertsScreen::updateActiveAlerts);
        connect(alertController, &AlertController::allAlertsAcknowledged, this, &AlertsScreen::updateActiveAlerts);
        
        // Connect signals for alert history
        connect(alertController, &AlertController::alertAdded, this, &AlertsScreen::updateAlertHistory);
        connect(alertController, &AlertController::alertAcknowledged, this, &AlertsScreen::updateAlertHistory);
        connect(alertController, &AlertController::allAlertsAcknowledged, this, &AlertsScreen::updateAlertHistory);
        
        // Update alerts list
        updateActiveAlerts();
    }
}

void AlertsScreen::setHistoryScreen(HistoryScreen *screen)
{
    historyScreen = screen;
}

void AlertsScreen::setDataStorage(DataStorage *storage)
{
    dataStorage = storage;
}

void AlertsScreen::setupUi()
{
    // Set dark background
    setStyleSheet("background-color: #222222;");
    
    // Main layout for entire screen
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(16, 16, 16, 16);
    mainLayout->setSpacing(12);
    
    // Title
    titleLabel = new QLabel("Alerts & Reminders");
    titleLabel->setStyleSheet("color: white; font-size: 24px; font-weight: bold;");
    titleLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(titleLabel);
    
    // Tab widget for different sections
    tabWidget = new QTabWidget();
    tabWidget->setStyleSheet(
        "QTabWidget::pane { border: 1px solid #444444; background-color: #333333; }"
        "QTabBar::tab { background-color: #222222; color: white; padding: 8px 16px; margin-right: 2px; }"
        "QTabBar::tab:selected { background-color: #333333; border-bottom: 2px solid #00B2FF; }"
        "QTabBar::tab:hover { background-color: #2a2a2a; }"
    );
    
    // *** SETTINGS TAB ***
    settingsTab = new QWidget();
    QVBoxLayout *settingsLayout = new QVBoxLayout(settingsTab);
    settingsLayout->setContentsMargins(12, 12, 12, 12);
    settingsLayout->setSpacing(16);
    
    // Enable/disable all alerts
    enableAlertsCheckBox = new QCheckBox("Enable All Alerts and Reminders");
    enableAlertsCheckBox->setStyleSheet("color: white; font-size: 16px;");
    enableAlertsCheckBox->setChecked(true);
    settingsLayout->addWidget(enableAlertsCheckBox);
    
    // Glucose thresholds
    QGroupBox *glucoseGroup = new QGroupBox("Glucose Alert Thresholds");
    glucoseGroup->setStyleSheet("QGroupBox { color: white; font-weight: bold; border: 1px solid #444444; border-radius: 5px; padding: 10px; }");
    
    QFormLayout *glucoseLayout = new QFormLayout(glucoseGroup);
    glucoseLayout->setLabelAlignment(Qt::AlignLeft);
    glucoseLayout->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
    
    lowGlucoseLabel = new QLabel("Low Glucose:");
    lowGlucoseLabel->setStyleSheet("color: white;");
    
    lowGlucoseSpinBox = new QDoubleSpinBox();
    lowGlucoseSpinBox->setRange(3.0, 5.0);
    lowGlucoseSpinBox->setSingleStep(0.1);
    lowGlucoseSpinBox->setValue(3.9);
    lowGlucoseSpinBox->setSuffix(" mmol/L");
    lowGlucoseSpinBox->setStyleSheet(
        "QDoubleSpinBox { background-color: #444444; color: white; border: 1px solid #666666; border-radius: 3px; padding: 4px; }"
        "QDoubleSpinBox::up-button, QDoubleSpinBox::down-button { width: 16px; }"
    );
    
    urgentLowGlucoseLabel = new QLabel("Urgent Low Glucose:");
    urgentLowGlucoseLabel->setStyleSheet("color: white;");
    
    urgentLowGlucoseSpinBox = new QDoubleSpinBox();
    urgentLowGlucoseSpinBox->setRange(2.2, 3.5);
    urgentLowGlucoseSpinBox->setSingleStep(0.1);
    urgentLowGlucoseSpinBox->setValue(3.1);
    urgentLowGlucoseSpinBox->setSuffix(" mmol/L");
    urgentLowGlucoseSpinBox->setStyleSheet(
        "QDoubleSpinBox { background-color: #444444; color: white; border: 1px solid #666666; border-radius: 3px; padding: 4px; }"
        "QDoubleSpinBox::up-button, QDoubleSpinBox::down-button { width: 16px; }"
    );
    
    highGlucoseLabel = new QLabel("High Glucose:");
    highGlucoseLabel->setStyleSheet("color: white;");
    
    highGlucoseSpinBox = new QDoubleSpinBox();
    highGlucoseSpinBox->setRange(7.0, 15.0);
    highGlucoseSpinBox->setSingleStep(0.1);
    highGlucoseSpinBox->setValue(10.0);
    highGlucoseSpinBox->setSuffix(" mmol/L");
    highGlucoseSpinBox->setStyleSheet(
        "QDoubleSpinBox { background-color: #444444; color: white; border: 1px solid #666666; border-radius: 3px; padding: 4px; }"
        "QDoubleSpinBox::up-button, QDoubleSpinBox::down-button { width: 16px; }"
    );
    
    urgentHighGlucoseLabel = new QLabel("Urgent High Glucose:");
    urgentHighGlucoseLabel->setStyleSheet("color: white;");
    
    urgentHighGlucoseSpinBox = new QDoubleSpinBox();
    urgentHighGlucoseSpinBox->setRange(10.0, 22.0);
    urgentHighGlucoseSpinBox->setSingleStep(0.1);
    urgentHighGlucoseSpinBox->setValue(13.9);
    urgentHighGlucoseSpinBox->setSuffix(" mmol/L");
    urgentHighGlucoseSpinBox->setStyleSheet(
        "QDoubleSpinBox { background-color: #444444; color: white; border: 1px solid #666666; border-radius: 3px; padding: 4px; }"
        "QDoubleSpinBox::up-button, QDoubleSpinBox::down-button { width: 16px; }"
    );
    
    glucoseLayout->addRow(lowGlucoseLabel, lowGlucoseSpinBox);
    glucoseLayout->addRow(urgentLowGlucoseLabel, urgentLowGlucoseSpinBox);
    glucoseLayout->addRow(highGlucoseLabel, highGlucoseSpinBox);
    glucoseLayout->addRow(urgentHighGlucoseLabel, urgentHighGlucoseSpinBox);
    
    settingsLayout->addWidget(glucoseGroup);
    
    // Insulin thresholds
    QGroupBox *insulinGroup = new QGroupBox("Insulin Alert Thresholds");
    insulinGroup->setStyleSheet("QGroupBox { color: white; font-weight: bold; border: 1px solid #444444; border-radius: 5px; padding: 10px; }");
    
    QFormLayout *insulinLayout = new QFormLayout(insulinGroup);
    insulinLayout->setLabelAlignment(Qt::AlignLeft);
    insulinLayout->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
    
    lowInsulinLabel = new QLabel("Low Insulin:");
    lowInsulinLabel->setStyleSheet("color: white;");
    
    lowInsulinSpinBox = new QDoubleSpinBox();
    lowInsulinSpinBox->setRange(20.0, 100.0);
    lowInsulinSpinBox->setSingleStep(5.0);
    lowInsulinSpinBox->setValue(50.0);
    lowInsulinSpinBox->setSuffix(" units");
    lowInsulinSpinBox->setStyleSheet(
        "QDoubleSpinBox { background-color: #444444; color: white; border: 1px solid #666666; border-radius: 3px; padding: 4px; }"
        "QDoubleSpinBox::up-button, QDoubleSpinBox::down-button { width: 16px; }"
    );
    
    criticalInsulinLabel = new QLabel("Critical Low Insulin:");
    criticalInsulinLabel->setStyleSheet("color: white;");
    
    criticalInsulinSpinBox = new QDoubleSpinBox();
    criticalInsulinSpinBox->setRange(5.0, 30.0);
    criticalInsulinSpinBox->setSingleStep(1.0);
    criticalInsulinSpinBox->setValue(10.0);
    criticalInsulinSpinBox->setSuffix(" units");
    criticalInsulinSpinBox->setStyleSheet(
        "QDoubleSpinBox { background-color: #444444; color: white; border: 1px solid #666666; border-radius: 3px; padding: 4px; }"
        "QDoubleSpinBox::up-button, QDoubleSpinBox::down-button { width: 16px; }"
    );
    
    insulinLayout->addRow(lowInsulinLabel, lowInsulinSpinBox);
    insulinLayout->addRow(criticalInsulinLabel, criticalInsulinSpinBox);
    
    settingsLayout->addWidget(insulinGroup);
    
    // Battery thresholds
    QGroupBox *batteryGroup = new QGroupBox("Battery Alert Thresholds");
    batteryGroup->setStyleSheet("QGroupBox { color: white; font-weight: bold; border: 1px solid #444444; border-radius: 5px; padding: 10px; }");
    
    QFormLayout *batteryLayout = new QFormLayout(batteryGroup);
    batteryLayout->setLabelAlignment(Qt::AlignLeft);
    batteryLayout->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
    
    lowBatteryLabel = new QLabel("Low Battery:");
    lowBatteryLabel->setStyleSheet("color: white;");
    
    lowBatterySpinBox = new QSpinBox();
    lowBatterySpinBox->setRange(10, 40);
    lowBatterySpinBox->setSingleStep(5);
    lowBatterySpinBox->setValue(20);
    lowBatterySpinBox->setSuffix(" %");
    lowBatterySpinBox->setStyleSheet(
        "QSpinBox { background-color: #444444; color: white; border: 1px solid #666666; border-radius: 3px; padding: 4px; }"
        "QSpinBox::up-button, QSpinBox::down-button { width: 16px; }"
    );
    
    criticalBatteryLabel = new QLabel("Critical Low Battery:");
    criticalBatteryLabel->setStyleSheet("color: white;");
    
    criticalBatterySpinBox = new QSpinBox();
    criticalBatterySpinBox->setRange(2, 15);
    criticalBatterySpinBox->setSingleStep(1);
    criticalBatterySpinBox->setValue(5);
    criticalBatterySpinBox->setSuffix(" %");
    criticalBatterySpinBox->setStyleSheet(
        "QSpinBox { background-color: #444444; color: white; border: 1px solid #666666; border-radius: 3px; padding: 4px; }"
        "QSpinBox::up-button, QSpinBox::down-button { width: 16px; }"
    );
    
    batteryLayout->addRow(lowBatteryLabel, lowBatterySpinBox);
    batteryLayout->addRow(criticalBatteryLabel, criticalBatterySpinBox);
    
    settingsLayout->addWidget(batteryGroup);
    
    // Save settings button
    saveSettingsButton = new QPushButton("Save Settings");
    saveSettingsButton->setStyleSheet(
        "QPushButton { background-color: #00B2FF; color: white; font-weight: bold; border-radius: 5px; padding: 10px; }"
        "QPushButton:pressed { background-color: #0080FF; }"
    );
    settingsLayout->addWidget(saveSettingsButton);
    
    // Add stretch to push everything to the top
    settingsLayout->addStretch(1);
    
    // *** ACTIVE ALERTS TAB ***
    alertsTab = new QWidget();
    QVBoxLayout *alertsLayout = new QVBoxLayout(alertsTab);
    alertsLayout->setContentsMargins(12, 12, 12, 12);
    alertsLayout->setSpacing(16);
    
    QLabel *activeAlertsTitle = new QLabel("Active Alerts");
    activeAlertsTitle->setStyleSheet("color: white; font-size: 18px; font-weight: bold;");
    alertsLayout->addWidget(activeAlertsTitle);
    
    activeAlertsList = new QListWidget();
    activeAlertsList->setStyleSheet(
        "QListWidget { background-color: #333333; color: white; border: 1px solid #666666; border-radius: 5px; }"
        "QListWidget::item { padding: 8px; border-bottom: 1px solid #555555; }"
        "QListWidget::item:selected { background-color: #00B2FF; }"
    );
    activeAlertsList->setAlternatingRowColors(true);
    activeAlertsList->setSelectionMode(QAbstractItemView::SingleSelection);
    alertsLayout->addWidget(activeAlertsList, 1); // Give list stretch factor of 1
    
    // Alert action buttons
    QHBoxLayout *alertButtonLayout = new QHBoxLayout();
    
    acknowledgeAlertButton = new QPushButton("Acknowledge Selected");
    acknowledgeAlertButton->setStyleSheet(
        "QPushButton { background-color: #00B2FF; color: white; border-radius: 5px; padding: 8px; }"
        "QPushButton:pressed { background-color: #0080FF; }"
    );
    
    clearAllAlertsButton = new QPushButton("Clear All Alerts");
    clearAllAlertsButton->setStyleSheet(
        "QPushButton { background-color: #FF3B30; color: white; border-radius: 5px; padding: 8px; }"
        "QPushButton:pressed { background-color: #CC2F27; }"
    );
    
    alertButtonLayout->addWidget(acknowledgeAlertButton);
    alertButtonLayout->addWidget(clearAllAlertsButton);
    
    alertsLayout->addLayout(alertButtonLayout);
    
    // *** REMINDERS TAB ***
    remindersTab = new QWidget();
    QVBoxLayout *remindersLayout = new QVBoxLayout(remindersTab);
    remindersLayout->setContentsMargins(12, 12, 12, 12);
    remindersLayout->setSpacing(16);
    
    QLabel *remindersTitle = new QLabel("Scheduled Reminders");
    remindersTitle->setStyleSheet("color: white; font-size: 18px; font-weight: bold;");
    remindersLayout->addWidget(remindersTitle);
    
    remindersList = new QListWidget();
    remindersList->setStyleSheet(
        "QListWidget { background-color: #333333; color: white; border: 1px solid #666666; border-radius: 5px; }"
        "QListWidget::item { padding: 8px; border-bottom: 1px solid #555555; }"
        "QListWidget::item:selected { background-color: #00B2FF; }"
    );
    remindersList->setAlternatingRowColors(true);
    remindersList->setSelectionMode(QAbstractItemView::SingleSelection);
    remindersLayout->addWidget(remindersList, 1); // Give list stretch factor of 1
    
    // New reminder form
    QGroupBox *newReminderGroup = new QGroupBox("Create New Reminder");
    newReminderGroup->setStyleSheet("QGroupBox { color: white; font-weight: bold; border: 1px solid #444444; border-radius: 5px; padding: 10px; }");
    
    QFormLayout *reminderFormLayout = new QFormLayout(newReminderGroup);
    reminderFormLayout->setLabelAlignment(Qt::AlignLeft);
    reminderFormLayout->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
    
    reminderTypeLabel = new QLabel("Reminder Type:");
    reminderTypeLabel->setStyleSheet("color: white;");
    
    reminderTypeComboBox = new QComboBox();
    reminderTypeComboBox->addItem("Infusion Set Change");
    reminderTypeComboBox->addItem("CGM Sensor Change");
    reminderTypeComboBox->addItem("Reservoir Change");
    reminderTypeComboBox->addItem("Pump Battery Change");
    reminderTypeComboBox->addItem("Custom Reminder");
    reminderTypeComboBox->setStyleSheet(
        "QComboBox { background-color: #444444; color: white; border: 1px solid #666666; border-radius: 3px; padding: 4px; }"
        "QComboBox::drop-down { border: 0px; width: 20px; }"
        "QComboBox QAbstractItemView { background-color: #444444; color: white; selection-background-color: #00B2FF; }"
    );
    
    reminderTimeLabel = new QLabel("Reminder Time:");
    reminderTimeLabel->setStyleSheet("color: white;");
    
    reminderTimeEdit = new QDateTimeEdit(QDateTime::currentDateTime().addDays(3));
    reminderTimeEdit->setDisplayFormat("yyyy-MM-dd hh:mm AP");
    reminderTimeEdit->setCalendarPopup(true);
    reminderTimeEdit->setStyleSheet(
        "QDateTimeEdit { background-color: #444444; color: white; border: 1px solid #666666; border-radius: 3px; padding: 4px; }"
        "QDateTimeEdit::drop-down { border: 0px; width: 20px; }"
    );
    
    reminderFormLayout->addRow(reminderTypeLabel, reminderTypeComboBox);
    reminderFormLayout->addRow(reminderTimeLabel, reminderTimeEdit);
    
    // Reminder buttons
    QHBoxLayout *reminderButtonLayout = new QHBoxLayout();
    
    setReminderButton = new QPushButton("Set Reminder");
    setReminderButton->setStyleSheet(
        "QPushButton { background-color: #00B2FF; color: white; border-radius: 5px; padding: 8px; }"
        "QPushButton:pressed { background-color: #0080FF; }"
    );
    
    deleteReminderButton = new QPushButton("Delete Selected");
    deleteReminderButton->setStyleSheet(
        "QPushButton { background-color: #FF3B30; color: white; border-radius: 5px; padding: 8px; }"
        "QPushButton:pressed { background-color: #CC2F27; }"
    );
    
    reminderButtonLayout->addWidget(setReminderButton);
    reminderButtonLayout->addWidget(deleteReminderButton);
    
    remindersLayout->addWidget(newReminderGroup);
    remindersLayout->addLayout(reminderButtonLayout);
    
    // *** ALERTS HISTORY TAB ***
    historyTab = new QWidget();
    QVBoxLayout *historyLayout = new QVBoxLayout(historyTab);
    historyLayout->setContentsMargins(12, 12, 12, 12);
    historyLayout->setSpacing(16);
    
    QLabel *historyTitle = new QLabel("Alert History");
    historyTitle->setStyleSheet("color: white; font-size: 18px; font-weight: bold;");
    historyLayout->addWidget(historyTitle);
    
    alertHistoryTable = new QTableWidget();
    alertHistoryTable->setObjectName("alertsTable"); // For finding in updateAlertHistory
    alertHistoryTable->setColumnCount(3);
    alertHistoryTable->setHorizontalHeaderLabels(QStringList() << "Time" << "Alert" << "Level");
    alertHistoryTable->setStyleSheet(
        "QTableWidget { background-color: #333333; color: white; border: 1px solid #666666; border-radius: 5px; }"
        "QHeaderView::section { background-color: #444444; color: white; border: 1px solid #555555; padding: 4px; }"
        "QTableWidget::item { border-bottom: 1px solid #555555; }"
    );
    alertHistoryTable->horizontalHeader()->setStretchLastSection(true);
    alertHistoryTable->verticalHeader()->setVisible(false);
    alertHistoryTable->setAlternatingRowColors(true);
    historyLayout->addWidget(alertHistoryTable, 1);
    
    // Add tabs to tabWidget
    tabWidget->addTab(settingsTab, "Settings");
    tabWidget->addTab(alertsTab, "Active Alerts");
    tabWidget->addTab(remindersTab, "Reminders");
    tabWidget->addTab(historyTab, "Alert History");
    
    mainLayout->addWidget(tabWidget);
    
    // Back button
    QHBoxLayout *backButtonLayout = new QHBoxLayout();
    backButtonLayout->addStretch();
    
    backButton = new QPushButton("Back");
    backButton->setStyleSheet(
        "QPushButton { background-color: #444444; color: white; border-radius: 5px; padding: 8px; }"
        "QPushButton:pressed { background-color: #666666; }"
    );
    backButton->setFixedWidth(80);
    
    backButtonLayout->addWidget(backButton);
    mainLayout->addLayout(backButtonLayout);
}

void AlertsScreen::connectSignals()
{
    // Connect button signals
    connect(backButton, &QPushButton::clicked, this, &AlertsScreen::onBackButtonClicked);
    connect(saveSettingsButton, &QPushButton::clicked, this, &AlertsScreen::onSaveSettingsButtonClicked);
    connect(acknowledgeAlertButton, &QPushButton::clicked, this, &AlertsScreen::onAcknowledgeAlertButtonClicked);
    connect(clearAllAlertsButton, &QPushButton::clicked, this, &AlertsScreen::onClearAllAlertsButtonClicked);
    connect(setReminderButton, &QPushButton::clicked, this, &AlertsScreen::onSetReminderButtonClicked);
    connect(deleteReminderButton, &QPushButton::clicked, this, &AlertsScreen::onDeleteReminderButtonClicked);
    
    // Connect settings changes
    connect(enableAlertsCheckBox, &QCheckBox::toggled, this, &AlertsScreen::onEnableAlertsToggled);
    connect(lowGlucoseSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &AlertsScreen::onLowGlucoseThresholdChanged);
    connect(highGlucoseSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &AlertsScreen::onHighGlucoseThresholdChanged);
    connect(urgentLowGlucoseSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &AlertsScreen::onUrgentLowGlucoseThresholdChanged);
    connect(urgentHighGlucoseSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &AlertsScreen::onUrgentHighGlucoseThresholdChanged);
    connect(lowInsulinSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &AlertsScreen::onLowInsulinThresholdChanged);
    connect(criticalInsulinSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &AlertsScreen::onCriticalInsulinThresholdChanged);
    connect(lowBatterySpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &AlertsScreen::onLowBatteryThresholdChanged);
    connect(criticalBatterySpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &AlertsScreen::onCriticalBatteryThresholdChanged);
}

void AlertsScreen::loadSettings()
{
    // Load from QSettings
    QSettings settings("TandemDiabetes", "tslimx2simulator");
    
    settings.beginGroup("Alerts");
    
    // Load general settings
    bool alertsEnabled = settings.value("AlertsEnabled", true).toBool();
    enableAlertsCheckBox->setChecked(alertsEnabled);
    
    // Load glucose thresholds
    lowGlucoseSpinBox->setValue(settings.value("LowGlucoseThreshold", 3.9).toDouble());
    highGlucoseSpinBox->setValue(settings.value("HighGlucoseThreshold", 10.0).toDouble());
    urgentLowGlucoseSpinBox->setValue(settings.value("UrgentLowGlucoseThreshold", 3.1).toDouble());
    urgentHighGlucoseSpinBox->setValue(settings.value("UrgentHighGlucoseThreshold", 13.9).toDouble());
    
    // Load insulin thresholds
    lowInsulinSpinBox->setValue(settings.value("LowInsulinThreshold", 50.0).toDouble());
    criticalInsulinSpinBox->setValue(settings.value("CriticalInsulinThreshold", 10.0).toDouble());
    
    // Load battery thresholds
    lowBatterySpinBox->setValue(settings.value("LowBatteryThreshold", 20).toInt());
    criticalBatterySpinBox->setValue(settings.value("CriticalBatteryThreshold", 5).toInt());
    
    // Load reminders
    int reminderCount = settings.beginReadArray("Reminders");
    reminders.clear();
    for (int i = 0; i < reminderCount; ++i) {
        settings.setArrayIndex(i);
        Reminder reminder;
        reminder.type = settings.value("Type").toString();
        reminder.time = settings.value("Time").toDateTime();
        reminder.acknowledged = settings.value("Acknowledged", false).toBool();
        
        // Only add future or unacknowledged reminders
        if (reminder.time > QDateTime::currentDateTime() || !reminder.acknowledged) {
            reminders.append(reminder);
        }
    }
    settings.endArray();
    
    settings.endGroup();
    
    // Update the UI
    updateReminders();
    
    // Update alert history
    updateAlertHistory();
    
    // Apply settings to the controller if it exists
    if (alertController) {
        alertController->enableAlerts(alertsEnabled);
        
        alertController->setGlucoseAlertThresholds(
            lowGlucoseSpinBox->value(),
            highGlucoseSpinBox->value(),
            urgentLowGlucoseSpinBox->value(),
            urgentHighGlucoseSpinBox->value()
        );
        
        alertController->setInsulinAlertThresholds(
            lowInsulinSpinBox->value(),
            criticalInsulinSpinBox->value()
        );
        
        alertController->setBatteryAlertThresholds(
            lowBatterySpinBox->value(),
            criticalBatterySpinBox->value()
        );
    }
}

void AlertsScreen::saveSettings()
{
    // Save to QSettings
    QSettings settings("TandemDiabetes", "tslimx2simulator");
    
    settings.beginGroup("Alerts");
    
    // Save general settings
    settings.setValue("AlertsEnabled", enableAlertsCheckBox->isChecked());
    
    // Save glucose thresholds
    settings.setValue("LowGlucoseThreshold", lowGlucoseSpinBox->value());
    settings.setValue("HighGlucoseThreshold", highGlucoseSpinBox->value());
    settings.setValue("UrgentLowGlucoseThreshold", urgentLowGlucoseSpinBox->value());
    settings.setValue("UrgentHighGlucoseThreshold", urgentHighGlucoseSpinBox->value());
    
    // Save insulin thresholds
    settings.setValue("LowInsulinThreshold", lowInsulinSpinBox->value());
    settings.setValue("CriticalInsulinThreshold", criticalInsulinSpinBox->value());
    
    // Save battery thresholds
    settings.setValue("LowBatteryThreshold", lowBatterySpinBox->value());
    settings.setValue("CriticalBatteryThreshold", criticalBatterySpinBox->value());
    
    // Save reminders
    settings.beginWriteArray("Reminders");
    for (int i = 0; i < reminders.size(); ++i) {
        settings.setArrayIndex(i);
        settings.setValue("Type", reminders[i].type);
        settings.setValue("Time", reminders[i].time);
        settings.setValue("Acknowledged", reminders[i].acknowledged);
    }
    settings.endArray();
    
    settings.endGroup();
    
    // Apply settings to controller
    if (alertController) {
        alertController->enableAlerts(enableAlertsCheckBox->isChecked());
        
        alertController->setGlucoseAlertThresholds(
            lowGlucoseSpinBox->value(),
            highGlucoseSpinBox->value(),
            urgentLowGlucoseSpinBox->value(),
            urgentHighGlucoseSpinBox->value()
        );
        
        alertController->setInsulinAlertThresholds(
            lowInsulinSpinBox->value(),
            criticalInsulinSpinBox->value()
        );
        
        alertController->setBatteryAlertThresholds(
            lowBatterySpinBox->value(),
            criticalBatterySpinBox->value()
        );
    }
}

void AlertsScreen::updateActiveAlerts()
{
    activeAlertsList->clear();
    
    if (!alertController) return;
    
    QVector<QPair<QString, PumpModel::AlertLevel>> alerts = alertController->getActiveAlerts();
    
    for (const auto &alert : alerts) {
        QListWidgetItem *item = new QListWidgetItem(alert.first);
        
        // Set icon or color based on alert level
        switch (alert.second) {
            case PumpModel::Critical:
                item->setForeground(QColor(255, 59, 48)); // Red
                item->setIcon(QIcon(":/icons/critical.png"));
                break;
            case PumpModel::Warning:
                item->setForeground(QColor(255, 149, 0)); // Orange
                item->setIcon(QIcon(":/icons/warning.png"));
                break;
            case PumpModel::Info:
                item->setForeground(QColor(0, 122, 255)); // Blue
                item->setIcon(QIcon(":/icons/info.png"));
                break;
        }
        
        activeAlertsList->addItem(item);
    }
    
    // Update button enabled states
    acknowledgeAlertButton->setEnabled(!alerts.isEmpty());
    clearAllAlertsButton->setEnabled(!alerts.isEmpty());
}

void AlertsScreen::updateAlertHistory()
{
    // Clear the alert history table
    alertHistoryTable->setRowCount(0);
    
    if (!alertController || !dataStorage) {
        return;
    }
    
    // Get alert history from DataStorage
    QVector<DataStorage::LogEvent> alertHistory = dataStorage->loadEventLog(QDir::homePath() + "/.tslimx2simulator/event_log.json");
    
    // Add each alert to the table
    for (const auto &alert : alertHistory) {
        int row = alertHistoryTable->rowCount();
        alertHistoryTable->insertRow(row);
        
        QTableWidgetItem* timeItem = new QTableWidgetItem(alert.timestamp.toString("yyyy-MM-dd hh:mm:ss"));
        QTableWidgetItem* messageItem = new QTableWidgetItem(alert.message);
        
        QString levelStr;
        QColor levelColor;
        
        switch (alert.level) {
            case 0: // Info
                levelStr = "Info";
                levelColor = QColor(0, 122, 255); // Blue
                break;
            case 1: // Warning
                levelStr = "Warning";
                levelColor = QColor(255, 149, 0); // Orange
                break;
            case 2: // Error
                levelStr = "Error";
                levelColor = QColor(255, 59, 48); // Red
                break;
            case 3: // Critical
                levelStr = "Critical";
                levelColor = QColor(255, 0, 0); // Bright red
                break;
            default:
                levelStr = "Unknown";
                levelColor = QColor(255, 255, 255); // White
        }
        
        QTableWidgetItem* levelItem = new QTableWidgetItem(levelStr);
        
        // Set colors based on level
        messageItem->setForeground(levelColor);
        levelItem->setForeground(levelColor);
        
        alertHistoryTable->setItem(row, 0, timeItem);
        alertHistoryTable->setItem(row, 1, messageItem);
        alertHistoryTable->setItem(row, 2, levelItem);
    }
    
    // Sort by time (newest first)
    alertHistoryTable->sortItems(0, Qt::DescendingOrder);
    
    // Resize columns to content
    alertHistoryTable->resizeColumnsToContents();
}

void AlertsScreen::updateReminders()
{
    remindersList->clear();
    
    // Sort reminders by time
    std::sort(reminders.begin(), reminders.end(), [](const Reminder &a, const Reminder &b) {
        return a.time < b.time;
    });
    
    // Add reminders to list
    for (const auto &reminder : reminders) {
        QString displayText = reminder.type + " - " + reminder.time.toString("yyyy-MM-dd hh:mm AP");
        
        QListWidgetItem *item = new QListWidgetItem(displayText);
        
        // Style based on whether the reminder is due or acknowledged
        if (reminder.acknowledged) {
            item->setForeground(QColor(128, 128, 128)); // Gray for acknowledged
        } else if (reminder.time <= QDateTime::currentDateTime()) {
            item->setForeground(QColor(255, 59, 48)); // Red for overdue
        } else if (reminder.time.addSecs(-24 * 60 * 60) <= QDateTime::currentDateTime()) {
            item->setForeground(QColor(255, 149, 0)); // Orange for due soon (within 24 hours)
        } else {
            item->setForeground(QColor(0, 122, 255)); // Blue for future
        }
        
        remindersList->addItem(item);
    }
    
    // Update button enabled states
    deleteReminderButton->setEnabled(remindersList->currentRow() >= 0);
}

void AlertsScreen::addReminder(const QString &type, const QDateTime &time)
{
    Reminder reminder;
    reminder.type = type;
    reminder.time = time;
    reminder.acknowledged = false;
    
    reminders.append(reminder);
    updateReminders();
    
    // Check if we need to show an alert for this
    if (time <= QDateTime::currentDateTime() && alertController) {
        alertController->addAlert("Reminder: " + type, PumpModel::Warning);
    }
    
    // Save reminders to settings
    saveSettings();
}

void AlertsScreen::onBackButtonClicked()
{
    emit backButtonClicked();
}

void AlertsScreen::onSaveSettingsButtonClicked()
{
    // Apply settings to controller
    if (alertController) {
        alertController->enableAlerts(enableAlertsCheckBox->isChecked());
        
        alertController->setGlucoseAlertThresholds(
            lowGlucoseSpinBox->value(),
            highGlucoseSpinBox->value(),
            urgentLowGlucoseSpinBox->value(),
            urgentHighGlucoseSpinBox->value()
        );
        
        alertController->setInsulinAlertThresholds(
            lowInsulinSpinBox->value(),
            criticalInsulinSpinBox->value()
        );
        
        alertController->setBatteryAlertThresholds(
            lowBatterySpinBox->value(),
            criticalBatterySpinBox->value()
        );
    }
    
    // Save to settings
    saveSettings();
    
    // Show confirmation
    QMessageBox::information(this, "Settings Saved", "Alert settings have been saved successfully.");
}

void AlertsScreen::onAcknowledgeAlertButtonClicked()
{
    if (!alertController) return;
    
    int selectedRow = activeAlertsList->currentRow();
    if (selectedRow >= 0) {
        alertController->acknowledgeAlert(selectedRow);
        updateActiveAlerts();
        updateAlertHistory(); // Update history after acknowledgment
    }
}

void AlertsScreen::onClearAllAlertsButtonClicked()
{
    if (!alertController) return;
    
    QMessageBox::StandardButton reply = QMessageBox::question(
        this, 
        "Clear All Alerts", 
        "Are you sure you want to acknowledge all active alerts?",
        QMessageBox::Yes | QMessageBox::No
    );
    
    if (reply == QMessageBox::Yes) {
        alertController->acknowledgeAllAlerts();
        updateActiveAlerts();
        updateAlertHistory(); // Update history after clearing all
    }
}

void AlertsScreen::onSetReminderButtonClicked()
{
    QString type = reminderTypeComboBox->currentText();
    QDateTime time = reminderTimeEdit->dateTime();
    
    if (time < QDateTime::currentDateTime()) {
        QMessageBox::warning(this, "Invalid Time", "Please select a future time for the reminder.");
        return;
    }
    
    addReminder(type, time);
    
    // Show confirmation
    QMessageBox::information(this, "Reminder Set", 
                           "Reminder for " + type + " has been set for " + 
                           time.toString("yyyy-MM-dd hh:mm AP"));
}

void AlertsScreen::onDeleteReminderButtonClicked()
{
    int selectedRow = remindersList->currentRow();
    if (selectedRow >= 0 && selectedRow < reminders.size()) {
        QMessageBox::StandardButton reply = QMessageBox::question(
            this, 
            "Delete Reminder", 
            "Are you sure you want to delete this reminder?",
            QMessageBox::Yes | QMessageBox::No
        );
        
        if (reply == QMessageBox::Yes) {
            reminders.remove(selectedRow);
            updateReminders();
            saveSettings();
        }
    }
}

void AlertsScreen::onEnableAlertsToggled(bool checked)
{
    if (alertController) {
        alertController->enableAlerts(checked);
    }
}

void AlertsScreen::onLowGlucoseThresholdChanged(double value)
{
    // Ensure thresholds are consistent
    if (value <= urgentLowGlucoseSpinBox->value()) {
        urgentLowGlucoseSpinBox->setValue(value - 0.1);
    }
}

void AlertsScreen::onHighGlucoseThresholdChanged(double value)
{
    // Ensure thresholds are consistent
    if (value >= urgentHighGlucoseSpinBox->value()) {
        urgentHighGlucoseSpinBox->setValue(value + 0.1);
    }
}

void AlertsScreen::onUrgentLowGlucoseThresholdChanged(double value)
{
    // Ensure thresholds are consistent
    if (value >= lowGlucoseSpinBox->value()) {
        lowGlucoseSpinBox->setValue(value + 0.1);
    }
}

void AlertsScreen::onUrgentHighGlucoseThresholdChanged(double value)
{
    // Ensure thresholds are consistent
    if (value <= highGlucoseSpinBox->value()) {
        highGlucoseSpinBox->setValue(value - 0.1);
    }
}

void AlertsScreen::onLowInsulinThresholdChanged(double value)
{
    // Ensure thresholds are consistent
    if (value <= criticalInsulinSpinBox->value()) {
        criticalInsulinSpinBox->setValue(value - 5.0);
    }
}

void AlertsScreen::onCriticalInsulinThresholdChanged(double value)
{
    // Ensure thresholds are consistent
    if (value >= lowInsulinSpinBox->value()) {
        lowInsulinSpinBox->setValue(value + 5.0);
    }
}

void AlertsScreen::onLowBatteryThresholdChanged(int value)
{
    // Ensure thresholds are consistent
    if (value <= criticalBatterySpinBox->value()) {
        criticalBatterySpinBox->setValue(value - 5);
    }
}

void AlertsScreen::onCriticalBatteryThresholdChanged(int value)
{
    // Ensure thresholds are consistent
    if (value >= lowBatterySpinBox->value()) {
        lowBatterySpinBox->setValue(value + 5);
    }
}

void AlertsScreen::paintEvent(QPaintEvent * /* event */)
{
    QStyleOption opt;
    opt.init(this);
    QPainter painter(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &painter, this);
}
