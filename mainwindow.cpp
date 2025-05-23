#include "mainwindow.h"
#include <QVBoxLayout>
#include <QTimer>
#include <QMessageBox>
#include <QApplication>
#include <QScreen>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QWindow>
#include <QScrollArea>
#include <QDir>
#include <QStandardPaths>
#include "testpanel.h"
#include "forceresizable.h"
#include "views/alertsscreen.h"
#include <QCoreApplication>
#include <iostream>


// Constructor with improved resizing approach
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      isPoweredOn(false),
      isLocked(false),
      isSleeping(false)
{
    // Set window title
    setWindowTitle("t:slim X2 Simulator");
    
    // Setup the UI components with layouts
    setupUi();
    
    // Initialize the pump controller
    setupPumpController();
    
    // Connect signals and slots
    connectSignals();
    
    // Apply force resizable helper
    resizableHelper = new ForceResizable(this);
    
    // Start the pump in powered off state
    powerOff();
    
    // Simulate power on for demo purposes
    QTimer::singleShot(500, this, &MainWindow::powerOn);
}

MainWindow::~MainWindow()
{
    // Clean up resources
    if (pumpController) {
        delete pumpController;
    }
}

void MainWindow::setupUi()
{
    // Create central widget with stacked layout in a scroll area for better resizing
    QWidget *centralWidget = new QWidget(this);
    centralWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    
    // Create a scroll area to contain the stacked widget
    QScrollArea *scrollArea = new QScrollArea();
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    
    QWidget *scrollContent = new QWidget();
    QVBoxLayout *scrollLayout = new QVBoxLayout(scrollContent);
    scrollLayout->setContentsMargins(0, 0, 0, 0);
    
    stackedWidget = new QStackedWidget();
    stackedWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    scrollLayout->addWidget(stackedWidget);
    
    scrollArea->setWidget(scrollContent);
    mainLayout->addWidget(scrollArea);
    
    setCentralWidget(centralWidget);

    sleepOverlay = new QWidget(this);
    sleepOverlay->setStyleSheet("background-color: rgba(0, 0, 0, 255);");
    sleepOverlay->setGeometry(this->rect());
    sleepOverlay->hide();
    
    // Create all screen widgets
    homeScreen = new HomeScreen(this);
    bolusScreen = new BolusScreen(this);
    profileScreen = new ProfileScreen(this);
    optionsScreen = new OptionsScreen(this);
    historyScreen = new HistoryScreen(this);
    controlIQScreen = new ControlIQScreen(this);
    alertsScreen = new AlertsScreen(this);
    pinLockScreen = new PinLockScreen(this);
    pinSettingsScreen = new PinSettingsScreen(this);
    
    // Add all screens to stacked widget
    stackedWidget->addWidget(homeScreen);
    stackedWidget->addWidget(bolusScreen);
    stackedWidget->addWidget(profileScreen);
    stackedWidget->addWidget(optionsScreen);
    stackedWidget->addWidget(historyScreen);
    stackedWidget->addWidget(controlIQScreen);
    stackedWidget->addWidget(alertsScreen);
    stackedWidget->addWidget(pinLockScreen);
    stackedWidget->addWidget(pinSettingsScreen);
    
    // Create menu bar with additional options
    QMenuBar *menuBar = new QMenuBar(this);
    QMenu *toolsMenu = menuBar->addMenu("Tools");
    QMenu *viewMenu = menuBar->addMenu("View");
    QMenu *historyMenu = menuBar->addMenu("History");
    
    // Tools menu
    QAction *testPanelAction = toolsMenu->addAction("Test Panel");
    connect(testPanelAction, &QAction::triggered, this, &MainWindow::showTestPanel);
    
    QAction *savePumpAction = toolsMenu->addAction("Save Pump State");
    connect(savePumpAction, &QAction::triggered, this, &MainWindow::savePumpState);
    
    QAction *loadPumpAction = toolsMenu->addAction("Load Pump State");
    connect(loadPumpAction, &QAction::triggered, this, &MainWindow::loadPumpState);
    
    // View menu
    QAction *normalSizeAction = viewMenu->addAction("Normal Size (1x)");
    QAction *largeSizeAction = viewMenu->addAction("Large Size (1.5x)");
    QAction *extraLargeSizeAction = viewMenu->addAction("Extra Large Size (2x)");
    
    connect(normalSizeAction, &QAction::triggered, this, [this]() { setScaleFactor(1.0); });
    connect(largeSizeAction, &QAction::triggered, this, [this]() { setScaleFactor(1.5); });
    connect(extraLargeSizeAction, &QAction::triggered, this, [this]() { setScaleFactor(2.0); });
    
    // History menu
    QAction *glucoseHistoryAction = historyMenu->addAction("Glucose History");
    QAction *insulinHistoryAction = historyMenu->addAction("Insulin History");
    QAction *alertsHistoryAction = historyMenu->addAction("Alerts History");
    QAction *controlIQHistoryAction = historyMenu->addAction("Control-IQ History");
    
    connect(glucoseHistoryAction, &QAction::triggered, this, [this]() { showHistoryScreen(0); });
    connect(insulinHistoryAction, &QAction::triggered, this, [this]() { showHistoryScreen(1); });
    connect(alertsHistoryAction, &QAction::triggered, this, [this]() { showHistoryScreen(2); });
    connect(controlIQHistoryAction, &QAction::triggered, this, [this]() { showHistoryScreen(3); });
    
    setMenuBar(menuBar);
    
    // Set initial window size to something reasonable
    QScreen *screen = QGuiApplication::primaryScreen();
    QRect screenGeometry = screen->availableGeometry();
    int desiredWidth = qMin(screenGeometry.width() - 100, 640);
    int desiredHeight = qMin(screenGeometry.height() - 100, 960);
    resize(desiredWidth, desiredHeight);
    
    // Center the window on the screen
    move((screenGeometry.width() - width()) / 2, (screenGeometry.height() - height()) / 2);
}

