#ifndef PUMPCONTROLLER_H
#define PUMPCONTROLLER_H

#include <QObject>
#include <QTimer>
#include "../models/pumpmodel.h"
#include "../models/profilemodel.h"
#include "../models/glucosemodel.h"
#include "../models/insulinmodel.h"
#include "../utils/controliqalgorithm.h"
#include "../utils/datastorage.h"
#include "../utils/errorhandler.h"
#include "../controllers/alertcontroller.h"

class PumpController : public QObject
{
    Q_OBJECT

public:
    explicit PumpController(QObject *parent = nullptr);
    ~PumpController();
    
    // Initialization
    void initializeSimulator();
    void generateHistoricalInsulinData(int hoursBack = 48);
    
    // Pump state
    void startPump();
    void stopPump();
    bool isPumpRunning() const;
    
    // Battery management
    int getBatteryLevel() const;
    bool isCharging() const;
    void startCharging();
    void stopCharging();
    
    // Insulin management
    double getInsulinRemaining() const;
    double getCurrentBasalRate() const;
    double getInsulinOnBoard() const;
    
    // Glucose data
    double getCurrentGlucose() const;
    QDateTime getLastGlucoseReading() const;
    GlucoseModel::TrendDirection getGlucoseTrend() const;
    
    // Control-IQ
    double getControlIQDelivery() const;
    void enableControlIQ(bool enable);
    bool isControlIQEnabled() const;
    ControlIQAlgorithm* getControlIQAlgorithm() const { return controlIQAlgorithm; }
    
    // Alerts
    AlertController* getAlertController() const { return alertController; }
    ErrorHandler* getErrorHandler() const;
    DataStorage* getDataStorage() const;
    
    // Profile management
    void setActiveProfile(const QString &profileName);
    Profile getActiveProfile() const;
    QString getActiveProfileName() const;
    QVector<Profile> getAllProfiles() const;
    bool createProfile(const Profile &profile);
    bool updateProfile(const QString &name, const Profile &profile);
    bool deleteProfile(const QString &name);
    
    // Data access
    QVector<QPair<QDateTime, double>> getGlucoseHistory(const QDateTime &start, const QDateTime &end) const;
    QVector<QPair<QDateTime, double>> getInsulinHistory(const QDateTime &start, const QDateTime &end) const;
    
    // Bolus delivery
    bool deliverBolus(double units, bool extended = false, int duration = 0);
    bool cancelBolus();
    bool isBolusActive() const;
    
    // Data management
    bool saveData(const QString &directory);
    bool loadData(const QString &directory);
    
    // Test panel methods
    void updateBatteryLevel(int level);
    void updateInsulinRemaining(double units);
    void updateGlucoseLevel(double value);
    void updateGlucoseTrend(GlucoseModel::TrendDirection trend);
    void generateTestAlert(const QString &message, PumpModel::AlertLevel level);
    
public slots:
    void simulateBatteryDrain();
    void processGlucoseReading(double value, const QDateTime &timestamp);
    void updateInsulinOnBoard();
    void updateBasalConsumption();
    void runControlIQ();
    void checkReminders();
    void checkForOcclusion();
    void simulateGlucoseReading();
    void savePumpState();
    void loadPumpState();
    
signals:
    void pumpStarted();
    void pumpStopped();
    void batteryLevelChanged(int level);
    void chargingStateChanged(bool charging);
    void insulinRemainingChanged(double units);
    void basalRateChanged(double rate);
    void insulinOnBoardChanged(double units);
    void glucoseLevelChanged(double value);
    void glucoseTrendChanged(GlucoseModel::TrendDirection trend);
    void controlIQActionChanged(double value);
    void profileChanged(const QString &name);
    void bolusDeliveryStarted(double units);
    void bolusDeliveryCompleted(double units);
    void bolusDeliveryCancelled(double delivered, double requested);
    void alertTriggered(const QString &message, PumpModel::AlertLevel level);
    void graphDataChanged(const QVector<QPair<QDateTime, double>> &data);
    void shutdownRequested();
    
private:
    PumpModel *pumpModel;
    ProfileModel *profileModel;
    GlucoseModel *glucoseModel;
    InsulinModel *insulinModel;
    ControlIQAlgorithm *controlIQAlgorithm;
    DataStorage *dataStorage;
    ErrorHandler *errorHandler;
    AlertController *alertController;
    
    QTimer *batteryTimer;
    QTimer *glucoseTimer;
    QTimer *iobTimer;
    QTimer *controlIQTimer;
    QTimer *reminderTimer;
    QTimer *occlusionTimer;
    QTimer *basalConsumptionTimer;
    
    bool running;
    bool controlIQEnabled;
    int simulationSpeedFactor;
    
    void setupTimers();
    void connectModelSignals();
    void startSimulation();
    void stopSimulation();
    
    void checkLowBattery();
    void checkLowInsulin();
    void checkGlucoseAlerts();
};

#endif // PUMPCONTROLLER_H
