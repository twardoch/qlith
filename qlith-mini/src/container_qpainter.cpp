// this_file: qlith-mini/src/container_qpainter.cpp
#include "container_qpainter.h"
#include "container_qpainter_p.h"

#include <QApplication>
#include <QDebug>
#include <QDir>
#include <QFontDatabase>
#include <QFontMetrics>
#include <QPainter>
#include <QScreen>
#include <QUrl>
#include <QFile>

// Default font settings
static const int DEFAULT_FONT_SIZE = 16;
static const QString DEFAULT_FONT_FAMILY = "Arial";

// Constructor
ContainerQPainter::ContainerQPainter(QObject* parent)
    : QObject(parent)
    , m_painter(nullptr)
    , m_devicePixelRatio(1.0)
    , m_defaultFontName(DEFAULT_FONT_FAMILY)
    , m_defaultFontSize(DEFAULT_FONT_SIZE)
    , m_nextFontId(1)
{
    // Initialize with system fonts
    QFontDatabase fontDb;
    if (fontDb.families().contains("Arial")) {
        m_defaultFontName = "Arial";
    } else if (fontDb.families().contains("Helvetica")) {
        m_defaultFontName = "Helvetica";
    } else if (!fontDb.families().isEmpty()) {
        m_defaultFontName = fontDb.families().first();
    }
    
    // Get device pixel ratio from primary screen
    if (QApplication::primaryScreen()) {
        m_devicePixelRatio = QApplication::primaryScreen()->devicePixelRatio();
    }
}

// Destructor
ContainerQPainter::~ContainerQPainter()
{
    // Clean up any remaining resources
    m_images.clear();
    m_fonts.clear();
}

// Set the base URL for resolving relative paths
void ContainerQPainter::setBaseUrl(const QString& baseUrl)
{
    m_baseUrl = baseUrl;
}

// Get the current base URL
QString ContainerQPainter::baseUrl() const
{
    return m_baseUrl;
}

// Set the device pixel ratio for high-DPI screens
void ContainerQPainter::setDevicePixelRatio(qreal ratio)
{
    m_devicePixelRatio = ratio;
}

// Get the current device pixel ratio
qreal ContainerQPainter::devicePixelRatio() const
{
    return m_devicePixelRatio;
}

// Set the default font name
void ContainerQPainter::setDefaultFontName(const QString& fontName)
{
    m_defaultFontName = fontName;
}

// Get the default font name
QString ContainerQPainter::defaultFontName() const
{
    return m_defaultFontName;
}

// Set the default font size
void ContainerQPainter::setDefaultFontSize(int size)
{
    m_defaultFontSize = size;
}

// Get the default font size
int ContainerQPainter::defaultFontSize() const
{
    return m_defaultFontSize;
}

// Start painting on the given QPainter
void ContainerQPainter::beginPaint(QPainter* painter, const QRect& rect)
{
    m_painter = painter;
    m_paintRect = rect;
    
    if (m_painter) {
        m_painter->save();
        m_painter->setClipRect(m_paintRect);
    }
}

// Finish painting
void ContainerQPainter::endPaint()
{
    if (m_painter) {
        m_painter->restore();
        m_painter = nullptr;
    }
}

// Create a font for use with the container
litehtml::uint_ptr ContainerQPainter::create_font(const litehtml::tchar_t* faceName, 
                                                 int size, 
                                                 int weight, 
                                                 litehtml::font_style italic, 
                                                 unsigned int decoration, 
                                                 litehtml::font_metrics* fm)
{
    // Create a QFont from the parameters
    QFont font;
    font.setFamily(QString::fromUtf8(faceName));
    font.setPixelSize(size);
    font.setWeight(weight);
    font.setItalic(italic == litehtml::font_style_italic);
    font.setUnderline(decoration & litehtml::font_decoration_underline);
    font.setStrikeOut(decoration & litehtml::font_decoration_linethrough);
    
    // Store font metrics
    font_metrics_t metrics(font);
    
    // Fill in the metrics if requested
    if (fm) {
        QFontMetrics qfm(font);
        fm->height = qfm.height();
        fm->ascent = qfm.ascent();
        fm->descent = qfm.descent();
        fm->x_height = qfm.boundingRect('x').height();
    }
    
    // Store the font and assign an ID
    int fontId = m_nextFontId++;
    m_fonts[fontId] = metrics;
    
    return static_cast<litehtml::uint_ptr>(fontId);
}

