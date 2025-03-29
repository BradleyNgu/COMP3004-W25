#ifndef HOMESCREEN_H
#define HOMESCREEN_H

#include <QWidget>
#include <QDateTime>
#include <QTimer>
#include <QLabel>
#include <QPushButton>
#include <QProgressBar>
#include <QVector>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <QMenu>
#include <QEvent>
#include "graphview.h"
#include "../controllers/pumpcontroller.h"

namespace Ui {
class HomeScreen;
}

class HomeScreen : public QWidget
{
    Q_OBJECT

public:
    explicit HomeScreen(QWidget *parent = nullptr);
    ~HomeScreen();
    
    void updateAllData(PumpController *controller);
    void updateFontSizes();
    void setTimelineRange(int hours);
    
public slots:
    void updateBatteryLevel(int level);
    void updateInsulinRemaining(double units);
    void updateGlucoseLevel(double value);
    void updateGlucoseTrend(GlucoseModel::TrendDirection trend);
    void updateInsulinOnBoard(double units);
    void updateControlIQAction(double value);
    void updateGlucoseGraph(const QVector<QPair<QDateTime, double>> &data);
    void updateDateTime();
    void showTimelineOptions(const QPoint &pos = QPoint());
    void onTimeRangeChanged(int hours);
    
signals:
    void bolusButtonClicked();
    void optionsButtonClicked();
    void powerButtonPressed();
    void historyButtonClicked();
    void controlIQButtonClicked();
    
protected:
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;
    
private slots:
    void onBolusButtonClicked();
    void onOptionsButtonClicked();
    void onPowerButtonClicked();
    
private:
    Ui::HomeScreen *ui;
    QTimer *dateTimeTimer = nullptr;
    QTimer *graphUpdateTimer = nullptr;
    QVector<QPair<QDateTime, double>> graphData;
    
    // Main layouts
    QVBoxLayout *mainLayout;
    
    // Status bar elements
    QFrame *statusBar;
    QLabel *timeLabel;
    QLabel *dateLabel;
    
    // Battery and insulin indicators
    QWidget *batteryWidget;
    QVector<QLabel*> batteryBars;
    QLabel *batteryLabel;
    
    QWidget *insulinWidget;
    QVector<QLabel*> insulinBars;
    QLabel *insulinLabel;
    
    // Status icons
    QLabel *cgmIcon;
    QLabel *dropIcon;
    QLabel *bluetoothIcon;
    
    // Graph and glucose display
    GraphView *graphView;
    QLabel *glucoseLabel;
    QLabel *glucoseUnitLabel;
    QLabel *glucoseTrendLabel;
    QLabel *timeframeLabel;
    
    // Control area
    QLabel *insulinOnBoardLabel;
    QLabel *iobValueLabel;
    QLabel *controlIQLabel;
    QPushButton *bolusButton;
    QPushButton *optionsButton;
    QPushButton *powerButton;
    
    // Bolus status indicators
    QLabel *bolusStatus1;
    QLabel *bolusStatus2;
    QLabel *bolusStatus3;
    
    void setupUi();
    void generateSampleGraphData();
};

#endif // HOMESCREEN_H