void MainWindow::setupPumpController()
{
    pumpController = new PumpController(this);
    
    // Set controller connections for screens
    historyScreen->setPumpController(pumpController);
    controlIQScreen->setPumpController(pumpController);
    controlIQScreen->setControlIQAlgorithm(pumpController->getControlIQAlgorithm());
    alertsScreen->setAlertController(pumpController->getAlertController());
    
    // Create test panel
    testPanel = new TestPanel(pumpController, this);
    
    // Setup simulation data updates at faster intervals for testing
    simulationTimer = new QTimer(this);
    connect(simulationTimer, &QTimer::timeout, this, [this]() {
        // Only update when running
        if (isPoweredOn && !isSleeping) {
            homeScreen->updateAllData(pumpController);
        }
    });
    simulationTimer->start(1000); // Update every second for more responsive UI
    
    // Setup a separate timer for background updates even when in sleep mode
    backgroundTimer = new QTimer(this);
    connect(backgroundTimer, &QTimer::timeout, this, [this]() {
        if (isPoweredOn) {
            // Update critical data even in sleep mode
            if (isSleeping && stackedWidget->currentWidget() == homeScreen) {
                // Force insulin level to update even in sleep mode
                double insulinLevel = pumpController->getInsulinRemaining();
                // Store the value for when we wake up
                homeScreen->updateInsulinRemaining(insulinLevel);
            }
        }
    });
    backgroundTimer->start(5000); // Every 5 seconds for background updates
}

