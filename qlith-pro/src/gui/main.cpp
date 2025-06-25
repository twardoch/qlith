// -*- coding: utf-8 -*-

#include <QApplication>
#include <QFile>
#include <QDir>
#include <QStandardPaths>
#include <QProcessEnvironment>
#include <QCommandLineParser>
#include <QUrl>
#include <QFileInfo>
#include <QTimer>
#include <QThread>
#include "qlith/container_qt5.h"
#include "qlith/context.h"
#include "qlith/mainwindow.h"
#include <QDebug>
#include <QWidget>

#include "qlith/fontcache.h"

// Function to safely load a font, returning true if successful
bool tryLoadFont(const QString &path, const QString &name, int size)
{
  QFont *font = FontCache::getInstance()->addFont(path, name, size);
  return (font != nullptr);
}

int main(int argc, char **argv)
{
  QApplication app(argc, argv);
  
  // Set application information
  QCoreApplication::setApplicationName("Qlith Pro");
  QCoreApplication::setApplicationVersion("1.0");
  QCoreApplication::setOrganizationName("Qlith");

  // Parse command line arguments
  QCommandLineParser parser;
  parser.setApplicationDescription("Qlith Pro - Advanced HTML renderer");
  parser.addHelpOption();
  parser.addVersionOption();
  parser.addPositionalArgument("url", "The HTML file to open, optional.", "[url]");
  
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
  
  // Get input file / URL
  QString userInput;
  const QStringList args = parser.positionalArguments();
  if (!args.isEmpty()) {
    userInput = args.first();
  }

  QUrl urlToLoad;
  if (!userInput.isEmpty()) {
      QFileInfo fileInfo(userInput);
      if (fileInfo.exists() && fileInfo.isRelative()) {
          // If it's a relative path and the file exists, convert to absolute path
          urlToLoad = QUrl::fromLocalFile(fileInfo.absoluteFilePath());
      } else {
          // Otherwise, let QUrl try to figure it out (handles absolute paths, http, https, etc.)
          urlToLoad = QUrl::fromUserInput(userInput);
      }
  }

  // Check for debug mode
  bool debugMode = QProcessEnvironment::systemEnvironment().contains("QLITH_DEBUG");
  if (debugMode)
  {
    qDebug() << "Running in debug mode";
  }

  // Register metatype for proper signal/slot connections
  qRegisterMetaType<container_qt5 *>("container_qt5*");

  // Initialize fonts - try multiple path formats for better reliability
  qDebug() << "Current directory:" << QDir::currentPath();
  qDebug() << "Application directory:" << QCoreApplication::applicationDirPath();

  const int defaultStaticFontSize = 16; // Define a local default font size

  // Try different resource paths for font loading
  bool fontLoaded = false;

  // Try to load Cousine Regular font with different paths
  fontLoaded = tryLoadFont(":/font/Cousine-Regular.ttf", "Cousine", defaultStaticFontSize);
  if (!fontLoaded)
  {
    fontLoaded = tryLoadFont(":/res/font/Cousine-Regular.ttf", "Cousine", defaultStaticFontSize);
  }

  // Don't worry too much about other fonts - they're optional
  // and we don't want to clutter the debug output with failures

  // Handle export mode
  bool exportMode = !svgPath.isEmpty() || !pngPath.isEmpty();

  qDebug() << "Export mode:" << (exportMode ? "true" : "false");
  if (exportMode) {
    qDebug() << "PNG export path:" << pngPath;
    qDebug() << "SVG export path:" << svgPath;
  }

  // Create main window with debug flag
  MainWindow w(debugMode);
  
  // Set the render size for export
  w.setRenderSize(QSize(renderWidth, renderHeight));
  
  // Always show the window, even in export mode for proper rendering
  w.show();
  
  // Load file if specified
  if (urlToLoad.isValid()) {
    qDebug() << "Loading URL/file:" << urlToLoad.toString();
    // MainWindow::loadFile expects a QString path for local files,
    // or MainWindow::loadUrl can take a QUrl.
    // Since urlToLoad is a QUrl, we can pass it to loadUrl,
    // and MainWindow::loadUrl can then decide if it's a local file.
    w.loadUrl(urlToLoad.toString()); // Pass the string representation of the QUrl
    
    // If in export mode, perform export after page load
    if (exportMode) {
      // Wait for document to load and then export
      QEventLoop waitLoop;
      
      // Track loading success
      bool loadingSuccess = false;
      
      // Connect documentLoaded signal to exit the wait loop
      QObject::connect(&w, &MainWindow::documentLoaded, [&waitLoop, &loadingSuccess](bool success) {
        qDebug() << "Document loaded signal received, success:" << success;
        loadingSuccess = success;
        // Continue whether or not loading was successful
        waitLoop.quit();
      });
      
      // Set a longer timeout for the waitLoop to ensure rendering completes (increased from 5000 to 10000)
      QTimer::singleShot(10000, &waitLoop, &QEventLoop::quit);
      
      // Wait for document to load or timeout
      qDebug() << "Waiting for document to load...";
      waitLoop.exec();
      
      if (!loadingSuccess) {
        qWarning() << "Document loading may have failed, but proceeding with export attempt";
      }
      
      // Add additional delay to ensure rendering happens (increased from 500 to 1000)
      QThread::msleep(1000);

      // Process events multiple times to ensure rendering happens (increased loops from 3 to 5)
      for (int i = 0; i < 5; i++) {
        QApplication::processEvents();
        QThread::msleep(200); // Increased from 100 to 200
      }
      
      // Force the window to repaint
      w.repaint();
      QApplication::processEvents();
      
      // Verify widget exists but don't access its methods directly
      QWidget* htmlWidget = w.findChild<QWidget*>();
      if (htmlWidget) {
        qDebug() << "Widget found, current size:" << htmlWidget->size();
        
        // Force a render update on the widget
        htmlWidget->update();
        QApplication::processEvents();
      } else {
        qWarning() << "Could not find widget in MainWindow";
      }
      
      // Perform export operations
      bool exportSuccess = false;
      
      if (!svgPath.isEmpty()) {
        qDebug() << "Exporting to SVG:" << svgPath;
        exportSuccess = w.exportToSvg(svgPath);
        qDebug() << "SVG export result:" << (exportSuccess ? "Success" : "Failed");
      }
      
      if (!pngPath.isEmpty()) {
        qDebug() << "Exporting to PNG:" << pngPath;
        exportSuccess = w.exportToPng(pngPath);
        qDebug() << "PNG export result:" << (exportSuccess ? "Success" : "Failed");
      }
      
      // Quit immediately after export without waiting for user to close window
      if (exportSuccess) {
        qDebug() << "Export completed successfully. Exiting application.";
        QTimer::singleShot(100, &app, QApplication::quit);
        return app.exec();
      } else {
        qCritical() << "Export failed! Exiting application.";
        QTimer::singleShot(100, &app, QApplication::quit);
        return 1;  // Return non-zero exit code on failure
      }
    }
  }

  return app.exec();
}
