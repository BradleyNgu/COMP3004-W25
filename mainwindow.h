#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>
#include <QResizeEvent>
#include <QEvent>
#include <QScrollArea>
#include "controllers/pumpcontroller.h"
#include "views/homescreen.h"
#include "views/bolusscreen.h"
#include "views/profilescreen.h"
#include "views/optionsscreen.h"
#include "views/historyscreen.h"
#include "views/controliqscreen.h"
#include "views/alertsscreen.h"
#include "views/pinlockscreen.h"
#include "views/pinsettingsscreen.h"
#include "testpanel.h"
#include "forceresizable.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    void navigateToScreen(const QString &screenName);
    void showHomeScreen();
    void showBolusScreen();
    void showProfileScreen();
    void showOptionsScreen();
    void showHistoryScreen(int tabIndex = 0);
    void showControlIQScreen();
    void showAlertsScreen();
    void showPinLockScreen();
    void showPinSettingsScreen();
    void checkPinLock();
    void showTestPanel();
    void handlePowerButtonPressed();
    void handlePumpShutdown();
    void simulatePowerOff();
    void setScaleFactor(double factor);
    void savePumpState();
    void loadPumpState();

protected:
    // Override to ensure proper resizing
    void resizeEvent(QResizeEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;

private:
    QStackedWidget *stackedWidget;
    QTimer* simulationTimer = nullptr;
    QWidget *sleepOverlay = nullptr;
    HomeScreen *homeScreen;
    BolusScreen *bolusScreen;
    ProfileScreen *profileScreen;
    OptionsScreen *optionsScreen;
    HistoryScreen *historyScreen;
    ControlIQScreen *controlIQScreen;
    AlertsScreen *alertsScreen;
    PinLockScreen *pinLockScreen;
    PinSettingsScreen *pinSettingsScreen;
    PumpController *pumpController;
    TestPanel *testPanel;
    ForceResizable *resizableHelper;
    
    bool isPoweredOn;
    bool isLocked;
    bool isSleeping = false;
    
    void setupUi();
    void connectSignals();
    void setupPumpController();
    void powerOn();
    void powerOff();
    void exitSleepMode();
    void enterSleepMode();
};

#endif // MAINWINDOW_H
