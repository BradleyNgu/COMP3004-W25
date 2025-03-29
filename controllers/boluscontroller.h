#ifndef BOLUSCONTROLLER_H
#define BOLUSCONTROLLER_H

#include <QObject>
#include "../models/insulinmodel.h"
#include "../models/glucosemodel.h"
#include "../models/profilemodel.h"

class BolusController : public QObject
{
    Q_OBJECT

public:
    explicit BolusController(QObject *parent = nullptr);
    
    // Set models
    void setInsulinModel(InsulinModel *model);
    void setGlucoseModel(GlucoseModel *model);
    void setProfileModel(ProfileModel *model);
    
    // Bolus calculation
    double calculateSuggestedBolus(double glucoseValue, double carbAmount);
    double calculateCarbBolus(double carbAmount, double carbRatio);
    double calculateCorrectionBolus(double glucoseValue, double targetGlucose, double correctionFactor);
    
    // Bolus delivery
    bool deliverBolus(double units, bool extended = false, int duration = 0);
    bool isDeliveryInProgress() const;
    bool cancelDelivery();
    
    // Safety checks
    bool isBolusSafe(double units, double currentGlucose, double insulinOnBoard);
    double getMaxBolus() const;
    void setMaxBolus(double max);
    
    // History
    QVector<InsulinModel::BolusDelivery> getRecentBoluses(int count) const;
    
signals:
    void bolusDeliveryStarted(double units);
    void bolusDeliveryCompleted(double units);
    void bolusDeliveryCancelled(double deliveredUnits, double requestedUnits);
    void bolusCalculated(double suggestedUnits);
    void maxBolusExceeded(double requestedUnits, double maxUnits);
    
private:
    InsulinModel *insulinModel;
    GlucoseModel *glucoseModel;
    ProfileModel *profileModel;
    double maxBolusUnits;
    
    bool validateSettings();
};

#endif // BOLUSCONTROLLER_H
