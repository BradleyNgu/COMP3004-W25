#ifndef HOMESCREEN_H
#define HOMESCREEN_H

#include <QWidget>
#include <QDateTime>
#include <QTimer>
#include <QLabel>
#include <QProgressBar>
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
    
public slots:
    void updateBatteryLevel(int level);
    void updateInsulinRemaining(double units);
    void updateGlucoseLevel(double value);
    void updateGlucoseTrend(GlucoseModel::TrendDirection trend);
    void updateInsulinOnBoard(double units);
    void updateControlIQAction(double value);
    void updateGlucoseGraph(const QVector<QPair<QDateTime, double>> &data);
    void updateDateTime();
    
signals:
    void bolusButtonClicked();
    void optionsButtonClicked();
    void powerButtonPressed();
    
protected:
    void paintEvent(QPaintEvent *event) override;
    
private slots:
    void on_bolusButton_clicked();
    void on_optionsButton_clicked();
    void on_powerButton_clicked();
    
private:
    Ui::HomeScreen *ui;
    QTimer *dateTimeTimer;
    QVector<QPair<QDateTime, double>> graphData;
    
    // Dynamic UI elements
    QProgressBar *batteryBar;
    QProgressBar *insulinBar;
    
    void drawGlucoseGraph(QPainter &painter);
};

#endif // HOMESCREEN_H
