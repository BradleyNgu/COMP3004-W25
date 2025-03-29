#ifndef PINLOCKSCREEN_H
#define PINLOCKSCREEN_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QSettings>

class PinLockScreen : public QWidget
{
    Q_OBJECT

public:
    explicit PinLockScreen(QWidget *parent = nullptr);
    ~PinLockScreen();
    
    bool isPinEnabled() const;
    void enablePin(bool enable);
    bool validatePin(const QString &pin);
    void setPin(const QString &pin);
    bool checkCurrentPin(const QString &pin);
    
signals:
    void pinAccepted();
    void pinRejected();
    void backButtonClicked();
    
protected:
    void paintEvent(QPaintEvent *event) override;
    
private slots:
    void onDigitButtonClicked();
    void onClearButtonClicked();
    void onEnterButtonClicked();
    void onBackButtonClicked();
    
private:
    QLabel *titleLabel;
    QLabel *messageLabel;
    QLineEdit *pinDisplay;
    QPushButton *digitButtons[10];
    QPushButton *clearButton;
    QPushButton *enterButton;
    QPushButton *backButton;
    
    QString currentPin;
    bool pinEnabled;
    int failedAttempts;
    
    void setupUi();
    void connectSignals();
    void loadSettings();
    void saveSettings();
    
    void lockoutAfterFailedAttempts();
};

#endif // PINLOCKSCREEN_H
