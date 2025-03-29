#include "graphview.h"
#include <QPainter>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QStyleOption>
#include <QtMath>
#include <algorithm>
#include <QPainterPath>
#include <QMenu>
#include <QAction>

GraphView::GraphView(QWidget *parent)
    : QWidget(parent),
      displayType(GlucoseData),
      targetLow(3.9),
      targetHigh(10.0),
      timeRangeHours(3),
      isInteractive(true)
{
    // Set default time range to last timeRangeHours hours
    rangeEnd = QDateTime::currentDateTime();
    rangeStart = rangeEnd.addSecs(-timeRangeHours * 60 * 60);
    
    // Enable mouse tracking for tooltips
    setMouseTracking(true);
    
    // Set focus policy to accept focus by click and tab
    setFocusPolicy(Qt::StrongFocus);
    
    // Add context menu for timeline options
    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, &GraphView::customContextMenuRequested, this, &GraphView::showContextMenu);
}

void GraphView::setGlucoseData(const QVector<QPair<QDateTime, double>> &data)
{
    glucoseData = data;
    update();
}

void GraphView::setInsulinData(const QVector<QPair<QDateTime, double>> &data)
{
    insulinData = data;
    update();
}

void GraphView::setTimeRange(const QDateTime &start, const QDateTime &end)
{
    rangeStart = start;
    rangeEnd = end;
    
    // Calculate time range in hours
    timeRangeHours = qRound(rangeStart.secsTo(rangeEnd) / 3600.0);
    
    emit timeRangeChanged(timeRangeHours);
    update();
}

void GraphView::setTimeRangeHours(int hours)
{
    if (hours <= 0)
        return;
    
    timeRangeHours = hours;
    rangeEnd = QDateTime::currentDateTime();
    rangeStart = rangeEnd.addSecs(-timeRangeHours * 60 * 60);
    
    emit timeRangeChanged(timeRangeHours);
    update();
}

int GraphView::getTimeRangeHours() const
{
    return timeRangeHours;
}

void GraphView::setDisplayType(DataType type)
{
    displayType = type;
    update();
}

void GraphView::setTargetRange(double lowLimit, double highLimit)
{
    targetLow = lowLimit;
    targetHigh = highLimit;
    update();
}

void GraphView::setInteractive(bool interactive)
{
    isInteractive = interactive;
}

QSize GraphView::sizeHint() const
{
    return QSize(400, 200);
}

QSize GraphView::minimumSizeHint() const
{
    return QSize(200, 100);
}

void GraphView::paintEvent(QPaintEvent *event)
{
    // Draw base widget
    QStyleOption opt;
    opt.init(this);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &painter, this);
    
    // Define graph area (leave margin for axes)
    QRect rect = this->rect().adjusted(50, 20, -20, -30);
    
    // Draw grid
    drawGridLines(painter, rect);
    
    // Draw appropriate graph based on display type
    switch (displayType) {
        case GlucoseData:
            drawGlucoseGraph(painter, rect);
            break;
        case InsulinData:
            drawInsulinGraph(painter, rect);
            break;
        case CombinedData:
            drawCombinedGraph(painter, rect);
            break;
    }
    
    // Draw axes
    drawTimeAxis(painter, rect);
    
    if (displayType == GlucoseData || displayType == CombinedData) {
        double minValue = qMin(2.0, findMinValue(glucoseData));
        double maxValue = qMax(20.0, findMaxValue(glucoseData));
        drawValueAxis(painter, rect, minValue, maxValue);
    } else {
        double maxValue = qMax(5.0, findMaxValue(insulinData) * 1.2);
        drawValueAxis(painter, rect, 0.0, maxValue);
    }
    
    // Draw current time marker
    drawCurrentTimeMarker(painter, rect);
    
    // Draw timeline display
    drawTimelineDisplay(painter, rect);
}

