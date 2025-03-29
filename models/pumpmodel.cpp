#include "pumpmodel.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

PumpModel::PumpModel(QObject *parent)
    : QObject(parent),
      batteryLevel(100),
      charging(false),
      insulinRemaining(300.0),
      state(PoweredOff),
      currentProfileName("Default"),
      insulinOnBoard(0.0),
      controlIQDelivery(0.0)
{
    lastActionTime = QDateTime::currentDateTime();
}

int PumpModel::getBatteryLevel() const
{
    return batteryLevel;
}

bool PumpModel::isCharging() const
{
    return charging;
}

void PumpModel::startCharging()
{
    charging = true;
    updateLastActionTime();
}

void PumpModel::stopCharging()
{
    charging = false;
    updateLastActionTime();
}

void PumpModel::updateBatteryLevel(int level)
{
    if (level < 0) level = 0;
    if (level > 100) level = 100;
    
    if (batteryLevel != level) {
        batteryLevel = level;
        emit batteryLevelChanged(batteryLevel);
        updateLastActionTime();
    }
}

double PumpModel::getInsulinRemaining() const
{
    return insulinRemaining;
}

void PumpModel::updateInsulinRemaining(double units)
{
    if (units < 0) units = 0;
    if (units > 300) units = 300;
    
    if (insulinRemaining != units) {
        insulinRemaining = units;
        emit insulinRemainingChanged(insulinRemaining);
        updateLastActionTime();
    }
}

void PumpModel::reduceInsulin(double units)
{
    if (units <= 0) return;
    
    double newInsulin = insulinRemaining - units;
    if (newInsulin < 0) newInsulin = 0;
    
    updateInsulinRemaining(newInsulin);
    addInsulinDelivery(QDateTime::currentDateTime(), units);
}

PumpModel::PumpState PumpModel::getPumpState() const
{
    return state;
}

void PumpModel::setPumpState(PumpState newState)
{
    if (state != newState) {
        state = newState;
        emit pumpStateChanged(state);
        updateLastActionTime();
    }
}

QDateTime PumpModel::getLastActionTime() const
{
    return lastActionTime;
}

QString PumpModel::getCurrentProfileName() const
{
    return currentProfileName;
}

void PumpModel::setCurrentProfileName(const QString &profileName)
{
    if (currentProfileName != profileName) {
        currentProfileName = profileName;
        emit profileChanged(currentProfileName);
        updateLastActionTime();
    }
}

double PumpModel::getInsulinOnBoard() const
{
    return insulinOnBoard;
}

void PumpModel::updateInsulinOnBoard(double units)
{
    if (units < 0) units = 0;
    
    if (insulinOnBoard != units) {
        insulinOnBoard = units;
        emit insulinOnBoardChanged(insulinOnBoard);
        updateLastActionTime();
    }
}

double PumpModel::getControlIQDelivery() const
{
    return controlIQDelivery;
}

void PumpModel::updateControlIQDelivery(double units)
{
    if (units < 0) units = 0;
    
    if (controlIQDelivery != units) {
        controlIQDelivery = units;
        emit controlIQDeliveryChanged(controlIQDelivery);
        updateLastActionTime();
    }
}

void PumpModel::addAlert(const QString &message, PumpModel::AlertLevel level)
{
    alerts.append(qMakePair(message, level));
    emit alertAdded(message, level);
    updateLastActionTime();
}

QVector<QPair<QString, PumpModel::AlertLevel>> PumpModel::getActiveAlerts() const
{
    return alerts;
}

void PumpModel::clearAlert(int index)
{
    if (index >= 0 && index < alerts.size()) {
        alerts.removeAt(index);
        emit alertCleared(index);
        updateLastActionTime();
    }
}

QVector<QPair<QDateTime, double>> PumpModel::getGlucoseHistory() const
{
    return glucoseHistory;
}

QVector<QPair<QDateTime, double>> PumpModel::getInsulinHistory() const
{
    return insulinHistory;
}

void PumpModel::addGlucoseReading(QDateTime timestamp, double value)
{
    glucoseHistory.append(qMakePair(timestamp, value));
    emit glucoseReadingAdded(timestamp, value);
    updateLastActionTime();
}

void PumpModel::addInsulinDelivery(QDateTime timestamp, double units)
{
    insulinHistory.append(qMakePair(timestamp, units));
    emit insulinDeliveryAdded(timestamp, units);
    updateLastActionTime();
}

