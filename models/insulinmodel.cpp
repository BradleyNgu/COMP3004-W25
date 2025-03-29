#include "insulinmodel.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QTimer>

InsulinModel::InsulinModel(QObject *parent)
    : QObject(parent),
      insulinOnBoard(0.0),
      basalActive(false),
      currentBasalRate(0.0),
      currentProfileName(""),
      basalIsAutomatic(false),
      bolusActive(false),
      lastControlIQAdjustment(0.0)
{
    // Setup timer to update IOB every minute
    QTimer *iobTimer = new QTimer(this);
    connect(iobTimer, &QTimer::timeout, this, &InsulinModel::updateIOB);
    iobTimer->start(60000); // 60 seconds
}

double InsulinModel::getInsulinOnBoard() const
{
    return insulinOnBoard;
}

double InsulinModel::getCurrentBasalRate() const
{
    if (!basalActive) {
        return 0.0;
    }
    
    return currentBasalRate;
}

bool InsulinModel::isBolusActive() const
{
    return bolusActive;
}

InsulinModel::BolusDelivery InsulinModel::getCurrentBolus() const
{
    return currentBolus;
}

InsulinModel::BolusDelivery InsulinModel::getLastCompletedBolus() const
{
    return lastCompletedBolus;
}

void InsulinModel::startBasal(double rate, const QString &profileName, bool automatic)
{
    // Cap basal rate for safety
    if (rate < 0.0) rate = 0.0;
    if (rate > 5.0) rate = 5.0;
    
    // Record previous basal segment if active
    if (basalActive) {
        BasalDelivery segment;
        segment.startTime = QDateTime::currentDateTime().addSecs(-3600); // Assume it was running for an hour
        segment.endTime = QDateTime::currentDateTime();
        segment.rate = currentBasalRate;
        segment.profileName = currentProfileName;
        segment.automatic = basalIsAutomatic;
        basalHistory.append(segment);
    }
    
    // Update current state
    currentBasalRate = rate;
    currentProfileName = profileName;
    basalIsAutomatic = automatic;
    basalActive = true;
    
    // Notify
    emit basalRateChanged(rate);
    emit basalStateChanged(true);
}

void InsulinModel::stopBasal()
{
    if (!basalActive) {
        return;
    }
    
    // Record current segment
    BasalDelivery segment;
    segment.startTime = QDateTime::currentDateTime().addSecs(-3600); // Assume it was running for an hour
    segment.endTime = QDateTime::currentDateTime();
    segment.rate = currentBasalRate;
    segment.profileName = currentProfileName;
    segment.automatic = basalIsAutomatic;
    basalHistory.append(segment);
    
    // Update state
    basalActive = false;
    
    // Notify
    emit basalRateChanged(0.0);
    emit basalStateChanged(false);
}

void InsulinModel::suspendBasal()
{
    // Same as stop but with a different name for clarity
    stopBasal();
}

void InsulinModel::resumeBasal()
{
    // If we have a previous basal rate, resume with that
    if (!currentProfileName.isEmpty() && currentBasalRate > 0.0) {
        startBasal(currentBasalRate, currentProfileName, basalIsAutomatic);
    }
}

void InsulinModel::adjustBasalRate(double newRate, bool automatic)
{
    // Cap for safety
    if (newRate < 0.0) newRate = 0.0;
    if (newRate > 5.0) newRate = 5.0;
    
    if (!basalActive) {
        // Start basal if not active
        startBasal(newRate, currentProfileName.isEmpty() ? "Default" : currentProfileName, automatic);
        return;
    }
    
    // Record previous segment
    BasalDelivery segment;
    segment.startTime = QDateTime::currentDateTime().addSecs(-3600); // Assume it was running for an hour
    segment.endTime = QDateTime::currentDateTime();
    segment.rate = currentBasalRate;
    segment.profileName = currentProfileName;
    segment.automatic = basalIsAutomatic;
    basalHistory.append(segment);
    
    // Record the adjustment amount
    double adjustment = newRate - currentBasalRate;
    
    // Update state
    currentBasalRate = newRate;
    basalIsAutomatic = automatic;
    
    // For Control-IQ adjustments
    if (automatic) {
        lastControlIQAdjustment = adjustment;
        emit controlIQAdjustmentChanged(lastControlIQAdjustment);
    }
    
    // Notify
    emit basalRateChanged(newRate);
}