void GraphView::drawGlucoseGraph(QPainter &painter, const QRect &rect)
{
    if (glucoseData.isEmpty()) {
        drawNoDataMessage(painter, rect);
        return;
    }
    
    // Find min and max values (with reasonable defaults)
    double minValue = qMin(2.0, findMinValue(glucoseData));
    double maxValue = qMax(20.0, findMaxValue(glucoseData));
    
    // Draw target range
    drawTargetRange(painter, rect, minValue, maxValue);
    
    // Sort data by timestamp to ensure correct order
    auto sortedData = glucoseData;
    std::sort(sortedData.begin(), sortedData.end(), 
             [](const QPair<QDateTime, double> &a, const QPair<QDateTime, double> &b) {
                 return a.first < b.first;
             });
    
    // Draw glucose line
    QPainterPath path;
    bool firstPoint = true;
    
    for (const auto &point : sortedData) {
        // Skip points outside the time range
        if (point.first < rangeStart || point.first > rangeEnd) {
            continue;
        }
        
        int x = timeToX(point.first, rect);
        int y = valueToY(point.second, rect, minValue, maxValue);
        
        if (firstPoint) {
            path.moveTo(x, y);
            firstPoint = false;
        } else {
            path.lineTo(x, y);
        }
    }
    
    // Draw the path
    painter.setPen(QPen(QColor(0, 178, 255), 2));
    painter.drawPath(path);
    
    // Draw points
    for (const auto &point : sortedData) {
        // Skip points outside the time range
        if (point.first < rangeStart || point.first > rangeEnd) {
            continue;
        }
        
        int x = timeToX(point.first, rect);
        int y = valueToY(point.second, rect, minValue, maxValue);
        
        // Different colors based on range
        QColor pointColor;
        if (point.second < targetLow) {
            pointColor = QColor(255, 59, 48); // Red for low
        } else if (point.second > targetHigh) {
            pointColor = QColor(255, 149, 0); // Orange for high
        } else {
            pointColor = QColor(0, 178, 255); // Blue for in-range
        }
        
        painter.setPen(QPen(pointColor, 1));
        painter.setBrush(pointColor);
        painter.drawEllipse(QPoint(x, y), 3, 3);
        
        // For latest point, draw a label with the current value
        if (point.first == sortedData.last().first) {
            QString valueLabel = QString::number(point.second, 'f', 1);
            QRect textRect(x + 5, y - 10, 50, 20);
            painter.drawText(textRect, Qt::AlignLeft | Qt::AlignVCenter, valueLabel);
        }
    }
}

void GraphView::drawInsulinGraph(QPainter &painter, const QRect &rect)
{
    if (insulinData.isEmpty()) {
        drawNoDataMessage(painter, rect);
        return;
    }
    
    // Find max value (min is always 0 for insulin)
    double maxValue = qMax(5.0, findMaxValue(insulinData) * 1.2);
    
    // Sort data by timestamp
    auto sortedData = insulinData;
    std::sort(sortedData.begin(), sortedData.end(), 
             [](const QPair<QDateTime, double> &a, const QPair<QDateTime, double> &b) {
                 return a.first < b.first;
             });
    
    // Draw insulin bars
    painter.setPen(QPen(QColor(0, 122, 255), 1));
    painter.setBrush(QColor(0, 122, 255, 180));
    
    for (const auto &point : sortedData) {
        // Skip points outside the time range
        if (point.first < rangeStart || point.first > rangeEnd) {
            continue;
        }
        
        int x = timeToX(point.first, rect);
        int y = valueToY(point.second, rect, 0.0, maxValue);
        int zeroY = valueToY(0.0, rect, 0.0, maxValue);
        
        // Draw a bar from zero to value
        QRect barRect(x - 4, y, 8, zeroY - y);
        painter.drawRect(barRect);
        
        // For latest point, draw a label with the current value
        if (point.first == sortedData.last().first) {
            QString valueLabel = QString::number(point.second, 'f', 1) + " u";
            QRect textRect(x + 5, y - 10, 50, 20);
            painter.drawText(textRect, Qt::AlignLeft | Qt::AlignVCenter, valueLabel);
        }
    }
}

