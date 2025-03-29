#ifndef PROFILESCREEN_H
#define PROFILESCREEN_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QListWidget>
#include <QDoubleSpinBox>
#include <QLineEdit>
#include <QStackedWidget>  // Added this include for QStackedWidget
#include "../controllers/pumpcontroller.h"
#include "../models/profilemodel.h"

namespace Ui {
class ProfileScreen;
}

class ProfileScreen : public QWidget
{
    Q_OBJECT

public:
    explicit ProfileScreen(QWidget *parent = nullptr);
    ~ProfileScreen();
    
    void loadProfiles(PumpController *controller);
    
signals:
    void backButtonClicked();
    void profileCreated(const Profile &profile);
    void profileUpdated(const QString &name, const Profile &profile);
    void profileDeleted(const QString &name);
    void profileActivated(const QString &name);
    
protected:
    void paintEvent(QPaintEvent *event) override;
    
private slots:
    void on_backButton_clicked();
    void on_newProfileButton_clicked();
    void on_editProfileButton_clicked();
    void on_deleteProfileButton_clicked();
    void on_activateProfileButton_clicked();
    void on_profilesList_currentRowChanged(int currentRow);
    void on_saveProfileButton_clicked();
    void on_cancelEditButton_clicked();
    
private:
    Ui::ProfileScreen *ui;
    PumpController *pumpController;
    QVector<Profile> profiles;
    bool editMode;
    QString editingProfileName;
    
    // UI elements
    QLabel *titleLabel;
    QStackedWidget *stackedWidget;  // Added this declaration
    QListWidget *profilesList;
    QPushButton *newProfileButton;
    QPushButton *editProfileButton;
    QPushButton *deleteProfileButton;
    QPushButton *activateProfileButton;
    QPushButton *backButton;
    
    // Edit form elements
    QWidget *editFormContainer;
    QLineEdit *profileNameEdit;
    QDoubleSpinBox *basalRateSpinBox;
    QDoubleSpinBox *carbRatioSpinBox;
    QDoubleSpinBox *correctionFactorSpinBox;
    QDoubleSpinBox *targetGlucoseSpinBox;
    QPushButton *saveProfileButton;
    QPushButton *cancelEditButton;
    
    void setupUi();
    void updateProfilesList();
    void showEditForm(bool show);
    void populateEditForm(const Profile &profile);
    Profile getProfileFromForm();
    bool validateProfileForm();
};

#endif // PROFILESCREEN_H
