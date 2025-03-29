#ifndef CONTROLIQALGORITHM_H
#define CONTROLIQALGORITHM_H

#include <QObject>
#include "../models/glucosemodel.h"

class ControlIQAlgorithm : public QObject
{
    Q_OBJECT

public:
    explicit ControlIQAlgorithm(QObject *parent = nullptr);
    
    // Calculate basal adjustment based on current glucose and trend
    double calculateBasalAdjustment(double currentGlucose, 
                                  GlucoseModel::TrendDirection trend,
                                  double currentBasalRate,
                                  double targetGlucose,
                                  double insulinOnBoard);
    
    // Core settings configuration
    void setTargetRange(double targetLow, double targetHigh);
    void setHypoPreventionEnabled(bool enabled);
    void setAggressiveness(int level); // 1-5 scale
    void setActivityMode(const QString &mode); // Normal, Sleep, Exercise
    void setMaxBasalRate(double max);
    
    // Core getters
    double getTargetLow() const;
    double getTargetHigh() const;
    bool getHypoPreventionEnabled() const;
    int getAggressiveness() const;
    QString getActivityMode() const;
    double getMaxBasalRate() const;
    
    // Methods added to match ControlIQScreen expectations
    bool isSleepModeActive() const;
    bool isExerciseModeActive() const;
    bool isHypoPreventionActive() const;
    
    void setSleepSetting(bool enabled);
    void setExerciseSetting(bool enabled);
    void setHypoPrevention(bool enabled);
    
private:
    // Configuration
    double targetLowGlucose;
    double targetHighGlucose;
    bool hypoPreventionEnabled;
    int aggressivenessLevel; // 1-5
    QString activityMode;
    double maxBasalRate;
    
    // Mode flags
    bool sleepModeActive;
    bool exerciseModeActive;
};

#endif // CONTROLIQALGORITHM_H
