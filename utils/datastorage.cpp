#include "datastorage.h"
#include <QJsonArray>
#include <QJsonObject>
#include <QTextStream>

DataStorage::DataStorage(QObject *parent)
    : QObject(parent)
{
}

bool DataStorage::saveGlucoseData(const QVector<QPair<QDateTime, double>> &data, const QString &filename)
{
    // Create JSON document
    QJsonArray glucoseArray;
    
    for (const auto &reading : data) {
        QJsonObject readingObj;
        readingObj["timestamp"] = reading.first.toString(Qt::ISODate);
        readingObj["value"] = reading.second;
        glucoseArray.append(readingObj);
    }
    
    QJsonObject rootObj;
    rootObj["glucoseReadings"] = glucoseArray;
    QJsonDocument doc(rootObj);
    
    return writeJsonToFile(doc, filename);
}

QVector<QPair<QDateTime, double>> DataStorage::loadGlucoseData(const QString &filename)
{
    QVector<QPair<QDateTime, double>> result;
    
    QJsonDocument doc = readJsonFromFile(filename);
    if (doc.isNull() || !doc.isObject()) {
        return result;
    }
    
    QJsonObject rootObj = doc.object();
    QJsonArray glucoseArray = rootObj["glucoseReadings"].toArray();
    
    for (const QJsonValue &value : glucoseArray) {
        QJsonObject readingObj = value.toObject();
        QDateTime timestamp = QDateTime::fromString(readingObj["timestamp"].toString(), Qt::ISODate);
        double glucoseValue = readingObj["value"].toDouble();
        
        result.append(qMakePair(timestamp, glucoseValue));
    }
    
    return result;
}

bool DataStorage::saveInsulinData(const QVector<QPair<QDateTime, double>> &data, const QString &filename)
{
    // Create JSON document
    QJsonArray insulinArray;
    
    for (const auto &delivery : data) {
        QJsonObject deliveryObj;
        deliveryObj["timestamp"] = delivery.first.toString(Qt::ISODate);
        deliveryObj["units"] = delivery.second;
        insulinArray.append(deliveryObj);
    }
    
    QJsonObject rootObj;
    rootObj["insulinDeliveries"] = insulinArray;
    QJsonDocument doc(rootObj);
    
    return writeJsonToFile(doc, filename);
}

QVector<QPair<QDateTime, double>> DataStorage::loadInsulinData(const QString &filename)
{
    QVector<QPair<QDateTime, double>> result;
    
    QJsonDocument doc = readJsonFromFile(filename);
    if (doc.isNull() || !doc.isObject()) {
        return result;
    }
    
    QJsonObject rootObj = doc.object();
    QJsonArray insulinArray = rootObj["insulinDeliveries"].toArray();
    
    for (const QJsonValue &value : insulinArray) {
        QJsonObject deliveryObj = value.toObject();
        QDateTime timestamp = QDateTime::fromString(deliveryObj["timestamp"].toString(), Qt::ISODate);
        double units = deliveryObj["units"].toDouble();
        
        result.append(qMakePair(timestamp, units));
    }
    
    return result;
}

bool DataStorage::saveBolusHistory(const QVector<BolusRecord> &history, const QString &filename)
{
    // Create JSON document
    QJsonArray bolusArray;
    
    for (const auto &bolus : history) {
        QJsonObject bolusObj;
        bolusObj["timestamp"] = bolus.timestamp.toString(Qt::ISODate);
        bolusObj["units"] = bolus.units;
        bolusObj["reason"] = bolus.reason;
        bolusObj["extended"] = bolus.extended;
        bolusObj["duration"] = bolus.duration;
        bolusObj["completed"] = bolus.completed;
        bolusArray.append(bolusObj);
    }
    
    QJsonObject rootObj;
    rootObj["bolusHistory"] = bolusArray;
    QJsonDocument doc(rootObj);
    
    return writeJsonToFile(doc, filename);
}

QVector<DataStorage::BolusRecord> DataStorage::loadBolusHistory(const QString &filename)
{
    QVector<BolusRecord> result;
    
    QJsonDocument doc = readJsonFromFile(filename);
    if (doc.isNull() || !doc.isObject()) {
        return result;
    }
    
    QJsonObject rootObj = doc.object();
    QJsonArray bolusArray = rootObj["bolusHistory"].toArray();
    
    for (const QJsonValue &value : bolusArray) {
        QJsonObject bolusObj = value.toObject();
        
        BolusRecord bolus;
        bolus.timestamp = QDateTime::fromString(bolusObj["timestamp"].toString(), Qt::ISODate);
        bolus.units = bolusObj["units"].toDouble();
        bolus.reason = bolusObj["reason"].toString();
        bolus.extended = bolusObj["extended"].toBool();
        bolus.duration = bolusObj["duration"].toInt();
        bolus.completed = bolusObj["completed"].toBool();
        
        result.append(bolus);
    }
    
    return result;
}