void GraphView::drawCombinedGraph(QPainter &painter, const QRect &rect)
{
    // Draw glucose graph first
    drawGlucoseGraph(painter, rect);
    
    // Then draw insulin as transparent bars
    if (!insulinData.isEmpty()) {
        double maxInsulin = qMax(5.0, findMaxValue(insulinData) * 1.2);
        
        // Sort data by timestamp
        auto sortedData = insulinData;
        std::sort(sortedData.begin(), sortedData.end(), 
                 [](const QPair<QDateTime, double> &a, const QPair<QDateTime, double> &b) {
                     return a.first < b.first;
                 });
        
        painter.setPen(QPen(QColor(0, 122, 255, 150), 1));
        painter.setBrush(QColor(0, 122, 255, 100));
        
        for (const auto &point : sortedData) {
            // Skip points outside the time range
            if (point.first < rangeStart || point.first > rangeEnd) {
                continue;
            }
            
            int x = timeToX(point.first, rect);
            // Scale insulin values to fit in the bottom third of the graph
            double scaledValue = point.second / maxInsulin * (rect.height() / 3.0);
            int barHeight = qMin(rect.height() / 3, static_cast<int>(scaledValue * rect.height()));
            
            QRect barRect(x - 4, rect.bottom() - barHeight, 8, barHeight);
            painter.drawRect(barRect);
        }
    }
}

void GraphView::drawTimeAxis(QPainter &painter, const QRect &rect)
{
    painter.setPen(QPen(Qt::lightGray, 1));
    
    // Draw time axis line
    painter.drawLine(rect.left(), rect.bottom(), rect.right(), rect.bottom());
    
    // Calculate time interval based on total timerange
    qint64 totalSeconds = rangeStart.secsTo(rangeEnd);
    qint64 intervalSeconds;
    
    if (totalSeconds <= 6 * 60 * 60) { // 6 hours or less
        intervalSeconds = 60 * 60; // 1 hour
    } else if (totalSeconds <= 24 * 60 * 60) { // 24 hours or less
        intervalSeconds = 4 * 60 * 60; // 4 hours
    } else if (totalSeconds <= 72 * 60 * 60) { // 3 days or less
        intervalSeconds = 8 * 60 * 60; // 8 hours
    } else {
        intervalSeconds = 24 * 60 * 60; // 1 day
    }
    
    // Round start time to the nearest interval
    QDateTime tickTime = rangeStart;
    tickTime = QDateTime(tickTime.date(), QTime(tickTime.time().hour(), 0, 0));
    
    // Move to the next full interval
    while (tickTime < rangeStart) {
        tickTime = tickTime.addSecs(intervalSeconds);
    }
    
    // Draw time ticks and labels
    while (tickTime <= rangeEnd) {
        int x = timeToX(tickTime, rect);
        
        // Draw tick mark
        painter.drawLine(x, rect.bottom(), x, rect.bottom() + 5);
        
        // Draw time label
        QString timeLabel;
        if (intervalSeconds <= 4 * 60 * 60) {
            timeLabel = tickTime.toString("hh:mm");
        } else if (intervalSeconds <= 24 * 60 * 60) {
            timeLabel = tickTime.toString("hh:mm");
        } else {
            timeLabel = tickTime.toString("MMM d");
        }
        
        QRect textRect(x - 40, rect.bottom() + 5, 80, 25);
        painter.drawText(textRect, Qt::AlignCenter, timeLabel);
        
        // Move to next tick
        tickTime = tickTime.addSecs(intervalSeconds);
    }
}

