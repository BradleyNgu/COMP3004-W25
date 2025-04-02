#include "homescreen.h"
#include "ui_homescreen.h"
#include <QPainter>
#include <QDateTime>
#include <QStyleOption>
#include <QTimer>
#include <QMessageBox>
#include <QFont>
#include <QFontMetrics>
#include <QtMath>
#include <QResizeEvent>
#include <QMenu>
#include <QAction>
#include <cstdlib> // For rand() and RAND_MAX
#include <ctime>   // For time()
#include <algorithm>
#include <QRandomGenerator>

HomeScreen::HomeScreen(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::HomeScreen)
{
    ui->setupUi(this);
    
    // Set the dark background color for entire screen
    setStyleSheet("QWidget { background-color: #222222; }");
    
    // Set up UI components using layouts
    setupUi();
    
    // Set up timer to update date and time
    dateTimeTimer = new QTimer(this);
    connect(dateTimeTimer, &QTimer::timeout, this, &HomeScreen::updateDateTime);
    dateTimeTimer->start(60000); // Update every minute
    
    // Initially update date/time
    updateDateTime();
}

HomeScreen::~HomeScreen()
{
    delete ui;
    if (graphUpdateTimer) {
        graphUpdateTimer->stop();
    }
    if (dateTimeTimer) {
        dateTimeTimer->stop();
    }
}