// Delete a font created with create_font
void ContainerQPainter::delete_font(litehtml::uint_ptr hFont)
{
    int fontId = static_cast<int>(hFont);
    m_fonts.remove(fontId);
}

// Get text width for measurement
int ContainerQPainter::text_width(const litehtml::tchar_t* text, litehtml::uint_ptr hFont)
{
    int fontId = static_cast<int>(hFont);
    if (!m_fonts.contains(fontId)) {
        return 0;
    }
    
    const font_metrics_t& metrics = m_fonts[fontId];
    return metrics.metrics.horizontalAdvance(QString::fromUtf8(text));
}

// Draw text with the given font and color
void ContainerQPainter::draw_text(litehtml::uint_ptr hdc, 
                                 const litehtml::tchar_t* text, 
                                 litehtml::uint_ptr hFont, 
                                 litehtml::web_color color, 
                                 const litehtml::position& pos)
{
    if (!m_painter) return;
    
    int fontId = static_cast<int>(hFont);
    if (!m_fonts.contains(fontId)) {
        return;
    }
    
    // Set up painter
    m_painter->save();
    m_painter->setPen(QColor(color.red, color.green, color.blue, color.alpha));
    m_painter->setFont(m_fonts[fontId].font);
    
    // Draw the text
    const QFontMetrics& metrics = m_fonts[fontId].metrics;
    m_painter->drawText(pos.x, pos.y + metrics.ascent(), QString::fromUtf8(text));
    
    m_painter->restore();
}

// Convert points to pixels
int ContainerQPainter::pt_to_px(int pt) const
{
    // Standard conversion: 1 point = 1/72 inch
    // Use device pixel ratio for high DPI screens
    return static_cast<int>(pt * m_devicePixelRatio * (96.0 / 72.0));
}

// Get the default font size
int ContainerQPainter::get_default_font_size() const
{
    return m_defaultFontSize;
}

// Get the default font name
const litehtml::tchar_t* ContainerQPainter::get_default_font_name() const
{
    static QByteArray fontName;
    fontName = m_defaultFontName.toUtf8();
    return fontName.constData();
}

// Draw list marker (bullet, number, etc.)
void ContainerQPainter::draw_list_marker(litehtml::uint_ptr hdc, const litehtml::list_marker& marker)
{
    if (!m_painter) return;
    
    m_painter->save();
    
    // Draw based on marker type
    QColor markerColor(marker.color.red, marker.color.green, marker.color.blue, marker.color.alpha);
    m_painter->setPen(markerColor);
    m_painter->setBrush(markerColor);
    
    if (marker.image.empty()) {
        // Draw the marker based on type
        QRect rect(marker.pos.x, marker.pos.y, marker.pos.width, marker.pos.height);
        
        switch (marker.marker_type) {
            case litehtml::list_style_type_circle:
                m_painter->drawEllipse(rect);
                break;
                
            case litehtml::list_style_type_disc:
                m_painter->setBrush(markerColor);
                m_painter->drawEllipse(rect);
                break;
                
            case litehtml::list_style_type_square:
                m_painter->drawRect(rect);
                break;
                
            default:
                // For other types (decimal, alpha, roman, etc.), draw the text
                if (!marker.text.empty()) {
                    m_painter->drawText(rect, Qt::AlignCenter, QString::fromUtf8(marker.text.c_str()));
                }
                break;
        }
    } else {
        // Draw an image marker if available
        QString imgUrl = QString::fromUtf8(marker.image.c_str());
        if (m_images.contains(imgUrl)) {
            QRect rect(marker.pos.x, marker.pos.y, marker.pos.width, marker.pos.height);
            m_painter->drawImage(rect, m_images[imgUrl]);
        }
    }
    
    m_painter->restore();
}

