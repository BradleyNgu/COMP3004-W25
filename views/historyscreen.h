#ifndef HISTORYSCREEN_H
#define HISTORYSCREEN_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QTabWidget>
#include <QTableWidget>
#include <QDateTimeEdit>
#include <QComboBox>
#include <QRadioButton>
#include "graphview.h"
#include "../controllers/pumpcontroller.h"

namespace Ui {
class HistoryScreen;
}

class HistoryScreen : public QWidget
{
    Q_OBJECT

public:
    explicit HistoryScreen(QWidget *parent = nullptr);
    ~HistoryScreen();
    
    void setPumpController(PumpController *controller);
    QTabWidget* getTabWidget() const { return tabWidget; }
    void updateHistoryData();
    
signals:
    void backButtonClicked();
    
protected:
    void paintEvent(QPaintEvent *event) override;
    
private slots:
    void onTimelineRangeChanged(int index);
    void onGraphTypeChanged();
    void setTodayRange();
    void set3DayRange();
    void set1WeekRange();
    void set1MonthRange();
    
private:
    Ui::HistoryScreen *ui;
    PumpController *pumpController;
    
    // UI elements
    QLabel *titleLabel;
    QTabWidget *tabWidget;
    QTableWidget *glucoseTable;
    QTableWidget *insulinTable;
    QTableWidget *alertsTable;
    QTableWidget *controlIQTable;
    GraphView *graphView;
    QDateTimeEdit *fromDateEdit;
    QDateTimeEdit *toDateEdit;
    QComboBox *timelineComboBox;
    QLabel *timelineLabel;
    QRadioButton *glucoseRadio;
    QRadioButton *insulinRadio;
    QRadioButton *combinedRadio;
    QPushButton *backButton;
    
    void setupUi();
    void connectSignals();
    void updateGlucoseTable(const QDateTime &start, const QDateTime &end);
    void updateInsulinTable(const QDateTime &start, const QDateTime &end);
    void updateControlIQTable(const QDateTime &start, const QDateTime &end);
    void updateAlertsTable(const QDateTime &start, const QDateTime &end);
    void updateGraphView(const QDateTime &start, const QDateTime &end);
};

#endif // HISTORYSCREEN_H