void GraphView::drawValueAxis(QPainter &painter, const QRect &rect, double min, double max)
{
    painter.setPen(QPen(Qt::lightGray, 1));
    
    // Draw value axis line
    painter.drawLine(rect.left(), rect.top(), rect.left(), rect.bottom());
    
    // Calculate value interval
    double range = max - min;
    double interval;
    
    if (range <= 5) {
        interval = 1.0;
    } else if (range <= 20) {
        interval = 2.0;
    } else {
        interval = 5.0;
    }
    
    // Round min to nearest interval
    double tickValue = qFloor(min / interval) * interval;
    
    // Draw value ticks and labels
    while (tickValue <= max) {
        int y = valueToY(tickValue, rect, min, max);
        
        // Draw tick mark
        painter.drawLine(rect.left(), y, rect.left() - 5, y);
        
        // Draw value label
        QString valueLabel = QString::number(tickValue, 'f', 1);
        
        QRect textRect(rect.left() - 45, y - 10, 40, 20);
        painter.drawText(textRect, Qt::AlignRight | Qt::AlignVCenter, valueLabel);
        
        // Move to next tick
        tickValue += interval;
    }
}

void GraphView::drawGridLines(QPainter &painter, const QRect &rect)
{
    painter.setPen(QPen(QColor(60, 60, 60), 1, Qt::DotLine));
    
    // Calculate time interval based on total timerange
    qint64 totalSeconds = rangeStart.secsTo(rangeEnd);
    qint64 intervalSeconds;
    
    if (totalSeconds <= 6 * 60 * 60) { // 6 hours or less
        intervalSeconds = 60 * 60; // 1 hour
    } else if (totalSeconds <= 24 * 60 * 60) { // 24 hours or less
        intervalSeconds = 4 * 60 * 60; // 4 hours
    } else if (totalSeconds <= 72 * 60 * 60) { // 3 days or less
        intervalSeconds = 8 * 60 * 60; // 8 hours
    } else {
        intervalSeconds = 24 * 60 * 60; // 1 day
    }
    
    // Round start time to the nearest interval
    QDateTime tickTime = rangeStart;
    tickTime = QDateTime(tickTime.date(), QTime(tickTime.time().hour(), 0, 0));
    
    // Move to the next full interval
    while (tickTime < rangeStart) {
        tickTime = tickTime.addSecs(intervalSeconds);
    }
    
    // Vertical grid lines (time)
    while (tickTime <= rangeEnd) {
        int x = timeToX(tickTime, rect);
        painter.drawLine(x, rect.top(), x, rect.bottom());
        tickTime = tickTime.addSecs(intervalSeconds);
    }
    
    // Horizontal grid lines (values)
    double min, max;
    if (displayType == InsulinData) {
        min = 0.0;
        max = qMax(5.0, findMaxValue(insulinData) * 1.2);
    } else {
        min = qMin(2.0, findMinValue(glucoseData));
        max = qMax(20.0, findMaxValue(glucoseData));
    }
    
    double range = max - min;
    double interval;
    
    if (range <= 5) {
        interval = 1.0;
    } else if (range <= 20) {
        interval = 2.0;
    } else {
        interval = 5.0;
    }
    
    double tickValue = qFloor(min / interval) * interval;
    while (tickValue <= max) {
        int y = valueToY(tickValue, rect, min, max);
        painter.drawLine(rect.left(), y, rect.right(), y);
        tickValue += interval;
    }
}

void GraphView::drawTargetRange(QPainter &painter, const QRect &rect, double min, double max)
{
    if (displayType == InsulinData) {
        return; // Target range only applies to glucose
    }
    
    // Draw target range background
    int highY = valueToY(targetHigh, rect, min, max);
    int lowY = valueToY(targetLow, rect, min, max);
    
    QRect targetRect(rect.left(), highY, rect.width(), lowY - highY);
    painter.fillRect(targetRect, QColor(0, 178, 255, 30));
    
    // Draw target range lines
    painter.setPen(QPen(QColor(0, 178, 255, 100), 1, Qt::DashLine));
    painter.drawLine(rect.left(), highY, rect.right(), highY);
    painter.drawLine(rect.left(), lowY, rect.right(), lowY);
}

