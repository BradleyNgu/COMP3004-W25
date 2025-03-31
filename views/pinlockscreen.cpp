#include "pinlockscreen.h"
#include <QStyleOption>
#include <QPainter>
#include <QMessageBox>
#include <QTimer>
#include <QCryptographicHash>

PinLockScreen::PinLockScreen(QWidget *parent)
    : QWidget(parent),
      pinEnabled(false),
      failedAttempts(0)
{
    setupUi();
    connectSignals();
    loadSettings();
}

PinLockScreen::~PinLockScreen()
{
}

void PinLockScreen::setupUi()
{
    // Set dark background
    setStyleSheet("background-color: #222222;");
    
    // Main layout
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(15);
    
    // Title
    titleLabel = new QLabel("Security PIN");
    titleLabel->setStyleSheet("color: white; font-size: 24px; font-weight: bold;");
    titleLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(titleLabel);
    
    // Message
    messageLabel = new QLabel("Enter PIN to unlock pump");
    messageLabel->setStyleSheet("color: white; font-size: 16px;");
    messageLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(messageLabel);
    
    // PIN display
    pinDisplay = new QLineEdit();
    pinDisplay->setEchoMode(QLineEdit::Password);
    pinDisplay->setAlignment(Qt::AlignCenter);
    pinDisplay->setStyleSheet(
        "QLineEdit { background-color: #333333; color: white; border: 1px solid #666666; "
        "border-radius: 5px; padding: 10px; font-size: 20px; font-weight: bold; }"
    );
    pinDisplay->setReadOnly(true);
    pinDisplay->setMaxLength(6); // Limit PIN to 6 digits
    mainLayout->addWidget(pinDisplay);
    
    // Add some spacing
    mainLayout->addSpacing(20);
    
    // Number pad grid
    QGridLayout *numberPadLayout = new QGridLayout();
    numberPadLayout->setSpacing(10);
    
    // Create digit buttons
    for (int i = 0; i < 10; i++) {
        digitButtons[i] = new QPushButton(QString::number(i));
        digitButtons[i]->setStyleSheet(
            "QPushButton { background-color: #444444; color: white; border-radius: 25px; "
            "font-size: 20px; font-weight: bold; min-width: 50px; min-height: 50px; }"
            "QPushButton:pressed { background-color: #666666; }"
        );
        
        // Place buttons in grid
        if (i == 0) {
            // Put 0 in the middle of the bottom row
            numberPadLayout->addWidget(digitButtons[i], 3, 1);
        } else {
            // 1-9 follow telephone layout (123, 456, 789)
            int row = (i - 1) / 3;
            int col = (i - 1) % 3;
            numberPadLayout->addWidget(digitButtons[i], row, col);
        }
    }
    
    // Clear button (bottom left)
    clearButton = new QPushButton("Clear");
    clearButton->setStyleSheet(
        "QPushButton { background-color: #FF3B30; color: white; border-radius: 25px; "
        "font-size: 16px; font-weight: bold; min-width: 50px; min-height: 50px; }"
        "QPushButton:pressed { background-color: #CC2F27; }"
    );
    numberPadLayout->addWidget(clearButton, 3, 0);
    
    // Enter button (bottom right)
    enterButton = new QPushButton("Enter");
    enterButton->setStyleSheet(
        "QPushButton { background-color: #4CD964; color: white; border-radius: 25px; "
        "font-size: 16px; font-weight: bold; min-width: 50px; min-height: 50px; }"
        "QPushButton:pressed { background-color: #3CB371; }"
    );
    numberPadLayout->addWidget(enterButton, 3, 2);
    
    // Add number pad to main layout
    mainLayout->addLayout(numberPadLayout);
    
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

void PinLockScreen::connectSignals()
{
    // Connect digit buttons
    for (int i = 0; i < 10; i++) {
        connect(digitButtons[i], &QPushButton::clicked, this, &PinLockScreen::onDigitButtonClicked);
    }
    
    // Connect function buttons
    connect(clearButton, &QPushButton::clicked, this, &PinLockScreen::onClearButtonClicked);
    connect(enterButton, &QPushButton::clicked, this, &PinLockScreen::onEnterButtonClicked);
    connect(backButton, &QPushButton::clicked, this, &PinLockScreen::onBackButtonClicked);
}

void PinLockScreen::loadSettings()
{
    QSettings settings("TandemDiabetes", "tslimx2simulator");
    
    settings.beginGroup("Security");
    pinEnabled = settings.value("PinEnabled", false).toBool();
    currentPin = settings.value("Pin", "").toString();
    settings.endGroup();
}

void PinLockScreen::saveSettings()
{
    QSettings settings("TandemDiabetes", "tslimx2simulator");
    
    settings.beginGroup("Security");
    settings.setValue("PinEnabled", pinEnabled);
    settings.setValue("Pin", currentPin);
    settings.endGroup();
}

bool PinLockScreen::isPinEnabled() const
{
    return pinEnabled;
}

void PinLockScreen::enablePin(bool enable)
{
    pinEnabled = enable;
    saveSettings();
}

void PinLockScreen::setPin(const QString &pin)
{
    // Hash the PIN for security
    QCryptographicHash hash(QCryptographicHash::Sha256);
    hash.addData(pin.toUtf8());
    
    currentPin = QString(hash.result().toHex());
    saveSettings();
}

bool PinLockScreen::validatePin(const QString &pin)
{
    // Enforce PIN rules (e.g., minimum length, no repeating digits)
    if (pin.length() < 4) {
        QMessageBox::warning(this, "Invalid PIN", "PIN must be at least 4 digits long.");
        return false;
    }
    
    // Check for sequential digits (e.g., 1234)
    bool sequential = true;
    for (int i = 1; i < pin.length(); i++) {
        if (pin[i].digitValue() != pin[i-1].digitValue() + 1) {
            sequential = false;
            break;
        }
    }
    
    if (sequential) {
        QMessageBox::warning(this, "Invalid PIN", "PIN cannot be sequential digits (e.g., 1234).");
        return false;
    }
    
    // Check for repeating digits (e.g., 1111)
    bool allSame = true;
    for (int i = 1; i < pin.length(); i++) {
        if (pin[i] != pin[0]) {
            allSame = false;
            break;
        }
    }
    
    if (allSame) {
        QMessageBox::warning(this, "Invalid PIN", "PIN cannot be all the same digit (e.g., 1111).");
        return false;
    }
    
    return true;
}

bool PinLockScreen::checkCurrentPin(const QString &pin)
{
    // Hash the input PIN and compare with stored hash
    QCryptographicHash hash(QCryptographicHash::Sha256);
    hash.addData(pin.toUtf8());
    QString hashedInput = QString(hash.result().toHex());
    
    return hashedInput == currentPin;
}

void PinLockScreen::onDigitButtonClicked()
{
    QPushButton *button = qobject_cast<QPushButton*>(sender());
    if (button) {
        pinDisplay->setText(pinDisplay->text() + button->text());
    }
}

void PinLockScreen::onClearButtonClicked()
{
    pinDisplay->clear();
}

void PinLockScreen::onEnterButtonClicked()
{
    QString enteredPin = pinDisplay->text();
    
    if (enteredPin.isEmpty()) {
        messageLabel->setText("Please enter a PIN");
        messageLabel->setStyleSheet("color: #FF3B30; font-size: 16px;");
        return;
    }
    
    // For setting a new PIN
    if (currentPin.isEmpty()) {
        if (validatePin(enteredPin)) {
            // In a real implementation, you'd want to confirm the PIN
            setPin(enteredPin);
            emit pinAccepted();
        }
    } 
    // For checking existing PIN
    else {
        if (checkCurrentPin(enteredPin)) {
            failedAttempts = 0;
            emit pinAccepted();
        } else {
            failedAttempts++;
            messageLabel->setText("Incorrect PIN. Please try again.");
            messageLabel->setStyleSheet("color: #FF3B30; font-size: 16px;");
            pinDisplay->clear();
            
            // Implement lockout after too many failed attempts
            if (failedAttempts >= 5) {
                lockoutAfterFailedAttempts();
            }
            
            emit pinRejected();
        }
    }
}

void PinLockScreen::onBackButtonClicked()
{
    emit backButtonClicked();
}

void PinLockScreen::lockoutAfterFailedAttempts()
{
    // Disable all input
    for (int i = 0; i < 10; i++) {
        digitButtons[i]->setEnabled(false);
    }
    clearButton->setEnabled(false);
    enterButton->setEnabled(false);
    
    messageLabel->setText("Too many failed attempts. Locked for 30 seconds.");
    messageLabel->setStyleSheet("color: #FF3B30; font-size: 16px; font-weight: bold;");
    
    // Reset after 30 seconds
    QTimer::singleShot(30000, this, [this]() {
        for (int i = 0; i < 10; i++) {
            digitButtons[i]->setEnabled(true);
        }
        clearButton->setEnabled(true);
        enterButton->setEnabled(true);
        failedAttempts = 0;
        
        messageLabel->setText("Enter PIN to unlock pump");
        messageLabel->setStyleSheet("color: white; font-size: 16px;");
        pinDisplay->clear();
    });
}

void PinLockScreen::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QStyleOption opt;
    opt.init(this);
    QPainter painter(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &painter, this);
}