void HomeScreen::setupUi()
{
    // Main layout for the entire screen
    mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(8, 8, 8, 8);
    mainLayout->setSpacing(4);
    
    // *** STATUS BAR SECTION ***
    
    // Status bar at the top
    statusBar = new QFrame(this);
    statusBar->setStyleSheet("QFrame { background-color: #333333; border-radius: 4px; }");
    statusBar->setFixedHeight(40);
    
    QHBoxLayout *statusBarLayout = new QHBoxLayout(statusBar);
    statusBarLayout->setContentsMargins(4, 4, 4, 4);
    
    // Battery indicator
    batteryWidget = new QWidget();
    batteryWidget->setFixedWidth(60);
    QVBoxLayout *batteryLayout = new QVBoxLayout(batteryWidget);
    batteryLayout->setContentsMargins(2, 2, 2, 2);
    batteryLayout->setSpacing(1);
    
    batteryBars.clear();
    QHBoxLayout *batteryBarsLayout = new QHBoxLayout();
    batteryBarsLayout->setSpacing(1);
    for (int i = 0; i < 5; i++) {
        QLabel *bar = new QLabel();
        bar->setStyleSheet("background-color: #4CD964; border-radius: 2px;");
        bar->setFixedHeight(10);
        batteryBars.append(bar);
        batteryBarsLayout->addWidget(bar);
    }
    
    batteryLabel = new QLabel("100%");
    batteryLabel->setStyleSheet("color: #4CD964; font-weight: bold;");
    batteryLabel->setAlignment(Qt::AlignCenter);
    
    batteryLayout->addLayout(batteryBarsLayout);
    batteryLayout->addWidget(batteryLabel);
    
    // Time and date
    QWidget *timeWidget = new QWidget();
    QVBoxLayout *timeLayout = new QVBoxLayout(timeWidget);
    timeLayout->setContentsMargins(2, 2, 2, 2);
    timeLayout->setSpacing(0);
    
    timeLabel = new QLabel("12:34");
    timeLabel->setStyleSheet("color: white; font-weight: bold;");
    timeLabel->setAlignment(Qt::AlignCenter);
    
    dateLabel = new QLabel("21 Mar");
    dateLabel->setStyleSheet("color: white;");
    dateLabel->setAlignment(Qt::AlignCenter);
    
    timeLayout->addWidget(timeLabel);
    timeLayout->addWidget(dateLabel);
    
    // Status icons
    QWidget *iconWidget = new QWidget();
    QHBoxLayout *iconLayout = new QHBoxLayout(iconWidget);
    
    tandemLogoIcon = new QLabel("T");
    tandemLogoIcon->setStyleSheet("color: #00B2FF; font-size: 20px; font-weight: bold;");
    iconLayout->addWidget(tandemLogoIcon);
    
    // Insulin indicator
    insulinWidget = new QWidget();
    insulinWidget->setFixedWidth(60);
    QVBoxLayout *insulinLayout = new QVBoxLayout(insulinWidget);
    insulinLayout->setContentsMargins(2, 2, 2, 2);
    insulinLayout->setSpacing(1);
    
    insulinBars.clear();
    QHBoxLayout *insulinBarsLayout = new QHBoxLayout();
    insulinBarsLayout->setSpacing(1);
    for (int i = 0; i < 5; i++) {
        QLabel *bar = new QLabel();
        bar->setStyleSheet("background-color: #00B2FF; border-radius: 2px;");
        bar->setFixedHeight(10);
        insulinBars.append(bar);
        insulinBarsLayout->addWidget(bar);
    }
    
    insulinLabel = new QLabel("300 u");
    insulinLabel->setStyleSheet("color: #00B2FF; font-weight: bold;");
    insulinLabel->setAlignment(Qt::AlignCenter);
    
    insulinLayout->addLayout(insulinBarsLayout);
    insulinLayout->addWidget(insulinLabel);
    
    // Add all elements to status bar
    statusBarLayout->addWidget(batteryWidget);
    statusBarLayout->addWidget(timeWidget, 2);
    statusBarLayout->addWidget(iconWidget);
    statusBarLayout->addWidget(insulinWidget);
    
    // *** GRAPH SECTION ***
    
    // Create GraphView for glucose data
    graphView = new GraphView();
    graphView->setStyleSheet("background-color: #171717; border: 2px solid #444444; border-radius: 4px;");
    graphView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    graphView->setMinimumHeight(150);
    graphView->setDisplayType(GraphView::GlucoseData);
    graphView->setTargetRange(3.9, 10.0);
    
    // Set context menu policy for timeline options
    graphView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(graphView, &QWidget::customContextMenuRequested, this, &HomeScreen::showTimelineOptions);
    connect(graphView, &GraphView::timeRangeChanged, this, &HomeScreen::onTimeRangeChanged);
    
    // *** GLUCOSE INFO SECTION ***
    
    // Glucose display - uses a horizontal layout
    QWidget *glucoseWidget = new QWidget();
    QHBoxLayout *glucoseLayout = new QHBoxLayout(glucoseWidget);
    glucoseLayout->setContentsMargins(4, 4, 4, 4);
    
    // Center glucose value display
    QWidget *glucoseValueWidget = new QWidget();
    QVBoxLayout *glucoseValueLayout = new QVBoxLayout(glucoseValueWidget);
    glucoseValueLayout->setAlignment(Qt::AlignCenter);
    
    glucoseLabel = new QLabel("5.5");
    glucoseLabel->setStyleSheet("color: white; font-weight: bold;");
    glucoseLabel->setAlignment(Qt::AlignCenter);
    
    glucoseUnitLabel = new QLabel("mmol/L");
    glucoseUnitLabel->setStyleSheet("color: white;");
    glucoseUnitLabel->setAlignment(Qt::AlignCenter);
    
    timeframeLabel = new QLabel("3 HRS");
    timeframeLabel->setStyleSheet("color: white;");
    timeframeLabel->setAlignment(Qt::AlignCenter);
    
    // Make timeframeLabel clickable
    timeframeLabel->setCursor(Qt::PointingHandCursor);
    timeframeLabel->setToolTip("Click to change timeline");
    
    glucoseValueLayout->addWidget(glucoseLabel);
    glucoseValueLayout->addWidget(glucoseUnitLabel);
    glucoseValueLayout->addWidget(timeframeLabel);
    
    // Glucose trend arrow
    glucoseTrendLabel = new QLabel("→");
    glucoseTrendLabel->setStyleSheet("color: white;");
    glucoseTrendLabel->setAlignment(Qt::AlignCenter);
    
    // Add to glucose layout with proper spacing
    glucoseLayout->addStretch(1);
    glucoseLayout->addWidget(glucoseValueWidget, 3);
    glucoseLayout->addWidget(glucoseTrendLabel, 1);
    glucoseLayout->addStretch(1);
    
    // *** CONTROL SECTION ***
    
    // Insulin on board info
    QWidget *iobWidget = new QWidget();
    QHBoxLayout *iobLayout = new QHBoxLayout(iobWidget);
    iobLayout->setContentsMargins(8, 2, 8, 2);
    
    insulinOnBoardLabel = new QLabel("INSULIN ON BOARD");
    insulinOnBoardLabel->setStyleSheet("color: white;");
    
    iobValueLabel = new QLabel("0.0 u");
    iobValueLabel->setStyleSheet("color: white; font-weight: bold;");
    iobValueLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    
    iobLayout->addWidget(insulinOnBoardLabel);
    iobLayout->addWidget(iobValueLabel);
    
    // Control buttons and indicators
    QWidget *controlWidget = new QWidget();
    QHBoxLayout *controlLayout = new QHBoxLayout(controlWidget);
    controlLayout->setContentsMargins(4, 4, 4, 4);
    
    // Options button on left
    optionsButton = new QPushButton("⚙");
    optionsButton->setStyleSheet(
        "QPushButton { background-color: transparent; color: white; font-weight: bold; border: none; }"
        "QPushButton:pressed { color: #BBBBBB; }"
    );
    optionsButton->setFixedSize(40, 40);
    
    // Bolus button in center
    QWidget *bolusWidget = new QWidget();
    QVBoxLayout *bolusLayout = new QVBoxLayout(bolusWidget);
    bolusLayout->setContentsMargins(0, 0, 0, 0);
    bolusLayout->setAlignment(Qt::AlignCenter);
    
    bolusButton = new QPushButton("BOLUS");
    bolusButton->setStyleSheet(
        "QPushButton { background-color: transparent; color: #00B2FF; font-weight: bold; border: none; }"
        "QPushButton:pressed { color: #0077CC; }"
    );
    
    // Bolus status indicators (dots)
    QWidget *dotsWidget = new QWidget();
    QHBoxLayout *dotsLayout = new QHBoxLayout(dotsWidget);
    dotsLayout->setContentsMargins(0, 0, 0, 0);
    dotsLayout->setAlignment(Qt::AlignCenter);
    dotsLayout->setSpacing(4);
    
    bolusStatus1 = new QLabel();
    bolusStatus1->setStyleSheet("QLabel { background-color: white; border-radius: 5px; }");
    bolusStatus1->setFixedSize(10, 10);
    
    bolusStatus2 = new QLabel();
    bolusStatus2->setStyleSheet("QLabel { background-color: white; border-radius: 5px; }");
    bolusStatus2->setFixedSize(10, 10);
    
    bolusStatus3 = new QLabel();
    bolusStatus3->setStyleSheet("QLabel { background-color: #00B2FF; border-radius: 5px; }");
    bolusStatus3->setFixedSize(10, 10);
    
    dotsLayout->addWidget(bolusStatus1);
    dotsLayout->addWidget(bolusStatus2);
    dotsLayout->addWidget(bolusStatus3);
    
    bolusLayout->addWidget(bolusButton);
    bolusLayout->addWidget(dotsWidget);
    
    // Control-IQ label
    controlIQLabel = new QLabel("Control-IQ: 0.00 u");
    controlIQLabel->setStyleSheet("color: white;");
    controlIQLabel->setAlignment(Qt::AlignCenter);
    
    // Power button on right
    powerButton = new QPushButton("X");
    powerButton->setStyleSheet(
        "QPushButton { background-color: #990000; color: white; font-weight: bold; border-radius: 10px; }"
        "QPushButton:pressed { background-color: #CC0000; }"
    );
    powerButton->setFixedSize(40, 40);
    
    // Add all control elements
    controlLayout->addWidget(optionsButton);
    controlLayout->addWidget(bolusWidget, 3);
    controlLayout->addWidget(controlIQLabel, 2);
    controlLayout->addWidget(powerButton);
    
    // *** ADD ALL SECTION TO MAIN LAYOUT ***
    
    mainLayout->addWidget(statusBar);
    mainLayout->addWidget(graphView, 5);
    mainLayout->addWidget(glucoseWidget, 2);
    mainLayout->addWidget(iobWidget, 1);
    mainLayout->addWidget(controlWidget, 1);
    
    // Connect button signals to slots
    connect(bolusButton, &QPushButton::clicked, this, &HomeScreen::onBolusButtonClicked);
    connect(optionsButton, &QPushButton::clicked, this, &HomeScreen::onOptionsButtonClicked);
    connect(powerButton, &QPushButton::clicked, this, &HomeScreen::onPowerButtonClicked);
    
    // Fix the problematic connection by using a lambda that adapts the parameter types
    // Instead of directly connecting to showTimelineOptions(const QPoint&)
    connect(timeframeLabel, &QLabel::linkActivated, [this](const QString&) {
        this->showTimelineOptions();
    });
    
    // Make timeframeLabel clickable
    timeframeLabel->installEventFilter(this);
    
    // Initial update of font sizes
    updateFontSizes();
    
    // Create sample data for the graph
    generateSampleGraphData();
}

