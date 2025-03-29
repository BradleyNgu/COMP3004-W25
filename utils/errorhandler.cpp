#include "errorhandler.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QDir>
#include <QTextStream>

ErrorHandler::ErrorHandler(QObject *parent)
    : QObject(parent)
{
}

void ErrorHandler::logError(const QString &message, const QString &source, ErrorLevel level)
{
    // Create new error record
    ErrorRecord error;
    error.timestamp = QDateTime::currentDateTime();
    error.message = message;
    error.source = source;
    error.level = level;
    error.acknowledged = false;
    
    // Add to log
    errorLog.append(error);
    
    // Emit signals
    emit errorLogged(message, level);
    
    // For critical errors, emit special signal
    if (level == Critical) {
        emit criticalErrorDetected(message);
    }
    
    // Keep log to reasonable size (max 1000 entries)
    while (errorLog.size() > 1000) {
        errorLog.removeFirst();
    }
    
    // For debugging
    qDebug("[%s] %s: %s", 
           qPrintable(getErrorLevelString(level)),
           qPrintable(source),
           qPrintable(message));
    
    // Auto-save error log
    saveErrorLog(QDir::homePath() + "/.tslimx2simulator/error_log.json");
    
    // Add to history manager if available
    if (historyManager) {
        historyManager->addEventLog(message, level);
    }
}

// Specific alert methods as required
void ErrorHandler::lowBatteryAlert(int batteryLevel)
{
    QString message;
    ErrorLevel level;
    
    if (batteryLevel <= 5) {
        message = QString("BATTERY CRITICALLY LOW: %1% remaining").arg(batteryLevel);
        level = Critical;
    } else if (batteryLevel <= 20) {
        message = QString("Battery low: %1% remaining").arg(batteryLevel);
        level = Warning;
    } else {
        return; // No alert needed
    }
    
    logError(message, "BatteryManager", level);
}

void ErrorHandler::lowInsulinAlert(double insulinUnits)
{
    QString message;
    ErrorLevel level;
    
    if (insulinUnits <= 10.0) {
        message = QString("INSULIN CRITICALLY LOW: %1 units remaining").arg(insulinUnits, 0, 'f', 1);
        level = Critical;
    } else if (insulinUnits <= 50.0) {
        message = QString("Insulin low: %1 units remaining").arg(insulinUnits, 0, 'f', 1);
        level = Warning;
    } else {
        return; // No alert needed
    }
    
    logError(message, "InsulinManager", level);
}

void ErrorHandler::cgmDisconnectedAlert(int minutesSinceLastReading)
{
    QString message = QString("CGM data gap: No readings for %1 minutes").arg(minutesSinceLastReading);
    logError(message, "GlucoseModel", Warning);
}

void ErrorHandler::occlusionAlert()
{
    QString message = "OCCLUSION DETECTED: Check infusion set for blockages";
    logError(message, "PumpModel", Critical);
}

void ErrorHandler::highGlucoseAlert(double glucoseValue)
{
    QString message;
    ErrorLevel level;
    
    if (glucoseValue >= 13.9) {
        message = QString("URGENT HIGH GLUCOSE: %1 mmol/L").arg(glucoseValue, 0, 'f', 1);
        level = Critical;
    } else if (glucoseValue > 10.0) {
        message = QString("High glucose: %1 mmol/L").arg(glucoseValue, 0, 'f', 1);
        level = Warning;
    } else {
        return; // No alert needed
    }
    
    logError(message, "GlucoseModel", level);
}

void ErrorHandler::lowGlucoseAlert(double glucoseValue)
{
    QString message;
    ErrorLevel level;
    
    if (glucoseValue <= 3.1) {
        message = QString("URGENT LOW GLUCOSE: %1 mmol/L").arg(glucoseValue, 0, 'f', 1);
        level = Critical;
    } else if (glucoseValue < 3.9) {
        message = QString("Low glucose: %1 mmol/L").arg(glucoseValue, 0, 'f', 1);
        level = Warning;
    } else {
        return; // No alert needed
    }
    
    logError(message, "GlucoseModel", level);
}