bool InsulinModel::deliverBolus(double units, const QString &reason, bool extended, int duration)
{
    // Validation
    if (bolusActive) return false;
    if (units <= 0.0) return false;
    if (units > 25.0) units = 25.0; // Safety cap
    
    // Setup bolus
    currentBolus.timestamp = QDateTime::currentDateTime();
    currentBolus.units = units;
    currentBolus.reason = reason;
    currentBolus.extended = extended;
    currentBolus.duration = duration;
    currentBolus.completed = false;
    bolusActive = true;
    
    // For standard bolus, complete quickly
    if (!extended) {
        // Deliver bolus after a short delay
        QTimer::singleShot(2000, this, [this]() {
            if (bolusActive) {
                // Mark as complete
                currentBolus.completed = true;
                lastCompletedBolus = currentBolus;
                
                // Add to history
                bolusHistory.append(currentBolus);
                
                // Reset state
                bolusActive = false;
                
                // Update IOB
                updateIOB();
                
                // Notify
                emit bolusCompleted(currentBolus.units);
            }
        });
    } else {
        // Extended bolus simulation
        int intervalMs = duration * 60 * 1000 / 10; // 10 steps
        
        QTimer *timer = new QTimer(this);
        connect(timer, &QTimer::timeout, this, [this, timer]() {
            static int stepsCompleted = 0;
            if (!bolusActive) {
                timer->stop();
                timer->deleteLater();
                return;
            }
            
            stepsCompleted++;
            if (stepsCompleted >= 10) {
                // Mark as complete
                currentBolus.completed = true;
                lastCompletedBolus = currentBolus;
                
                // Add to history
                bolusHistory.append(currentBolus);
                
                // Reset state
                bolusActive = false;
                
                // Clean up
                timer->stop();
                timer->deleteLater();
                stepsCompleted = 0;
                
                // Update IOB
                updateIOB();
                
                // Notify
                emit bolusCompleted(currentBolus.units);
            }
        });
        
        timer->start(intervalMs);
    }
    
    // Notify of bolus start
    emit bolusStarted(units);
    
    // Update IOB
    updateIOB();
    
    return true;
}

bool InsulinModel::cancelBolus()
{
    if (!bolusActive) {
        return false;
    }
    
    // Calculate partial delivery (simplified)
    double delivered = currentBolus.units * 0.5; // Assume half delivered
    
    // Record partial delivery
    BolusDelivery partial = currentBolus;
    partial.units = delivered;
    partial.completed = false;
    
    // Add to history
    bolusHistory.append(partial);
    
    // Save requested amount
    double requested = currentBolus.units;
    
    // Reset state
    bolusActive = false;
    
    // Update IOB
    updateIOB();
    
    // Notify
    emit bolusCancelled(delivered, requested);
    
    return true;
}

QVector<InsulinModel::BolusDelivery> InsulinModel::getBolusHistory(const QDateTime &start, const QDateTime &end) const
{
    QVector<BolusDelivery> result;
    
    for (const auto &bolus : bolusHistory) {
        if (bolus.timestamp >= start && bolus.timestamp <= end) {
            result.append(bolus);
        }
    }
    
    return result;
}

QVector<InsulinModel::BasalDelivery> InsulinModel::getBasalHistory(const QDateTime &start, const QDateTime &end) const
{
    QVector<BasalDelivery> result;
    
    for (const auto &basal : basalHistory) {
        // Include if any part overlaps
        if ((basal.startTime >= start && basal.startTime <= end) ||
            (basal.endTime >= start && basal.endTime <= end) ||
            (basal.startTime <= start && basal.endTime >= end)) {
            result.append(basal);
        }
    }
    
    return result;
}

double InsulinModel::getTotalInsulin(const QDateTime &start, const QDateTime &end) const
{
    return getTotalBasal(start, end) + getTotalBolus(start, end);
}

double InsulinModel::getTotalBasal(const QDateTime &start, const QDateTime &end) const
{
    double total = 0.0;
    
    for (const auto &basal : getBasalHistory(start, end)) {
        // Calculate the overlap period
        QDateTime overlapStart = basal.startTime < start ? start : basal.startTime;
        QDateTime overlapEnd = basal.endTime > end ? end : basal.endTime;
        
        // Calculate hours
        double hours = overlapStart.secsTo(overlapEnd) / 3600.0;
        
        // Add basal insulin
        total += basal.rate * hours;
    }
    
    return total;
}

double InsulinModel::getTotalBolus(const QDateTime &start, const QDateTime &end) const
{
    double total = 0.0;
    
    for (const auto &bolus : getBolusHistory(start, end)) {
        total += bolus.units;
    }
    
    return total;
}

