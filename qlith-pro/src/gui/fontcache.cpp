// -*- coding: utf-8 -*-

#include "qlith/fontcache.h"

#include <QFontInfo>

/* Null, because instance will be initialized on demand. */
FontCache *FontCache::instance = nullptr;

QString FontCache::defaultFontName = R"raw("Cousine Regular")raw";

FontCache *FontCache::getInstance()
{
  if (instance == 0)
  {
    instance = new FontCache();
  }

  return instance;
}

QString FontCache::getDefaultFontName()
{
  return defaultFontName;
}

FontCache::FontCache()
{
}

FontCache::~FontCache()
{
  for (QFont *font : cache)
  {
    delete font;
  }
}

QFont *FontCache::getFont(const QString &name) const
{
  if (!cache.contains(name) || cache[name] == nullptr)
  {
    qWarning() << "Font " << name << " cannot be found! Using system default.";
    // Return a default font if the requested one is not available
    static QFont *defaultFont = nullptr;
    if (!defaultFont)
    {
      defaultFont = new QFont();
      defaultFont->setFamily("Arial");
      defaultFont->setPixelSize(16);
    }
    return defaultFont;
  }
  return cache[name];
}

QFont *FontCache::addFont(const QString &path, const QString &name, int pixelSize)
{
  QFile fontFile(path);
  if (!fontFile.open(QIODevice::ReadOnly))
  {
    qCritical() << "failed to open font file, path = " << path;

    // Try with alternative paths if the direct path fails
    // This provides a fallback mechanism for different resource path formats
    QString altPath = path;
    if (path.startsWith(":/res/"))
    {
      // Already has the correct format
    }
    else if (path.startsWith(":/"))
    {
      // Try adding 'res/' after the colon
      altPath = QString(":/res") + path.mid(1);
      fontFile.setFileName(altPath);
      if (!fontFile.open(QIODevice::ReadOnly))
      {
        qWarning() << "Also failed with alternative path: " << altPath;
        return nullptr;
      }
    }
  }

  QByteArray fontData = fontFile.readAll();
  if (fontData.isEmpty())
  {
    qCritical() << "empty font file, path = " << path;
    return nullptr;
  }

  int id = QFontDatabase::addApplicationFontFromData(fontData);
  if (id < 0)
  {
    qCritical() << "font " << path << " cannot be loaded !";
    return nullptr;
  }

  QStringList families = QFontDatabase::applicationFontFamilies(id);
  if (families.isEmpty())
  {
    qCritical() << "No font families found for " << path;
    return nullptr;
  }

  QString family = families.at(0);
  qDebug() << "Adding font family " << family << " for font " << name;

  QFont *font = new QFont;
  font->setFamily(family);
  font->setPixelSize(pixelSize);
  font->setWeight(QFont::Weight::Normal);
  font->setStyle(QFont::Style::StyleNormal);
  cache[name] = font;

  QFontInfo fontInfo(*font);
  qDebug() << "Adding font:" << font->family() << " font family:" << fontInfo.family();

  return font;
}