// Load image from URL
void ContainerQPainter::load_image(const litehtml::tchar_t* src, const litehtml::tchar_t* baseurl, bool redraw_on_ready)
{
    QString url = QString::fromUtf8(src);
    QString base = QString::fromUtf8(baseurl);
    
    // Resolve relative URLs
    QUrl resolvedUrl;
    if (QUrl(url).isRelative() && !base.isEmpty()) {
        resolvedUrl = QUrl(base).resolved(QUrl(url));
    } else {
        resolvedUrl = QUrl(url);
    }
    
    QString urlString = resolvedUrl.toString();
    
    // Skip if already loaded
    if (m_images.contains(urlString)) {
        return;
    }
    
    // For local files, load directly
    if (resolvedUrl.isLocalFile()) {
        QImage img(resolvedUrl.toLocalFile());
        if (!img.isNull()) {
            m_images[urlString] = img;
            if (redraw_on_ready) {
                // Emit a signal or update UI if needed
            }
        }
    } else {
        // For remote URLs, we'd need to implement network fetching
        // For now, just log that we need to implement this
        qDebug() << "Remote image loading not implemented for:" << urlString;
    }
}

// Get image size for layout
void ContainerQPainter::get_image_size(const litehtml::tchar_t* src, const litehtml::tchar_t* baseurl, litehtml::size& sz)
{
    QString url = QString::fromUtf8(src);
    QString base = QString::fromUtf8(baseurl);
    
    // Resolve URL
    QUrl resolvedUrl;
    if (QUrl(url).isRelative() && !base.isEmpty()) {
        resolvedUrl = QUrl(base).resolved(QUrl(url));
    } else {
        resolvedUrl = QUrl(url);
    }
    
    QString urlString = resolvedUrl.toString();
    
    // If image is loaded, get its size
    if (m_images.contains(urlString)) {
        const QImage& img = m_images[urlString];
        sz.width = img.width();
        sz.height = img.height();
    } else {
        // Default size if image not loaded yet
        sz.width = 0;
        sz.height = 0;
    }
}

// Draw background (color, image, gradient)
void ContainerQPainter::draw_background(litehtml::uint_ptr hdc, const litehtml::background_paint& bg)
{
    if (!m_painter) return;
    
    m_painter->save();
    
    // Draw background color
    if (bg.color.alpha > 0) {
        QColor bgColor(bg.color.red, bg.color.green, bg.color.blue, bg.color.alpha);
        m_painter->fillRect(bg.clip_box.x, bg.clip_box.y, bg.clip_box.width, bg.clip_box.height, bgColor);
    }
    
    // Draw background image if available
    if (!bg.image.empty() && m_images.contains(QString::fromUtf8(bg.image.c_str()))) {
        const QImage& img = m_images[QString::fromUtf8(bg.image.c_str())];
        
        QRect destRect(bg.position_x, bg.position_y, bg.img_width, bg.img_height);
        QRect clipRect(bg.clip_box.x, bg.clip_box.y, bg.clip_box.width, bg.clip_box.height);
        
        m_painter->setClipRect(clipRect);
        
        // Handle different background-repeat values
        if (bg.repeat == litehtml::background_repeat_no_repeat) {
            m_painter->drawImage(destRect, img);
        } else if (bg.repeat == litehtml::background_repeat_repeat_x) {
            for (int x = bg.position_x; x < clipRect.right(); x += bg.img_width) {
                m_painter->drawImage(QRect(x, bg.position_y, bg.img_width, bg.img_height), img);
            }
        } else if (bg.repeat == litehtml::background_repeat_repeat_y) {
            for (int y = bg.position_y; y < clipRect.bottom(); y += bg.img_height) {
                m_painter->drawImage(QRect(bg.position_x, y, bg.img_width, bg.img_height), img);
            }
        } else if (bg.repeat == litehtml::background_repeat_repeat) {
            for (int y = bg.position_y; y < clipRect.bottom(); y += bg.img_height) {
                for (int x = bg.position_x; x < clipRect.right(); x += bg.img_width) {
                    m_painter->drawImage(QRect(x, y, bg.img_width, bg.img_height), img);
                }
            }
        }
    }
    
    m_painter->restore();
}

