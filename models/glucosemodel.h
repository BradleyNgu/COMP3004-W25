#ifndef GLUCOSEMODEL_H
#define GLUCOSEMODEL_H

#include <QObject>
#include <QDateTime>
#include <QVector>

class GlucoseModel : public QObject
{
    Q_OBJECT

public:
    explicit GlucoseModel(QObject *parent = nullptr);
    
    enum TrendDirection {
        Rising,
        RisingQuickly,
        Stable,
        Falling,
        FallingQuickly,
        Unknown
    };
    
    // Current glucose data
    double getCurrentGlucose() const;
    QDateTime getLastReadingTime() const;
    TrendDirection getTrendDirection() const;
    void forceTrend(TrendDirection trend);
    
    // Historical data
    QVector<QPair<QDateTime, double>> getReadings(const QDateTime &start, const QDateTime &end) const;
    
    // Generate fixed pattern data for demo
    void generateFixedPattern(int hoursBack);
    
    // Add new reading
    void addReading(double value, const QDateTime &timestamp = QDateTime::currentDateTime());
    void clearReadings();
    
    // Load/save 
    bool saveReadings(const QString &filename);
    bool loadReadings(const QString &filename);
    
signals:
    void newReading(double value, const QDateTime &timestamp);
    void trendDirectionChanged(TrendDirection direction);
    
private:
    QVector<QPair<QDateTime, double>> readings;
    TrendDirection currentTrend;
    
    void calculateTrendDirection();
};

#endif // GLUCOSEMODEL_H