void HomeScreen::generateSampleGraphData()
{
    // Generate 3 hours of sample data
    QDateTime endTime = QDateTime::currentDateTime();
    QDateTime startTime = endTime.addSecs(-3 * 60 * 60);
    
    QVector<QPair<QDateTime, double>> sampleData;
    
    // Create a realistic glucose pattern
    double baseValue = 5.5; // Start at normal level
    
    for (int i = 0; i <= 36; i++) { // 36 points = 5 minute intervals for 3 hours
        QDateTime pointTime = startTime.addSecs(i * 5 * 60);
        
        // Create a pattern with minor fluctuations around a curve
        double timeProgress = i / 36.0; // 0.0 to 1.0
        double patternValue;
        
        if (timeProgress < 0.3) {
            // Rising slightly
            patternValue = baseValue + timeProgress * 3.0;
        } else if (timeProgress < 0.6) {
            // Falling
            patternValue = baseValue + 0.9 - (timeProgress - 0.3) * 4.0;
        } else {
            // Gradual recovery
            patternValue = baseValue - 0.3 + (timeProgress - 0.6) * 1.5;
        }
        
        // Add small random noise using qrand() instead of QRandomGenerator
        double noise = (QRandomGenerator::global()->generateDouble() - 0.5) * 0.3;

        double value = patternValue + noise;
        
        // Ensure within realistic range
        value = qBound(3.0, value, 15.0);
        
        sampleData.append(qMakePair(pointTime, value));
    }
    
    // Set the sample data on the graph view
    graphView->setGlucoseData(sampleData);
    graphView->setTimeRange(startTime, endTime);
    
    // Update current glucose value display
    if (!sampleData.isEmpty()) {
        updateGlucoseLevel(sampleData.last().second);
    }
}