void MainWindow::connectSignals()
{
    // Connect navigation signals from home screen
    connect(homeScreen, &HomeScreen::bolusButtonClicked, this, &MainWindow::showBolusScreen);
    connect(homeScreen, &HomeScreen::optionsButtonClicked, this, &MainWindow::showOptionsScreen);
    connect(homeScreen, &HomeScreen::powerButtonPressed, this, &MainWindow::handlePowerButtonPressed);
    
    // Use lambda for signals without parameters connecting to slots with parameters
    connect(homeScreen, &HomeScreen::historyButtonClicked, this, [this]() {
        showHistoryScreen(0); // Default to first tab
    });
    
    connect(homeScreen, &HomeScreen::controlIQButtonClicked, this, &MainWindow::showControlIQScreen);
    
    // Connect navigation signals from other screens
    connect(bolusScreen, &BolusScreen::backButtonClicked, this, &MainWindow::showOptionsScreen);
    connect(profileScreen, &ProfileScreen::backButtonClicked, this, &MainWindow::showOptionsScreen);
    connect(optionsScreen, &OptionsScreen::backButtonClicked, this, &MainWindow::showHomeScreen);
    connect(historyScreen, &HistoryScreen::backButtonClicked, this, &MainWindow::showOptionsScreen);
    connect(controlIQScreen, &ControlIQScreen::backButtonClicked, this, &MainWindow::showOptionsScreen);
    connect(alertsScreen, &AlertsScreen::backButtonClicked, this, &MainWindow::showOptionsScreen);
    
    // Connect PIN screen signals
    connect(pinLockScreen, &PinLockScreen::pinAccepted, this, [this]() {
        isLocked = false;
        showHomeScreen();
    });
    
    connect(pinLockScreen, &PinLockScreen::backButtonClicked, this, [this]() {
        if (!pumpController->isPumpRunning()) {
            // If pump is off, allow going back
            showHomeScreen();
        } else {
            // Otherwise PIN is required
            QMessageBox::warning(this, "PIN Required", "You must enter your PIN to access the pump.");
        }
    });
    
    connect(pinSettingsScreen, &PinSettingsScreen::backButtonClicked, this, &MainWindow::showOptionsScreen);
    
    // Connect options screen navigation
    connect(optionsScreen, &OptionsScreen::profilesButtonClicked, this, &MainWindow::showProfileScreen);
    connect(optionsScreen, &OptionsScreen::alertsButtonClicked, this, &MainWindow::showAlertsScreen);
    connect(optionsScreen, &OptionsScreen::securitySettingsButtonClicked, this, &MainWindow::showPinSettingsScreen);
    
    // Use lambda for signals without parameters connecting to slots with parameters
    connect(optionsScreen, &OptionsScreen::historyButtonClicked, this, [this]() {
        showHistoryScreen(0); // Default to first tab
    });
    
    connect(optionsScreen, &OptionsScreen::controlIQButtonClicked, this, &MainWindow::showControlIQScreen);
    
    // Connect controller signals to UI updates
    connect(pumpController, &PumpController::batteryLevelChanged, homeScreen, &HomeScreen::updateBatteryLevel);
    connect(pumpController, &PumpController::insulinRemainingChanged, homeScreen, &HomeScreen::updateInsulinRemaining);
    connect(pumpController, &PumpController::glucoseLevelChanged, homeScreen, &HomeScreen::updateGlucoseLevel);
    connect(pumpController, &PumpController::glucoseTrendChanged, homeScreen, &HomeScreen::updateGlucoseTrend);
    connect(pumpController, &PumpController::insulinOnBoardChanged, homeScreen, &HomeScreen::updateInsulinOnBoard);
    connect(pumpController, &PumpController::controlIQActionChanged, homeScreen, &HomeScreen::updateControlIQAction);
    connect(pumpController, &PumpController::graphDataChanged, homeScreen, &HomeScreen::updateGlucoseGraph);
    connect(pumpController, &PumpController::shutdownRequested, this, &MainWindow::handlePumpShutdown);
    
    // Connect bolus controller signals
    connect(bolusScreen, &BolusScreen::bolusRequested, pumpController, &PumpController::deliverBolus);
    
    // Connect profile signals
    connect(profileScreen, &ProfileScreen::profileCreated, pumpController, &PumpController::createProfile);
    connect(profileScreen, &ProfileScreen::profileUpdated, pumpController, &PumpController::updateProfile);
    connect(profileScreen, &ProfileScreen::profileDeleted, pumpController, &PumpController::deleteProfile);
    connect(profileScreen, &ProfileScreen::profileActivated, pumpController, &PumpController::setActiveProfile);

    connect(bolusScreen, &BolusScreen::homeButtonClicked, this, &MainWindow::showHomeScreen);
    connect(profileScreen, &ProfileScreen::homeButtonClicked, this, &MainWindow::showHomeScreen);
    connect(optionsScreen, &OptionsScreen::homeButtonClicked, this, &MainWindow::showHomeScreen);
    connect(historyScreen, &HistoryScreen::homeButtonClicked, this, &MainWindow::showHomeScreen);
    connect(controlIQScreen, &ControlIQScreen::homeButtonClicked, this, &MainWindow::showHomeScreen);
    connect(alertsScreen, &AlertsScreen::homeButtonClicked, this, &MainWindow::showHomeScreen);
    connect(pinSettingsScreen, &PinSettingsScreen::homeButtonClicked, this, &MainWindow::showHomeScreen);
}

