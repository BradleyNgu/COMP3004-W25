#include "historyscreen.h"
#include "ui_historyscreen.h"
#include <QStyleOption>
#include <QPainter>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTabWidget>
#include <QTableWidget>
#include <QHeaderView>
#include <QDateEdit>
#include <QTimeEdit>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <QGroupBox>
#include <QRadioButton>

HistoryScreen::HistoryScreen(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::HistoryScreen),
    pumpController(nullptr)
{
    ui->setupUi(this);
    
    // Setup custom UI with layouts
    setupUi();
    
    // Connect signals to slots
    connectSignals();
}

HistoryScreen::~HistoryScreen()
{
    delete ui;
}

void HistoryScreen::setPumpController(PumpController *controller)
{
    pumpController = controller;
    
    // Initialize date range
    QDateTime endDate = QDateTime::currentDateTime();
    QDateTime startDate = endDate.addDays(-7); // Default to 7 days of history
    
    fromDateEdit->setDateTime(startDate);
    toDateEdit->setDateTime(endDate);
    
    // Update data
    updateHistoryData();
}

void HistoryScreen::setupUi()
{
    // Set dark background
    setStyleSheet("background-color: #222222;");
    
    // Main layout for entire screen
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(16, 16, 16, 16);
    mainLayout->setSpacing(12);

    QHBoxLayout *topLayout = new QHBoxLayout();
    topLayout->setAlignment(Qt::AlignRight);

    homeButton = new QPushButton("T");
    homeButton->setStyleSheet(
        "QPushButton { background-color: transparent; color: #00B2FF; font-size: 20px; font-weight: bold; border: none; }"
        "QPushButton:pressed { color: #0077CC; }"
    );
    homeButton->setFixedSize(40, 40);

    topLayout->addWidget(homeButton);
    mainLayout->addLayout(topLayout);
    
    // Title
    titleLabel = new QLabel("History");
    titleLabel->setStyleSheet("color: white; font-size: 24px; font-weight: bold;");
    titleLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(titleLabel);
    
    // Date range selector
    QGroupBox *dateRangeGroup = new QGroupBox("Date Range");
    dateRangeGroup->setStyleSheet("QGroupBox { color: white; font-weight: bold; border: 1px solid #444444; border-radius: 5px; padding: 10px; }");
    
    QHBoxLayout *dateRangeLayout = new QHBoxLayout(dateRangeGroup);
    dateRangeLayout->setContentsMargins(8, 16, 8, 8);
    dateRangeLayout->setSpacing(8);
    
    QLabel *fromLabel = new QLabel("From:");
    fromLabel->setStyleSheet("color: white;");
    
    fromDateEdit = new QDateTimeEdit();
    fromDateEdit->setCalendarPopup(true);
    fromDateEdit->setDisplayFormat("yyyy-MM-dd hh:mm");
    fromDateEdit->setStyleSheet(
        "QDateTimeEdit { background-color: #333333; color: white; border: 1px solid #666666; border-radius: 3px; padding: 4px; }"
        "QDateTimeEdit::drop-down { border: 0px; width: 20px; }"
    );
    
    QLabel *toLabel = new QLabel("To:");
    toLabel->setStyleSheet("color: white;");
    
    toDateEdit = new QDateTimeEdit();
    toDateEdit->setCalendarPopup(true);
    toDateEdit->setDisplayFormat("yyyy-MM-dd hh:mm");
    toDateEdit->setStyleSheet(
        "QDateTimeEdit { background-color: #333333; color: white; border: 1px solid #666666; border-radius: 3px; padding: 4px; }"
        "QDateTimeEdit::drop-down { border: 0px; width: 20px; }"
    );
    
    QPushButton *updateButton = new QPushButton("Update");
    updateButton->setStyleSheet(
        "QPushButton { background-color: #00B2FF; color: white; font-weight: bold; border-radius: 3px; padding: 5px 15px; }"
        "QPushButton:pressed { background-color: #0080FF; }"
    );
    connect(updateButton, &QPushButton::clicked, this, &HistoryScreen::updateHistoryData);
    
    // Quick selection buttons
    QPushButton *today = new QPushButton("Today");
    QPushButton *day3 = new QPushButton("3 Days");
    QPushButton *week1 = new QPushButton("1 Week");
    QPushButton *month1 = new QPushButton("1 Month");
    
    today->setStyleSheet("QPushButton { background-color: #333333; color: white; border-radius: 3px; padding: 5px; }");
    day3->setStyleSheet("QPushButton { background-color: #333333; color: white; border-radius: 3px; padding: 5px; }");
    week1->setStyleSheet("QPushButton { background-color: #333333; color: white; border-radius: 3px; padding: 5px; }");
    month1->setStyleSheet("QPushButton { background-color: #333333; color: white; border-radius: 3px; padding: 5px; }");
    
    connect(today, &QPushButton::clicked, this, &HistoryScreen::setTodayRange);
    connect(day3, &QPushButton::clicked, this, &HistoryScreen::set3DayRange);
    connect(week1, &QPushButton::clicked, this, &HistoryScreen::set1WeekRange);
    connect(month1, &QPushButton::clicked, this, &HistoryScreen::set1MonthRange);
    
    QHBoxLayout *quickSelectLayout = new QHBoxLayout();
    quickSelectLayout->addWidget(today);
    quickSelectLayout->addWidget(day3);
    quickSelectLayout->addWidget(week1);
    quickSelectLayout->addWidget(month1);
    
    dateRangeLayout->addWidget(fromLabel);
    dateRangeLayout->addWidget(fromDateEdit, 1);
    dateRangeLayout->addWidget(toLabel);
    dateRangeLayout->addWidget(toDateEdit, 1);
    dateRangeLayout->addWidget(updateButton);
    
    QVBoxLayout *dateGroupLayout = new QVBoxLayout();
    dateGroupLayout->addLayout(dateRangeLayout);
    dateGroupLayout->addLayout(quickSelectLayout);
    dateRangeGroup->setLayout(dateGroupLayout);
    
    mainLayout->addWidget(dateRangeGroup);
    
    // Create tab widget for different history views
    tabWidget = new QTabWidget();
    tabWidget->setStyleSheet(
        "QTabWidget::pane { border: 1px solid #444444; background-color: #333333; }"
        "QTabBar::tab { background-color: #222222; color: white; padding: 8px 16px; margin-right: 2px; }"
        "QTabBar::tab:selected { background-color: #333333; border-bottom: 2px solid #00B2FF; }"
        "QTabBar::tab:hover { background-color: #2a2a2a; }"
    );
    
    // Create glucose table
    glucoseTable = new QTableWidget();
    glucoseTable->setColumnCount(3);
    glucoseTable->setHorizontalHeaderLabels(QStringList() << "Time" << "Glucose (mmol/L)" << "Trend");
    glucoseTable->setStyleSheet(
        "QTableWidget { background-color: #333333; color: white; gridline-color: #444444; }"
        "QHeaderView::section { background-color: #444444; color: white; padding: 4px; border: 1px solid #555555; }"
        "QTableWidget::item { padding: 4px; }"
    );
    glucoseTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    glucoseTable->verticalHeader()->setVisible(false);
    
    // Create insulin table
    insulinTable = new QTableWidget();
    insulinTable->setColumnCount(3);
    insulinTable->setHorizontalHeaderLabels(QStringList() << "Time" << "Type" << "Units");
    insulinTable->setStyleSheet(
        "QTableWidget { background-color: #333333; color: white; gridline-color: #444444; }"
        "QHeaderView::section { background-color: #444444; color: white; padding: 4px; border: 1px solid #555555; }"
        "QTableWidget::item { padding: 4px; }"
    );
    insulinTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    insulinTable->verticalHeader()->setVisible(false);
    
    // Create alerts table
    alertsTable = new QTableWidget();
    alertsTable->setColumnCount(3);
    alertsTable->setHorizontalHeaderLabels(QStringList() << "Time" << "Alert" << "Level");
    alertsTable->setStyleSheet(
        "QTableWidget { background-color: #333333; color: white; gridline-color: #444444; }"
        "QHeaderView::section { background-color: #444444; color: white; padding: 4px; border: 1px solid #555555; }"
        "QTableWidget::item { padding: 4px; }"
    );
    alertsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    alertsTable->verticalHeader()->setVisible(false);
    
    // Create graph view for visual history
    graphView = new GraphView();
    graphView->setMinimumHeight(300);
    
    // Create control-IQ table
    controlIQTable = new QTableWidget();
    controlIQTable->setColumnCount(4);
    controlIQTable->setHorizontalHeaderLabels(QStringList() << "Time" << "Action" << "Reason" << "Adjustment");
    controlIQTable->setStyleSheet(
        "QTableWidget { background-color: #333333; color: white; gridline-color: #444444; }"
        "QHeaderView::section { background-color: #444444; color: white; padding: 4px; border: 1px solid #555555; }"
        "QTableWidget::item { padding: 4px; }"
    );
    controlIQTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    controlIQTable->verticalHeader()->setVisible(false);
    
    // Timeline control for graph
    QGroupBox *timelineGroup = new QGroupBox("Timeline");
    timelineGroup->setStyleSheet("QGroupBox { color: white; font-weight: bold; border: 1px solid #444444; border-radius: 5px; padding: 10px; }");
    
    QHBoxLayout *timelineLayout = new QHBoxLayout(timelineGroup);
    timelineLayout->setContentsMargins(8, 16, 8, 8);
    
    QStringList timeRanges = {"1 Hour", "3 Hours", "6 Hours", "12 Hours", "24 Hours", "48 Hours"};
    timelineComboBox = new QComboBox();
    timelineComboBox->addItems(timeRanges);
    timelineComboBox->setCurrentIndex(1); // Default to 3 hours
    timelineComboBox->setStyleSheet(
        "QComboBox { background-color: #333333; color: white; border: 1px solid #666666; border-radius: 3px; padding: 4px; }"
        "QComboBox::drop-down { border: 0px; width: 20px; }"
        "QComboBox QAbstractItemView { background-color: #333333; color: white; selection-background-color: #00B2FF; }"
    );
    
    connect(timelineComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &HistoryScreen::onTimelineRangeChanged);
    
    timelineLabel = new QLabel("Display Range:");
    timelineLabel->setStyleSheet("color: white;");
    
    timelineLayout->addWidget(timelineLabel);
    timelineLayout->addWidget(timelineComboBox);
    timelineLayout->addStretch(1);
    
    // Data type selector for graph
    QGroupBox *dataTypeGroup = new QGroupBox("Data Display");
    dataTypeGroup->setStyleSheet("QGroupBox { color: white; font-weight: bold; border: 1px solid #444444; border-radius: 5px; padding: 10px; }");
    
    QHBoxLayout *dataTypeLayout = new QHBoxLayout(dataTypeGroup);
    dataTypeLayout->setContentsMargins(8, 16, 8, 8);
    
    glucoseRadio = new QRadioButton("Glucose");
    insulinRadio = new QRadioButton("Insulin");
    combinedRadio = new QRadioButton("Combined");
    
    glucoseRadio->setStyleSheet("QRadioButton { color: white; }");
    insulinRadio->setStyleSheet("QRadioButton { color: white; }");
    combinedRadio->setStyleSheet("QRadioButton { color: white; }");
    
    glucoseRadio->setChecked(true); // Default to glucose view
    
    connect(glucoseRadio, &QRadioButton::toggled, this, &HistoryScreen::onGraphTypeChanged);
    connect(insulinRadio, &QRadioButton::toggled, this, &HistoryScreen::onGraphTypeChanged);
    connect(combinedRadio, &QRadioButton::toggled, this, &HistoryScreen::onGraphTypeChanged);
    
    dataTypeLayout->addWidget(glucoseRadio);
    dataTypeLayout->addWidget(insulinRadio);
    dataTypeLayout->addWidget(combinedRadio);
    dataTypeLayout->addStretch(1);
    
    // Layout for graph tab
    QWidget *graphTab = new QWidget();
    QVBoxLayout *graphLayout = new QVBoxLayout(graphTab);
    graphLayout->addWidget(graphView);
    graphLayout->addWidget(timelineGroup);
    graphLayout->addWidget(dataTypeGroup);
    
    // Add tabs to tab widget
    tabWidget->addTab(graphTab, "Graph");
    tabWidget->addTab(glucoseTable, "Glucose");
    tabWidget->addTab(insulinTable, "Insulin");
    tabWidget->addTab(controlIQTable, "Control-IQ");
    tabWidget->addTab(alertsTable, "Alerts");
    
    mainLayout->addWidget(tabWidget, 1);
    
    // Back button
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    
    backButton = new QPushButton("Back");
    backButton->setStyleSheet(
        "QPushButton { background-color: #444444; color: white; border-radius: 5px; padding: 8px 16px; }"
        "QPushButton:pressed { background-color: #666666; }"
    );
    backButton->setFixedWidth(100);
    
    buttonLayout->addWidget(backButton);
    
    mainLayout->addLayout(buttonLayout);
    
    // Set layout
    setLayout(mainLayout);
}

