#ifndef OPTIONSSCREEN_H
#define OPTIONSSCREEN_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QListWidget>

namespace Ui {
class OptionsScreen;
}

class OptionsScreen : public QWidget
{
    Q_OBJECT

public:
    explicit OptionsScreen(QWidget *parent = nullptr);
    ~OptionsScreen();
    
signals:
    void backButtonClicked();
    void profilesButtonClicked();
    void historyButtonClicked();
    void startInsulinButtonClicked();
    void stopInsulinButtonClicked();
    void alertsButtonClicked();
    void controlIQButtonClicked();
    void securitySettingsButtonClicked();
    
protected:
    void paintEvent(QPaintEvent *event) override;
    
private slots:
    void onBackButtonClicked();
    void onProfilesButtonClicked();
    void onStartInsulinButtonClicked();
    void onStopInsulinButtonClicked();
    void onAlertsButtonClicked();
    void onHistoryButtonClicked();
    void onControlIQButtonClicked();
    void onSecuritySettingsButtonClicked();
    
private:
    Ui::OptionsScreen *ui;
    
    void setupUi();
    void connectSignals();
    
    // UI elements
    QLabel *titleLabel;
    QPushButton *profilesButton;
    QPushButton *startInsulinButton;
    QPushButton *stopInsulinButton;
    QPushButton *alertsButton;
    QPushButton *historyButton;
    QPushButton *controlIQButton;
    QPushButton *securitySettingsButton;
    QPushButton *backButton;
};

#endif // OPTIONSSCREEN_H