void MainWindow::setScaleFactor(double factor)
{
    // Apply scale factor to font sizes and layouts
    QFont font = QApplication::font();
    int baseFontSize = 9; // Default base font size
    font.setPointSize(qRound(baseFontSize * factor));
    QApplication::setFont(font);
    
    // Force update of UI elements that depend on font size
    homeScreen->updateFontSizes();
    
    // Adjust window size proportionally
    QSize currentSize = size();
    resize(qRound(currentSize.width() * factor), qRound(currentSize.height() * factor));
    
    // Center window again
    QScreen *screen = QGuiApplication::primaryScreen();
    QRect screenGeometry = screen->availableGeometry();
    move((screenGeometry.width() - width()) / 2, (screenGeometry.height() - height()) / 2);
}

void MainWindow::navigateToScreen(const QString &screenName)
{
    if (screenName == "home") {
        showHomeScreen();
    } else if (screenName == "bolus") {
        showBolusScreen();
    } else if (screenName == "profile") {
        showProfileScreen();
    } else if (screenName == "options") {
        showOptionsScreen();
    } else if (screenName == "history") {
        showHistoryScreen(0);
    } else if (screenName == "controliq") {
        showControlIQScreen();
    } else if (screenName == "alerts") {
        showAlertsScreen();
    } else if (screenName == "pin") {
        showPinLockScreen();
    } else if (screenName == "pinsettings") {
        showPinSettingsScreen();
    }
}

void MainWindow::showHomeScreen()
{
    // If PIN is enabled and screen is locked, show PIN screen instead
    if (pinLockScreen->isPinEnabled() && isLocked) {
        showPinLockScreen();
        return;
    }
    
    // Otherwise show home screen as usual
    stackedWidget->setCurrentWidget(homeScreen);
    
    // Ensure full refresh of data
    if (!isSleeping) {
        homeScreen->updateAllData(pumpController);
    }
}

void MainWindow::showBolusScreen()
{
    stackedWidget->setCurrentWidget(bolusScreen);
    bolusScreen->updateCurrentValues(pumpController);
}

void MainWindow::showProfileScreen()
{
    stackedWidget->setCurrentWidget(profileScreen);
    profileScreen->loadProfiles(pumpController);
}

void MainWindow::showOptionsScreen()
{
    stackedWidget->setCurrentWidget(optionsScreen);
}

void MainWindow::showHistoryScreen(int tabIndex)
{
    stackedWidget->setCurrentWidget(historyScreen);
    
    // If a specific tab index was provided, switch to that tab
    if (historyScreen->getTabWidget() && tabIndex >= 0 && tabIndex < historyScreen->getTabWidget()->count()) {
        historyScreen->getTabWidget()->setCurrentIndex(tabIndex);
    }
    
    // Refresh the history data
    historyScreen->updateHistoryData();
}

void MainWindow::showControlIQScreen()
{
    stackedWidget->setCurrentWidget(controlIQScreen);
    controlIQScreen->updateUIFromSettings();
}

void MainWindow::showAlertsScreen()
{
    stackedWidget->setCurrentWidget(alertsScreen);
}

void MainWindow::showPinLockScreen()
{
    stackedWidget->setCurrentWidget(pinLockScreen);
}

void MainWindow::showPinSettingsScreen()
{
    pinSettingsScreen->updateSettings();
    stackedWidget->setCurrentWidget(pinSettingsScreen);
}

void MainWindow::checkPinLock()
{
    // Check if PIN is enabled
    if (pinLockScreen->isPinEnabled()) {
        // Lock the screen
        isLocked = true;
        showPinLockScreen();
    } else {
        // No PIN, just show home screen
        isLocked = false;
        showHomeScreen();
    }
}

void MainWindow::showTestPanel()
{
    testPanel->show();
}

void MainWindow::handlePowerButtonPressed()
{
    if (isPoweredOn) {
        // Show power options dialog
        QMessageBox msgBox;
        msgBox.setWindowTitle("Power Options");
        msgBox.setText("Power Options");
        msgBox.setInformativeText("What would you like to do?");
        QPushButton *sleepButton = msgBox.addButton("Sleep", QMessageBox::ActionRole);
        QPushButton *powerOffButton = msgBox.addButton("Power Off", QMessageBox::ActionRole);
        msgBox.addButton("Cancel", QMessageBox::RejectRole);

        msgBox.exec();

        if (msgBox.clickedButton() == powerOffButton) {
            simulatePowerOff();
        } else if (msgBox.clickedButton() == sleepButton) {
            // Enter sleep mode
            enterSleepMode();
        }

        // Otherwise do nothing (Cancel was clicked)
    } else {
        powerOn();
    }
}


void MainWindow::handlePumpShutdown()
{
    QMessageBox::critical(this, "Critical Error", "Pump has encountered a critical error and must shut down.");
    simulatePowerOff();
}