void HistoryScreen::connectSignals()
{
    connect(backButton, &QPushButton::clicked, this, &HistoryScreen::backButtonClicked);
    connect(homeButton, &QPushButton::clicked, this, [this]() {emit homeButtonClicked();
    });
}

void HistoryScreen::updateHistoryData()
{
    if (!pumpController) return;
    
    // Get date range
    QDateTime startDate = fromDateEdit->dateTime();
    QDateTime endDate = toDateEdit->dateTime();
    
    // Update glucose data
    updateGlucoseTable(startDate, endDate);
    
    // Update insulin data
    updateInsulinTable(startDate, endDate);
    
    // Update control-IQ data
    updateControlIQTable(startDate, endDate);
    
    // Update alerts data
    updateAlertsTable(startDate, endDate);
    
    // Update graph view
    updateGraphView(startDate, endDate);
}

void HistoryScreen::updateGlucoseTable(const QDateTime &start, const QDateTime &end)
{
    if (!pumpController) return;
    
    // Get glucose history
    QVector<QPair<QDateTime, double>> glucoseHistory = pumpController->getGlucoseHistory(start, end);
    
    // Clear table
    glucoseTable->setRowCount(0);
    
    // Add data to table
    for (const auto &reading : glucoseHistory) {
        int row = glucoseTable->rowCount();
        glucoseTable->insertRow(row);
        
        // Time
        QTableWidgetItem *timeItem = new QTableWidgetItem(reading.first.toString("yyyy-MM-dd hh:mm:ss"));
        
        // Glucose value
        QTableWidgetItem *valueItem = new QTableWidgetItem(QString::number(reading.second, 'f', 1));
        
        // Trend (this would need to be stored with each reading in a real implementation)
        QTableWidgetItem *trendItem = new QTableWidgetItem("–");
        
        // Color based on value
        if (reading.second < 3.9) {
            valueItem->setForeground(QColor(255, 59, 48)); // Red for low
        } else if (reading.second > 10.0) {
            valueItem->setForeground(QColor(255, 149, 0)); // Orange for high
        } else {
            valueItem->setForeground(QColor(0, 178, 255)); // Blue for in-range
        }
        
        glucoseTable->setItem(row, 0, timeItem);
        glucoseTable->setItem(row, 1, valueItem);
        glucoseTable->setItem(row, 2, trendItem);
    }
    
    // Sort by time (newest first)
    glucoseTable->sortItems(0, Qt::DescendingOrder);
}