bool PumpModel::saveState(const QString &filename)
{
    QJsonObject pumpState;
    
    // Save basic pump data
    pumpState["batteryLevel"] = batteryLevel;
    pumpState["charging"] = charging;
    pumpState["insulinRemaining"] = insulinRemaining;
    pumpState["pumpState"] = state;
    pumpState["lastActionTime"] = lastActionTime.toString(Qt::ISODate);
    pumpState["currentProfileName"] = currentProfileName;
    pumpState["insulinOnBoard"] = insulinOnBoard;
    pumpState["controlIQDelivery"] = controlIQDelivery;
    
    // Save alerts
    QJsonArray alertsArray;
    for (const auto &alert : alerts) {
        QJsonObject alertObj;
        alertObj["message"] = alert.first;
        alertObj["level"] = alert.second;
        alertsArray.append(alertObj);
    }
    pumpState["alerts"] = alertsArray;
    
    // Save glucose history
    QJsonArray glucoseArray;
    for (const auto &reading : glucoseHistory) {
        QJsonObject readingObj;
        readingObj["timestamp"] = reading.first.toString(Qt::ISODate);
        readingObj["value"] = reading.second;
        glucoseArray.append(readingObj);
    }
    pumpState["glucoseHistory"] = glucoseArray;
    
    // Save insulin history
    QJsonArray insulinArray;
    for (const auto &delivery : insulinHistory) {
        QJsonObject deliveryObj;
        deliveryObj["timestamp"] = delivery.first.toString(Qt::ISODate);
        deliveryObj["units"] = delivery.second;
        insulinArray.append(deliveryObj);
    }
    pumpState["insulinHistory"] = insulinArray;
    
    // Write to file
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly)) {
        return false;
    }
    
    QJsonDocument doc(pumpState);
    file.write(doc.toJson());
    file.close();
    
    return true;
}

bool PumpModel::loadState(const QString &filename)
{
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }
    
    QByteArray data = file.readAll();
    file.close();
    
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (doc.isNull() || !doc.isObject()) {
        return false;
    }
    
    QJsonObject pumpState = doc.object();
    
    // Load basic pump data
    batteryLevel = pumpState["batteryLevel"].toInt(100);
    charging = pumpState["charging"].toBool(false);
    insulinRemaining = pumpState["insulinRemaining"].toDouble(300.0);
    state = static_cast<PumpState>(pumpState["pumpState"].toInt(PoweredOff));
    lastActionTime = QDateTime::fromString(pumpState["lastActionTime"].toString(), Qt::ISODate);
    currentProfileName = pumpState["currentProfileName"].toString("Default");
    insulinOnBoard = pumpState["insulinOnBoard"].toDouble(0.0);
    controlIQDelivery = pumpState["controlIQDelivery"].toDouble(0.0);
    
    // Load alerts
    alerts.clear();
    QJsonArray alertsArray = pumpState["alerts"].toArray();
    for (const QJsonValue &value : alertsArray) {
        QJsonObject alertObj = value.toObject();
        QString message = alertObj["message"].toString();
        AlertLevel level = static_cast<AlertLevel>(alertObj["level"].toInt());
        alerts.append(qMakePair(message, level));
    }
    
    // Load glucose history
    glucoseHistory.clear();
    QJsonArray glucoseArray = pumpState["glucoseHistory"].toArray();
    for (const QJsonValue &value : glucoseArray) {
        QJsonObject readingObj = value.toObject();
        QDateTime timestamp = QDateTime::fromString(readingObj["timestamp"].toString(), Qt::ISODate);
        double glucoseValue = readingObj["value"].toDouble();
        glucoseHistory.append(qMakePair(timestamp, glucoseValue));
    }
    
    // Load insulin history
    insulinHistory.clear();
    QJsonArray insulinArray = pumpState["insulinHistory"].toArray();
    for (const QJsonValue &value : insulinArray) {
        QJsonObject deliveryObj = value.toObject();
        QDateTime timestamp = QDateTime::fromString(deliveryObj["timestamp"].toString(), Qt::ISODate);
        double units = deliveryObj["units"].toDouble();
        insulinHistory.append(qMakePair(timestamp, units));
    }
    
    // Emit all signals to update UI
    emit batteryLevelChanged(batteryLevel);
    emit insulinRemainingChanged(insulinRemaining);
    emit pumpStateChanged(state);
    emit profileChanged(currentProfileName);
    emit insulinOnBoardChanged(insulinOnBoard);
    emit controlIQDeliveryChanged(controlIQDelivery);
    
    return true;
}

void PumpModel::updateLastActionTime()
{
    lastActionTime = QDateTime::currentDateTime();
}
