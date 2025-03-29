#ifndef DATASTORAGE_H
#define DATASTORAGE_H

#include <QObject>
#include <QDateTime>
#include <QVector>
#include <QPair>
#include <QFile>
#include <QJsonDocument>
#include <QDir>

class DataStorage : public QObject
{
    Q_OBJECT

public:
    explicit DataStorage(QObject *parent = nullptr);
    
    // Glucose data
    bool saveGlucoseData(const QVector<QPair<QDateTime, double>> &data, const QString &filename);
    QVector<QPair<QDateTime, double>> loadGlucoseData(const QString &filename);
    
    // Insulin data
    bool saveInsulinData(const QVector<QPair<QDateTime, double>> &data, const QString &filename);
    QVector<QPair<QDateTime, double>> loadInsulinData(const QString &filename);
    
    // Bolus history
    struct BolusRecord {
        QDateTime timestamp;
        double units;
        QString reason;
        bool extended;
        int duration;
        bool completed;
    };
    
    bool saveBolusHistory(const QVector<BolusRecord> &history, const QString &filename);
    QVector<BolusRecord> loadBolusHistory(const QString &filename);
    
    // Basal history
    struct BasalRecord {
        QDateTime startTime;
        QDateTime endTime;
        double rate;
        QString profileName;
        bool automatic;
    };
    
    bool saveBasalHistory(const QVector<BasalRecord> &history, const QString &filename);
    QVector<BasalRecord> loadBasalHistory(const QString &filename);
    
    // Profile data
    struct ProfileRecord {
        QString name;
        double basalRate;
        double carbRatio;
        double correctionFactor;
        double targetGlucose;
    };
    
    bool saveProfiles(const QVector<ProfileRecord> &profiles, const QString &filename);
    QVector<ProfileRecord> loadProfiles(const QString &filename);
    
    // Event log
    struct LogEvent {
        QDateTime timestamp;
        QString message;
        int level; // 0=Info, 1=Warning, 2=Error
    };
    
    bool saveEventLog(const QVector<LogEvent> &events, const QString &filename);
    QVector<LogEvent> loadEventLog(const QString &filename);
    void addLogEvent(const QString &message, int level = 0);
    void addEventLog(const QString &message, int level); // Added this missing declaration
    
    // Statistics and reporting
    QVector<QPair<QString, double>> calculateDailyStatistics(
        const QDateTime &startDate, 
        const QDateTime &endDate,
        const QVector<QPair<QDateTime, double>> &glucoseData
    );
    
    QVector<QPair<int, double>> calculateHourlyAverages(
        const QDateTime &startDate, 
        const QDateTime &endDate,
        const QVector<QPair<QDateTime, double>> &glucoseData
    );
    
    QString generateCSVReport(
        const QDateTime &startDate, 
        const QDateTime &endDate,
        const QVector<QPair<QDateTime, double>> &glucoseData,
        const QVector<QPair<QDateTime, double>> &insulinData
    );

signals:
    void eventLogged(const QString &message, int level);  // Added this missing signal declaration
    
private:
    QVector<LogEvent> eventLog;
    
    bool createDirectoryIfNeeded(const QString &path);
    bool writeJsonToFile(const QJsonDocument &doc, const QString &filename);
    QJsonDocument readJsonFromFile(const QString &filename);
};

#endif // DATASTORAGE_H