void HistoryScreen::updateInsulinTable(const QDateTime &start, const QDateTime &end)
{
    if (!pumpController) return;
    
    // Get insulin history
    QVector<QPair<QDateTime, double>> insulinHistory = pumpController->getInsulinHistory(start, end);
    
    // Clear table
    insulinTable->setRowCount(0);
    
    // Add data to table
    for (const auto &delivery : insulinHistory) {
        int row = insulinTable->rowCount();
        insulinTable->insertRow(row);
        
        // Time
        QTableWidgetItem *timeItem = new QTableWidgetItem(delivery.first.toString("yyyy-MM-dd hh:mm:ss"));
        
        // Type (in a real implementation, this would distinguish between bolus and basal)
        QTableWidgetItem *typeItem = new QTableWidgetItem("Insulin");
        
        // Units
        QTableWidgetItem *unitsItem = new QTableWidgetItem(QString::number(delivery.second, 'f', 2) + " u");
        
        insulinTable->setItem(row, 0, timeItem);
        insulinTable->setItem(row, 1, typeItem);
        insulinTable->setItem(row, 2, unitsItem);
    }
    
    // Sort by time (newest first)
    insulinTable->sortItems(0, Qt::DescendingOrder);
}

void HistoryScreen::updateControlIQTable(const QDateTime &start, const QDateTime &end)
{
    // For demo purposes - in a real implementation, we would fetch actual Control-IQ history
    controlIQTable->setRowCount(0);
    
    // Sample data
    QDateTime now = QDateTime::currentDateTime();
    
    for (int i = 0; i < 10; i++) {
        QDateTime timestamp = now.addSecs(-i * 3600); // Every hour
        
        if (timestamp < start || timestamp > end) {
            continue;
        }
        
        int row = controlIQTable->rowCount();
        controlIQTable->insertRow(row);
        
        // Time
        QTableWidgetItem *timeItem = new QTableWidgetItem(timestamp.toString("yyyy-MM-dd hh:mm:ss"));
        
        // Action
        QString action = (i % 3 == 0) ? "Decreased Basal" : (i % 3 == 1) ? "Increased Basal" : "No Change";
        QTableWidgetItem *actionItem = new QTableWidgetItem(action);
        
        // Reason
        QString reason = (i % 3 == 0) ? "Glucose Trending Down" : 
                        (i % 3 == 1) ? "Glucose Trending Up" : "Stable Glucose";
        QTableWidgetItem *reasonItem = new QTableWidgetItem(reason);
        
        // Adjustment
        double adjustment = (i % 3 == 0) ? -0.1 - (i % 5) * 0.1 : 
                          (i % 3 == 1) ? 0.1 + (i % 5) * 0.1 : 0.0;
        QTableWidgetItem *adjustmentItem = new QTableWidgetItem(QString::number(adjustment, 'f', 2) + " u");
        
        if (adjustment < 0) {
            adjustmentItem->setForeground(QColor(255, 59, 48)); // Red for decrease
        } else if (adjustment > 0) {
            adjustmentItem->setForeground(QColor(52, 199, 89)); // Green for increase
        }
        
        controlIQTable->setItem(row, 0, timeItem);
        controlIQTable->setItem(row, 1, actionItem);
        controlIQTable->setItem(row, 2, reasonItem);
        controlIQTable->setItem(row, 3, adjustmentItem);
    }
    
    // Sort by time (newest first)
    controlIQTable->sortItems(0, Qt::DescendingOrder);
}