void InsulinModel::addBolusToHistory(const QDateTime &timestamp, double units, const QString &reason, 
                                   bool extended, int duration, bool completed)
{
    BolusDelivery bolus;
    bolus.timestamp = timestamp;
    bolus.units = units;
    bolus.reason = reason;
    bolus.extended = extended;
    bolus.duration = duration;
    bolus.completed = completed;
    
    bolusHistory.append(bolus);
    
    // Update IOB if recent
    if (QDateTime::currentDateTime().secsTo(timestamp) > -14400) { // Within 4 hours
        updateIOB();
    }
}

void InsulinModel::addBasalToHistory(const BasalDelivery &segment)
{
    basalHistory.append(segment);
}

double InsulinModel::getLastControlIQAdjustment() const
{
    return lastControlIQAdjustment;
}

void InsulinModel::updateIOB()
{
    // Simplified IOB calculation - just keep a running total based on recent boluses
    double total = 0.0;
    
    // Get boluses from the last 4 hours
    QDateTime now = QDateTime::currentDateTime();
    QDateTime fourHoursAgo = now.addSecs(-4 * 3600);
    
    for (const auto &bolus : bolusHistory) {
        if (bolus.timestamp >= fourHoursAgo) {
            // Apply a simple linear decay over 4 hours
            int secondsSince = bolus.timestamp.secsTo(now);
            double hoursActive = 4.0;
            double hoursElapsed = secondsSince / 3600.0;
            
            if (hoursElapsed < hoursActive) {
                double remainingFraction = 1.0 - (hoursElapsed / hoursActive);
                total += bolus.units * remainingFraction;
            }
        }
    }
    
    // Include current active bolus if any
    if (bolusActive) {
        total += currentBolus.units * 0.8; // Assume 80% still active
    }
    
    // Update if changed
    if (qAbs(total - insulinOnBoard) > 0.01) {
        insulinOnBoard = total;
        emit insulinOnBoardChanged(insulinOnBoard);
    }
}

bool InsulinModel::saveInsulinData(const QString &filename)
{
    QJsonObject rootObj;
    
    // Save current state
    QJsonObject stateObj;
    stateObj["insulinOnBoard"] = insulinOnBoard;
    stateObj["basalActive"] = basalActive;
    stateObj["currentBasalRate"] = currentBasalRate;
    stateObj["currentProfileName"] = currentProfileName;
    stateObj["basalIsAutomatic"] = basalIsAutomatic;
    stateObj["bolusActive"] = bolusActive;
    stateObj["lastControlIQAdjustment"] = lastControlIQAdjustment;
    rootObj["state"] = stateObj;
    
    // Save current bolus if active
    if (bolusActive) {
        QJsonObject bolusObj;
        bolusObj["timestamp"] = currentBolus.timestamp.toString(Qt::ISODate);
        bolusObj["units"] = currentBolus.units;
        bolusObj["reason"] = currentBolus.reason;
        bolusObj["extended"] = currentBolus.extended;
        bolusObj["duration"] = currentBolus.duration;
        bolusObj["completed"] = currentBolus.completed;
        rootObj["currentBolus"] = bolusObj;
    }
    
    // Save last completed bolus
    QJsonObject lastBolusObj;
    lastBolusObj["timestamp"] = lastCompletedBolus.timestamp.toString(Qt::ISODate);
    lastBolusObj["units"] = lastCompletedBolus.units;
    lastBolusObj["reason"] = lastCompletedBolus.reason;
    lastBolusObj["extended"] = lastCompletedBolus.extended;
    lastBolusObj["duration"] = lastCompletedBolus.duration;
    lastBolusObj["completed"] = lastCompletedBolus.completed;
    rootObj["lastCompletedBolus"] = lastBolusObj;
    
    // Save bolus history
    QJsonArray bolusHistoryArray;
    for (const auto &bolus : bolusHistory) {
        QJsonObject bolusObj;
        bolusObj["timestamp"] = bolus.timestamp.toString(Qt::ISODate);
        bolusObj["units"] = bolus.units;
        bolusObj["reason"] = bolus.reason;
        bolusObj["extended"] = bolus.extended;
        bolusObj["duration"] = bolus.duration;
        bolusObj["completed"] = bolus.completed;
        bolusHistoryArray.append(bolusObj);
    }
    rootObj["bolusHistory"] = bolusHistoryArray;
    
    // Save basal history
    QJsonArray basalHistoryArray;
    for (const auto &basal : basalHistory) {
        QJsonObject basalObj;
        basalObj["startTime"] = basal.startTime.toString(Qt::ISODate);
        basalObj["endTime"] = basal.endTime.toString(Qt::ISODate);
        basalObj["rate"] = basal.rate;
        basalObj["profileName"] = basal.profileName;
        basalObj["automatic"] = basal.automatic;
        basalHistoryArray.append(basalObj);
    }
    rootObj["basalHistory"] = basalHistoryArray;
    
    // Write to file
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly)) {
        return false;
    }
    
    QJsonDocument doc(rootObj);
    file.write(doc.toJson());
    file.close();
    
    return true;
}

