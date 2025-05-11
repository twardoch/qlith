// -*- coding: utf-8 -*-

#include <QApplication>
#include <QFile>
#include <QDir>
#include <QStandardPaths>
#include <QProcessEnvironment>
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

  // Create main window with debug flag
  MainWindow w(debugMode);
  w.show();

  return app.exec();
}
