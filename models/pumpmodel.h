#ifndef PUMPMODEL_H
#define PUMPMODEL_H

#include <QObject>
#include <QDateTime>
#include <QMap>
#include <QVector>
#include <QString>

class PumpModel : public QObject
{
    Q_OBJECT

public:
    explicit PumpModel(QObject *parent = nullptr);
    
    enum PumpState {
        PoweredOff,
        PoweredOn,
        Suspended,
        Delivering,
        Error
    };
    
    enum AlertLevel {
        Info,
        Warning,
        Critical
    };
    
    // Battery management
    int getBatteryLevel() const;
    bool isCharging() const;
    void startCharging();
    void stopCharging();
    void updateBatteryLevel(int level);
    
    // Insulin management
    double getInsulinRemaining() const;
    void updateInsulinRemaining(double units);
    void reduceInsulin(double units);
    
    // Pump state
    PumpState getPumpState() const;
    void setPumpState(PumpState state);
    QDateTime getLastActionTime() const;
    QString getCurrentProfileName() const;
    void setCurrentProfileName(const QString &profileName);
    
    // Insulin on Board
    double getInsulinOnBoard() const;
    void updateInsulinOnBoard(double units);
    
    // Control-IQ
    double getControlIQDelivery() const;
    void updateControlIQDelivery(double units);
    
    // Alerts and errors
    void addAlert(const QString &message, AlertLevel level);
    QVector<QPair<QString, AlertLevel>> getActiveAlerts() const;
    void clearAlert(int index);
    
    // Data management
    QVector<QPair<QDateTime, double>> getGlucoseHistory() const;
    QVector<QPair<QDateTime, double>> getInsulinHistory() const;
    void addGlucoseReading(QDateTime timestamp, double value);
    void addInsulinDelivery(QDateTime timestamp, double units);
    
    // Save and load state
    bool saveState(const QString &filename);
    bool loadState(const QString &filename);
    
signals:
    void batteryLevelChanged(int level);
    void insulinRemainingChanged(double units);
    void pumpStateChanged(PumpState state);
    void profileChanged(const QString &profileName);
    void insulinOnBoardChanged(double units);
    void controlIQDeliveryChanged(double units);
    void alertAdded(const QString &message, AlertLevel level);
    void alertCleared(int index);
    void glucoseReadingAdded(QDateTime timestamp, double value);
    void insulinDeliveryAdded(QDateTime timestamp, double units);
    
private:
    int batteryLevel;
    bool charging;
    double insulinRemaining;
    PumpState state;
    QDateTime lastActionTime;
    QString currentProfileName;
    double insulinOnBoard;
    double controlIQDelivery;
    QVector<QPair<QString, AlertLevel>> alerts;
    QVector<QPair<QDateTime, double>> glucoseHistory;
    QVector<QPair<QDateTime, double>> insulinHistory;
    
    void updateLastActionTime();
};

#endif // PUMPMODEL_H
