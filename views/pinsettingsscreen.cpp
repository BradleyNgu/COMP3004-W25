#include "pinsettingsscreen.h"
#include <QStyleOption>
#include <QPainter>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QMessageBox>
#include <QSettings>
#include <QInputDialog>  // Add this include for QInputDialog

PinSettingsScreen::PinSettingsScreen(QWidget *parent)
    : QWidget(parent),
      pinLockScreen(new PinLockScreen(this))
{
    setupUi();
    connectSignals();
    loadSettings();
}

PinSettingsScreen::~PinSettingsScreen()
{
}

void PinSettingsScreen::setupUi()
{
    // Set dark background
    setStyleSheet("background-color: #222222;");
    
    // Main layout
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(15);
    
    // Title
    titleLabel = new QLabel("PIN Security Settings");
    titleLabel->setStyleSheet("color: white; font-size: 24px; font-weight: bold;");
    titleLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(titleLabel);
    
    // PIN settings group box
    QGroupBox *pinSettingsGroup = new QGroupBox("PIN Settings");
    pinSettingsGroup->setStyleSheet("QGroupBox { color: white; font-weight: bold; border: 1px solid #444444; border-radius: 5px; padding: 10px; }");
    
    QVBoxLayout *pinSettingsLayout = new QVBoxLayout(pinSettingsGroup);
    pinSettingsLayout->setContentsMargins(10, 20, 10, 10);
    pinSettingsLayout->setSpacing(15);
    
    // Enable PIN checkbox
    enablePinCheckBox = new QCheckBox("Enable PIN Security");
    enablePinCheckBox->setStyleSheet("QCheckBox { color: white; font-size: 16px; }");
    pinSettingsLayout->addWidget(enablePinCheckBox);
    
    // PIN action buttons
    changeCurrentPinButton = new QPushButton("Change Current PIN");
    changeCurrentPinButton->setStyleSheet(
        "QPushButton { background-color: #00B2FF; color: white; border-radius: 5px; padding: 10px; }"
        "QPushButton:pressed { background-color: #0080FF; }"
    );
    pinSettingsLayout->addWidget(changeCurrentPinButton);
    
    // Add group box to main layout
    mainLayout->addWidget(pinSettingsGroup);
    
    // New PIN group box
    QGroupBox *newPinGroup = new QGroupBox("Set New PIN");
    newPinGroup->setStyleSheet("QGroupBox { color: white; font-weight: bold; border: 1px solid #444444; border-radius: 5px; padding: 10px; }");
    
    QVBoxLayout *newPinLayout = new QVBoxLayout(newPinGroup);
    newPinLayout->setContentsMargins(10, 20, 10, 10);
    newPinLayout->setSpacing(10);
    
    newPinLabel = new QLabel("New PIN (4-6 digits):");
    newPinLabel->setStyleSheet("color: white;");
    newPinInput = new QLineEdit();
    newPinInput->setStyleSheet(
        "QLineEdit { background-color: #333333; color: white; border: 1px solid #666666; border-radius: 3px; padding: 5px; }"
    );
    newPinInput->setEchoMode(QLineEdit::Password);
    newPinInput->setMaxLength(6);
    
    confirmPinLabel = new QLabel("Confirm PIN:");
    confirmPinLabel->setStyleSheet("color: white;");
    confirmPinInput = new QLineEdit();
    confirmPinInput->setStyleSheet(
        "QLineEdit { background-color: #333333; color: white; border: 1px solid #666666; border-radius: 3px; padding: 5px; }"
    );
    confirmPinInput->setEchoMode(QLineEdit::Password);
    confirmPinInput->setMaxLength(6);
    
    setNewPinButton = new QPushButton("Set New PIN");
    setNewPinButton->setStyleSheet(
        "QPushButton { background-color: #4CD964; color: white; border-radius: 5px; padding: 10px; }"
        "QPushButton:pressed { background-color: #3CB371; }"
    );
    
    newPinLayout->addWidget(newPinLabel);
    newPinLayout->addWidget(newPinInput);
    newPinLayout->addWidget(confirmPinLabel);
    newPinLayout->addWidget(confirmPinInput);
    newPinLayout->addWidget(setNewPinButton);
    
    mainLayout->addWidget(newPinGroup);
    
    // Add stretch to push everything to the top
    mainLayout->addStretch(1);
    
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

void PinSettingsScreen::connectSignals()
{
    connect(enablePinCheckBox, &QCheckBox::toggled, this, &PinSettingsScreen::onEnablePinToggled);
    connect(changeCurrentPinButton, &QPushButton::clicked, this, &PinSettingsScreen::onChangeCurrentPinClicked);
    connect(setNewPinButton, &QPushButton::clicked, this, &PinSettingsScreen::onSetNewPinClicked);
    connect(backButton, &QPushButton::clicked, this, &PinSettingsScreen::onBackButtonClicked);
}

void PinSettingsScreen::loadSettings()
{
    QSettings settings("TandemDiabetes", "tslimx2simulator");
    
    settings.beginGroup("Security");
    bool pinEnabled = settings.value("PinEnabled", false).toBool();
    settings.endGroup();
    
    enablePinCheckBox->setChecked(pinEnabled);
    
    // Update button states
    changeCurrentPinButton->setEnabled(pinEnabled);
}

void PinSettingsScreen::saveSettings()
{
    QSettings settings("TandemDiabetes", "tslimx2simulator");
    
    settings.beginGroup("Security");
    settings.setValue("PinEnabled", enablePinCheckBox->isChecked());
    settings.endGroup();
}

void PinSettingsScreen::updateSettings()
{
    loadSettings();
}

void PinSettingsScreen::onEnablePinToggled(bool checked)
{
    changeCurrentPinButton->setEnabled(checked);
    
    // If enabling PIN for the first time, prompt to set one
    if (checked) {
        QSettings settings("TandemDiabetes", "tslimx2simulator");
        settings.beginGroup("Security");
        QString existingPin = settings.value("Pin", "").toString();
        settings.endGroup();
        
        if (existingPin.isEmpty()) {
            QMessageBox::information(this, "Set PIN", "You'll need to set a PIN to enable PIN security.");
        }
    }
    
    // Update PIN lock screen state
    pinLockScreen->enablePin(checked);
    
    // Save settings
    saveSettings();
}

void PinSettingsScreen::onChangeCurrentPinClicked()
{
    // For a real implementation, you'd verify the current PIN first
    QSettings settings("TandemDiabetes", "tslimx2simulator");
    settings.beginGroup("Security");
    QString existingPin = settings.value("Pin", "").toString();
    settings.endGroup();
    
    if (existingPin.isEmpty()) {
        QMessageBox::warning(this, "No PIN Set", "There is no PIN currently set. Please set a new PIN first.");
        return;
    }
    
    // In a real application, you would first show a PIN entry screen to verify the current PIN
    bool ok;
    QString currentPin = QInputDialog::getText(this, "Verify Current PIN", 
                                             "Enter your current PIN:", QLineEdit::Password,
                                             "", &ok);
    if (ok && !currentPin.isEmpty()) {
        if (pinLockScreen->checkCurrentPin(currentPin)) {
            // Current PIN verified, now focus on the new PIN fields
            newPinInput->setFocus();
            QMessageBox::information(this, "Enter New PIN", "Current PIN verified. Please enter and confirm your new PIN.");
        } else {
            QMessageBox::warning(this, "Incorrect PIN", "The PIN you entered is incorrect.");
        }
    }
}

void PinSettingsScreen::onSetNewPinClicked()
{
    QString newPin = newPinInput->text();
    QString confirmPin = confirmPinInput->text();
    
    // Validate PIN format
    if (!pinLockScreen->validatePin(newPin)) {
        return; // validatePin shows error messages
    }
    
    // Check if PINs match
    if (newPin != confirmPin) {
        QMessageBox::warning(this, "PIN Mismatch", "The PINs you entered do not match.");
        return;
    }
    
    // Set the new PIN
    pinLockScreen->setPin(newPin);
    
    QMessageBox::information(this, "PIN Set", "Your new PIN has been set successfully.");
    
    // Clear fields
    newPinInput->clear();
    confirmPinInput->clear();
    
    // Make sure PIN is enabled
    enablePinCheckBox->setChecked(true);
    saveSettings();
}

void PinSettingsScreen::onBackButtonClicked()
{
    emit backButtonClicked();
}

void PinSettingsScreen::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QStyleOption opt;
    opt.init(this);
    QPainter painter(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &painter, this);
}
