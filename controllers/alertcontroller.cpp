#include "alertcontroller.h"
#include <QApplication>
//#include <QSound>

AlertController::AlertController(QObject *parent)
    : QObject(parent),
      pumpModel(nullptr),
      glucoseModel(nullptr),
      insulinModel(nullptr),
      lowGlucoseThreshold(3.9),
      highGlucoseThreshold(10.0),
      urgentLowGlucoseThreshold(3.1),
      urgentHighGlucoseThreshold(13.9),
      lowInsulinThreshold(50.0),
      criticalLowInsulinThreshold(10.0),
      lowBatteryThreshold(20),
      criticalLowBatteryThreshold(5),
      alertsEnabled(true)
{
    // Setup alert monitoring timer
    alertTimer = new QTimer(this);
    connect(alertTimer, &QTimer::timeout, this, [this]() {
        if (alertsEnabled) {
            checkGlucoseAlerts();
            checkInsulinAlerts();
            checkBatteryAlerts();
            checkMiscAlerts();
        }
    });
}

void AlertController::setPumpModel(PumpModel *model)
{
    pumpModel = model;
    
    if (pumpModel) {
        connect(pumpModel, &PumpModel::alertAdded, this, [this](const QString &message, PumpModel::AlertLevel level) {
            addAlert(message, level);
        });
    }
}

void AlertController::setGlucoseModel(GlucoseModel *model)
{
    glucoseModel = model;
    
    if (glucoseModel) {
        connect(glucoseModel, &GlucoseModel::newReading, this, [this](double value, const QDateTime &) {
            checkGlucoseAlerts();
        });
    }
}

void AlertController::setInsulinModel(InsulinModel *model)
{
    insulinModel = model;
}

void AlertController::addAlert(const QString &message, PumpModel::AlertLevel level, bool autoAcknowledge)
{
    // Don't add duplicate alerts
    if (isAlertActive(message)) {
        return;
    }
    
    // Add to active alerts
    activeAlerts.append(qMakePair(message, level));
    alertTimes.append(QDateTime::currentDateTime());
    
    // Notify
    emit alertAdded(message, level);
    
    // For critical alerts, emit special signal
    if (level == PumpModel::Critical) {
        emit criticalAlertActive(message);
    }
    
    // Auto-acknowledge if requested (for info alerts)
    if (autoAcknowledge && level == PumpModel::Info) {
        QTimer::singleShot(5000, this, [this, message]() {
            for (int i = 0; i < activeAlerts.size(); ++i) {
                if (activeAlerts[i].first == message) {
                    acknowledgeAlert(i);
                    break;
                }
            }
        });
    }
}

bool AlertController::acknowledgeAlert(int index)
{
    if (index < 0 || index >= activeAlerts.size()) {
        return false;
    }
    
    // Remove from active alerts
    activeAlerts.removeAt(index);
    alertTimes.removeAt(index);
    
    emit alertAcknowledged(index);
    
    return true;
}

void AlertController::acknowledgeAllAlerts()
{
    activeAlerts.clear();
    alertTimes.clear();
    
    emit allAlertsAcknowledged();
}

QVector<QPair<QString, PumpModel::AlertLevel>> AlertController::getActiveAlerts() const
{
    return activeAlerts;
}

bool AlertController::hasActiveAlerts() const
{
    return !activeAlerts.isEmpty();
}

bool AlertController::hasCriticalAlerts() const
{
    for (const auto &alert : activeAlerts) {
        if (alert.second == PumpModel::Critical) {
            return true;
        }
    }
    
    return false;
}

void AlertController::setGlucoseAlertThresholds(double lowThreshold, double highThreshold, double urgentLowThreshold, double urgentHighThreshold)
{
    lowGlucoseThreshold = lowThreshold;
    highGlucoseThreshold = highThreshold;
    urgentLowGlucoseThreshold = urgentLowThreshold;
    urgentHighGlucoseThreshold = urgentHighThreshold;
}

void AlertController::setInsulinAlertThresholds(double lowInsulinThreshold, double criticalLowInsulinThreshold)
{
    this->lowInsulinThreshold = lowInsulinThreshold;
    this->criticalLowInsulinThreshold = criticalLowInsulinThreshold;
}

void AlertController::setBatteryAlertThresholds(int lowBatteryThreshold, int criticalLowBatteryThreshold)
{
    this->lowBatteryThreshold = lowBatteryThreshold;
    this->criticalLowBatteryThreshold = criticalLowBatteryThreshold;
}

void AlertController::enableAlerts(bool enable)
{
    alertsEnabled = enable;
}

