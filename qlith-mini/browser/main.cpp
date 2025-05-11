// this_file: qlith/browser/main.cpp
#include <QApplication>
#include <QCommandLineParser>
#include <QUrl>
#include <QSurfaceFormat>
#include <QDir>
#include <QStandardPaths>
#include <QFileInfo>
#include <QFile>

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
    
    parser.process(app);
    
    // Create debug directory if in debug mode
#ifdef QLITH_DEBUG_DIR
    QDir dir(QLITH_DEBUG_DIR);
    if (!dir.exists()) {
        dir.mkpath(".");
    }
#endif
    
    // Create main window
    MainWindow mainWindow;
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
        mainWindow.load(url);
    } else {
        // Load default homepage
        QString homePage = "https://example.com";
        mainWindow.load(QUrl(homePage));
    }
    
    return app.exec();
} 