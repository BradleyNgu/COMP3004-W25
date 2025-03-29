#include "homescreen.h"
#include "ui_homescreen.h"
#include <QPainter>
#include <QDateTime>
#include <QStyleOption>
#include <QPainterPath>
#include <QTimer>

HomeScreen::HomeScreen(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::HomeScreen)
{
    ui->setupUi(this);
    
    // Create battery indicator
    batteryBar = new QProgressBar(ui->batteryFrame);
    batteryBar->setGeometry(0, 0, 100, 30);
    batteryBar->setRange(0, 100);
    batteryBar->setValue(100);
    batteryBar->setTextVisible(true);
    batteryBar->setFormat("%p%");
    batteryBar->setStyleSheet(
        "QProgressBar { background-color: #333333; border: 1px solid #666666; border-radius: 5px; text-align: center; }"
        "QProgressBar::chunk { background-color: #4CD964; border-radius: 5px; }"
    );
    
    // Create insulin indicator
    insulinBar = new QProgressBar(ui->insulinFrame);
    insulinBar->setGeometry(0, 0, 100, 30);
    insulinBar->setRange(0, 300);
    insulinBar->setValue(260);
    insulinBar->setTextVisible(true);
    insulinBar->setFormat("%v u");
    insulinBar->setStyleSheet(
        "QProgressBar { background-color: #333333; border: 1px solid #666666; border-radius: 5px; text-align: center; }"
        "QProgressBar::chunk { background-color: #007AFF; border-radius: 5px; }"
    );
    
    // Set initial values
    ui->timeLabel->setText("07:35");
    ui->dateLabel->setText("14 Nov");
    ui->glucoseLabel->setText("9.0");
    ui->glucoseTrendLabel->setText("↑");
    ui->iobValueLabel->setText("3.4 u");
    ui->controlIQLabel->setText("Control-IQ: 0.80 u");
    ui->timeframeLabel->setText("3 HRS");
    ui->bluetoothLabel->setText("B");
    
    // Create signal strength indicator
    ui->signalLabel->setText("▮▮▮");
    ui->signalLabel->setStyleSheet("color: white;");
    
    // Set up timer to update date and time
    dateTimeTimer = new QTimer(this);
    connect(dateTimeTimer, &QTimer::timeout, this, &HomeScreen::updateDateTime);
    dateTimeTimer->start(60000); // Update every minute
    
    // Connect button signals
    connect(ui->bolusButton, &QPushButton::clicked, this, &HomeScreen::on_bolusButton_clicked);
    connect(ui->optionsButton, &QPushButton::clicked, this, &HomeScreen::on_optionsButton_clicked);
    connect(ui->powerButton, &QPushButton::clicked, this, &HomeScreen::on_powerButton_clicked);
    
    // Initial update
    updateDateTime();
}

HomeScreen::~HomeScreen()
{
    delete ui;
}

void HomeScreen::updateAllData(PumpController *controller)
{
    updateBatteryLevel(controller->getBatteryLevel());
    updateInsulinRemaining(controller->getInsulinRemaining());
    updateGlucoseLevel(controller->getCurrentGlucose());
    updateGlucoseTrend(controller->getGlucoseTrend());
    updateInsulinOnBoard(controller->getInsulinOnBoard());
    updateControlIQAction(controller->getControlIQDelivery());
    updateGlucoseGraph(controller->getGlucoseHistory(QDateTime::currentDateTime().addSecs(-3 * 60 * 60), QDateTime::currentDateTime()));
    updateDateTime();
}

void HomeScreen::updateBatteryLevel(int level)
{
    batteryBar->setValue(level);
    
    // Update color based on level
    QString colorStyle;
    if (level <= 20) {
        colorStyle = "QProgressBar::chunk { background-color: #FF3B30; border-radius: 5px; }";
    } else if (level <= 50) {
        colorStyle = "QProgressBar::chunk { background-color: #FFCC00; border-radius: 5px; }";
    } else {
        colorStyle = "QProgressBar::chunk { background-color: #4CD964; border-radius: 5px; }";
    }
    
    batteryBar->setStyleSheet(
        "QProgressBar { background-color: #333333; border: 1px solid #666666; border-radius: 5px; text-align: center; }" +
        colorStyle
    );
    
    update();
}

void HomeScreen::updateInsulinRemaining(double /* units */)
{
    insulinBar->setValue(units);
    
    // Update color based on level
    QString colorStyle;
    if (units <= 30.0) {
        colorStyle = "QProgressBar::chunk { background-color: #FF3B30; border-radius: 5px; }";
    } else if (units <= 100.0) {
        colorStyle = "QProgressBar::chunk { background-color: #FFCC00; border-radius: 5px; }";
    } else {
        colorStyle = "QProgressBar::chunk { background-color: #007AFF; border-radius: 5px; }";
    }
    
    insulinBar->setStyleSheet(
        "QProgressBar { background-color: #333333; border: 1px solid #666666; border-radius: 5px; text-align: center; }" +
        colorStyle
    );
    
    update();
}

void HomeScreen::updateGlucoseLevel(double /* value */)
{
    ui->glucoseLabel->setText(QString::number(value, 'f', 1));
    
    // Update color based on glucose level
    QString colorStyle;
    if (value < 3.9) {
        colorStyle = "color: #FF3B30;"; // Red for low
    } else if (value > 10.0) {
        colorStyle = "color: #FF9500;"; // Orange for high
    } else {
        colorStyle = "color: white;"; // White for normal
    }
    
    ui->glucoseLabel->setStyleSheet(colorStyle + " font-size: 36px; font-weight: bold;");
    
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
    
    ui->glucoseTrendLabel->setText(trendText);
    update();
}

