#ifndef ERRORHANDLER_H
#define ERRORHANDLER_H

#include <QObject>
#include <QDateTime>
#include <QString>
#include <QVector>
#include <QPair>
#include "../utils/datastorage.h" // Added for history integration

class ErrorHandler : public QObject
{
    Q_OBJECT

public:
    explicit ErrorHandler(QObject *parent = nullptr);
    
    enum ErrorLevel {
        Info,
        Warning,
        Error,
        Critical
    };
    
    struct ErrorRecord {
        QDateTime timestamp;
        QString message;
        QString source;
        ErrorLevel level;
        bool acknowledged;
    };
    
    // Error management
    void logError(const QString &message, const QString &source = "", ErrorLevel level = Warning);
    bool acknowledgeError(int index);
    void acknowledgeAllErrors();
    
    // Specific alert types required by requirements
    void lowBatteryAlert(int batteryLevel);
    void lowInsulinAlert(double insulinUnits);
    void cgmDisconnectedAlert(int minutesSinceLastReading);
    void occlusionAlert();
    void highGlucoseAlert(double glucoseValue);
    void lowGlucoseAlert(double glucoseValue);
    void provideTroubleshootingGuidance(const QString &errorCode);
    void contactSupportPrompt(const QString &errorCode);
    
    // History integration
    void setHistoryManager(DataStorage* history);
    
    // Error retrieval
    QVector<ErrorRecord> getAllErrors() const;
    QVector<ErrorRecord> getActiveErrors() const;
    QVector<ErrorRecord> getErrorsOfLevel(ErrorLevel level) const;
    int getErrorCount() const;
    int getActiveErrorCount() const;
    
    // Error handling
    bool hasCriticalErrors() const;
    void clearAllErrors();
    
    // Error recovery
    bool attemptRecovery(int errorIndex);
    bool canRecover(int errorIndex) const;
    QString getRecoveryInstructions(int errorIndex) const;
    
signals:
    void errorLogged(const QString &message, ErrorLevel level);
    void errorAcknowledged(int index);
    void allErrorsAcknowledged();
    void criticalErrorDetected(const QString &message);
    void errorRecovered(int index);
    
private:
    QVector<ErrorRecord> errorLog;
    DataStorage* historyManager = nullptr;
    
    QString getErrorLevelString(ErrorLevel level) const;
    QString generateErrorReport() const;
    bool saveErrorLog(const QString &filename) const;
    bool loadErrorLog(const QString &filename);
};

#endif // ERRORHANDLER_H