// Draw borders
void ContainerQPainter::draw_borders(litehtml::uint_ptr hdc, const litehtml::borders& borders, const litehtml::position& draw_pos, bool root)
{
    if (!m_painter || root) return;
    
    m_painter->save();
    
    QRect borderRect(draw_pos.x, draw_pos.y, draw_pos.width, draw_pos.height);
    
    // Draw top border
    if (borders.top.width > 0 && borders.top.style != litehtml::border_style_none) {
        QPen pen(QColor(borders.top.color.red, borders.top.color.green, borders.top.color.blue, borders.top.color.alpha));
        pen.setWidth(borders.top.width);
        
        switch (borders.top.style) {
            case litehtml::border_style_dotted:
                pen.setStyle(Qt::DotLine);
                break;
            case litehtml::border_style_dashed:
                pen.setStyle(Qt::DashLine);
                break;
            default:
                pen.setStyle(Qt::SolidLine);
                break;
        }
        
        m_painter->setPen(pen);
        m_painter->drawLine(borderRect.left(), borderRect.top() + borders.top.width/2, 
                            borderRect.right(), borderRect.top() + borders.top.width/2);
    }
    
    // Draw right border
    if (borders.right.width > 0 && borders.right.style != litehtml::border_style_none) {
        QPen pen(QColor(borders.right.color.red, borders.right.color.green, borders.right.color.blue, borders.right.color.alpha));
        pen.setWidth(borders.right.width);
        
        switch (borders.right.style) {
            case litehtml::border_style_dotted:
                pen.setStyle(Qt::DotLine);
                break;
            case litehtml::border_style_dashed:
                pen.setStyle(Qt::DashLine);
                break;
            default:
                pen.setStyle(Qt::SolidLine);
                break;
        }
        
        m_painter->setPen(pen);
        m_painter->drawLine(borderRect.right() - borders.right.width/2, borderRect.top(), 
                            borderRect.right() - borders.right.width/2, borderRect.bottom());
    }
    
    // Draw bottom border
    if (borders.bottom.width > 0 && borders.bottom.style != litehtml::border_style_none) {
        QPen pen(QColor(borders.bottom.color.red, borders.bottom.color.green, borders.bottom.color.blue, borders.bottom.color.alpha));
        pen.setWidth(borders.bottom.width);
        
        switch (borders.bottom.style) {
            case litehtml::border_style_dotted:
                pen.setStyle(Qt::DotLine);
                break;
            case litehtml::border_style_dashed:
                pen.setStyle(Qt::DashLine);
                break;
            default:
                pen.setStyle(Qt::SolidLine);
                break;
        }
        
        m_painter->setPen(pen);
        m_painter->drawLine(borderRect.left(), borderRect.bottom() - borders.bottom.width/2, 
                            borderRect.right(), borderRect.bottom() - borders.bottom.width/2);
    }
    
    // Draw left border
    if (borders.left.width > 0 && borders.left.style != litehtml::border_style_none) {
        QPen pen(QColor(borders.left.color.red, borders.left.color.green, borders.left.color.blue, borders.left.color.alpha));
        pen.setWidth(borders.left.width);
        
        switch (borders.left.style) {
            case litehtml::border_style_dotted:
                pen.setStyle(Qt::DotLine);
                break;
            case litehtml::border_style_dashed:
                pen.setStyle(Qt::DashLine);
                break;
            default:
                pen.setStyle(Qt::SolidLine);
                break;
        }
        
        m_painter->setPen(pen);
        m_painter->drawLine(borderRect.left() + borders.left.width/2, borderRect.top(), 
                            borderRect.left() + borders.left.width/2, borderRect.bottom());
    }
    
    m_painter->restore();
}

