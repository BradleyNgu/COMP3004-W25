#include "profilecontroller.h"

ProfileController::ProfileController(QObject *parent)
    : QObject(parent),
      profileModel(nullptr),
      insulinModel(nullptr)
{
}

void ProfileController::setProfileModel(ProfileModel *model)
{
    profileModel = model;
    
    if (profileModel) {
        connect(profileModel, &ProfileModel::profileCreated, this, &ProfileController::profileCreated);
        connect(profileModel, &ProfileModel::profileUpdated, this, &ProfileController::profileUpdated);
        connect(profileModel, &ProfileModel::profileDeleted, this, &ProfileController::profileDeleted);
        connect(profileModel, &ProfileModel::activeProfileChanged, this, [this](const QString &name) {
            // Apply profile when it changes
            Profile profile = profileModel->getProfile(name);
            applyProfileToInsulinDelivery(profile);
            emit profileActivated(name);
        });
    }
}

void ProfileController::setInsulinModel(InsulinModel *model)
{
    insulinModel = model;
}

QVector<Profile> ProfileController::getAllProfiles() const
{
    if (!profileModel) {
        return QVector<Profile>();
    }
    
    return profileModel->getAllProfiles();
}

Profile ProfileController::getProfile(const QString &name) const
{
    if (!profileModel) {
        return Profile();
    }
    
    return profileModel->getProfile(name);
}

QString ProfileController::getActiveProfileName() const
{
    if (!profileModel) {
        return QString();
    }
    
    return profileModel->getActiveProfileName();
}

Profile ProfileController::getActiveProfile() const
{
    if (!profileModel) {
        return Profile();
    }
    
    return profileModel->getActiveProfile();
}

bool ProfileController::createProfile(const Profile &profile)
{
    if (!profileModel) {
        return false;
    }
    
    return profileModel->createProfile(profile);
}

bool ProfileController::updateProfile(const QString &name, const Profile &updatedProfile)
{
    if (!profileModel) {
        return false;
    }
    
    bool success = profileModel->updateProfile(name, updatedProfile);
    
    // If this was the active profile, reapply it
    if (success && name == profileModel->getActiveProfileName()) {
        applyProfileToInsulinDelivery(updatedProfile);
    }
    
    return success;
}

bool ProfileController::deleteProfile(const QString &name)
{
    if (!profileModel) {
        return false;
    }
    
    return profileModel->deleteProfile(name);
}

bool ProfileController::activateProfile(const QString &name)
{
    if (!profileModel) {
        return false;
    }
    
    return profileModel->setActiveProfile(name);
}

double ProfileController::calculateBasalRate(const QString &profileName, const QDateTime &time) const
{
    if (!profileModel) {
        return 0.0;
    }
    
    // Get base basal rate from profile
    Profile profile = profileModel->getProfile(profileName);
    double baseRate = profile.basalRate;
    
    // Apply time-based adjustments if any
    if (timeAdjustments.contains(profileName)) {
        QTime timeOfDay = time.time();
        for (const auto &adjustment : timeAdjustments[profileName]) {
            if ((adjustment.startTime <= timeOfDay && timeOfDay < adjustment.endTime) ||
                (adjustment.startTime > adjustment.endTime && // Handle overnight adjustments
                 (timeOfDay >= adjustment.startTime || timeOfDay < adjustment.endTime))) {
                
                baseRate = baseRate * (adjustment.basalPercentage / 100.0);
                break; // Apply only first matching adjustment
            }
        }
    }
    
    return baseRate;
}

double ProfileController::calculateCarbRatio(const QString &profileName, const QDateTime &time) const
{
    if (!profileModel) {
        return 0.0;
    }
    
    // Get carb ratio from profile (currently we don't support time-based adjustments for this)
    Profile profile = profileModel->getProfile(profileName);
    return profile.carbRatio;
}

double ProfileController::calculateCorrectionFactor(const QString &profileName, const QDateTime &time) const
{
    if (!profileModel) {
        return 0.0;
    }
    
    // Get correction factor from profile (currently we don't support time-based adjustments for this)
    Profile profile = profileModel->getProfile(profileName);
    return profile.correctionFactor;
}

double ProfileController::calculateTargetGlucose(const QString &profileName, const QDateTime &time) const
{
    if (!profileModel) {
        return 0.0;
    }
    
    // Get target glucose from profile (currently we don't support time-based adjustments for this)
    Profile profile = profileModel->getProfile(profileName);
    return profile.targetGlucose;
}

void ProfileController::setTimeBasedAdjustment(const QString &profileName, const QTime &startTime, const QTime &endTime, double basalPercentage)
{
    // Create adjustment
    TimeAdjustment adjustment;
    adjustment.startTime = startTime;
    adjustment.endTime = endTime;
    adjustment.basalPercentage = basalPercentage;
    
    // Add to map
    if (!timeAdjustments.contains(profileName)) {
        timeAdjustments[profileName] = QVector<TimeAdjustment>();
    }
    
    timeAdjustments[profileName].append(adjustment);
    
    // If this is the active profile, reapply it
    if (profileModel && profileName == profileModel->getActiveProfileName()) {
        applyProfileToInsulinDelivery(profileModel->getActiveProfile());
    }
}

void ProfileController::clearTimeBasedAdjustments(const QString &profileName)
{
    if (timeAdjustments.contains(profileName)) {
        timeAdjustments.remove(profileName);
        
        // If this is the active profile, reapply it
        if (profileModel && profileName == profileModel->getActiveProfileName()) {
            applyProfileToInsulinDelivery(profileModel->getActiveProfile());
        }
    }
}

bool ProfileController::applyProfileToInsulinDelivery(const Profile &profile)
{
    if (!insulinModel) {
        return false;
    }
    
    // Calculate current basal rate based on time
    double basalRate = calculateBasalRate(profile.name, QDateTime::currentDateTime());
    
    // Apply to insulin model
    insulinModel->startBasal(basalRate, profile.name);
    
    emit insulinDeliveryChanged();
    
    return true;
}
