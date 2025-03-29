#include "mainwindow.h"
#include <QApplication>
#include <QStyle>
#include <QScreen>
#include <QDir>
#include <QFile>
#include <iostream>
#include <QFontDatabase>

// Helper function to remove fixed size constraints from UI file
bool removeFixedSizeConstraints()
{
    // Find the mainwindow.ui file
    QString uiPath = QDir::currentPath() + "/mainwindow.ui";
    if (!QFile::exists(uiPath)) {
        uiPath = QDir::currentPath() + "/../mainwindow.ui";
    }
    
    if (!QFile::exists(uiPath)) {
        std::cout << "Warning: Could not find mainwindow.ui file to remove size constraints\n";
        return false;
    }
    
    // Read the UI file
    QFile file(uiPath);
    if (!file.open(QIODevice::ReadWrite | QIODevice::Text)) {
        std::cout << "Warning: Could not open mainwindow.ui file\n";
        return false;
    }
    
    QString content = file.readAll();
    
    // Remove maximumSize and fixed size constraints
    content.replace(QRegExp("<property name=\"maximumSize\">.*</property>", Qt::CaseInsensitive), "");
    content.replace(QRegExp("<property name=\"sizePolicy\">.*<sizetype>Fixed</sizetype>.*</property>", Qt::CaseInsensitive), 
                    "<property name=\"sizePolicy\"><sizepolicy hsizetype=\"Expanding\" vsizetype=\"Expanding\"><horstretch>0</horstretch><verstretch>0</verstretch></sizepolicy></property>");
    
    // Reset file and write the modified content
    file.seek(0);
    file.resize(0);
    file.write(content.toUtf8());
    file.close();
    
    std::cout << "Removed fixed size constraints from UI file\n";
    return true;
}

// Remove size constraints from all UI files
bool removeAllFixedSizeConstraints()
{
    bool success = true;
    
    // List of UI files to process
    QStringList uiFiles = {
        "mainwindow.ui",
        "views/homescreen.ui",
        "views/bolusscreen.ui",
        "views/profilescreen.ui",
        "views/optionsscreen.ui"
    };
    
    for(const QString &uiFilename : uiFiles) {
        QString uiPath = QDir::currentPath() + "/" + uiFilename;
        if (!QFile::exists(uiPath)) {
            uiPath = QDir::currentPath() + "/../" + uiFilename;
        }
        
        if (!QFile::exists(uiPath)) {
            std::cout << "Warning: Could not find " << uiFilename.toStdString() << " file\n";
            success = false;
            continue;
        }
        
        // Read the UI file
        QFile file(uiPath);
        if (!file.open(QIODevice::ReadWrite | QIODevice::Text)) {
            std::cout << "Warning: Could not open " << uiFilename.toStdString() << " file\n";
            success = false;
            continue;
        }
        
        QString content = file.readAll();
        
        // Remove maximumSize and fixed size constraints
        content.replace(QRegExp("<property name=\"maximumSize\">.*</property>", Qt::CaseInsensitive), "");
        content.replace(QRegExp("<property name=\"sizePolicy\">.*<sizetype>Fixed</sizetype>.*</property>", Qt::CaseInsensitive), 
                        "<property name=\"sizePolicy\"><sizepolicy hsizetype=\"Expanding\" vsizetype=\"Expanding\"><horstretch>0</horstretch><verstretch>0</verstretch></sizepolicy></property>");
        
        // Reset file and write the modified content
        file.seek(0);
        file.resize(0);
        file.write(content.toUtf8());
        file.close();
        
        std::cout << "Removed fixed size constraints from " << uiFilename.toStdString() << " file\n";
    }
    
    return success;
}

int main(int argc, char *argv[])
{
    // Remove size constraints from UI files before creating QApplication
    removeAllFixedSizeConstraints();
    
    QApplication app(argc, argv);
    app.setApplicationName("t:slim X2 Simulator");
    
    // High DPI support
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
    
    // Set application style
    app.setStyle("Fusion");
    
    // Set dark palette to match the t:slim X2 UI
    QPalette darkPalette;
    darkPalette.setColor(QPalette::Window, QColor(53, 53, 53));
    darkPalette.setColor(QPalette::WindowText, Qt::white);
    darkPalette.setColor(QPalette::Base, QColor(25, 25, 25));
    darkPalette.setColor(QPalette::AlternateBase, QColor(53, 53, 53));
    darkPalette.setColor(QPalette::ToolTipBase, Qt::white);
    darkPalette.setColor(QPalette::ToolTipText, Qt::white);
    darkPalette.setColor(QPalette::Text, Qt::white);
    darkPalette.setColor(QPalette::Button, QColor(53, 53, 53));
    darkPalette.setColor(QPalette::ButtonText, Qt::white);
    darkPalette.setColor(QPalette::BrightText, Qt::red);
    darkPalette.setColor(QPalette::Link, QColor(42, 130, 218));
    darkPalette.setColor(QPalette::Highlight, QColor(42, 130, 218));
    darkPalette.setColor(QPalette::HighlightedText, Qt::black);
    app.setPalette(darkPalette);
    
    // Load and configure the application fonts
    int fontId = QFontDatabase::addApplicationFont(":/fonts/Roboto-Regular.ttf");
    if (fontId != -1) {
        QFont font("Roboto");
        font.setPointSize(10);
        app.setFont(font);
    }
    
    // Create and show the main window
    MainWindow mainWindow;
    
    // Set a reasonable initial size
    QScreen *screen = QGuiApplication::primaryScreen();
    QRect screenGeometry = screen->availableGeometry();
    
    // Set the window size based on screen size (but keep the aspect ratio)
    double aspectRatio = 3.0 / 4.0; // 3:4 aspect ratio (height is greater than width)
    int desiredWidth = qMin(screenGeometry.width() - 100, 400);
    int desiredHeight = qRound(desiredWidth / aspectRatio);
    
    // Make sure the height isn't too tall
    if (desiredHeight > screenGeometry.height() - 100) {
        desiredHeight = screenGeometry.height() - 100;
        desiredWidth = qRound(desiredHeight * aspectRatio);
    }
    
    mainWindow.resize(desiredWidth, desiredHeight);
    
    // Center on screen
    mainWindow.move(
        (screenGeometry.width() - mainWindow.width()) / 2,
        (screenGeometry.height() - mainWindow.height()) / 2
    );
    
    mainWindow.show();
    
    return app.exec();
}