void HomeScreen::updateInsulinOnBoard(double /* units */)
{
    ui->iobValueLabel->setText(QString::number(units, 'f', 1) + " u");
    update();
}

void HomeScreen::updateControlIQAction(double /* value */)
{
    ui->controlIQLabel->setText("Control-IQ: " + QString::number(value, 'f', 2) + " u");
    update();
}

void HomeScreen::updateGlucoseGraph(const QVector<QPair<QDateTime, double>> &data)
{
    graphData = data;
    update();
}

void HomeScreen::updateDateTime()
{
    QDateTime now = QDateTime::currentDateTime();
    ui->timeLabel->setText(now.toString("hh:mm"));
    ui->dateLabel->setText(now.toString("dd MMM"));
}

void HomeScreen::paintEvent(QPaintEvent *event)
{
    // Draw base widget
    QStyleOption opt;
    opt.init(this);
    QPainter painter(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &painter, this);
    
    // Draw glucose graph
    drawGlucoseGraph(painter);
}

void HomeScreen::drawGlucoseGraph(QPainter &painter)
{
    // Define graph area
    QRect graphRect = ui->graphFrame->geometry();
    
    // Draw graph border
    painter.setPen(QPen(QColor(80, 80, 80), 1));
    painter.drawRect(graphRect);
    
    // Draw target range (3.9 - 10.0 mmol/L)
    double minValue = 2.0;
    double maxValue = 22.0;
    double normalLow = 3.9;
    double normalHigh = 10.0;
    
    int lowY = graphRect.bottom() - (normalLow - minValue) / (maxValue - minValue) * graphRect.height();
    int highY = graphRect.bottom() - (normalHigh - minValue) / (maxValue - minValue) * graphRect.height();
    
    // Draw hypo zone (below 3.9 mmol/L) - red
    QRect hypoRect(graphRect.left(), lowY, graphRect.width(), graphRect.bottom() - lowY);
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(255, 0, 0, 50));
    painter.drawRect(hypoRect);
    
    // Draw target zone (3.9 - 10.0 mmol/L) - gray
    QRect targetRect(graphRect.left(), highY, graphRect.width(), lowY - highY);
    painter.setBrush(QColor(100, 100, 100, 50));
    painter.drawRect(targetRect);
    
    // Draw hyper zone (above 10.0 mmol/L) - yellow/orange
    QRect hyperRect(graphRect.left(), graphRect.top(), graphRect.width(), highY - graphRect.top());
    painter.setBrush(QColor(255, 149, 0, 50));
    painter.drawRect(hyperRect);
    
    // Draw time intervals (hourly marks)
    painter.setPen(QPen(QColor(80, 80, 80), 1, Qt::DotLine));
    for (int i = 1; i < 3; i++) {
        int x = graphRect.left() + i * graphRect.width() / 3;
        painter.drawLine(x, graphRect.top(), x, graphRect.bottom());
    }
    
    // Draw glucose level lines
    painter.setPen(QPen(QColor(80, 80, 80), 1, Qt::DotLine));
    QVector<double> levels = {2, 6, 10, 14, 18, 22};
    for (double level : levels) {
        int y = graphRect.bottom() - (level - minValue) / (maxValue - minValue) * graphRect.height();
        painter.drawLine(graphRect.left(), y, graphRect.right(), y);
    }
    
    // Draw glucose graph line
    if (!graphData.isEmpty()) {
        QPainterPath path;
        bool firstPoint = true;
        
        // Find time range - 3 hours as specified in the image
        QDateTime endTime = QDateTime::currentDateTime();
        QDateTime startTime = endTime.addSecs(-3 * 60 * 60); // 3 hours ago
        
        for (const auto &point : graphData) {
            // Skip points outside the time range
            if (point.first < startTime || point.first > endTime) {
                continue;
            }
            
            // Calculate position
            double timeRatio = startTime.secsTo(point.first) / (double)startTime.secsTo(endTime);
            int x = graphRect.left() + timeRatio * graphRect.width();
            
            double valueRatio = (point.second - minValue) / (maxValue - minValue);
            valueRatio = qBound(0.0, valueRatio, 1.0); // Constrain to graph area
            int y = graphRect.bottom() - valueRatio * graphRect.height();
            
            if (firstPoint) {
                path.moveTo(x, y);
                firstPoint = false;
            } else {
                path.lineTo(x, y);
            }
        }
        
        painter.setPen(QPen(QColor(255, 255, 255), 2));
        painter.drawPath(path);
        
        // Draw current value indicator
        if (!graphData.isEmpty()) {
            QPair<QDateTime, double> lastPoint = graphData.last();
            double timeRatio = startTime.secsTo(lastPoint.first) / (double)startTime.secsTo(endTime);
            int x = graphRect.left() + timeRatio * graphRect.width();
            
            double valueRatio = (lastPoint.second - minValue) / (maxValue - minValue);
            valueRatio = qBound(0.0, valueRatio, 1.0);
            int y = graphRect.bottom() - valueRatio * graphRect.height();
            
            painter.setPen(Qt::NoPen);
            painter.setBrush(QColor(255, 255, 255));
            painter.drawEllipse(QPoint(x, y), 4, 4);
        }
    }
}

void HomeScreen::on_bolusButton_clicked()
{
    emit bolusButtonClicked();
}

void HomeScreen::on_optionsButton_clicked()
{
    emit optionsButtonClicked();
}

void HomeScreen::on_powerButton_clicked()
{
    emit powerButtonPressed();
}