bool DataStorage::saveBasalHistory(const QVector<BasalRecord> &history, const QString &filename)
{
    // Create JSON document
    QJsonArray basalArray;
    
    for (const auto &basal : history) {
        QJsonObject basalObj;
        basalObj["startTime"] = basal.startTime.toString(Qt::ISODate);
        basalObj["endTime"] = basal.endTime.toString(Qt::ISODate);
        basalObj["rate"] = basal.rate;
        basalObj["profileName"] = basal.profileName;
        basalObj["automatic"] = basal.automatic;
        basalArray.append(basalObj);
    }
    
    QJsonObject rootObj;
    rootObj["basalHistory"] = basalArray;
    QJsonDocument doc(rootObj);
    
    return writeJsonToFile(doc, filename);
}

QVector<DataStorage::BasalRecord> DataStorage::loadBasalHistory(const QString &filename)
{
    QVector<BasalRecord> result;
    
    QJsonDocument doc = readJsonFromFile(filename);
    if (doc.isNull() || !doc.isObject()) {
        return result;
    }
    
    QJsonObject rootObj = doc.object();
    QJsonArray basalArray = rootObj["basalHistory"].toArray();
    
    for (const QJsonValue &value : basalArray) {
        QJsonObject basalObj = value.toObject();
        
        BasalRecord basal;
        basal.startTime = QDateTime::fromString(basalObj["startTime"].toString(), Qt::ISODate);
        basal.endTime = QDateTime::fromString(basalObj["endTime"].toString(), Qt::ISODate);
        basal.rate = basalObj["rate"].toDouble();
        basal.profileName = basalObj["profileName"].toString();
        basal.automatic = basalObj["automatic"].toBool();
        
        result.append(basal);
    }
    
    return result;
}

bool DataStorage::saveProfiles(const QVector<ProfileRecord> &profiles, const QString &filename)
{
    // Create JSON document
    QJsonArray profileArray;
    
    for (const auto &profile : profiles) {
        QJsonObject profileObj;
        profileObj["name"] = profile.name;
        profileObj["basalRate"] = profile.basalRate;
        profileObj["carbRatio"] = profile.carbRatio;
        profileObj["correctionFactor"] = profile.correctionFactor;
        profileObj["targetGlucose"] = profile.targetGlucose;
        profileArray.append(profileObj);
    }
    
    QJsonObject rootObj;
    rootObj["profiles"] = profileArray;
    QJsonDocument doc(rootObj);
    
    return writeJsonToFile(doc, filename);
}

QVector<DataStorage::ProfileRecord> DataStorage::loadProfiles(const QString &filename)
{
    QVector<ProfileRecord> result;
    
    QJsonDocument doc = readJsonFromFile(filename);
    if (doc.isNull() || !doc.isObject()) {
        return result;
    }
    
    QJsonObject rootObj = doc.object();
    QJsonArray profileArray = rootObj["profiles"].toArray();
    
    for (const QJsonValue &value : profileArray) {
        QJsonObject profileObj = value.toObject();
        
        ProfileRecord profile;
        profile.name = profileObj["name"].toString();
        profile.basalRate = profileObj["basalRate"].toDouble();
        profile.carbRatio = profileObj["carbRatio"].toDouble();
        profile.correctionFactor = profileObj["correctionFactor"].toDouble();
        profile.targetGlucose = profileObj["targetGlucose"].toDouble();
        
        result.append(profile);
    }
    
    return result;
}

bool DataStorage::saveEventLog(const QVector<LogEvent> &events, const QString &filename)
{
    // Create JSON document
    QJsonArray eventArray;
    
    for (const auto &event : events) {
        QJsonObject eventObj;
        eventObj["timestamp"] = event.timestamp.toString(Qt::ISODate);
        eventObj["message"] = event.message;
        eventObj["level"] = event.level;
        eventArray.append(eventObj);
    }
    
    QJsonObject rootObj;
    rootObj["events"] = eventArray;
    QJsonDocument doc(rootObj);
    
    return writeJsonToFile(doc, filename);
}

QVector<DataStorage::LogEvent> DataStorage::loadEventLog(const QString &filename)
{
    QVector<LogEvent> result;
    
    QJsonDocument doc = readJsonFromFile(filename);
    if (doc.isNull() || !doc.isObject()) {
        return result;
    }
    
    QJsonObject rootObj = doc.object();
    QJsonArray eventArray = rootObj["events"].toArray();
    
    for (const QJsonValue &value : eventArray) {
        QJsonObject eventObj = value.toObject();
        
        LogEvent event;
        event.timestamp = QDateTime::fromString(eventObj["timestamp"].toString(), Qt::ISODate);
        event.message = eventObj["message"].toString();
        event.level = eventObj["level"].toInt();
        
        result.append(event);
    }
    
    return result;
}

void DataStorage::addLogEvent(const QString &message, int level)
{
    LogEvent event;
    event.timestamp = QDateTime::currentDateTime();
    event.message = message;
    event.level = level;
    
    eventLog.append(event);
    
    // Keep only the last 1000 events
    while (eventLog.size() > 1000) {
        eventLog.removeFirst();
    }
}

