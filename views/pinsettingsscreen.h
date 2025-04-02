#ifndef PINSETTINGSSCREEN_H
#define PINSETTINGSSCREEN_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QCheckBox>
#include <QLineEdit>
#include "pinlockscreen.h"

class PinSettingsScreen : public QWidget
{
    Q_OBJECT

public:
    explicit PinSettingsScreen(QWidget *parent = nullptr);
    ~PinSettingsScreen();
    
    void updateSettings();
    
signals:
    void backButtonClicked();
    void homeButtonClicked();
    
protected:
    void paintEvent(QPaintEvent *event) override;
    
private slots:
    void onEnablePinToggled(bool checked);
    void onChangeCurrentPinClicked();
    void onSetNewPinClicked();
    void onBackButtonClicked();
    void onHomeButtonClicked();
    
private:
    QLabel *titleLabel;
    QCheckBox *enablePinCheckBox;
    QPushButton *changeCurrentPinButton;
    QPushButton *setNewPinButton;
    QPushButton *backButton;
    QPushButton *homeButton;
    
    // New PIN input fields
    QLabel *newPinLabel;
    QLineEdit *newPinInput;
    QLabel *confirmPinLabel;
    QLineEdit *confirmPinInput;
    
    PinLockScreen *pinLockScreen;
    
    void setupUi();
    void connectSignals();
    void loadSettings();
    void saveSettings();
};

#endif // PINSETTINGSSCREEN_H