void HistoryScreen::updateAlertsTable(const QDateTime &start, const QDateTime &end)
{
    // For demo purposes - in a real implementation, we would fetch actual alert history
    alertsTable->setRowCount(0);
    
    // Sample data
    QDateTime now = QDateTime::currentDateTime();
    
    QStringList alertMessages = {
        "Low Glucose: 3.2 mmol/L",
        "High Glucose: 13.8 mmol/L",
        "Insulin Reservoir Low",
        "Battery Low: 15%",
        "CGM Signal Lost",
        "Basal Delivery Suspended"
    };
    
    QStringList alertLevels = {
        "Warning",
        "Warning",
        "Info",
        "Warning",
        "Warning",
        "Critical"
    };
    
    QList<QColor> alertColors = {
        QColor(255, 59, 48),   // Red for critical/low glucose
        QColor(255, 149, 0),   // Orange for high glucose
        QColor(255, 255, 255), // White for info
        QColor(255, 204, 0),   // Yellow for warnings
        QColor(255, 204, 0),   // Yellow for warnings
        QColor(255, 59, 48)    // Red for critical
    };
    
    for (int i = 0; i < alertMessages.size(); i++) {
        QDateTime timestamp = now.addSecs(-i * 7200); // Every 2 hours
        
        if (timestamp < start || timestamp > end) {
            continue;
        }
        
        int row = alertsTable->rowCount();
        alertsTable->insertRow(row);
        
        // Time
        QTableWidgetItem *timeItem = new QTableWidgetItem(timestamp.toString("yyyy-MM-dd hh:mm:ss"));
        
        // Alert message
        QTableWidgetItem *alertItem = new QTableWidgetItem(alertMessages[i]);
        alertItem->setForeground(alertColors[i]);
        
        // Alert level
        QTableWidgetItem *levelItem = new QTableWidgetItem(alertLevels[i]);
        levelItem->setForeground(alertColors[i]);
        
        alertsTable->setItem(row, 0, timeItem);
        alertsTable->setItem(row, 1, alertItem);
        alertsTable->setItem(row, 2, levelItem);
    }
    
    // Sort by time (newest first)
    alertsTable->sortItems(0, Qt::DescendingOrder);
}