void HomeScreen::updateFontSizes()
{
    // Calculate base font size based on window size
    int width = this->width();
    int baseFontSize = qMax(9, qMin(18, width / 50));
    
    // Status bar
    int statusFontSize = baseFontSize;
    timeLabel->setStyleSheet(QString("color: white; font-size: %1px; font-weight: bold;").arg(statusFontSize));
    dateLabel->setStyleSheet(QString("color: white; font-size: %1px;").arg(statusFontSize - 2));
    batteryLabel->setStyleSheet(QString("color: #4CD964; font-size: %1px; font-weight: bold;").arg(statusFontSize - 2));
    insulinLabel->setStyleSheet(QString("color: #00B2FF; font-size: %1px; font-weight: bold;").arg(statusFontSize - 2));
    cgmIcon->setStyleSheet(QString("color: white; font-size: %1px; font-weight: bold;").arg(statusFontSize));
    dropIcon->setStyleSheet(QString("color: #00B2FF; font-size: %1px; font-weight: bold;").arg(statusFontSize + 2));
    bluetoothIcon->setStyleSheet(QString("color: #00B2FF; font-size: %1px; font-weight: bold;").arg(statusFontSize));
    
    // Glucose display
    int glucoseFontSize = baseFontSize * 3;
    int unitFontSize = baseFontSize * 1.2;
    int trendFontSize = baseFontSize * 2;
    
    glucoseLabel->setStyleSheet(QString("color: white; font-size: %1px; font-weight: bold;").arg(glucoseFontSize));
    glucoseUnitLabel->setStyleSheet(QString("color: white; font-size: %1px;").arg(unitFontSize));
    glucoseTrendLabel->setStyleSheet(QString("color: white; font-size: %1px;").arg(trendFontSize));
    timeframeLabel->setStyleSheet(QString("color: white; font-size: %1px;").arg(unitFontSize));
    
    // Control area
    int controlFontSize = baseFontSize * 1.2;
    int buttonFontSize = baseFontSize * 1.5;
    
    insulinOnBoardLabel->setStyleSheet(QString("color: white; font-size: %1px;").arg(controlFontSize));
    iobValueLabel->setStyleSheet(QString("color: white; font-size: %1px; font-weight: bold;").arg(controlFontSize));
    controlIQLabel->setStyleSheet(QString("color: white; font-size: %1px;").arg(controlFontSize));
    
    bolusButton->setStyleSheet(QString(
        "QPushButton { background-color: transparent; color: #00B2FF; font-size: %1px; font-weight: bold; border: none; }"
        "QPushButton:pressed { color: #0077CC; }"
    ).arg(buttonFontSize));
    
    optionsButton->setStyleSheet(QString(
        "QPushButton { background-color: transparent; color: white; font-size: %1px; border: none; }"
        "QPushButton:pressed { color: #BBBBBB; }"
    ).arg(buttonFontSize));
    
    powerButton->setStyleSheet(QString(
        "QPushButton { background-color: #990000; color: white; font-weight: bold; border-radius: 10px; font-size: %1px; }"
        "QPushButton:pressed { background-color: #CC0000; }"
    ).arg(buttonFontSize - 2));
    
    // Update status indicator sizes
    int dotSize = qMax(8, qMin(16, width / 60));
    bolusStatus1->setFixedSize(dotSize, dotSize);
    bolusStatus2->setFixedSize(dotSize, dotSize);
    bolusStatus3->setFixedSize(dotSize, dotSize);
    
    // Set appropriate border radius for the status dots
    int radius = dotSize / 2;
    bolusStatus1->setStyleSheet(QString("QLabel { background-color: white; border-radius: %1px; }").arg(radius));
    bolusStatus2->setStyleSheet(QString("QLabel { background-color: white; border-radius: %1px; }").arg(radius));
    bolusStatus3->setStyleSheet(QString("QLabel { background-color: #00B2FF; border-radius: %1px; }").arg(radius));
}