void ErrorHandler::provideTroubleshootingGuidance(const QString &errorCode)
{
    QString guidance;
    
    // Lookup error code and provide appropriate guidance
    if (errorCode.contains("OCCLUSION", Qt::CaseInsensitive)) {
        guidance = "Check your infusion site for blockages. Remove and replace infusion set if needed.";
    } else if (errorCode.contains("BATTERY", Qt::CaseInsensitive)) {
        guidance = "Connect pump to charger immediately. If problem persists, contact support.";
    } else if (errorCode.contains("INSULIN", Qt::CaseInsensitive)) {
        guidance = "Replace insulin cartridge soon. Ensure you have backup supplies available.";
    } else if (errorCode.contains("CGM", Qt::CaseInsensitive)) {
        guidance = "Check CGM sensor connection. Move pump closer to sensor or replace sensor if needed.";
    } else if (errorCode.contains("GLUCOSE", Qt::CaseInsensitive)) {
        guidance = "Check blood glucose with finger stick. Take corrective action according to treatment plan.";
    } else {
        guidance = "If issue persists, contact Tandem Diabetes support for assistance.";
    }
    
    // Log the guidance provided
    logError(QString("GUIDANCE: %1").arg(guidance), "SupportSystem", Info);
}

void ErrorHandler::contactSupportPrompt(const QString &errorCode)
{
    QString message = QString("Critical error %1 detected. Please contact Tandem Diabetes support at 1-877-801-6901").arg(errorCode);
    logError(message, "SupportSystem", Critical);
}

bool ErrorHandler::acknowledgeError(int index)
{
    if (index < 0 || index >= errorLog.size()) {
        return false;
    }
    
    // Mark as acknowledged
    errorLog[index].acknowledged = true;
    
    emit errorAcknowledged(index);
    
    return true;
}

void ErrorHandler::acknowledgeAllErrors()
{
    for (int i = 0; i < errorLog.size(); ++i) {
        errorLog[i].acknowledged = true;
    }
    
    emit allErrorsAcknowledged();
}

QVector<ErrorHandler::ErrorRecord> ErrorHandler::getAllErrors() const
{
    return errorLog;
}

QVector<ErrorHandler::ErrorRecord> ErrorHandler::getActiveErrors() const
{
    QVector<ErrorRecord> activeErrors;
    
    for (const auto &error : errorLog) {
        if (!error.acknowledged) {
            activeErrors.append(error);
        }
    }
    
    return activeErrors;
}

QVector<ErrorHandler::ErrorRecord> ErrorHandler::getErrorsOfLevel(ErrorLevel level) const
{
    QVector<ErrorRecord> filteredErrors;
    
    for (const auto &error : errorLog) {
        if (error.level == level) {
            filteredErrors.append(error);
        }
    }
    
    return filteredErrors;
}

int ErrorHandler::getErrorCount() const
{
    return errorLog.size();
}

int ErrorHandler::getActiveErrorCount() const
{
    int count = 0;
    
    for (const auto &error : errorLog) {
        if (!error.acknowledged) {
            count++;
        }
    }
    
    return count;
}

bool ErrorHandler::hasCriticalErrors() const
{
    for (const auto &error : errorLog) {
        if (error.level == Critical && !error.acknowledged) {
            return true;
        }
    }
    
    return false;
}

void ErrorHandler::clearAllErrors()
{
    errorLog.clear();
}

bool ErrorHandler::attemptRecovery(int errorIndex)
{
    if (errorIndex < 0 || errorIndex >= errorLog.size()) {
        return false;
    }
    
    ErrorRecord &error = errorLog[errorIndex];
    
    // Check if we can recover
    if (!canRecover(errorIndex)) {
        return false;
    }
    
    // Attempt recovery based on error source and message
    bool recovered = false;
    
    if (error.source == "GlucoseModel" && 
        error.message.contains("CGM connection lost")) {
        // Simulate CGM reconnection
        recovered = true;
    } else if (error.source == "PumpModel" && 
               error.message.contains("Occlusion detected")) {
        // Cannot automatically recover from occlusion
        recovered = false;
    } else if (error.source == "InsulinModel" && 
               error.message.contains("Bolus interrupted")) {
        // Mark as recovered but user needs to restart bolus
        recovered = true;
    } else if (error.source == "BatteryModel" && 
               error.message.contains("Low battery")) {
        // Cannot automatically recover from low battery
        recovered = false;
    } else {
        // Generic recovery attempt for other errors
        recovered = (error.level != Critical);
    }
    
    if (recovered) {
        error.acknowledged = true;
        emit errorRecovered(errorIndex);
    }
    
    return recovered;
}

bool ErrorHandler::canRecover(int errorIndex) const
{
    if (errorIndex < 0 || errorIndex >= errorLog.size()) {
        return false;
    }
    
    const ErrorRecord &error = errorLog[errorIndex];
    
    // Critical errors generally cannot be auto-recovered
    if (error.level == Critical) {
        return false;
    }
    
    // Check specific error types
    if (error.source == "GlucoseModel" && 
        error.message.contains("CGM connection lost")) {
        return true;
    } else if (error.source == "PumpModel" && 
               error.message.contains("Occlusion detected")) {
        return false; // Requires physical intervention
    } else if (error.source == "InsulinModel" && 
               error.message.contains("Bolus interrupted")) {
        return true;
    } else if (error.source == "BatteryModel" && 
               error.message.contains("Low battery")) {
        return false; // Requires charging
    }
    
    // Default based on error level
    return (error.level != Critical);
}

