#ifndef PROFILECONTROLLER_H
#define PROFILECONTROLLER_H

#include <QObject>
#include "../models/profilemodel.h"
#include "../models/insulinmodel.h"

class ProfileController : public QObject
{
    Q_OBJECT

public:
    explicit ProfileController(QObject *parent = nullptr);
    
    // Set models
    void setProfileModel(ProfileModel *model);
    void setInsulinModel(InsulinModel *model);
    
    // Profile management
    QVector<Profile> getAllProfiles() const;
    Profile getProfile(const QString &name) const;
    QString getActiveProfileName() const;
    Profile getActiveProfile() const;
    
    // CRUD operations
    bool createProfile(const Profile &profile);
    bool updateProfile(const QString &name, const Profile &updatedProfile);
    bool deleteProfile(const QString &name);
    bool activateProfile(const QString &name);
    
    // Profile operations
    double calculateBasalRate(const QString &profileName, const QDateTime &time) const;
    double calculateCarbRatio(const QString &profileName, const QDateTime &time) const;
    double calculateCorrectionFactor(const QString &profileName, const QDateTime &time) const;
    double calculateTargetGlucose(const QString &profileName, const QDateTime &time) const;
    
    // Time-based adjustments
    void setTimeBasedAdjustment(const QString &profileName, const QTime &startTime, const QTime &endTime, double basalPercentage = 100.0);
    void clearTimeBasedAdjustments(const QString &profileName);
    
signals:
    void profileCreated(const QString &name);
    void profileUpdated(const QString &name);
    void profileDeleted(const QString &name);
    void profileActivated(const QString &name);
    void insulinDeliveryChanged();
    
private:
    ProfileModel *profileModel;
    InsulinModel *insulinModel;
    
    // Time-based adjustments
    struct TimeAdjustment {
        QTime startTime;
        QTime endTime;
        double basalPercentage;
    };
    
    QMap<QString, QVector<TimeAdjustment>> timeAdjustments;
    
    bool applyProfileToInsulinDelivery(const Profile &profile);
};

#endif // PROFILECONTROLLER_H