void GraphView::drawCurrentTimeMarker(QPainter &painter, const QRect &rect)
{
    QDateTime now = QDateTime::currentDateTime();
    
    // Only draw if current time is within the displayed range
    if (now < rangeStart || now > rangeEnd) {
        return;
    }
    
    int x = timeToX(now, rect);
    
    // Draw vertical line for current time
    painter.setPen(QPen(QColor(255, 255, 255, 150), 1, Qt::DashLine));
    painter.drawLine(x, rect.top(), x, rect.bottom());
    
    // Draw "Now" label
    painter.setPen(QColor(255, 255, 255));
    QRect textRect(x - 20, rect.top() - 15, 40, 15);
    painter.drawText(textRect, Qt::AlignCenter, "Now");
}

void GraphView::drawTimelineDisplay(QPainter &painter, const QRect &rect)
{
    // Draw timeline information at the bottom center of the graph
    QString timeRangeText = QString("%1 HRS").arg(timeRangeHours);
    
    // Use a more visible font color and increase text size
    painter.setPen(QColor(255, 255, 255)); // White for better visibility
    QFont font = painter.font();
    font.setPointSize(10); // Increase font size
    font.setBold(true);    // Make text bold
    painter.setFont(font);
    
    // Draw text in the center bottom of the graph with more space
    QRect textRect(rect.left(), rect.bottom() + 8, rect.width(), 25);
    painter.fillRect(textRect, QColor(34, 34, 34, 180)); // Semi-transparent background
    painter.drawText(textRect, Qt::AlignCenter, timeRangeText);
}

void GraphView::drawNoDataMessage(QPainter &painter, const QRect &rect)
{
    painter.setPen(QColor(150, 150, 150));
    QFont font = painter.font();
    font.setPointSize(12);
    painter.setFont(font);
    
    QString message = "No data available for this time range";
    painter.drawText(rect, Qt::AlignCenter, message);
}

int GraphView::timeToX(const QDateTime &time, const QRect &rect) const
{
    if (rangeStart == rangeEnd) {
        return rect.left();
    }
    
    double timeRatio = static_cast<double>(rangeStart.secsTo(time)) / rangeStart.secsTo(rangeEnd);
    return rect.left() + qRound(timeRatio * rect.width());
}

int GraphView::valueToY(double value, const QRect &rect, double min, double max) const
{
    if (min == max) {
        return rect.bottom() - rect.height() / 2;
    }
    
    double valueRatio = (value - min) / (max - min);
    return rect.bottom() - qRound(valueRatio * rect.height());
}

QDateTime GraphView::xToTime(int x, const QRect &rect) const
{
    if (rect.width() == 0) {
        return rangeStart;
    }
    
    double xRatio = static_cast<double>(x - rect.left()) / rect.width();
    qint64 secondsOffset = qRound(xRatio * rangeStart.secsTo(rangeEnd));
    
    return rangeStart.addSecs(secondsOffset);
}

double GraphView::yToValue(int y, const QRect &rect, double min, double max) const
{
    if (rect.height() == 0) {
        return min;
    }
    
    double yRatio = static_cast<double>(rect.bottom() - y) / rect.height();
    return min + yRatio * (max - min);
}

double GraphView::findMinValue(const QVector<QPair<QDateTime, double>> &data) const
{
    if (data.isEmpty()) {
        return 0.0;
    }
    
    double minValue = std::numeric_limits<double>::max();
    
    for (const auto &point : data) {
        if (point.first >= rangeStart && point.first <= rangeEnd) {
            minValue = qMin(minValue, point.second);
        }
    }
    
    return minValue == std::numeric_limits<double>::max() ? 0.0 : minValue;
}

double GraphView::findMaxValue(const QVector<QPair<QDateTime, double>> &data) const
{
    if (data.isEmpty()) {
        return 10.0;
    }
    
    double maxValue = std::numeric_limits<double>::lowest();
    
    for (const auto &point : data) {
        if (point.first >= rangeStart && point.first <= rangeEnd) {
            maxValue = qMax(maxValue, point.second);
        }
    }
    
    return maxValue == std::numeric_limits<double>::lowest() ? 10.0 : maxValue;
}