// New method to specifically handle adding alert events to history
void DataStorage::addEventLog(const QString &message, int level)
{
    LogEvent event;
    event.timestamp = QDateTime::currentDateTime();
    event.message = message;
    event.level = level;
    
    // Add to log
    eventLog.append(event);
    
    // Keep log to reasonable size (max 1000 entries)
    while (eventLog.size() > 1000) {
        eventLog.removeFirst();
    }
    
    // Auto-save the event log
    saveEventLog(eventLog, QDir::homePath() + "/.tslimx2simulator/event_log.json");
    
    // Emit signal that alert has been logged
    emit eventLogged(message, level);
}

QVector<QPair<QString, double>> DataStorage::calculateDailyStatistics(
    const QDateTime &startDate, 
    const QDateTime &endDate,
    const QVector<QPair<QDateTime, double>> &glucoseData)
{
    QVector<QPair<QString, double>> result;
    
    // Group data by day
    QMap<QString, QVector<double>> dailyValues;
    
    for (const auto &reading : glucoseData) {
        // Skip readings outside date range
        if (reading.first < startDate || reading.first > endDate) {
            continue;
        }
        
        // Group by day
        QString day = reading.first.toString("yyyy-MM-dd");
        dailyValues[day].append(reading.second);
    }
    
    // Calculate daily averages
    for (auto it = dailyValues.constBegin(); it != dailyValues.constEnd(); ++it) {
        const QString &day = it.key();
        const QVector<double> &values = it.value();
        
        if (values.isEmpty()) {
            continue;
        }
        
        // Calculate average
        double sum = 0.0;
        for (double value : values) {
            sum += value;
        }
        double average = sum / values.size();
        
        result.append(qMakePair(day, average));
    }
    
    return result;
}

QVector<QPair<int, double>> DataStorage::calculateHourlyAverages(
    const QDateTime &startDate, 
    const QDateTime &endDate,
    const QVector<QPair<QDateTime, double>> &glucoseData)
{
    QVector<QPair<int, double>> result;
    
    // Group data by hour
    QMap<int, QVector<double>> hourlyValues;
    
    for (const auto &reading : glucoseData) {
        // Skip readings outside date range
        if (reading.first < startDate || reading.first > endDate) {
            continue;
        }
        
        // Group by hour
        int hour = reading.first.time().hour();
        hourlyValues[hour].append(reading.second);
    }
    
    // Calculate hourly averages
    for (int hour = 0; hour < 24; ++hour) {
        const QVector<double> &values = hourlyValues[hour];
        
        if (values.isEmpty()) {
            result.append(qMakePair(hour, 0.0));
            continue;
        }
        
        // Calculate average
        double sum = 0.0;
        for (double value : values) {
            sum += value;
        }
        double average = sum / values.size();
        
        result.append(qMakePair(hour, average));
    }
    
    return result;
}

QString DataStorage::generateCSVReport(
    const QDateTime &startDate, 
    const QDateTime &endDate,
    const QVector<QPair<QDateTime, double>> &glucoseData,
    const QVector<QPair<QDateTime, double>> &insulinData)
{
    QString report;
    QTextStream stream(&report);
    
    // Write header
    stream << "Timestamp,Glucose (mmol/L),Insulin (units)\n";
    
    // Combine data
    QMap<QDateTime, QPair<double, double>> combinedData;
    
    // Add glucose data
    for (const auto &reading : glucoseData) {
        if (reading.first >= startDate && reading.first <= endDate) {
            combinedData[reading.first].first = reading.second;
        }
    }
    
    // Add insulin data
    for (const auto &delivery : insulinData) {
        if (delivery.first >= startDate && delivery.first <= endDate) {
            combinedData[delivery.first].second = delivery.second;
        }
    }
    
    // Write data
    for (auto it = combinedData.constBegin(); it != combinedData.constEnd(); ++it) {
        const QDateTime &timestamp = it.key();
        const QPair<double, double> &values = it.value();
        
        stream << timestamp.toString(Qt::ISODate) << ","
               << QString::number(values.first, 'f', 1) << ","
               << QString::number(values.second, 'f', 2) << "\n";
    }
    
    return report;
}

bool DataStorage::createDirectoryIfNeeded(const QString &path)
{
    QDir dir(path);
    if (!dir.exists()) {
        return dir.mkpath(".");
    }
    return true;
}

bool DataStorage::writeJsonToFile(const QJsonDocument &doc, const QString &filename)
{
    // Create directory if needed
    QFileInfo fileInfo(filename);
    if (!createDirectoryIfNeeded(fileInfo.path())) {
        return false;
    }
    
    // Write file
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly)) {
        return false;
    }
    
    file.write(doc.toJson());
    file.close();
    
    return true;
}

QJsonDocument DataStorage::readJsonFromFile(const QString &filename)
{
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly)) {
        return QJsonDocument();
    }
    
    QByteArray data = file.readAll();
    file.close();
    
    QJsonDocument doc = QJsonDocument::fromJson(data);
    return doc;
}