bool InsulinModel::loadInsulinData(const QString &filename)
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
    
    QJsonObject rootObj = doc.object();
    
    // Load current state
    QJsonObject stateObj = rootObj["state"].toObject();
    insulinOnBoard = stateObj["insulinOnBoard"].toDouble(0.0);
    basalActive = stateObj["basalActive"].toBool(false);
    currentBasalRate = stateObj["currentBasalRate"].toDouble(0.0);
    currentProfileName = stateObj["currentProfileName"].toString("");
    basalIsAutomatic = stateObj["basalIsAutomatic"].toBool(false);
    bolusActive = stateObj["bolusActive"].toBool(false);
    lastControlIQAdjustment = stateObj["lastControlIQAdjustment"].toDouble(0.0);
    
    // Load current bolus if active
    if (bolusActive) {
        QJsonObject bolusObj = rootObj["currentBolus"].toObject();
        currentBolus.timestamp = QDateTime::fromString(bolusObj["timestamp"].toString(), Qt::ISODate);
        currentBolus.units = bolusObj["units"].toDouble();
        currentBolus.reason = bolusObj["reason"].toString();
        currentBolus.extended = bolusObj["extended"].toBool();
        currentBolus.duration = bolusObj["duration"].toInt();
        currentBolus.completed = bolusObj["completed"].toBool();
    }
    
    // Load last completed bolus
    QJsonObject lastBolusObj = rootObj["lastCompletedBolus"].toObject();
    lastCompletedBolus.timestamp = QDateTime::fromString(lastBolusObj["timestamp"].toString(), Qt::ISODate);
    lastCompletedBolus.units = lastBolusObj["units"].toDouble();
    lastCompletedBolus.reason = lastBolusObj["reason"].toString();
    lastCompletedBolus.extended = lastBolusObj["extended"].toBool();
    lastCompletedBolus.duration = lastBolusObj["duration"].toInt();
    lastCompletedBolus.completed = lastBolusObj["completed"].toBool();
    
    // Load bolus history
    bolusHistory.clear();
    QJsonArray bolusHistoryArray = rootObj["bolusHistory"].toArray();
    for (const QJsonValue &value : bolusHistoryArray) {
        QJsonObject bolusObj = value.toObject();
        
        BolusDelivery bolus;
        bolus.timestamp = QDateTime::fromString(bolusObj["timestamp"].toString(), Qt::ISODate);
        bolus.units = bolusObj["units"].toDouble();
        bolus.reason = bolusObj["reason"].toString();
        bolus.extended = bolusObj["extended"].toBool();
        bolus.duration = bolusObj["duration"].toInt();
        bolus.completed = bolusObj["completed"].toBool();
        
        bolusHistory.append(bolus);
    }
    
    // Load basal history
    basalHistory.clear();
    QJsonArray basalHistoryArray = rootObj["basalHistory"].toArray();
    for (const QJsonValue &value : basalHistoryArray) {
        QJsonObject basalObj = value.toObject();
        
        BasalDelivery basal;
        basal.startTime = QDateTime::fromString(basalObj["startTime"].toString(), Qt::ISODate);
        basal.endTime = QDateTime::fromString(basalObj["endTime"].toString(), Qt::ISODate);
        basal.rate = basalObj["rate"].toDouble();
        basal.profileName = basalObj["profileName"].toString();
        basal.automatic = basalObj["automatic"].toBool();
        
        basalHistory.append(basal);
    }
    
    // Emit signals to update UI
    emit insulinOnBoardChanged(insulinOnBoard);
    emit basalRateChanged(currentBasalRate);
    emit basalStateChanged(basalActive);
    emit controlIQAdjustmentChanged(lastControlIQAdjustment);
    
    return true;
}