void HistoryScreen::updateGraphView(const QDateTime &start, const QDateTime &end)
{
    if (!pumpController) return;
    
    // Update graph data
    graphView->setGlucoseData(pumpController->getGlucoseHistory(start, end));
    graphView->setInsulinData(pumpController->getInsulinHistory(start, end));
    
    // Set time range
    graphView->setTimeRange(start, end);
    
    // Update graph display based on radio button selection
    onGraphTypeChanged();
}

void HistoryScreen::onTimelineRangeChanged(int index)
{
    // Convert index to hours
    int hours = 3; // Default
    
    switch (index) {
        case 0: hours = 1; break;
        case 1: hours = 3; break;
        case 2: hours = 6; break;
        case 3: hours = 12; break;
        case 4: hours = 24; break;
        case 5: hours = 48; break;
        default: hours = 3; break;
    }
    
    // Update graph view with new time range
    graphView->setTimeRangeHours(hours);
}

void HistoryScreen::onGraphTypeChanged()
{
    if (glucoseRadio->isChecked()) {
        graphView->setDisplayType(GraphView::GlucoseData);
    } else if (insulinRadio->isChecked()) {
        graphView->setDisplayType(GraphView::InsulinData);
    } else if (combinedRadio->isChecked()) {
        graphView->setDisplayType(GraphView::CombinedData);
    }
}

