#include "mainwindow.h"

#include <QApplication>
#include <QCommandLineParser>
#include <QFileInfo>
#include <QUrl>
#include <QTimer>
#include <QSize>
#include <QCoreApplication>
#include <QDebug>
#include <iostream>

int main(int argc, char *argv[])
{
    // Enable high DPI support
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
    
    // Set up basic exception handling
    try {
        QApplication app(argc, argv);
        
        app.setApplicationName("QLiteHTML Browser");
        app.setApplicationVersion("1.0");
        app.setOrganizationName("QLiteHTML");
        app.setOrganizationDomain("qlitehtml.org");
        
        QCommandLineParser parser;
        parser.setApplicationDescription("A lightweight HTML browser based on litehtml and Qt");
        parser.addHelpOption();
        parser.addVersionOption();
        
        // Add options for URL, dimensions, and export formats
        QCommandLineOption urlOption(QStringList() << "u" << "url", "The URL or file to open", "url");
        QCommandLineOption widthOption(QStringList() << "w" << "width", "Width of the browser window", "width", "800");
        QCommandLineOption heightOption(QStringList() << "ht" << "height", "Height of the browser window", "height", "600");
        QCommandLineOption pngOption(QStringList() << "png", "Save rendered page as PNG", "filename");
        QCommandLineOption svgOption(QStringList() << "svg", "Save rendered page as SVG", "filename");
        QCommandLineOption quitOption(QStringList() << "q" << "quit", "Quit after loading URL (and saving captures if specified)");
        QCommandLineOption safeModeOption(QStringList() << "safe-mode", "Use safe rendering mode for troubleshooting crashes");
        
        parser.addOption(urlOption);
        parser.addOption(widthOption);
        parser.addOption(heightOption);
        parser.addOption(pngOption);
        parser.addOption(svgOption);
        parser.addOption(quitOption);
        parser.addOption(safeModeOption);
        parser.addPositionalArgument("url", "The URL or file to open (alternative to --url)", "[url]");
        
        parser.process(app);
        
        // Set safe mode environment variable if requested
        if (parser.isSet(safeModeOption)) {
            qputenv("QLITEHTML_SAFE_MODE", "1");
            qDebug() << "Safe mode enabled for troubleshooting";
        }
        
        // Get window dimensions
        int width = parser.value(widthOption).toInt();
        int height = parser.value(heightOption).toInt();
        
        // Create window with specified dimensions
        MainWindow mainWindow;
        mainWindow.resize(width, height);
        
        // Check for URL from options or positional arguments
        QString urlStr;
        if (parser.isSet(urlOption)) {
            urlStr = parser.value(urlOption);
        } else {
            const QStringList args = parser.positionalArguments();
            if (!args.isEmpty()) {
                urlStr = args.at(0);
            }
        }
        
        // Handle export options
        bool saveAsPng = parser.isSet(pngOption);
        bool saveAsSvg = parser.isSet(svgOption);
        QString pngFilename = parser.value(pngOption);
        QString svgFilename = parser.value(svgOption);
        
        // Check quit option
        bool shouldQuit = parser.isSet(quitOption);
        
        // If export options or quit option are set, don't show the window
        bool hideWindow = saveAsPng || saveAsSvg || (shouldQuit && !urlStr.isEmpty());
        
        if (!hideWindow) {
            mainWindow.show();
        }
        
        // Load URL if provided
        if (!urlStr.isEmpty()) {
            QFileInfo fileInfo(urlStr);
            
            if (fileInfo.exists() && fileInfo.isFile()) {
                mainWindow.loadFile(fileInfo.absoluteFilePath());
            } else {
                QUrl url = QUrl::fromUserInput(urlStr);
                mainWindow.loadUrl(url);
            }
            
            // If export options are set or quit is requested, wait for page to load, then perform actions and quit
            if (saveAsPng || saveAsSvg || shouldQuit) {
                QObject::connect(&mainWindow, &MainWindow::loadFinished, [&]() {
                    // Add delay to allow rendering to complete
                    QTimer::singleShot(1000, [&]() {
                        if (saveAsPng) {
                            try {
                                mainWindow.saveAsPng(pngFilename);
                            } catch (...) {
                                qDebug() << "Error saving PNG, ignoring";
                            }
                        }
                        if (saveAsSvg) {
                            try {
                                mainWindow.saveAsSvg(svgFilename);
                            } catch (...) {
                                qDebug() << "Error saving SVG, ignoring";
                            }
                        }
                        // Quit after exporting or if quit option is set
                        QTimer::singleShot(500, &app, &QApplication::quit);
                    });
                });
            }
        }
        
        return app.exec();
    } catch (const std::exception& e) {
        std::cerr << "Fatal exception: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "Unknown fatal exception occurred" << std::endl;
        return 1;
    }
} 