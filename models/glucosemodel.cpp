#include "glucosemodel.h"
#include <QRandomGenerator>
#include <QtMath>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

GlucoseModel::GlucoseModel(QObject *parent)
    : QObject(parent),
      currentTrend(Stable)
{
    // Generate 48 hours of data on startup
    generateFixedPattern(48);
}

double GlucoseModel::getCurrentGlucose() const
{
    if (readings.isEmpty()) {
        return 5.5; // Default value
    }
    
    return readings.last().second;
}

QDateTime GlucoseModel::getLastReadingTime() const
{
    if (readings.isEmpty()) {
        return QDateTime::currentDateTime();
    }
    
    return readings.last().first;
}

GlucoseModel::TrendDirection GlucoseModel::getTrendDirection() const
{
    return currentTrend;
}

void GlucoseModel::forceTrend(TrendDirection trend)
{
    currentTrend = trend;
    emit trendDirectionChanged(currentTrend);
}

QVector<QPair<QDateTime, double>> GlucoseModel::getReadings(const QDateTime &start, const QDateTime &end) const
{
    QVector<QPair<QDateTime, double>> result;
    
    for (const auto &reading : readings) {
        if (reading.first >= start && reading.first <= end) {
            result.append(reading);
        }
    }
    
    return result;
}

void GlucoseModel::generateFixedPattern(int hoursBack)
{
    QDateTime current = QDateTime::currentDateTime();
    QDateTime start = current.addSecs(-hoursBack * 3600);
    
    // Clear existing readings
    readings.clear();
    
    // Generate readings every 5 minutes
    const int intervalMinutes = 5;
    QDateTime timestamp = start;
    
    while (timestamp <= current) {
        // Base sine wave with 3-hour period, centered at 7.0 mmol/L with amplitude of 3.0
        double hours = start.secsTo(timestamp) / 3600.0;
        double dayFraction = fmod(hours, 24.0) / 24.0; // 0-1 over the day
        
        // Base value following a sine wave with 3-hour period
        double baseValue = 7.0 + 3.0 * qSin(hours / 3.0 * 2 * M_PI);
        
        // Add meal spikes at fixed times
        double mealSpike = 0.0;
        
        // Breakfast spike around 7-9 AM
        if (dayFraction >= 7.0/24.0 && dayFraction < 9.0/24.0) {
            double mealProgress = (dayFraction - 7.0/24.0) / (2.0/24.0); // 0-1 during meal period
            if (mealProgress < 0.5) {
                mealSpike = mealProgress * 4.0; // Rise up to +4.0
            } else {
                mealSpike = 4.0 * (1.0 - (mealProgress - 0.5) * 2.0); // Fall back down
            }
        }
        
        // Lunch spike around 12-2 PM
        if (dayFraction >= 12.0/24.0 && dayFraction < 14.0/24.0) {
            double mealProgress = (dayFraction - 12.0/24.0) / (2.0/24.0); // 0-1 during meal period
            if (mealProgress < 0.5) {
                mealSpike = mealProgress * 4.5; // Rise up to +4.5
            } else {
                mealSpike = 4.5 * (1.0 - (mealProgress - 0.5) * 2.0); // Fall back down
            }
        }
        
        // Dinner spike around 6-8 PM
        if (dayFraction >= 18.0/24.0 && dayFraction < 20.0/24.0) {
            double mealProgress = (dayFraction - 18.0/24.0) / (2.0/24.0); // 0-1 during meal period
            if (mealProgress < 0.5) {
                mealSpike = mealProgress * 5.0; // Rise up to +5.0
            } else {
                mealSpike = 5.0 * (1.0 - (mealProgress - 0.5) * 2.0); // Fall back down
            }
        }
        
        // Small random variation
        double noise = (QRandomGenerator::global()->generateDouble() - 0.5) * 0.4;
        
        // Final glucose value
        double glucoseValue = baseValue + mealSpike + noise;
        
        // Ensure it's in a realistic range
        glucoseValue = qBound(2.8, glucoseValue, 20.0);
        
        // Add the reading
        readings.append(qMakePair(timestamp, glucoseValue));
        
        // Move to next sample time
        timestamp = timestamp.addSecs(intervalMinutes * 60);
    }
    
    // Calculate trend based on most recent readings
    calculateTrendDirection();
    
    // Notify about the new data
    if (!readings.isEmpty()) {
        emit newReading(readings.last().second, readings.last().first);
        emit trendDirectionChanged(currentTrend);
    }
}

