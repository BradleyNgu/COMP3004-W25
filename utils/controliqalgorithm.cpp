#include "controliqalgorithm.h"
#include <QtMath>

ControlIQAlgorithm::ControlIQAlgorithm(QObject *parent)
    : QObject(parent),
      targetLowGlucose(5.5),
      targetHighGlucose(7.0),
      hypoPreventionEnabled(true),
      aggressivenessLevel(3),
      activityMode("Normal"),
      maxBasalRate(3.0),
      sleepModeActive(false),
      exerciseModeActive(false)
{
}

double ControlIQAlgorithm::calculateBasalAdjustment(double currentGlucose, 
                                                  GlucoseModel::TrendDirection trend,
                                                  double currentBasalRate,
                                                  double targetGlucose,
                                                  double insulinOnBoard)
{
    Q_UNUSED(targetGlucose);
    Q_UNUSED(insulinOnBoard);
    // Simplified algorithm that returns predetermined adjustments based on glucose ranges
    
    // Very low glucose - suspend insulin
    if (currentGlucose < 3.9) {
        return -currentBasalRate; // Suspend all insulin
    }
    
    // Low glucose - reduce insulin
    if (currentGlucose < targetLowGlucose) {
        // Reduce by 50%
        return -0.5 * currentBasalRate;
    }
    
    // High glucose - increase insulin
    if (currentGlucose > targetHighGlucose) {
        // How high above target?
        double excessGlucose = currentGlucose - targetHighGlucose;
        
        // Tier the response
        if (excessGlucose > 5.0) {
            return 0.5 * currentBasalRate; // +50%
        } else if (excessGlucose > 2.5) {
            return 0.3 * currentBasalRate; // +30%
        } else {
            return 0.15 * currentBasalRate; // +15%
        }
    }
    
    // Activity mode modifiers
    double adjustment = 0.0;
    
    // Sleep mode reduces basal rate slightly
    if (sleepModeActive) {
        adjustment -= 0.05 * currentBasalRate;
    }
    
    // Exercise mode reduces basal rate more significantly
    if (exerciseModeActive) {
        adjustment -= 0.2 * currentBasalRate;
    }
    
    // In target range - adjust slightly based on trend
    if (trend == GlucoseModel::RisingQuickly) {
        adjustment += 0.2 * currentBasalRate; // +20%
    } else if (trend == GlucoseModel::Rising) {
        adjustment += 0.1 * currentBasalRate; // +10%
    } else if (trend == GlucoseModel::FallingQuickly) {
        adjustment -= 0.2 * currentBasalRate; // -20%
    } else if (trend == GlucoseModel::Falling) {
        adjustment -= 0.1 * currentBasalRate; // -10%
    }
    
    // Aggressiveness level affects adjustment magnitude (scale from 0.6 to 1.4)
    double aggressivenessFactor = 0.6 + (aggressivenessLevel * 0.2);
    adjustment *= aggressivenessFactor;
    
    // Apply hypo prevention - reduce negative adjustments if enabled
    if (hypoPreventionEnabled && adjustment < 0) {
        adjustment *= 0.7; // Reduce decreases by 30%
    }
    
    return adjustment;
}

void ControlIQAlgorithm::setTargetRange(double targetLow, double targetHigh)
{
    targetLowGlucose = targetLow;
    targetHighGlucose = targetHigh;
}

void ControlIQAlgorithm::setHypoPreventionEnabled(bool enabled)
{
    hypoPreventionEnabled = enabled;
}

void ControlIQAlgorithm::setAggressiveness(int level)
{
    if (level >= 1 && level <= 5) {
        aggressivenessLevel = level;
    }
}

void ControlIQAlgorithm::setActivityMode(const QString &mode)
{
    if (mode == "Normal" || mode == "Sleep" || mode == "Exercise") {
        activityMode = mode;
        
        // Update mode flags
        sleepModeActive = (mode == "Sleep");
        exerciseModeActive = (mode == "Exercise");
    }
}

void ControlIQAlgorithm::setMaxBasalRate(double max)
{
    if (max > 0.0) {
        maxBasalRate = max;
    }
}

double ControlIQAlgorithm::getTargetLow() const
{
    return targetLowGlucose;
}

double ControlIQAlgorithm::getTargetHigh() const
{
    return targetHighGlucose;
}

bool ControlIQAlgorithm::getHypoPreventionEnabled() const
{
    return hypoPreventionEnabled;
}

int ControlIQAlgorithm::getAggressiveness() const
{
    return aggressivenessLevel;
}

QString ControlIQAlgorithm::getActivityMode() const
{
    return activityMode;
}

double ControlIQAlgorithm::getMaxBasalRate() const
{
    return maxBasalRate;
}

// Methods to satisfy ControlIQScreen's expectations
bool ControlIQAlgorithm::isSleepModeActive() const
{
    return sleepModeActive;
}

bool ControlIQAlgorithm::isExerciseModeActive() const
{
    return exerciseModeActive;
}

bool ControlIQAlgorithm::isHypoPreventionActive() const
{
    return hypoPreventionEnabled;
}

void ControlIQAlgorithm::setSleepSetting(bool enabled)
{
    sleepModeActive = enabled;
    
    // Update activity mode for consistency
    if (enabled) {
        activityMode = "Sleep";
        exerciseModeActive = false;
    } else if (!exerciseModeActive) {
        activityMode = "Normal";
    }
}

void ControlIQAlgorithm::setExerciseSetting(bool enabled)
{
    exerciseModeActive = enabled;
    
    // Update activity mode for consistency
    if (enabled) {
        activityMode = "Exercise";
        sleepModeActive = false;
    } else if (!sleepModeActive) {
        activityMode = "Normal";
    }
}

void ControlIQAlgorithm::setHypoPrevention(bool enabled)
{
    // This just maps to the existing method for backward compatibility
    setHypoPreventionEnabled(enabled);
}