bool AlertController::areAlertsEnabled() const
{
    return alertsEnabled;
}

void AlertController::startMonitoring()
{
    alertTimer->start(60000); // Check every minute
}

void AlertController::stopMonitoring()
{
    alertTimer->stop();
}

void AlertController::checkGlucoseAlerts()
{
    if (!glucoseModel || !alertsEnabled) {
        return;
    }
    
    double currentGlucose = glucoseModel->getCurrentGlucose();
    
    // Check for urgent low glucose (highest priority)
    if (currentGlucose <= urgentLowGlucoseThreshold) {
        addAlert("URGENT LOW GLUCOSE: " + QString::number(currentGlucose, 'f', 1) + " mmol/L", PumpModel::Critical);
    }
    // Check for low glucose
    else if (currentGlucose < lowGlucoseThreshold) {
        addAlert("Low glucose: " + QString::number(currentGlucose, 'f', 1) + " mmol/L", PumpModel::Warning);
    }
    // Check for urgent high glucose
    else if (currentGlucose >= urgentHighGlucoseThreshold) {
        addAlert("URGENT HIGH GLUCOSE: " + QString::number(currentGlucose, 'f', 1) + " mmol/L", PumpModel::Critical);
    }
    // Check for high glucose
    else if (currentGlucose > highGlucoseThreshold) {
        addAlert("High glucose: " + QString::number(currentGlucose, 'f', 1) + " mmol/L", PumpModel::Warning);
    }
    
    // Check for rapid changes in glucose
    GlucoseModel::TrendDirection trend = glucoseModel->getTrendDirection();
    
    if (trend == GlucoseModel::RisingQuickly) {
        addAlert("Glucose rising quickly", PumpModel::Warning);
    } else if (trend == GlucoseModel::FallingQuickly) {
        addAlert("Glucose falling quickly", PumpModel::Warning);
    }
}

void AlertController::checkInsulinAlerts()
{
    if (!pumpModel || !alertsEnabled) {
        return;
    }
    
    double insulinRemaining = pumpModel->getInsulinRemaining();
    
    // Check for critically low insulin
    if (insulinRemaining <= criticalLowInsulinThreshold) {
        addAlert("INSULIN CRITICALLY LOW: " + QString::number(insulinRemaining, 'f', 1) + " units remaining", PumpModel::Critical);
    }
    // Check for low insulin
    else if (insulinRemaining <= lowInsulinThreshold) {
        addAlert("Insulin low: " + QString::number(insulinRemaining, 'f', 1) + " units remaining", PumpModel::Warning);
    }
}

void AlertController::checkBatteryAlerts()
{
    if (!pumpModel || !alertsEnabled) {
        return;
    }
    
    int batteryLevel = pumpModel->getBatteryLevel();
    
    // Check for critically low battery
    if (batteryLevel <= criticalLowBatteryThreshold) {
        addAlert("BATTERY CRITICALLY LOW: " + QString::number(batteryLevel) + "% remaining", PumpModel::Critical);
    }
    // Check for low battery
    else if (batteryLevel <= lowBatteryThreshold) {
        addAlert("Battery low: " + QString::number(batteryLevel) + "% remaining", PumpModel::Warning);
    }
}

void AlertController::checkMiscAlerts()
{
    if (!pumpModel || !glucoseModel || !insulinModel || !alertsEnabled) {
        return;
    }
    
    // Check for CGM data gap (no readings for over 10 mins)
    QDateTime lastReadingTime = glucoseModel->getLastReadingTime();
    if (lastReadingTime.isValid() && lastReadingTime.secsTo(QDateTime::currentDateTime()) > 600) {
        addAlert("CGM data gap: No readings for " + 
                 QString::number(lastReadingTime.secsTo(QDateTime::currentDateTime()) / 60) + 
                 " minutes", PumpModel::Warning);
    }
    
    // Check for active bolus that's running longer than expected
    if (insulinModel->isBolusActive()) {
        InsulinModel::BolusDelivery currentBolus = insulinModel->getCurrentBolus();
        int expectedDuration = currentBolus.extended ? currentBolus.duration : 1; // 1 minute for standard bolus
        
        // If bolus is running more than 2 minutes longer than expected
        if (currentBolus.timestamp.secsTo(QDateTime::currentDateTime()) > (expectedDuration + 2) * 60) {
            addAlert("Bolus delivery taking longer than expected", PumpModel::Warning);
        }
    }
}

bool AlertController::isAlertActive(const QString &message) const
{
    for (const auto &alert : activeAlerts) {
        if (alert.first == message) {
            return true;
        }
    }
    
    return false;
}