void GlucoseModel::addReading(double value, const QDateTime &timestamp)
{
    // Add the new reading
    readings.append(qMakePair(timestamp, value));
    
    // Keep history to a reasonable size (24 hours at 5-minute intervals = 288 readings)
    while (readings.size() > 288) {
        readings.removeFirst();
    }
    
    // Update the trend direction
    calculateTrendDirection();
    
    // Notify of the new reading
    emit newReading(value, timestamp);
    emit trendDirectionChanged(currentTrend);
}

void GlucoseModel::clearReadings()
{
    readings.clear();
    currentTrend = Unknown;
    emit trendDirectionChanged(currentTrend);
}

void GlucoseModel::calculateTrendDirection()
{
    // Need at least 3 readings to calculate trend
    if (readings.size() < 3) {
        currentTrend = Stable;
        return;
    }
    
    // Get the most recent 3 readings
    QVector<QPair<QDateTime, double>> recent;
    for (int i = readings.size() - 3; i < readings.size(); i++) {
        recent.append(readings[i]);
    }
    
    // Calculate simple linear regression slope
    double sumX = 0, sumY = 0, sumXY = 0, sumX2 = 0;
    double firstTime = recent[0].first.toSecsSinceEpoch();
    
    for (int i = 0; i < recent.size(); i++) {
        double x = recent[i].first.toSecsSinceEpoch() - firstTime;
        double y = recent[i].second;
        
        sumX += x;
        sumY += y;
        sumXY += x * y;
        sumX2 += x * x;
    }
    
    int n = recent.size();
    double slope = (n * sumXY - sumX * sumY) / (n * sumX2 - sumX * sumX);
    
    // Determine trend based on slope
    if (slope > 0.05) {
        currentTrend = RisingQuickly;
    } else if (slope > 0.02) {
        currentTrend = Rising;
    } else if (slope < -0.05) {
        currentTrend = FallingQuickly;
    } else if (slope < -0.02) {
        currentTrend = Falling;
    } else {
        currentTrend = Stable;
    }
}

bool GlucoseModel::saveReadings(const QString &filename)
{
    QJsonObject rootObj;
    
    // Save current trend
    rootObj["currentTrend"] = static_cast<int>(currentTrend);
    
    // Save all readings
    QJsonArray readingsArray;
    for (const auto &reading : readings) {
        QJsonObject readingObj;
        readingObj["timestamp"] = reading.first.toString(Qt::ISODate);
        readingObj["value"] = reading.second;
        readingsArray.append(readingObj);
    }
    rootObj["readings"] = readingsArray;
    
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

bool GlucoseModel::loadReadings(const QString &filename)
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
    
    // Clear existing readings
    readings.clear();
    
    // Load all readings
    QJsonArray readingsArray = rootObj["readings"].toArray();
    for (const QJsonValue &value : readingsArray) {
        QJsonObject readingObj = value.toObject();
        
        QDateTime timestamp = QDateTime::fromString(readingObj["timestamp"].toString(), Qt::ISODate);
        double glucoseValue = readingObj["value"].toDouble();
        
        readings.append(qMakePair(timestamp, glucoseValue));
    }
    
    // Load current trend
    currentTrend = static_cast<TrendDirection>(rootObj["currentTrend"].toInt(Stable));
    
    // Notify about trend
    emit trendDirectionChanged(currentTrend);
    
    // Notify about the latest reading if available
    if (!readings.isEmpty()) {
        emit newReading(readings.last().second, readings.last().first);
    }
    
    return true;
}
