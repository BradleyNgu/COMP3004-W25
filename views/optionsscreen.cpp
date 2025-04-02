#include "optionsscreen.h"
#include "ui_optionsscreen.h"
#include <QStyleOption>
#include <QPainter>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QScrollArea>

OptionsScreen::OptionsScreen(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::OptionsScreen)
{
    ui->setupUi(this);
    
    // Setup custom UI with layouts
    setupUi();
    
    // Connect signals to slots
    connectSignals();
}

OptionsScreen::~OptionsScreen()
{
    delete ui;
}

void OptionsScreen::setupUi()
{
    // Set dark background
    setStyleSheet("background-color: #222222;");
    
    // Main layout for entire screen
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(16, 16, 16, 16);
    mainLayout->setSpacing(12);

    QHBoxLayout *topLayout = new QHBoxLayout();
    topLayout->setAlignment(Qt::AlignRight);

    homeButton = new QPushButton("T");
    homeButton->setStyleSheet(
        "QPushButton { background-color: transparent; color: #00B2FF; font-size: 20px; font-weight: bold; border: none; }"
        "QPushButton:pressed { color: #0077CC; }"
    );
    homeButton->setFixedSize(40, 40);

    topLayout->addWidget(homeButton);
    mainLayout->addLayout(topLayout);
    
    // Title
    titleLabel = new QLabel("Options");
    titleLabel->setStyleSheet("color: white; font-size: 24px; font-weight: bold;");
    titleLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(titleLabel);
    
    // Scroll area for options
    QScrollArea *scrollArea = new QScrollArea();
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    
    // Container for option buttons
    QWidget *scrollContent = new QWidget();
    QVBoxLayout *optionsLayout = new QVBoxLayout(scrollContent);
    optionsLayout->setContentsMargins(8, 8, 8, 8);
    optionsLayout->setSpacing(12);
    
    // Option button style
    QString buttonStyle = 
        "QPushButton { background-color: #333333; color: white; border-radius: 8px; "
        "font-size: 16px; text-align: left; padding: 12px 20px; }"
        "QPushButton:pressed { background-color: #444444; }";
    
    // Option buttons
    profilesButton = new QPushButton("Personal Profiles");
    profilesButton->setStyleSheet(buttonStyle);
    profilesButton->setMinimumHeight(50);
    
    startInsulinButton = new QPushButton("Start Insulin");
    startInsulinButton->setStyleSheet(buttonStyle);
    startInsulinButton->setMinimumHeight(50);
    
    stopInsulinButton = new QPushButton("Stop Insulin");
    stopInsulinButton->setStyleSheet(buttonStyle);
    stopInsulinButton->setMinimumHeight(50);
    
    alertsButton = new QPushButton("Alerts & Reminders");
    alertsButton->setStyleSheet(buttonStyle);
    alertsButton->setMinimumHeight(50);
    
    historyButton = new QPushButton("History");
    historyButton->setStyleSheet(buttonStyle);
    historyButton->setMinimumHeight(50);
    
    controlIQButton = new QPushButton("Control-IQ Settings");
    controlIQButton->setStyleSheet(buttonStyle);
    controlIQButton->setMinimumHeight(50);
    
    securitySettingsButton = new QPushButton("Security Settings");
    securitySettingsButton->setStyleSheet(buttonStyle);
    securitySettingsButton->setMinimumHeight(50);
    
    // Add buttons to layout
    optionsLayout->addWidget(profilesButton);
    optionsLayout->addWidget(startInsulinButton);
    optionsLayout->addWidget(stopInsulinButton);
    optionsLayout->addWidget(alertsButton);
    optionsLayout->addWidget(historyButton);
    optionsLayout->addWidget(controlIQButton);
    optionsLayout->addWidget(securitySettingsButton);
    
    // Add stretch to push buttons to top
    optionsLayout->addStretch(1);
    
    // Set the scroll content
    scrollArea->setWidget(scrollContent);
    mainLayout->addWidget(scrollArea);
    
    // Back button container
    QHBoxLayout *backButtonLayout = new QHBoxLayout();
    backButtonLayout->addStretch();
    
    // Back button
    backButton = new QPushButton("Back");
    backButton->setStyleSheet(
        "QPushButton { background-color: #444444; color: white; border-radius: 5px; padding: 8px; }"
        "QPushButton:pressed { background-color: #666666; }"
    );
    backButton->setFixedWidth(80);
    
    backButtonLayout->addWidget(backButton);
    mainLayout->addLayout(backButtonLayout);
}

void OptionsScreen::connectSignals()
{
    // Connect all button signals to slots
    connect(backButton, &QPushButton::clicked, this, &OptionsScreen::onBackButtonClicked);
    connect(profilesButton, &QPushButton::clicked, this, &OptionsScreen::onProfilesButtonClicked);
    connect(startInsulinButton, &QPushButton::clicked, this, &OptionsScreen::onStartInsulinButtonClicked);
    connect(stopInsulinButton, &QPushButton::clicked, this, &OptionsScreen::onStopInsulinButtonClicked);
    connect(alertsButton, &QPushButton::clicked, this, &OptionsScreen::onAlertsButtonClicked);
    connect(historyButton, &QPushButton::clicked, this, &OptionsScreen::onHistoryButtonClicked);
    connect(controlIQButton, &QPushButton::clicked, this, &OptionsScreen::onControlIQButtonClicked);
    connect(securitySettingsButton, &QPushButton::clicked, this, &OptionsScreen::onSecuritySettingsButtonClicked);
    connect(homeButton, &QPushButton::clicked, this, &OptionsScreen::onHomeButtonClicked);
}

void OptionsScreen::onBackButtonClicked()
{
    emit backButtonClicked();
}

void OptionsScreen::onProfilesButtonClicked()
{
    emit profilesButtonClicked();
}

void OptionsScreen::onHistoryButtonClicked()
{
    emit historyButtonClicked();
}

void OptionsScreen::onControlIQButtonClicked()
{
    emit controlIQButtonClicked();
}

void OptionsScreen::onSecuritySettingsButtonClicked()
{
    emit securitySettingsButtonClicked();
}

void OptionsScreen::onStartInsulinButtonClicked()
{
    // Show confirmation dialog
    QMessageBox::StandardButton reply = QMessageBox::question(
        this, 
        "Start Insulin", 
        "Are you sure you want to start insulin delivery?",
        QMessageBox::Yes | QMessageBox::No
    );
    
    if (reply == QMessageBox::Yes) {
        emit startInsulinButtonClicked();
        QMessageBox::information(this, "Insulin Started", "Insulin delivery has been started.");
    }
}

void OptionsScreen::onStopInsulinButtonClicked()
{
    // Show confirmation dialog
    QMessageBox::StandardButton reply = QMessageBox::question(
        this, 
        "Stop Insulin", 
        "Are you sure you want to stop insulin delivery?\nThis will suspend all insulin delivery including basal insulin.",
        QMessageBox::Yes | QMessageBox::No
    );
    
    if (reply == QMessageBox::Yes) {
        emit stopInsulinButtonClicked();
        QMessageBox::warning(this, "Insulin Stopped", "Insulin delivery has been stopped.");
    }
}

void OptionsScreen::onAlertsButtonClicked()
{
    // Emit the signal to navigate to the alerts screen
    emit alertsButtonClicked();
}

void OptionsScreen::paintEvent(QPaintEvent * /* event */)
{
    QStyleOption opt;
    opt.init(this);
    QPainter painter(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &painter, this);
}
void OptionsScreen::onHomeButtonClicked()
{
    emit homeButtonClicked();
    emit backButtonClicked();
}