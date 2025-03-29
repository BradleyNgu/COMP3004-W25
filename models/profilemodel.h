#ifndef PROFILEMODEL_H
#define PROFILEMODEL_H

#include <QObject>
#include <QString>
#include <QVector>
#include <QMap>

struct Profile {
    QString name;
    double basalRate;           // Units per hour
    double carbRatio;           // Grams of carbs per unit of insulin
    double correctionFactor;    // mmol/L per unit of insulin
    double targetGlucose;       // Target glucose in mmol/L
};

class ProfileModel : public QObject
{
    Q_OBJECT

public:
    explicit ProfileModel(QObject *parent = nullptr);
    
    // CRUD operations for profiles
    bool createProfile(const Profile &profile);
    Profile getProfile(const QString &name) const;
    QVector<Profile> getAllProfiles() const;
    bool updateProfile(const QString &name, const Profile &updatedProfile);
    bool deleteProfile(const QString &name);
    
    // Active profile management
    bool setActiveProfile(const QString &name);
    Profile getActiveProfile() const;
    QString getActiveProfileName() const;
    
    // Save and load profiles
    bool saveProfiles(const QString &filename);
    bool loadProfiles(const QString &filename);
    
signals:
    void profileCreated(const QString &name);
    void profileUpdated(const QString &name);
    void profileDeleted(const QString &name);
    void activeProfileChanged(const QString &name);
    
private:
    QMap<QString, Profile> profiles;
    QString activeProfileName;
    
    void createDefaultProfiles();
};

#endif // PROFILEMODEL_H
