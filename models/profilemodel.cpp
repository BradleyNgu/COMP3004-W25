#include "profilemodel.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

ProfileModel::ProfileModel(QObject *parent)
    : QObject(parent),
      activeProfileName("Default")
{
    createDefaultProfiles();
}

void ProfileModel::createDefaultProfiles()
{
    // Create default profile
    Profile defaultProfile;
    defaultProfile.name = "Default";
    defaultProfile.basalRate = 1.0;
    defaultProfile.carbRatio = 10.0;
    defaultProfile.correctionFactor = 2.0;
    defaultProfile.targetGlucose = 5.5;
    profiles[defaultProfile.name] = defaultProfile;
    
    // Create sleep profile
    Profile sleepProfile;
    sleepProfile.name = "Sleep";
    sleepProfile.basalRate = 0.8;
    sleepProfile.carbRatio = 10.0;
    sleepProfile.correctionFactor = 2.0;
    sleepProfile.targetGlucose = 6.0;
    profiles[sleepProfile.name] = sleepProfile;
    
    // Create exercise profile
    Profile exerciseProfile;
    exerciseProfile.name = "Exercise";
    exerciseProfile.basalRate = 0.6;
    exerciseProfile.carbRatio = 15.0;
    exerciseProfile.correctionFactor = 2.5;
    exerciseProfile.targetGlucose = 6.5;
    profiles[exerciseProfile.name] = exerciseProfile;
}

bool ProfileModel::createProfile(const Profile &profile)
{
    // Validate profile data
    if (profile.name.isEmpty() || 
        profile.basalRate <= 0 || 
        profile.carbRatio <= 0 || 
        profile.correctionFactor <= 0 || 
        profile.targetGlucose <= 0) {
        return false;
    }
    
    // Check if profile with this name already exists
    if (profiles.contains(profile.name)) {
        return false;
    }
    
    // Add the profile
    profiles[profile.name] = profile;
    emit profileCreated(profile.name);
    
    return true;
}

Profile ProfileModel::getProfile(const QString &name) const
{
    if (profiles.contains(name)) {
        return profiles[name];
    }
    
    // Return empty profile if not found
    return Profile();
}

QVector<Profile> ProfileModel::getAllProfiles() const
{
    QVector<Profile> result;
    for (const auto &profile : profiles) {
        result.append(profile);
    }
    return result;
}

bool ProfileModel::updateProfile(const QString &name, const Profile &updatedProfile)
{
    // Validate profile data
    if (updatedProfile.name.isEmpty() || 
        updatedProfile.basalRate <= 0 || 
        updatedProfile.carbRatio <= 0 || 
        updatedProfile.correctionFactor <= 0 || 
        updatedProfile.targetGlucose <= 0) {
        return false;
    }
    
    // Check if profile exists
    if (!profiles.contains(name)) {
        return false;
    }
    
    // Handle name change
    if (name != updatedProfile.name) {
        // Check if new name conflicts with existing profile
        if (profiles.contains(updatedProfile.name)) {
            return false;
        }
        
        // Remove old profile and add with new name
        profiles.remove(name);
        profiles[updatedProfile.name] = updatedProfile;
        
        // Update active profile name if needed
        if (activeProfileName == name) {
            activeProfileName = updatedProfile.name;
            emit activeProfileChanged(activeProfileName);
        }
    } else {
        // Just update the profile
        profiles[name] = updatedProfile;
    }
    
    emit profileUpdated(updatedProfile.name);
    return true;
}

bool ProfileModel::deleteProfile(const QString &name)
{
    // Don't allow deleting the default profile
    if (name == "Default") {
        return false;
    }
    
    // Check if profile exists
    if (!profiles.contains(name)) {
        return false;
    }
    
    // Switch to default profile if deleting active profile
    if (activeProfileName == name) {
        setActiveProfile("Default");
    }
    
    // Remove the profile
    profiles.remove(name);
    emit profileDeleted(name);
    
    return true;
}

bool ProfileModel::setActiveProfile(const QString &name)
{
    // Check if profile exists
    if (!profiles.contains(name)) {
        return false;
    }
    
    // Set active profile
    if (activeProfileName != name) {
        activeProfileName = name;
        emit activeProfileChanged(name);
    }
    
    return true;
}

Profile ProfileModel::getActiveProfile() const
{
    return getProfile(activeProfileName);
}

QString ProfileModel::getActiveProfileName() const
{
    return activeProfileName;
}

bool ProfileModel::saveProfiles(const QString &filename)
{
    QJsonObject rootObj;
    
    // Save active profile name
    rootObj["activeProfile"] = activeProfileName;
    
    // Save all profiles
    QJsonArray profilesArray;
    for (const auto &profile : getAllProfiles()) {
        QJsonObject profileObj;
        profileObj["name"] = profile.name;
        profileObj["basalRate"] = profile.basalRate;
        profileObj["carbRatio"] = profile.carbRatio;
        profileObj["correctionFactor"] = profile.correctionFactor;
        profileObj["targetGlucose"] = profile.targetGlucose;
        profilesArray.append(profileObj);
    }
    rootObj["profiles"] = profilesArray;
    
    // Write to file
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly)) {
        return false;
    }
    
    QJsonDocument doc(rootObj);
    file.write(doc.toJson());
    file.close();
    
    return true;
}

bool ProfileModel::loadProfiles(const QString &filename)
{
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }
    
    QByteArray data = file.readAll();
    file.close();
    
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (doc.isNull() || !doc.isObject()) {
        return false;
    }
    
    QJsonObject rootObj = doc.object();
    
    // Clear existing profiles (except Default)
    QVector<QString> profilesToRemove;
    for (const auto &name : profiles.keys()) {
        if (name != "Default") {
            profilesToRemove.append(name);
        }
    }
    
    for (const auto &name : profilesToRemove) {
        profiles.remove(name);
    }
    
    // Load all profiles
    QJsonArray profilesArray = rootObj["profiles"].toArray();
    for (const QJsonValue &value : profilesArray) {
        QJsonObject profileObj = value.toObject();
        
        Profile profile;
        profile.name = profileObj["name"].toString();
        profile.basalRate = profileObj["basalRate"].toDouble();
        profile.carbRatio = profileObj["carbRatio"].toDouble();
        profile.correctionFactor = profileObj["correctionFactor"].toDouble();
        profile.targetGlucose = profileObj["targetGlucose"].toDouble();
        
        // Don't overwrite Default profile
        if (profile.name != "Default") {
            profiles[profile.name] = profile;
            emit profileCreated(profile.name);
        }
    }
    
    // Set active profile
    QString activeProfile = rootObj["activeProfile"].toString("Default");
    setActiveProfile(activeProfile);
    
    return true;
}