void GraphView::mousePressEvent(QMouseEvent *event)
{
    if (!isInteractive)
        return;
        
    lastMousePos = event->pos();
    
    if (event->button() == Qt::LeftButton) {
        isDragging = true;
        setCursor(Qt::ClosedHandCursor);
    }
    
    QWidget::mousePressEvent(event);
}

void GraphView::mouseMoveEvent(QMouseEvent *event)
{
    if (!isInteractive)
        return;
        
    if (isDragging) {
        QRect rect = this->rect().adjusted(50, 20, -20, -30);
        
        // Calculate time difference
        int dx = lastMousePos.x() - event->pos().x();
        if (dx == 0) {
            return;
        }
        
        // Calculate how much time this represents
        double timeRatio = static_cast<double>(dx) / rect.width();
        qint64 secondsToMove = qRound(timeRatio * rangeStart.secsTo(rangeEnd));
        
        // Move the time range
        rangeStart = rangeStart.addSecs(secondsToMove);
        rangeEnd = rangeEnd.addSecs(secondsToMove);
        
        lastMousePos = event->pos();
        update();
    }
    
    QWidget::mouseMoveEvent(event);
}

void GraphView::mouseReleaseEvent(QMouseEvent *event)
{
    if (!isInteractive)
        return;
        
    if (event->button() == Qt::LeftButton) {
        isDragging = false;
        setCursor(Qt::ArrowCursor);
    }
    
    QWidget::mouseReleaseEvent(event);
}

void GraphView::wheelEvent(QWheelEvent *event)
{
    if (!isInteractive)
        return;
        
    // Zoom in/out based on wheel direction
    if (event->angleDelta().y() > 0) {
        // Zoom in (reduce time range)
        zoomIn();
    } else {
        // Zoom out (increase time range)
        zoomOut();
    }
    
    event->accept();
}

void GraphView::zoomIn()
{
    if (timeRangeHours <= 1) {
        return; // Don't zoom in further than 1 hour
    }
    
    int newTimeRange = qMax(1, timeRangeHours / 2);
    setTimeRangeHours(newTimeRange);
}

void GraphView::zoomOut()
{
    if (timeRangeHours >= 48) {
        return; // Don't zoom out further than 48 hours
    }
    
    int newTimeRange = qMin(48, timeRangeHours * 2);
    setTimeRangeHours(newTimeRange);
}

void GraphView::showContextMenu(const QPoint &pos)
{
    if (!isInteractive)
        return;
        
    QMenu contextMenu(tr("Timeline Options"), this);
    
    QAction action1Hr(tr("1 Hour"), this);
    QAction action3Hr(tr("3 Hours"), this);
    QAction action6Hr(tr("6 Hours"), this);
    QAction action12Hr(tr("12 Hours"), this);
    QAction action24Hr(tr("24 Hours"), this);
    QAction action48Hr(tr("48 Hours"), this);
    
    connect(&action1Hr, &QAction::triggered, this, [this]() { setTimeRangeHours(1); });
    connect(&action3Hr, &QAction::triggered, this, [this]() { setTimeRangeHours(3); });
    connect(&action6Hr, &QAction::triggered, this, [this]() { setTimeRangeHours(6); });
    connect(&action12Hr, &QAction::triggered, this, [this]() { setTimeRangeHours(12); });
    connect(&action24Hr, &QAction::triggered, this, [this]() { setTimeRangeHours(24); });
    connect(&action48Hr, &QAction::triggered, this, [this]() { setTimeRangeHours(48); });
    
    contextMenu.addAction(&action1Hr);
    contextMenu.addAction(&action3Hr);
    contextMenu.addAction(&action6Hr);
    contextMenu.addAction(&action12Hr);
    contextMenu.addAction(&action24Hr);
    contextMenu.addAction(&action48Hr);
    
    contextMenu.exec(mapToGlobal(pos));
}