void MainWindow::simulatePowerOff()
{
    // Show shutdown animation or message
    QMessageBox msgBox;
    msgBox.setWindowTitle("Shutting Down");
    msgBox.setText("The pump is shutting down...");
    msgBox.setStandardButtons(QMessageBox::NoButton);
    
    // Create a timer to close the message box after a delay
    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, &msgBox, &QMessageBox::accept);
    timer->start(1500); // 1.5 seconds
    
    msgBox.exec();
    timer->deleteLater();
    
    // Actually power off
    powerOff();
}

void MainWindow::powerOn()
{
    isPoweredOn = true;
    pumpController->startPump();
    homeScreen->setEnabled(true);
    
    // Ensure the simulation timer is running
    if (simulationTimer && !simulationTimer->isActive()) {
        simulationTimer->start();
    }
    
    // Ensure the background timer is running
    if (backgroundTimer && !backgroundTimer->isActive()) {
        backgroundTimer->start();
    }
    
    // Check PIN lock instead of directly showing home screen
    checkPinLock();
    
    // Ensure we're not in sleep mode
    isSleeping = false;
    if (sleepOverlay) {
        sleepOverlay->hide();
    }
}

void MainWindow::powerOff()
{
    isPoweredOn = false;
    pumpController->stopPump();
    
    // Stop the timers
    if (simulationTimer && simulationTimer->isActive()) {
        simulationTimer->stop();
    }
    
    if (backgroundTimer && backgroundTimer->isActive()) {
        backgroundTimer->stop();
    }
    
    // Show black screen or startup logo
    homeScreen->setEnabled(false);
    stackedWidget->setCurrentIndex(0);
    
    // Actually quit the application when powered off
    QCoreApplication::quit();
}

void MainWindow::savePumpState()
{
    if (!pumpController) {
        return;
    }
    
    // Get the application data location
    QString dataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir dir(dataPath);
    if (!dir.exists()) {
        dir.mkpath(".");
    }
    
    // Save the pump state
    if (pumpController->saveData(dataPath)) {
        QMessageBox::information(this, "Save Successful", "Pump state saved successfully.");
    } else {
        QMessageBox::warning(this, "Save Failed", "Failed to save pump state.");
    }
}

void MainWindow::loadPumpState()
{
    if (!pumpController) {
        return;
    }
    
    // Get the application data location
    QString dataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    
    // Load the pump state
    if (pumpController->loadData(dataPath)) {
        QMessageBox::information(this, "Load Successful", "Pump state loaded successfully.");
        
        // Update UI - force a full refresh
        homeScreen->updateAllData(pumpController);
    } else {
        QMessageBox::warning(this, "Load Failed", "Failed to load pump state.");
    }
}

// Override to ensure resizing works
void MainWindow::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);
    
    // Update UI elements that depend on window size
    if (homeScreen) {
        homeScreen->updateFontSizes();
    }
    
    // Update overlay size
    if (sleepOverlay) {
        sleepOverlay->setGeometry(this->rect());
    }
}

void MainWindow::mousePressEvent(QMouseEvent *event)
{
    if (isSleeping) {
        exitSleepMode();
    }

    QMainWindow::mousePressEvent(event);
}

void MainWindow::enterSleepMode()
{
    if (isSleeping) return;
    
    isSleeping = true;
    
    // Stop the main UI update timer
    if (simulationTimer && simulationTimer->isActive()) {
        simulationTimer->stop();
    }
    
    // Make sure background timer is running
    if (backgroundTimer && !backgroundTimer->isActive()) {
        backgroundTimer->start();
    }
    
    homeScreen->setEnabled(false);
    stackedWidget->setEnabled(false);

    if (sleepOverlay) {
        sleepOverlay->raise();
        sleepOverlay->show();
    }
}

void MainWindow::exitSleepMode()
{
    if (!isSleeping) return;

    isSleeping = false;
    
    // Start the UI update timer
    if (simulationTimer && !simulationTimer->isActive() && isPoweredOn) {
        simulationTimer->start();
    }
    
    homeScreen->setEnabled(true);
    stackedWidget->setEnabled(true);

    if (sleepOverlay) {
        sleepOverlay->hide();
    }
    
    // Force update the UI to show the latest data
    if (isPoweredOn && stackedWidget->currentWidget() == homeScreen) {
        homeScreen->updateAllData(pumpController);
    }
}