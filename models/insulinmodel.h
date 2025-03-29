#ifndef INSULINMODEL_H
#define INSULINMODEL_H

#include <QObject>
#include <QDateTime>
#include <QVector>
#include <QPair>

class InsulinModel : public QObject
{
    Q_OBJECT

public:
    explicit InsulinModel(QObject *parent = nullptr);
    
    struct BolusDelivery {
        QDateTime timestamp;
        double units;
        QString reason;
        bool extended;
        int duration; // minutes for extended bolus
        bool completed;
    };
    
    struct BasalDelivery {
        QDateTime startTime;
        QDateTime endTime;
        double rate; // Units per hour
        QString profileName;
        bool automatic; // Whether controlled by Control-IQ
    };
    
    // Basic insulin delivery
    double getInsulinOnBoard() const;
    double getCurrentBasalRate() const;
    bool isBolusActive() const;
    BolusDelivery getCurrentBolus() const;
    BolusDelivery getLastCompletedBolus() const;
    
    // Control operations
    void startBasal(double rate, const QString &profileName, bool automatic = false);
    void stopBasal();
    void suspendBasal();
    void resumeBasal();
    void adjustBasalRate(double newRate, bool automatic = true);
    bool deliverBolus(double units, const QString &reason = "Manual", bool extended = false, int duration = 0);
    bool cancelBolus();
    
    // History
    QVector<BolusDelivery> getBolusHistory(const QDateTime &start, const QDateTime &end) const;
    QVector<BasalDelivery> getBasalHistory(const QDateTime &start, const QDateTime &end) const;
    double getTotalInsulin(const QDateTime &start, const QDateTime &end) const;
    double getTotalBasal(const QDateTime &start, const QDateTime &end) const;
    double getTotalBolus(const QDateTime &start, const QDateTime &end) const;
    
    // Helper methods for generating simulated history
    void addBolusToHistory(const QDateTime &timestamp, double units, const QString &reason, 
                         bool extended, int duration, bool completed);
    void addBasalToHistory(const BasalDelivery &segment);
    
    // ControlIQ
    double getLastControlIQAdjustment() const;
    
    // Save and load
    bool saveInsulinData(const QString &filename);
    bool loadInsulinData(const QString &filename);
    
signals:
    void insulinOnBoardChanged(double units);
    void basalRateChanged(double rate);
    void basalStateChanged(bool active);
    void bolusStarted(double units);
    void bolusCompleted(double units);
    void bolusCancelled(double unitsDelivered, double unitsRequested);
    void controlIQAdjustmentChanged(double adjustment);
    
public slots:
    void updateIOB();
    
private:
    double insulinOnBoard;
    
    // Current state
    bool basalActive;
    double currentBasalRate;
    QString currentProfileName;
    bool basalIsAutomatic;
    
    // Current bolus state
    bool bolusActive;
    BolusDelivery currentBolus;
    BolusDelivery lastCompletedBolus;
    
    // Control-IQ state
    double lastControlIQAdjustment;
    
    // History
    QVector<BolusDelivery> bolusHistory;
    QVector<BasalDelivery> basalHistory;
};

#endif // INSULINMODEL_H
