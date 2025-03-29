#ifndef GRAPHVIEW_H
#define GRAPHVIEW_H

#include <QWidget>
#include <QDateTime>
#include <QVector>
#include <QPair>

class GraphView : public QWidget
{
    Q_OBJECT

public:
    explicit GraphView(QWidget *parent = nullptr);
    
    enum DataType {
        GlucoseData,
        InsulinData,
        CombinedData
    };
    
    void setGlucoseData(const QVector<QPair<QDateTime, double>> &data);
    void setInsulinData(const QVector<QPair<QDateTime, double>> &data);
    void setTimeRange(const QDateTime &start, const QDateTime &end);
    void setTimeRangeHours(int hours);
    int getTimeRangeHours() const;
    void setDisplayType(DataType type);
    void setTargetRange(double lowLimit, double highLimit);
    void setInteractive(bool interactive);
    
    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;
    
public slots:
    void zoomIn();
    void zoomOut();
    void showContextMenu(const QPoint &pos);
    
signals:
    void timeRangeChanged(int hours);
    
protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    
private:
    QVector<QPair<QDateTime, double>> glucoseData;
    QVector<QPair<QDateTime, double>> insulinData;
    QDateTime rangeStart;
    QDateTime rangeEnd;
    DataType displayType;
    double targetLow;
    double targetHigh;
    QPoint lastMousePos;
    int timeRangeHours;
    bool isInteractive;
    bool isDragging = false;
    
    // Methods for drawing
    void drawGlucoseGraph(QPainter &painter, const QRect &rect);
    void drawInsulinGraph(QPainter &painter, const QRect &rect);
    void drawCombinedGraph(QPainter &painter, const QRect &rect);
    void drawTimeAxis(QPainter &painter, const QRect &rect);
    void drawValueAxis(QPainter &painter, const QRect &rect, double min, double max);
    void drawGridLines(QPainter &painter, const QRect &rect);
    void drawTargetRange(QPainter &painter, const QRect &rect, double min, double max);
    void drawCurrentTimeMarker(QPainter &painter, const QRect &rect);
    void drawTimelineDisplay(QPainter &painter, const QRect &rect);
    void drawNoDataMessage(QPainter &painter, const QRect &rect);
    
    // Utility methods
    int timeToX(const QDateTime &time, const QRect &rect) const;
    int valueToY(double value, const QRect &rect, double min, double max) const;
    QDateTime xToTime(int x, const QRect &rect) const;
    double yToValue(int y, const QRect &rect, double min, double max) const;
    
    // Finding min/max values in data
    double findMinValue(const QVector<QPair<QDateTime, double>> &data) const;
    double findMaxValue(const QVector<QPair<QDateTime, double>> &data) const;
};

#endif // GRAPHVIEW_H
