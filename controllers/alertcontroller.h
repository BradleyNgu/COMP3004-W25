#ifndef ALERTCONTROLLER_H
#define ALERTCONTROLLER_H

#include <QObject>
#include <QTimer>
#include <QVector>
#include <QPair>
#include <QDateTime>
#include "../models/pumpmodel.h"
#include "../models/glucosemodel.h"
#include "../models/insulinmodel.h"

class AlertController : public QObject
{
    Q_OBJECT

public:
    explicit AlertController(QObject *parent = nullptr);
    
    // Set models
    void setPumpModel(PumpModel *model);
    void setGlucoseModel(GlucoseModel *model);
    void setInsulinModel(InsulinModel *model);
    
    // Alert management
    void addAlert(const QString &message, PumpModel::AlertLevel level, bool autoAcknowledge = false);
    bool acknowledgeAlert(int index);
    void acknowledgeAllAlerts();
    QVector<QPair<QString, PumpModel::AlertLevel>> getActiveAlerts() const;
    bool hasActiveAlerts() const;
    bool hasCriticalAlerts() const;
    
    // Alert settings
    void setGlucoseAlertThresholds(double lowThreshold, double highThreshold, double urgentLowThreshold, double urgentHighThreshold);
    void setInsulinAlertThresholds(double lowInsulinThreshold, double criticalLowInsulinThreshold);
    void setBatteryAlertThresholds(int lowBatteryThreshold, int criticalLowBatteryThreshold);
    void enableAlerts(bool enable);
    bool areAlertsEnabled() const;
    
    // Alert monitoring
    void startMonitoring();
    void stopMonitoring();
    
public slots:
    void checkGlucoseAlerts();
    void checkInsulinAlerts();
    void checkBatteryAlerts();
    void checkMiscAlerts();
    
signals:
    void alertAdded(const QString &message, PumpModel::AlertLevel level);
    void alertAcknowledged(int index);
    void allAlertsAcknowledged();
    void criticalAlertActive(const QString &message);
    
private:
    PumpModel *pumpModel;
    GlucoseModel *glucoseModel;
    InsulinModel *insulinModel;
    
    QVector<QPair<QString, PumpModel::AlertLevel>> activeAlerts;
    QVector<QDateTime> alertTimes;
    
    double lowGlucoseThreshold;
    double highGlucoseThreshold;
    double urgentLowGlucoseThreshold;
    double urgentHighGlucoseThreshold;
    
    double lowInsulinThreshold;
    double criticalLowInsulinThreshold;
    
    int lowBatteryThreshold;
    int criticalLowBatteryThreshold;
    
    bool alertsEnabled;
    
    QTimer *alertTimer;
    
    bool isAlertActive(const QString &message) const;
};

#endif // ALERTCONTROLLER_H