void HomeScreen::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    updateFontSizes();
}

void HomeScreen::updateAllData(PumpController *controller)
{
    if (!controller) return;
    
    updateBatteryLevel(controller->getBatteryLevel());
    updateInsulinRemaining(controller->getInsulinRemaining());
    updateGlucoseLevel(controller->getCurrentGlucose());
    updateGlucoseTrend(controller->getGlucoseTrend());
    updateInsulinOnBoard(controller->getInsulinOnBoard());
    updateControlIQAction(controller->getControlIQDelivery());
    
    // Get glucose history for the last 3 hours by default
    QDateTime now = QDateTime::currentDateTime();
    QDateTime threeHoursAgo = now.addSecs(-3 * 60 * 60);
    updateGlucoseGraph(controller->getGlucoseHistory(threeHoursAgo, now));
    
    updateDateTime();
}

void HomeScreen::updateBatteryLevel(int level)
{
    // Update battery percentage text
    batteryLabel->setText(QString::number(level) + "%");
    
    // Update color based on level
    QString colorStyle;
    if (level <= 20) {
        colorStyle = "#FF3B30"; // Red for low
    } else if (level <= 50) {
        colorStyle = "#FFCC00"; // Yellow for medium
    } else {
        colorStyle = "#4CD964"; // Green for good
    }
    
    int fontSize = qMax(8, qMin(14, width() / 50));
    batteryLabel->setStyleSheet(QString("color: %1; font-size: %2px; font-weight: bold;").arg(colorStyle).arg(fontSize));
    
    // Update battery bars
    int activeBars = qRound(level / 20.0); // 5 bars, each represents 20%
    
    for (int i = 0; i < batteryBars.size(); i++) {
        if (i < activeBars) {
            batteryBars[i]->setStyleSheet("background-color: " + colorStyle + "; border-radius: 2px;");
            batteryBars[i]->show();
        } else {
            batteryBars[i]->hide();
        }
    }
    
    update();
}