void HistoryScreen::setTodayRange()
{
    QDateTime endDate = QDateTime::currentDateTime();
    QDateTime startDate = QDateTime(endDate.date(), QTime(0, 0, 0));
    
    fromDateEdit->setDateTime(startDate);
    toDateEdit->setDateTime(endDate);
    
    updateHistoryData();
}

void HistoryScreen::set3DayRange()
{
    QDateTime endDate = QDateTime::currentDateTime();
    QDateTime startDate = endDate.addDays(-3);
    
    fromDateEdit->setDateTime(startDate);
    toDateEdit->setDateTime(endDate);
    
    updateHistoryData();
}

void HistoryScreen::set1WeekRange()
{
    QDateTime endDate = QDateTime::currentDateTime();
    QDateTime startDate = endDate.addDays(-7);
    
    fromDateEdit->setDateTime(startDate);
    toDateEdit->setDateTime(endDate);
    
    updateHistoryData();
}

void HistoryScreen::set1MonthRange()
{
    QDateTime endDate = QDateTime::currentDateTime();
    QDateTime startDate = endDate.addMonths(-1);
    
    fromDateEdit->setDateTime(startDate);
    toDateEdit->setDateTime(endDate);
    
    updateHistoryData();
}

void HistoryScreen::paintEvent(QPaintEvent * /* event */)
{
    // Draw the widget background and style
    QStyleOption opt;
    opt.init(this);
    QPainter painter(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &painter, this);
}
