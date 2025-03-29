#ifndef FORCERESIZABLE_H
#define FORCERESIZABLE_H

#include <QMainWindow>
#include <QEvent>
#include <QTimer>
#include <QSizePolicy>

class ForceResizable : public QObject
{
    Q_OBJECT

public:
    explicit ForceResizable(QMainWindow *window, QObject *parent = nullptr);
    
protected:
    bool eventFilter(QObject *obj, QEvent *event) override;
    
private slots:
    void applyResizableSettings();
    
private:
    QMainWindow *m_window;
};

#endif // FORCERESIZABLE_H
