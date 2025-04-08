#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>
#include <QResizeEvent>
#include <QMouseEvent>
#include "controllers/pumpcontroller.h"
#include "views/homescreen.h"
#include "views/bolusscreen.h"
#include "views/profilescreen.h"
#include "views/optionsscreen.h"
#include "views/historyscreen.h"
#include "views/controliqscreen.h"
#include "views/pinlockscreen.h"
#include "views/pinsettingsscreen.h"

// Forward declarations
class TestPanel;
class ForceResizable;
class AlertsScreen;  // Add this forward declaration

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    
    // Navigation methods
    void navigateToScreen(const QString &screenName);
    
protected:
    // Override resize event to handle custom resizing behavior
    void resizeEvent(QResizeEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    
private slots:
    // Screen navigation
    void showHomeScreen();
    void showBolusScreen();
    void showProfileScreen();
    void showOptionsScreen();
    void showHistoryScreen(int tabIndex = 0);
    void showControlIQScreen();
    void showAlertsScreen();
    void showPinLockScreen();
    void showPinSettingsScreen();
    
    // Utility functions
    void showTestPanel();
    void handlePowerButtonPressed();
    void handlePumpShutdown();
    void simulatePowerOff();
    void powerOn();
    void powerOff();
    void savePumpState();
    void loadPumpState();
    void setScaleFactor(double factor);
    void checkPinLock();
    
    // Sleep mode functions
    void enterSleepMode();
    void exitSleepMode();
    
private:
    // Setup methods
    void setupUi();
    void setupPumpController();
    void connectSignals();
    
    // UI Components
    QStackedWidget *stackedWidget;
    HomeScreen *homeScreen;
    BolusScreen *bolusScreen;
    ProfileScreen *profileScreen;
    OptionsScreen *optionsScreen;
    HistoryScreen *historyScreen;
    ControlIQScreen *controlIQScreen;
    AlertsScreen *alertsScreen;
    PinLockScreen *pinLockScreen;
    PinSettingsScreen *pinSettingsScreen;
    
    // Controllers
    PumpController *pumpController;
    TestPanel *testPanel;
    ForceResizable *resizableHelper;
    
    // State variables
    bool isPoweredOn;
    bool isLocked;
    bool isSleeping;
    
    // Timer for simulation updates
    QTimer *simulationTimer;
    QTimer *backgroundTimer;
    
    // Sleep mode overlay
    QWidget *sleepOverlay;
};

#endif // MAINWINDOW_H