#include "forceresizable.h"

ForceResizable::ForceResizable(QMainWindow *window, QObject *parent)
    : QObject(parent), m_window(window)
{
    if (m_window) {
        // Install event filter to intercept events
        m_window->installEventFilter(this);
        
        // Apply initial settings - just once
        applyResizableSettings();
        
        // Use a timer to periodically check resize settings
        QTimer *checkTimer = new QTimer(this);
        connect(checkTimer, &QTimer::timeout, this, &ForceResizable::applyResizableSettings);
        checkTimer->start(1000); // Check every second (reduced from 500ms)
    }
}

bool ForceResizable::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == m_window) {
        if (event->type() == QEvent::WindowStateChange ||
            event->type() == QEvent::Show ||
            event->type() == QEvent::ShowToParent) {
            // Reapply settings when window state changes
            QTimer::singleShot(0, this, &ForceResizable::applyResizableSettings);
        }
    }
    return QObject::eventFilter(obj, event);
}

void ForceResizable::applyResizableSettings()
{
    if (!m_window)
        return;
    
    // Get current window state to preserve it
    Qt::WindowStates currentState = m_window->windowState();
    
    // Force window to be resizable
    Qt::WindowFlags flags = m_window->windowFlags();
    flags |= Qt::WindowMaximizeButtonHint;
    flags &= ~Qt::MSWindowsFixedSizeDialogHint; // Remove fixed size hint if present
    
    // Only update if flags changed to avoid infinite recursion
    if (flags != m_window->windowFlags()) {
        m_window->setWindowFlags(flags);
        // Restore window state
        m_window->setWindowState(currentState);
        m_window->show(); // Need to show again after changing flags
    }
    
    // Set maximum size to effectively unlimited
    m_window->setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
    
    // Ensure size policy is set to expanding
    m_window->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    
    // Also ensure central widget is properly sized
    if (m_window->centralWidget()) {
        m_window->centralWidget()->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    }
}
