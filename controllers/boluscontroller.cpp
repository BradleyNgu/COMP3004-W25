#include "boluscontroller.h"

BolusController::BolusController(QObject *parent)
    : QObject(parent),
      insulinModel(nullptr),
      glucoseModel(nullptr),
      profileModel(nullptr),
      maxBolusUnits(25.0)
{
}

void BolusController::setInsulinModel(InsulinModel *model)
{
    insulinModel = model;
    
    // Connect signals
    if (insulinModel) {
        connect(insulinModel, &InsulinModel::bolusStarted, this, &BolusController::bolusDeliveryStarted);
        connect(insulinModel, &InsulinModel::bolusCompleted, this, &BolusController::bolusDeliveryCompleted);
        connect(insulinModel, &InsulinModel::bolusCancelled, this, &BolusController::bolusDeliveryCancelled);
    }
}

void BolusController::setGlucoseModel(GlucoseModel *model)
{
    glucoseModel = model;
}

void BolusController::setProfileModel(ProfileModel *model)
{
    profileModel = model;
}

double BolusController::calculateSuggestedBolus(double glucoseValue, double carbAmount)
{
    if (!validateSettings()) {
        return 0.0;
    }
    
    // Get active profile
    Profile activeProfile = profileModel->getActiveProfile();
    
    // Calculate carb bolus
    double carbBolus = calculateCarbBolus(carbAmount, activeProfile.carbRatio);
    
    // Calculate correction bolus
    double correctionBolus = calculateCorrectionBolus(
        glucoseValue, 
        activeProfile.targetGlucose, 
        activeProfile.correctionFactor
    );
    
    // Total bolus
    double totalBolus = carbBolus + correctionBolus;
    
    // Account for insulin on board
    double iob = insulinModel->getInsulinOnBoard();
    
    // Only reduce suggested bolus if IOB is positive and we have a correction component
    if (iob > 0 && correctionBolus > 0) {
        // Only subtract IOB from correction component
        if (iob > correctionBolus) {
            totalBolus = carbBolus; // IOB covers all correction
        } else {
            totalBolus = carbBolus + (correctionBolus - iob);
        }
    }
    
    // Ensure non-negative bolus
    if (totalBolus < 0.0) {
        totalBolus = 0.0;
    }
    
    // Cap at max bolus
    if (totalBolus > maxBolusUnits) {
        totalBolus = maxBolusUnits;
    }
    
    emit bolusCalculated(totalBolus);
    
    return totalBolus;
}

double BolusController::calculateCarbBolus(double carbAmount, double carbRatio)
{
    if (carbRatio <= 0.0) {
        return 0.0;
    }
    
    return carbAmount / carbRatio;
}

double BolusController::calculateCorrectionBolus(double glucoseValue, double targetGlucose, double correctionFactor)
{
    if (correctionFactor <= 0.0) {
        return 0.0;
    }
    
    // Only correct if above target
    if (glucoseValue <= targetGlucose) {
        return 0.0;
    }
    
    return (glucoseValue - targetGlucose) / correctionFactor;
}

bool BolusController::deliverBolus(double units, bool extended, int duration)
{
    if (!validateSettings()) {
        return false;
    }
    
    // Safety check
    if (units > maxBolusUnits) {
        emit maxBolusExceeded(units, maxBolusUnits);
        return false;
    }
    
    // Check if delivery is in progress
    if (isDeliveryInProgress()) {
        return false;
    }
    
    // Deliver bolus
    return insulinModel->deliverBolus(units, "Manual", extended, duration);
}

bool BolusController::isDeliveryInProgress() const
{
    if (!insulinModel) {
        return false;
    }
    
    return insulinModel->isBolusActive();
}

bool BolusController::cancelDelivery()
{
    if (!insulinModel) {
        return false;
    }
    
    return insulinModel->cancelBolus();
}

bool BolusController::isBolusSafe(double units, double currentGlucose, double insulinOnBoard)
{
    // Check max bolus
    if (units > maxBolusUnits) {
        return false;
    }
    
    // Check for stacking (too much insulin on board)
    if (insulinOnBoard > 10.0 && units > 5.0) {
        return false;
    }
    
    // Check current glucose (don't bolus if too low)
    if (currentGlucose < 4.0 && units > 0.0) {
        return false;
    }
    
    return true;
}

double BolusController::getMaxBolus() const
{
    return maxBolusUnits;
}

void BolusController::setMaxBolus(double max)
{
    if (max > 0.0 && max <= 50.0) {
        maxBolusUnits = max;
    }
}

QVector<InsulinModel::BolusDelivery> BolusController::getRecentBoluses(int count) const
{
    if (!insulinModel) {
        return QVector<InsulinModel::BolusDelivery>();
    }
    
    QDateTime now = QDateTime::currentDateTime();
    QDateTime startTime = now.addDays(-7); // Get boluses from last 7 days
    
    QVector<InsulinModel::BolusDelivery> allBoluses = 
        insulinModel->getBolusHistory(startTime, now);
    
    // Sort by timestamp (most recent first)
    std::sort(allBoluses.begin(), allBoluses.end(), 
              [](const InsulinModel::BolusDelivery &a, const InsulinModel::BolusDelivery &b) {
                  return a.timestamp > b.timestamp;
              });
    
    // Return requested number of boluses
    if (count > 0 && allBoluses.size() > count) {
        return allBoluses.mid(0, count);
    }
    
    return allBoluses;
}

bool BolusController::validateSettings()
{
    if (!insulinModel || !glucoseModel || !profileModel) {
        return false;
    }
    
    return true;
}