void HomeScreen::updateInsulinRemaining(double units)
{
    // Update insulin text
    insulinLabel->setText(QString::number(qRound(units)) + " u");
    
    // Update color based on units
    QString colorStyle;
    if (units <= 30.0) {
        colorStyle = "#FF3B30"; // Red for low
    } else if (units <= 100.0) {
        colorStyle = "#FFCC00"; // Yellow for medium
    } else {
        colorStyle = "#00B2FF"; // Blue for good
    }
    
    int fontSize = qMax(8, qMin(14, width() / 50));
    insulinLabel->setStyleSheet(QString("color: %1; font-size: %2px; font-weight: bold;").arg(colorStyle).arg(fontSize));
    
    // Update insulin bars
    int activeBars = qMin(5, qMax(0, qRound(units / 60.0))); // 5 bars, each represents 60 units
    
    for (int i = 0; i < insulinBars.size(); i++) {
        if (i < activeBars) {
            insulinBars[i]->setStyleSheet("background-color: " + colorStyle + "; border-radius: 2px;");
            insulinBars[i]->show();
        } else {
            insulinBars[i]->hide();
        }
    }
    
    update();
}

void HomeScreen::updateGlucoseLevel(double value)
{
    glucoseLabel->setText(QString::number(value, 'f', 1));
    
    // Update color based on glucose level
    QString colorStyle;
    if (value < 3.9) {
        colorStyle = "#FF3B30"; // Red for low
    } else if (value > 10.0) {
        colorStyle = "#FF9500"; // Orange for high
    } else {
        colorStyle = "white"; // White for normal
    }
    
    int fontSize = qMax(18, qMin(64, width() / 15));
    glucoseLabel->setStyleSheet(QString("color: %1; font-size: %2px; font-weight: bold;").arg(colorStyle).arg(fontSize));
    
    update();
}

void HomeScreen::updateGlucoseTrend(GlucoseModel::TrendDirection trend)
{
    QString trendText;
    
    // Set trend direction arrow based on trend
    switch (trend) {
        case GlucoseModel::RisingQuickly:
            trendText = "↑↑";
            break;
        case GlucoseModel::Rising:
            trendText = "↑";
            break;
        case GlucoseModel::Stable:
            trendText = "→";
            break;
        case GlucoseModel::Falling:
            trendText = "↓";
            break;
        case GlucoseModel::FallingQuickly:
            trendText = "↓↓";
            break;
        default:
            trendText = "";
    }
    
    glucoseTrendLabel->setText(trendText);
    int fontSize = qMax(14, qMin(48, width() / 20));
    glucoseTrendLabel->setStyleSheet(QString("color: white; font-size: %1px;").arg(fontSize));
    update();
}

