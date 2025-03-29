#ifndef ALERTSSCREEN_H
#define ALERTSSCREEN_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QCheckBox>
#include <QTabWidget>
#include <QListWidget>
#include <QDateTime>
#include <QDateTimeEdit>
#include <QComboBox>
#include <QTableWidget>
#include <QHeaderView>  // Added for QHeaderView
#include "../controllers/alertcontroller.h"
#include "../models/pumpmodel.h"
#include "../utils/datastorage.h"  // Added for DataStorage

// Forward declaration
class HistoryScreen;

namespace Ui {
class AlertsScreen;
}

class AlertsScreen : public QWidget
{
    Q_OBJECT

public:
    explicit AlertsScreen(QWidget *parent = nullptr);
    ~AlertsScreen();
    
    void setAlertController(AlertController *controller);
    void updateActiveAlerts();

    // Add these new methods
    void setHistoryScreen(HistoryScreen *screen);
    void setDataStorage(DataStorage *storage);
    void updateAlertHistory();

signals:
    void backButtonClicked();
    
protected:
    void paintEvent(QPaintEvent *event) override;
    
private slots:
    void onBackButtonClicked();
    void onSaveSettingsButtonClicked();
    void onAcknowledgeAlertButtonClicked();
    void onClearAllAlertsButtonClicked();
    void onSetReminderButtonClicked();
    void onDeleteReminderButtonClicked();
    void onEnableAlertsToggled(bool checked);
    
    void onLowGlucoseThresholdChanged(double value);
    void onHighGlucoseThresholdChanged(double value);
    void onUrgentLowGlucoseThresholdChanged(double value);
    void onUrgentHighGlucoseThresholdChanged(double value);
    void onLowInsulinThresholdChanged(double value);
    void onCriticalInsulinThresholdChanged(double value);
    void onLowBatteryThresholdChanged(int value);
    void onCriticalBatteryThresholdChanged(int value);

private:
    Ui::AlertsScreen *ui;
    AlertController *alertController;
    HistoryScreen *historyScreen;  // Added member variable
    DataStorage *dataStorage;      // Added member variable
    
    // UI elements
    QTabWidget *tabWidget;
    
    // Settings tab
    QWidget *settingsTab;
    QLabel *titleLabel;
    QCheckBox *enableAlertsCheckBox;
    
    // Glucose thresholds
    QLabel *glucoseThresholdsLabel;
    QLabel *lowGlucoseLabel;
    QDoubleSpinBox *lowGlucoseSpinBox;
    QLabel *highGlucoseLabel;
    QDoubleSpinBox *highGlucoseSpinBox;
    QLabel *urgentLowGlucoseLabel;
    QDoubleSpinBox *urgentLowGlucoseSpinBox;
    QLabel *urgentHighGlucoseLabel;
    QDoubleSpinBox *urgentHighGlucoseSpinBox;
    
    // Insulin thresholds
    QLabel *insulinThresholdsLabel;
    QLabel *lowInsulinLabel;
    QDoubleSpinBox *lowInsulinSpinBox;
    QLabel *criticalInsulinLabel;
    QDoubleSpinBox *criticalInsulinSpinBox;
    
    // Battery thresholds
    QLabel *batteryThresholdsLabel;
    QLabel *lowBatteryLabel;
    QSpinBox *lowBatterySpinBox;
    QLabel *criticalBatteryLabel;
    QSpinBox *criticalBatterySpinBox;
    
    QPushButton *saveSettingsButton;
    
    // Active alerts tab
    QWidget *alertsTab;
    QListWidget *activeAlertsList;
    QPushButton *acknowledgeAlertButton;
    QPushButton *clearAllAlertsButton;
    
    // Alert history tab
    QWidget *historyTab;  // Changed from alertHistoryTab to historyTab
    QTableWidget *alertHistoryTable;
    
    // Reminders tab
    QWidget *remindersTab;
    QListWidget *remindersList;
    
    QLabel *reminderTypeLabel;
    QComboBox *reminderTypeComboBox;
    QLabel *reminderTimeLabel;
    QDateTimeEdit *reminderTimeEdit;
    QPushButton *setReminderButton;
    QPushButton *deleteReminderButton;
    
    // Bottom section
    QPushButton *backButton;
    
    // Methods
    void setupUi();
    void connectSignals();
    void loadSettings();
    void saveSettings();
    void updateReminders();
    void addReminder(const QString &type, const QDateTime &time);
    
    struct Reminder {
        QString type;
        QDateTime time;
        bool acknowledged;
    };
    
    QVector<Reminder> reminders;
};

#endif // ALERTSSCREEN_H