QString ErrorHandler::getRecoveryInstructions(int errorIndex) const
{
    if (errorIndex < 0 || errorIndex >= errorLog.size()) {
        return "No recovery instructions available.";
    }
    
    const ErrorRecord &error = errorLog[errorIndex];
    
    // Return instructions based on error source and message
    if (error.source == "GlucoseModel" && 
        error.message.contains("CGM connection lost")) {
        return "Check CGM sensor connection and move pump closer to sensor.";
    } else if (error.source == "PumpModel" && 
               error.message.contains("Occlusion detected")) {
        return "Check infusion set for kinks or blockages. Replace infusion set if necessary.";
    } else if (error.source == "InsulinModel" && 
               error.message.contains("Bolus interrupted")) {
        return "Restart bolus delivery if needed. Check insulin reservoir.";
    } else if (error.source == "BatteryModel" && 
               error.message.contains("Low battery")) {
        return "Connect pump to charger immediately.";
    }
    
    // Default instructions
    switch (error.level) {
        case Info:
            return "No action required.";
        case Warning:
            return "Acknowledge the warning and monitor the situation.";
        case Error:
            return "Review pump settings and status. Contact support if problem persists.";
        case Critical:
            return "Stop using the pump and contact support immediately.";
        default:
            return "No recovery instructions available.";
    }
}

QString ErrorHandler::getErrorLevelString(ErrorLevel level) const
{
    switch (level) {
        case Info:
            return "INFO";
        case Warning:
            return "WARNING";
        case Error:
            return "ERROR";
        case Critical:
            return "CRITICAL";
        default:
            return "UNKNOWN";
    }
}

QString ErrorHandler::generateErrorReport() const
{
    QString report;
    QTextStream stream(&report);
    
    stream << "ERROR REPORT - " << QDateTime::currentDateTime().toString(Qt::ISODate) << "\n";
    stream << "================================================\n\n";
    
    for (int i = 0; i < errorLog.size(); ++i) {
        const ErrorRecord &error = errorLog[i];
        
        stream << i << ". [" << getErrorLevelString(error.level) << "] "
               << error.timestamp.toString(Qt::ISODate) << "\n";
        stream << "   Source: " << error.source << "\n";
        stream << "   Message: " << error.message << "\n";
        stream << "   Status: " << (error.acknowledged ? "Acknowledged" : "Active") << "\n";
        stream << "\n";
    }
    
    return report;
}

bool ErrorHandler::saveErrorLog(const QString &filename) const
{
    // Create directory if needed
    QFileInfo fileInfo(filename);
    QDir dir(fileInfo.path());
    if (!dir.exists()) {
        dir.mkpath(".");
    }
    
    // Create JSON document
    QJsonArray errorArray;
    
    for (const auto &error : errorLog) {
        QJsonObject errorObj;
        errorObj["timestamp"] = error.timestamp.toString(Qt::ISODate);
        errorObj["message"] = error.message;
        errorObj["source"] = error.source;
        errorObj["level"] = static_cast<int>(error.level);
        errorObj["acknowledged"] = error.acknowledged;
        errorArray.append(errorObj);
    }
    
    QJsonObject rootObj;
    rootObj["errors"] = errorArray;
    QJsonDocument doc(rootObj);
    
    // Write file
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly)) {
        return false;
    }
    
    file.write(doc.toJson());
    file.close();
    
    return true;
}

bool ErrorHandler::loadErrorLog(const QString &filename)
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
    
    // Clear current log
    errorLog.clear();
    
    // Load errors
    QJsonArray errorArray = rootObj["errors"].toArray();
    for (const QJsonValue &value : errorArray) {
        QJsonObject errorObj = value.toObject();
        
        ErrorRecord error;
        error.timestamp = QDateTime::fromString(errorObj["timestamp"].toString(), Qt::ISODate);
        error.message = errorObj["message"].toString();
        error.source = errorObj["source"].toString();
        error.level = static_cast<ErrorLevel>(errorObj["level"].toInt());
        error.acknowledged = errorObj["acknowledged"].toBool();
        
        errorLog.append(error);
    }
    
    return true;
}

void ErrorHandler::setHistoryManager(DataStorage* history)
{
    historyManager = history;
}