// Get client rect for viewport
void ContainerQPainter::get_client_rect(litehtml::position& client) const
{
    if (m_painter) {
        QRect viewport = m_painter->viewport();
        client.x = viewport.x();
        client.y = viewport.y();
        client.width = viewport.width();
        client.height = viewport.height();
    } else {
        client.x = 0;
        client.y = 0;
        client.width = 800;  // Default fallback width
        client.height = 600; // Default fallback height
    }
}

// Get media features for media queries
void ContainerQPainter::get_media_features(litehtml::media_features& media) const
{
    QRect screenRect;
    
    if (QApplication::primaryScreen()) {
        screenRect = QApplication::primaryScreen()->geometry();
    } else {
        // Fallback if no screen is available
        screenRect = QRect(0, 0, 1024, 768);
    }
    
    media.type = litehtml::media_type_screen;
    media.width = screenRect.width();
    media.height = screenRect.height();
    media.device_width = screenRect.width();
    media.device_height = screenRect.height();
    media.color = 24; // Assume 24-bit color
    media.monochrome = 0;
    media.color_index = 0;
    media.resolution = 96; // Standard resolution
    
    // Apply device pixel ratio for high DPI screens
    if (m_devicePixelRatio > 1.0) {
        media.resolution = static_cast<int>(media.resolution * m_devicePixelRatio);
    }
}

// Handle document title
void ContainerQPainter::set_caption(const litehtml::tchar_t* caption)
{
    QString title = QString::fromUtf8(caption);
    emit titleChanged(title);
}

// Set base URL for document
void ContainerQPainter::set_base_url(const litehtml::tchar_t* base_url)
{
    m_baseUrl = QString::fromUtf8(base_url);
}

// Handle document link elements
void ContainerQPainter::link(const std::shared_ptr<litehtml::document>& doc, const litehtml::element::ptr& el)
{
    // Implementation for link processing if needed
}

// Handle anchor clicks
void ContainerQPainter::on_anchor_click(const litehtml::tchar_t* url, const litehtml::element::ptr& el)
{
    QString linkUrl = QString::fromUtf8(url);
    emit anchorClicked(linkUrl);
}

// Set cursor based on element
void ContainerQPainter::set_cursor(const litehtml::tchar_t* cursor)
{
    QString cursorName = QString::fromUtf8(cursor);
    emit cursorChanged(cursorName);
}

// Transform text based on CSS text-transform
void ContainerQPainter::transform_text(litehtml::tstring& text, litehtml::text_transform tt)
{
    if (text.empty()) return;
    
    QString qtext = QString::fromUtf8(text.c_str());
    
    switch (tt) {
        case litehtml::text_transform_capitalize:
            qtext = qtext.toLower();
            if (!qtext.isEmpty()) {
                qtext[0] = qtext[0].toUpper();
            }
            break;
            
        case litehtml::text_transform_uppercase:
            qtext = qtext.toUpper();
            break;
            
        case litehtml::text_transform_lowercase:
            qtext = qtext.toLower();
            break;
            
        default:
            break;
    }
    
    QByteArray utf8 = qtext.toUtf8();
    text = std::string(utf8.constData(), utf8.length());
}

// Import CSS from URL
void ContainerQPainter::import_css(litehtml::tstring& text, const litehtml::tstring& url, litehtml::tstring& baseurl)
{
    // Implementation for CSS loading if needed
    // For now, just leave it empty
}

// Set clipping region
void ContainerQPainter::set_clip(const litehtml::position& pos, const litehtml::border_radiuses& bdr_radius, bool valid_x, bool valid_y)
{
    if (!m_painter) return;
    
    QRect clipRect(pos.x, pos.y, pos.width, pos.height);
    m_painter->setClipRect(clipRect, Qt::IntersectClip);
}

// Remove clipping
void ContainerQPainter::del_clip()
{
    // QPainter handles clipping stack via save/restore, so we don't need explicit implementation
}

// Callback for image loading
void ContainerQPainter::onImageLoaded(const QString& url, const QImage& image)
{
    if (!image.isNull()) {
        m_images[url] = image;
    }
}

// Callback for CSS loading
void ContainerQPainter::onCssLoaded(const QString& url, const QString& css)
{
    // Implementation for CSS loading callback if needed
} 