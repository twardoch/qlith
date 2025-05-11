// this_file: qlith/browser/main.cpp
#include <QApplication>
#include <QCommandLineParser>
#include <QUrl>
#include <QSurfaceFormat>
#include <QDir>
#include <QStandardPaths>
#include <QFileInfo>
#include <QFile>
#include <QTimer>
#include <QDebug>

#include "mainwindow.h"

int main(int argc, char *argv[])
{
    // Set application information
    QCoreApplication::setApplicationName("Qlith Browser");
    QCoreApplication::setApplicationVersion("1.0");
    QCoreApplication::setOrganizationName("Qlith");
    QCoreApplication::setOrganizationDomain("qlith.org");
    
    // Create application
    QApplication app(argc, argv);

    // Set up high DPI scaling
    QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    
    // Set up OpenGL settings for better rendering performance
    QSurfaceFormat format;
    format.setDepthBufferSize(24);
    format.setStencilBufferSize(8);
    format.setSamples(4);
    QSurfaceFormat::setDefaultFormat(format);
    
    // Parse command line arguments
    QCommandLineParser parser;
    parser.setApplicationDescription("Qlith - A lightweight HTML browser");
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument("url", "The URL to open, optional.", "[url]");
    
    // Add export options
    QCommandLineOption svgOption(QStringList() << "svg", "Render to SVG file and quit.", "path");
    parser.addOption(svgOption);
    
    QCommandLineOption pngOption(QStringList() << "png", "Render to PNG file and quit.", "path");
    parser.addOption(pngOption);
    
    // Add width and height options
    QCommandLineOption widthOption(QStringList() << "width", "Set rendering width in pixels.", "pixels", "2048");
    parser.addOption(widthOption);
    
    QCommandLineOption heightOption(QStringList() << "height", "Set rendering height in pixels.", "pixels", "2048");
    parser.addOption(heightOption);
    
    parser.process(app);
    
    // Get export paths
    QString svgPath = parser.value(svgOption);
    QString pngPath = parser.value(pngOption);
    
    // Get width and height
    int renderWidth = parser.value(widthOption).toInt();
    int renderHeight = parser.value(heightOption).toInt();
    
    qDebug() << "Render size:" << renderWidth << "x" << renderHeight;
    
    // Create debug directory if in debug mode
#ifdef QLITH_DEBUG_DIR
    QDir dir(QLITH_DEBUG_DIR);
    if (!dir.exists()) {
        dir.mkpath(".");
    }
#endif
    
    // Create main window
    MainWindow mainWindow;
    
    // Set the render size for export
    mainWindow.setRenderSize(QSize(renderWidth, renderHeight));
    
    // Handle export mode
    bool exportMode = !svgPath.isEmpty() || !pngPath.isEmpty();
    
    qDebug() << "Export mode:" << (exportMode ? "true" : "false");
    if (exportMode) {
        qDebug() << "PNG export path:" << pngPath;
        qDebug() << "SVG export path:" << svgPath;
    }
    
    // Always show the window, even in export mode for proper rendering
    mainWindow.show();
    
    // Load URL if specified
    const QStringList args = parser.positionalArguments();
    if (!args.isEmpty()) {
        QString userInput = args.first();
        QUrl url;

        // Check if the user input is a relative path to an existing file
        QFileInfo fileInfo(userInput);
        if (fileInfo.exists() && fileInfo.isRelative())
        {
            // If it's a relative path and the file exists, convert to absolute path
            url = QUrl::fromLocalFile(fileInfo.absoluteFilePath());
        }
        else
        {
            // Otherwise, let QUrl try to figure it out (handles absolute paths, URLs)
            // Or if it's a relative path but the file *doesn't* exist,
            // fromUserInput might interpret it as a search term or a non-file URL, which is fine.
            url = QUrl::fromUserInput(userInput);
        }
        
        qDebug() << "Loading URL:" << url.toString();
        mainWindow.load(url);
        
        // If in export mode, perform export after page load
        if (exportMode) {
            // Explicitly capture needed variables to avoid dangling references
            QObject::connect(&mainWindow, &MainWindow::loadFinished, 
                [&app, &mainWindow, svgPath, pngPath]() {
                    qDebug() << "Load finished, exporting...";
                    if (!svgPath.isEmpty()) {
                        mainWindow.exportToSvg(svgPath);
                    }
                    if (!pngPath.isEmpty()) {
                        qDebug() << "Exporting to PNG:" << pngPath;
                        bool success = mainWindow.exportToPng(pngPath);
                        qDebug() << "PNG export result:" << (success ? "Success" : "Failed");
                    }
                    // Quit after export with a short delay to ensure proper cleanup
                    QTimer::singleShot(100, &app, &QApplication::quit);
                });
            
            // Set a backup timer in case loadFinished signal is never emitted
            QTimer::singleShot(5000, [&app, &mainWindow, svgPath, pngPath]() {
                qDebug() << "Backup timer triggered - forcing export";
                if (!svgPath.isEmpty()) {
                    mainWindow.exportToSvg(svgPath);
                }
                if (!pngPath.isEmpty()) {
                    qDebug() << "Exporting to PNG (backup):" << pngPath;
                    bool success = mainWindow.exportToPng(pngPath);
                    qDebug() << "Backup PNG export result:" << (success ? "Success" : "Failed");
                }
                // Quit after export
                QTimer::singleShot(100, &app, &QApplication::quit);
            });
        }
    } else {
        // Load default homepage
        QString homePage = "about:blank"; // Use a simpler default for faster load
        qDebug() << "Loading default URL:" << homePage;
        mainWindow.load(QUrl(homePage));
        
        // If in export mode, perform export after page load
        if (exportMode) {
            // Explicitly capture needed variables to avoid dangling references
            QObject::connect(&mainWindow, &MainWindow::loadFinished, 
                [&app, &mainWindow, svgPath, pngPath]() {
                    qDebug() << "Load finished, exporting...";
                    if (!svgPath.isEmpty()) {
                        mainWindow.exportToSvg(svgPath);
                    }
                    if (!pngPath.isEmpty()) {
                        qDebug() << "Exporting to PNG:" << pngPath;
                        bool success = mainWindow.exportToPng(pngPath);
                        qDebug() << "PNG export result:" << (success ? "Success" : "Failed");
                    }
                    // Quit after export with a short delay to ensure proper cleanup
                    QTimer::singleShot(100, &app, &QApplication::quit);
                });
            
            // Set a backup timer in case loadFinished signal is never emitted
            QTimer::singleShot(5000, [&app, &mainWindow, svgPath, pngPath]() {
                qDebug() << "Backup timer triggered - forcing export";
                if (!svgPath.isEmpty()) {
                    mainWindow.exportToSvg(svgPath);
                }
                if (!pngPath.isEmpty()) {
                    qDebug() << "Exporting to PNG (backup):" << pngPath;
                    bool success = mainWindow.exportToPng(pngPath);
                    qDebug() << "Backup PNG export result:" << (success ? "Success" : "Failed");
                }
                // Quit after export
                QTimer::singleShot(100, &app, &QApplication::quit);
            });
        }
    }
    
    return app.exec();
} 