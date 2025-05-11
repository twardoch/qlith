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
#include "qlith/container_qt5.h"
#include "qlith/context.h"
#include "qlith/mainwindow.h"
#include <QDebug>

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
  
  // Get input file
  QString inputFile;
  const QStringList args = parser.positionalArguments();
  if (!args.isEmpty()) {
    inputFile = args.first();
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
  if (!inputFile.isEmpty()) {
    qDebug() << "Loading file:" << inputFile;
    w.loadFile(inputFile);
    
    // If in export mode, perform export after page load
    if (exportMode) {
      // Explicitly capture needed variables to avoid dangling references
      QObject::connect(&w, &MainWindow::documentLoaded, [&app, &w, svgPath, pngPath](bool success) {
        qDebug() << "Document loaded signal received, success:" << success;
        if (success) {
          if (!svgPath.isEmpty()) {
            w.exportToSvg(svgPath);
          }
          if (!pngPath.isEmpty()) {
            qDebug() << "Exporting to PNG:" << pngPath;
            bool result = w.exportToPng(pngPath);
            qDebug() << "PNG export result:" << (result ? "Success" : "Failed");
          }
        } else {
          qWarning() << "Failed to load document for export";
        }
        // Quit after export with a short delay to ensure proper cleanup
        QTimer::singleShot(100, &app, &QApplication::quit);
      });
      
      // Set a backup timer in case documentLoaded signal is never emitted
      QTimer::singleShot(5000, [&app, &w, svgPath, pngPath]() {
        qDebug() << "Backup timer triggered - forcing export";
        if (!svgPath.isEmpty()) {
          w.exportToSvg(svgPath);
        }
        if (!pngPath.isEmpty()) {
          qDebug() << "Exporting to PNG (backup):" << pngPath;
          bool result = w.exportToPng(pngPath);
          qDebug() << "Backup PNG export result:" << (result ? "Success" : "Failed");
        }
        // Quit after export
        QTimer::singleShot(100, &app, &QApplication::quit);
      });
    }
  }

  return app.exec();
}