void HomeScreen::updateInsulinOnBoard(double units)
{
    iobValueLabel->setText(QString::number(units, 'f', 1) + " u");
    update();
}

void HomeScreen::updateControlIQAction(double value)
{
    controlIQLabel->setText("Control-IQ: " + QString::number(value, 'f', 2) + " u");
    update();
}

void HomeScreen::updateGlucoseGraph(const QVector<QPair<QDateTime, double>> &data)
{
    // Store the new data
    graphData = data;
    
    // Update the graph view
    graphView->setGlucoseData(graphData);
    
    // Update the timeframe label
    timeframeLabel->setText(QString("%1 HRS").arg(graphView->getTimeRangeHours()));
    
    // Force a repaint
    update();
}

void HomeScreen::updateDateTime()
{
    QDateTime now = QDateTime::currentDateTime();
    timeLabel->setText(now.toString("hh:mm"));
    dateLabel->setText(now.toString("dd MMM"));
}

void HomeScreen::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    
    // Draw base widget
    QStyleOption opt;
    opt.init(this);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &painter, this);
}

void HomeScreen::onBolusButtonClicked()
{
    emit bolusButtonClicked();
}

void HomeScreen::onOptionsButtonClicked()
{
    emit optionsButtonClicked();
}

void HomeScreen::onPowerButtonClicked()
{
    emit powerButtonPressed();
}

void HomeScreen::showTimelineOptions(const QPoint &pos)
{
    // Create a context menu for timeline options
    QMenu contextMenu(tr("Timeline Options"), this);
    
    QAction action1Hr(tr("1 Hour"), this);
    QAction action3Hr(tr("3 Hours"), this);
    QAction action6Hr(tr("6 Hours"), this);
    QAction action12Hr(tr("12 Hours"), this);
    QAction action24Hr(tr("24 Hours"), this);
    
    connect(&action1Hr, &QAction::triggered, this, [this]() { setTimelineRange(1); });
    connect(&action3Hr, &QAction::triggered, this, [this]() { setTimelineRange(3); });
    connect(&action6Hr, &QAction::triggered, this, [this]() { setTimelineRange(6); });
    connect(&action12Hr, &QAction::triggered, this, [this]() { setTimelineRange(12); });
    connect(&action24Hr, &QAction::triggered, this, [this]() { setTimelineRange(24); });
    
    contextMenu.addAction(&action1Hr);
    contextMenu.addAction(&action3Hr);
    contextMenu.addAction(&action6Hr);
    contextMenu.addAction(&action12Hr);
    contextMenu.addAction(&action24Hr);
    
    // Check the current timeline
    int currentHours = graphView->getTimeRangeHours();
    switch (currentHours) {
        case 1: action1Hr.setChecked(true); break;
        case 3: action3Hr.setChecked(true); break;
        case 6: action6Hr.setChecked(true); break;
        case 12: action12Hr.setChecked(true); break;
        case 24: action24Hr.setChecked(true); break;
    }
    
    contextMenu.exec(mapToGlobal(pos));
}

void HomeScreen::setTimelineRange(int hours)
{
    // Update graph view's time range
    QDateTime endTime = QDateTime::currentDateTime();
    QDateTime startTime = endTime.addSecs(-hours * 60 * 60);
    
    graphView->setTimeRange(startTime, endTime);
    
    // Update timeline text
    timeframeLabel->setText(QString("%1 HRS").arg(hours));
}

void HomeScreen::onTimeRangeChanged(int hours)
{
    // Update timeline text
    timeframeLabel->setText(QString("%1 HRS").arg(hours));
}

bool HomeScreen::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == timeframeLabel && event->type() == QEvent::MouseButtonPress) {
        // Show timeline options when timeframeLabel is clicked
        showTimelineOptions(timeframeLabel->mapToGlobal(QPoint(0, timeframeLabel->height())));
        return true;
    }
    
    return QWidget::eventFilter(watched, event);
}
