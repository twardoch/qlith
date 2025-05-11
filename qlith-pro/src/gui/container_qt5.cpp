#include "qlith/container_qt5.h"
#include <QDebug>
#include <QFont>
#include <QFontMetrics>
#include <QPainter>
#include <QPainterPath>
#include <QMouseEvent>
#include <QDesktopServices>
#include <QGuiApplication>
#include <QScreen>
#include <QImage>
#include <QFile>
#include <QDir>
#include <QUrl>
#include <cmath>
#include <QFontDatabase>
#include <QWindow>
#include <set>

// Default font settings - no longer needed as members are used
// static const int DEFAULT_FONT_SIZE = 16;
// static const QString DEFAULT_FONT_FAMILY = "Arial";

// Convert litehtml position to QRect
static QRect positionToRect(const litehtml::position& pos) {
    return QRect(pos.x, pos.y, pos.width, pos.height);
}

// Convert litehtml web_color to QColor
static QColor webColorToQColor(const litehtml::web_color& color) {
    return QColor(color.red, color.green, color.blue, color.alpha);
}

// Global clip stack
static litehtml::position::vector g_clips;

// Initialize static members
// int container_qt5::m_defaultFontSize = DEFAULT_FONT_SIZE; // Now a non-static member

// Constructor
container_qt5::container_qt5(QWidget* parent)
    : QObject(parent)
    , litehtml::document_container()
    , _doc(nullptr)
    , m_owner(parent)
    , m_painter(nullptr)
    , m_nextFontId(1)
// m_defaultFontName and m_defaultFontSize are initialized below
{
    Q_ASSERT(m_owner != nullptr);

    QFontDatabase fontDb;

    // First, try to find common system fonts in this specific order
    QStringList preferredFonts = {"Arial", "Helvetica", "Liberation Sans", "DejaVu Sans", "Roboto", "Verdana", "SansSerif"};

    bool fontFound = false;
    for (const QString &fontName : preferredFonts)
    {
        if (fontDb.families().contains(fontName, Qt::CaseInsensitive))
        {
            m_defaultFontName = fontName;
            fontFound = true;
            qDebug() << "container_qt5: Using preferred font:" << m_defaultFontName;
            break;
        }
    }

    // If none of the preferred fonts were found, fall back to the first available system font
    if (!fontFound)
    {
        if (!fontDb.families().isEmpty())
        {
            m_defaultFontName = fontDb.families().first();
            qDebug() << "container_qt5: Default font family set to the first available:" << m_defaultFontName;
        }
        else
        {
            m_defaultFontName = "Arial"; // Ultimate fallback
            qWarning() << "container_qt5: No system fonts found, falling back to Arial for defaultFontName.";
        }
    }

    m_defaultFontSize = 16; // Default size in pixels
    qDebug() << "container_qt5: Initialized with default font:" << m_defaultFontName << "size:" << m_defaultFontSize;
}

container_qt5::~container_qt5()
{
    m_images.clear();
    m_fonts.clear();
}

// Set the document to render
void container_qt5::set_document(std::shared_ptr<litehtml::document> doc)
{
    _doc = doc;
}

// Set the scroll position
void container_qt5::setScroll(const QPoint& val) {
    m_Scroll = val;
    
    // Update the document for the new scroll position
    if (_doc) {
        _doc->media_changed();
    }
}

void container_qt5::setScrollX(const int& val) {
    m_Scroll.setX(val);
    
    if (_doc) {
        _doc->media_changed();
    }
}

void container_qt5::setScrollY(const int& val) {
    m_Scroll.setY(val);
    
    if (_doc) {
        _doc->media_changed();
    }
}

// Get the current scroll position
QPoint container_qt5::getScroll() const {
    return m_Scroll;
}

// Track mouse coordinates
void container_qt5::setLastMouseCoords(int x, int y, int xClient, int yClient) {
    lastCursorX = x;
    lastCursorY = y;
    lastCursorClientX = xClient;
    lastCursorClientY = yClient;
}

// Set the painter to use for rendering
void container_qt5::setPainter(QPainter* painter) {
    m_painter = painter;
}

// Render the document
void container_qt5::repaint(QPainter& painter)
{
    if (!_doc) {
        qWarning() << "container_qt5::repaint called with null document";
        return;
    }

    // Set up painter
    setPainter(&painter);
    painter.setRenderHint(QPainter::Antialiasing);

    try
    {
        // Get widget size
        QRect rc = m_owner ? m_owner->rect() : QRect(0, 0, 800, 600);

        // Ensure positive dimensions
        int docWidth = std::max(rc.width(), 1);

        // Render document at appropriate width
        _doc->render(docWidth);

        // Set up clipping rectangle
        litehtml::position clipPos;
        clipPos.width = rc.width();
        clipPos.height = rc.height();
        clipPos.x = rc.x();
        clipPos.y = rc.y();

        // Draw the document - painter is guaranteed to be valid here
        _doc->draw(reinterpret_cast<litehtml::uint_ptr>(&painter), getScroll().x(), getScroll().y(), &clipPos);
    }
    catch (const std::exception &e)
    {
        qWarning() << "Exception in container_qt5::repaint:" << e.what();
    }
    catch (...) {
        qWarning() << "Unknown exception in container_qt5::repaint";
    }
    
    // Clean up
    setPainter(nullptr);

    // Notify if document size has changed
    if (_doc)
    {
        int newWidth = _doc->width();
        int newHeight = _doc->height();

        if (newWidth != m_lastDocWidth || newHeight != m_lastDocHeight)
        {
            emit docSizeChanged(newWidth, newHeight);
            emit documentSizeChanged(newWidth, newHeight);
            m_lastDocWidth = newWidth;
            m_lastDocHeight = newHeight;
        }
    }
}

// litehtml::document_container implementation

// Font creation
litehtml::uint_ptr container_qt5::create_font(const litehtml::font_description &descr, const litehtml::document * /*doc*/, litehtml::font_metrics *fm)
{
    QFont font;
    QString fontFamily = QString::fromUtf8(descr.family.c_str());
    QFontDatabase fontDb; // Keep QFontDatabase for checking

    // Simplified font family handling, closer to qlith-mini
    // If a specific font family is requested, try to use it.
    // Otherwise, or if not found, fall back to the container's default.
    if (!fontFamily.isEmpty() && fontDb.families().contains(fontFamily, Qt::CaseInsensitive)) {
        font.setFamily(fontFamily);
    } else {
        // Fallback to a generic family based on style hint if family is a generic one
        // This is a simplified version of the previous complex logic.
        bool genericSet = false;
        if (fontFamily.compare("serif", Qt::CaseInsensitive) == 0) {
            font.setStyleHint(QFont::Serif);
            genericSet = true;
        } else if (fontFamily.compare("sans-serif", Qt::CaseInsensitive) == 0) {
            font.setStyleHint(QFont::SansSerif);
            genericSet = true;
        } else if (fontFamily.compare("monospace", Qt::CaseInsensitive) == 0) {
            font.setStyleHint(QFont::Monospace);
            genericSet = true;
        }
        // If not a recognized generic or not found, use container's default.
        if (!genericSet) {
             qWarning() << "Font family not found or not a recognized generic:" << fontFamily << "- using default font:" << m_defaultFontName;
            font.setFamily(m_defaultFontName);
        }
    }

    // Set font size, ensuring it's positive or default.
    int fontSize = descr.size > 0 ? descr.size : m_defaultFontSize;
    font.setPixelSize(fontSize);

    // Apply weight, style, and decorations directly as in qlith-mini.
    font.setWeight(descr.weight);
    font.setItalic(descr.style == litehtml::font_style_italic);
    font.setUnderline(descr.decoration_line & litehtml::text_decoration_line_underline);
    font.setStrikeOut(descr.decoration_line & litehtml::text_decoration_line_line_through);

    // Create and store font metrics.
    font_metrics_t metrics(font);

    if (fm) {
        QFontMetrics qfm(font);
        fm->height = qfm.height();
        fm->ascent = qfm.ascent();
        fm->descent = qfm.descent();
        fm->x_height = qfm.boundingRect('x').height(); // Consistent with qlith-mini
    }

    int fontId = m_nextFontId++;
    m_fonts[fontId] = metrics;
    
    return static_cast<litehtml::uint_ptr>(fontId);
}

// Font deletion
void container_qt5::delete_font(litehtml::uint_ptr hFont)
{
    qDebug() << "container_qt5::delete_font - NO-OP (Diagnostic)";
}

// Measure text width
int container_qt5::text_width(const char* text, litehtml::uint_ptr hFont)
{
    qDebug() << "container_qt5::text_width - Returning default (Diagnostic)";
    if (!text) return 0;
    int fontId = static_cast<int>(hFont);
    if (m_fonts.contains(fontId)) {
        const font_metrics_t& metrics = m_fonts[fontId];
        return metrics.metrics.horizontalAdvance(QString::fromUtf8(text));
    }
    return 10 * strlen(text); // Fallback if font not found
}

// Draw text
void container_qt5::draw_text(litehtml::uint_ptr hdc, const char* text, litehtml::uint_ptr hFont, litehtml::web_color color, const litehtml::position& pos)
{
    if (!m_painter || !text) return;
    
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
    
    qDebug() << "container_qt5::draw_text - Drawing text:" << text;
}

// Convert points to pixels
int container_qt5::pt_to_px(int pt) const
{
    qreal dpr = 1.0;
    if (m_owner && m_owner->window() && m_owner->window()->windowHandle()) {
      dpr = m_owner->window()->windowHandle()->devicePixelRatio();
    } else if (QGuiApplication::primaryScreen()) {
        dpr = QGuiApplication::primaryScreen()->devicePixelRatio();
    }
    // A more robust way to convert pt to px considering DPI.
    // Standard CSS definition: 1pt = 1/72 inch. Standard DPI: 96dpi.
    // So, px = pt * (DPI / 72). We use 96 as a common screen DPI.
    return static_cast<int>(round((pt * 96.0 / 72.0) * dpr));
}

// Get default font size
int container_qt5::get_default_font_size() const
{
    return this->m_defaultFontSize;
}

// Get default font name
const char* container_qt5::get_default_font_name() const
{
    // CRITICAL FIX: Use a static QByteArray to ensure the pointer remains valid,
    // similar to the working qlith-mini implementation.
    static QByteArray fontNameHolder;
    fontNameHolder = this->m_defaultFontName.toUtf8();
    return fontNameHolder.constData();
}

// Draw a list marker (bullet, number, etc.)
void container_qt5::draw_list_marker(litehtml::uint_ptr hdc, const litehtml::list_marker& marker)
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
                m_painter->setBrush(Qt::NoBrush);
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
                if (marker.index >= 0) {
                    QString indexStr = QString::number(marker.index);
                    m_painter->drawText(rect, Qt::AlignCenter, indexStr);
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
    
    qDebug() << "container_qt5::draw_list_marker - Drawing list marker of type:" << (int)marker.marker_type;
}

// Draw image background
void container_qt5::draw_image(litehtml::uint_ptr hdc, const litehtml::background_layer& layer, const std::string& url, const std::string& base_url)
{
    if (!m_painter) return;
    
    m_painter->save();
    
    QString imgUrl = QString::fromStdString(url);
    if (m_images.contains(imgUrl)) {
        const QImage& img = m_images[imgUrl];
        
        QRect clipRect(layer.clip_box.x, layer.clip_box.y, layer.clip_box.width, layer.clip_box.height);
        m_painter->setClipRect(clipRect);
        
        // Calculate image position and size based on background-position and background-size
        // In this simplified implementation, we just place it at the origin_box position
        QRect imgRect(layer.origin_box.x, layer.origin_box.y, img.width(), img.height());
        
        // Handle different background-repeat values
        if (layer.repeat == litehtml::background_repeat_no_repeat) {
            m_painter->drawImage(imgRect, img);
            qDebug() << "container_qt5::draw_image - Drawing image (no-repeat):" << imgUrl;
        } else if (layer.repeat == litehtml::background_repeat_repeat_x) {
            for (int x = layer.origin_box.x; x < clipRect.right(); x += img.width()) {
                m_painter->drawImage(QRect(x, layer.origin_box.y, img.width(), img.height()), img);
            }
            qDebug() << "container_qt5::draw_image - Drawing image (repeat-x):" << imgUrl;
        } else if (layer.repeat == litehtml::background_repeat_repeat_y) {
            for (int y = layer.origin_box.y; y < clipRect.bottom(); y += img.height()) {
                m_painter->drawImage(QRect(layer.origin_box.x, y, img.width(), img.height()), img);
            }
            qDebug() << "container_qt5::draw_image - Drawing image (repeat-y):" << imgUrl;
        } else if (layer.repeat == litehtml::background_repeat_repeat) {
            for (int y = layer.origin_box.y; y < clipRect.bottom(); y += img.height()) {
                for (int x = layer.origin_box.x; x < clipRect.right(); x += img.width()) {
                    m_painter->drawImage(QRect(x, y, img.width(), img.height()), img);
                }
            }
            qDebug() << "container_qt5::draw_image - Drawing image (repeat):" << imgUrl;
        }
    } else {
        qDebug() << "container_qt5::draw_image - Image not found:" << imgUrl;
    }
    
    m_painter->restore();
}

// Load an image from a URL
void container_qt5::load_image(const char* src, const char* baseurl, bool redraw_on_ready)
{
    if (!src) {
        qWarning() << "container_qt5::load_image - Null source";
        return;
    }
    
    QString url = QString::fromUtf8(src);
    QString base = baseurl ? QString::fromUtf8(baseurl) : m_baseUrl;
    
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
            qDebug() << "container_qt5::load_image - Loaded local image:" << urlString;
            
            if (redraw_on_ready && m_owner) {
                m_owner->update();
            }
        } else {
            qWarning() << "container_qt5::load_image - Failed to load local image:" << urlString;
        }
    } else {
        // For remote URLs, just log that we need to implement this
        // In a full implementation, we would use QNetworkAccessManager to fetch the image
        qDebug() << "container_qt5::load_image - Remote image loading not implemented for:" << urlString;
    }
}

// Get image dimensions
void container_qt5::get_image_size(const char* src, const char* baseurl, litehtml::size& sz)
{
    if (!src) {
        sz.width = 0;
        sz.height = 0;
        return;
    }
    
    QString url = QString::fromUtf8(src);
    QString base = baseurl ? QString::fromUtf8(baseurl) : m_baseUrl;
    
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
        qDebug() << "container_qt5::get_image_size - Image size for" << urlString 
                 << "is" << sz.width << "x" << sz.height;
    } else {
        // Default size if image not loaded yet
        sz.width = 0;
        sz.height = 0;
        qDebug() << "container_qt5::get_image_size - Image not loaded:" << urlString;
    }
}

// Draw solid color background
void container_qt5::draw_solid_fill(litehtml::uint_ptr hdc, const litehtml::background_layer& layer, const litehtml::web_color& color)
{
    if (!m_painter) return;
    
    m_painter->save();
    
    QColor bgColor(color.red, color.green, color.blue, color.alpha);
    m_painter->fillRect(layer.clip_box.x, layer.clip_box.y, layer.clip_box.width, layer.clip_box.height, bgColor);
    
    m_painter->restore();
    
    qDebug() << "container_qt5::draw_solid_fill - Drawing solid fill with color:" 
             << bgColor.name() << "(alpha:" << bgColor.alpha() << ")";
}

// Draw linear gradient
void container_qt5::draw_linear_gradient(litehtml::uint_ptr hdc, const litehtml::background_layer& layer, const litehtml::background_layer::linear_gradient& gradient)
{
    if (!m_painter) return;
    
    m_painter->save();
    
    QRect clipRect(layer.clip_box.x, layer.clip_box.y, layer.clip_box.width, layer.clip_box.height);
    m_painter->setClipRect(clipRect);
    
    // Create a linear gradient
    QLinearGradient qgradient;
    qgradient.setStart(gradient.start.x, gradient.start.y);
    qgradient.setFinalStop(gradient.end.x, gradient.end.y);
    
    // Add color stops
    for (const auto& stop : gradient.color_points) {
        QColor stopColor(stop.color.red, stop.color.green, stop.color.blue, stop.color.alpha);
        qgradient.setColorAt(stop.offset, stopColor);
    }
    
    // Draw the gradient
    m_painter->fillRect(clipRect, qgradient);
    
    m_painter->restore();
    
    qDebug() << "container_qt5::draw_linear_gradient - Drawing gradient from" 
             << gradient.start.x << "," << gradient.start.y << " to " 
             << gradient.end.x << "," << gradient.end.y;
}

// Draw radial gradient
void container_qt5::draw_radial_gradient(litehtml::uint_ptr hdc, const litehtml::background_layer& layer, const litehtml::background_layer::radial_gradient& gradient)
{
    if (!m_painter) return;
    
    m_painter->save();
    
    QRect clipRect(layer.clip_box.x, layer.clip_box.y, layer.clip_box.width, layer.clip_box.height);
    m_painter->setClipRect(clipRect);
    
    // Create a radial gradient
    // Use the maximum of x and y radius for simplicity (Qt doesn't support elliptical gradients directly)
    qreal radius = std::max(gradient.radius.x, gradient.radius.y);
    QRadialGradient qgradient(gradient.position.x, gradient.position.y, radius);
    
    // Add color stops
    for (const auto& stop : gradient.color_points) {
        QColor stopColor(stop.color.red, stop.color.green, stop.color.blue, stop.color.alpha);
        qgradient.setColorAt(stop.offset, stopColor);
    }
    
    // Draw the gradient
    m_painter->fillRect(clipRect, qgradient);
    
    m_painter->restore();
    
    qDebug() << "container_qt5::draw_radial_gradient - Drawing gradient at" 
             << gradient.position.x << "," << gradient.position.y 
             << " with radius " << radius;
}

// Draw conic gradient
void container_qt5::draw_conic_gradient(litehtml::uint_ptr hdc, const litehtml::background_layer& layer, const litehtml::background_layer::conic_gradient& gradient)
{
    if (!m_painter) return;
    
    m_painter->save();
    
    QRect clipRect(layer.clip_box.x, layer.clip_box.y, layer.clip_box.width, layer.clip_box.height);
    m_painter->setClipRect(clipRect);
    
    // Qt doesn't have direct support for conic gradients in older versions
    // Use QConicalGradient which is similar
    QConicalGradient qgradient(gradient.position.x, gradient.position.y, gradient.angle);
    
    // Add color stops
    for (const auto& stop : gradient.color_points) {
        QColor stopColor(stop.color.red, stop.color.green, stop.color.blue, stop.color.alpha);
        qgradient.setColorAt(stop.offset, stopColor);
    }
    
    // Draw the gradient
    m_painter->fillRect(clipRect, qgradient);
    
    m_painter->restore();
    
    qDebug() << "container_qt5::draw_conic_gradient - Drawing gradient at" 
             << gradient.position.x << "," << gradient.position.y 
             << " with angle " << gradient.angle;
}

// Draw borders
void container_qt5::draw_borders(litehtml::uint_ptr hdc, const litehtml::borders& borders, const litehtml::position& draw_pos, bool root)
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
    
    qDebug() << "container_qt5::draw_borders - Drawing borders for rectangle:" 
             << borderRect.x() << "," << borderRect.y() << "," << borderRect.width() << "," << borderRect.height();
}

// Set caption (title)
void container_qt5::set_caption(const char* caption)
{
    qDebug() << "container_qt5::set_caption - Emitting titleChanged (Diagnostic)";
    emit titleChanged(QString::fromUtf8(caption));
}

// Set base URL
void container_qt5::set_base_url(const char* base_url)
{
    if (base_url && *base_url)
    {
        m_baseUrl = QString::fromUtf8(base_url);
        qDebug() << "container_qt5::set_base_url - Setting base URL to:" << m_baseUrl;
    }
}

// Link handling
void container_qt5::link(const std::shared_ptr<litehtml::document>& doc, const litehtml::element::ptr& el)
{
    qDebug() << "container_qt5::link - NO-OP (Diagnostic)";
}

// Anchor click handling
void container_qt5::on_anchor_click(const char* url, const litehtml::element::ptr& el)
{
    qDebug() << "container_qt5::on_anchor_click - Emitting anchorClicked (Diagnostic)";
    QString linkUrl = QString::fromUtf8(url);
    emit anchorClicked(linkUrl);
}

// Mouse event handling
void container_qt5::on_mouse_event(const litehtml::element::ptr& el, litehtml::mouse_event event)
{
    qDebug() << "container_qt5::on_mouse_event - NO-OP (Diagnostic)";
}

// Set cursor
void container_qt5::set_cursor(const char* cursor)
{
    qDebug() << "container_qt5::set_cursor - Emitting cursorChanged (Diagnostic)";
    QString cursorName = QString::fromUtf8(cursor);
    emit cursorChanged(cursorName);
}

// Transform text based on the specified transformation
void container_qt5::transform_text(litehtml::string& text, litehtml::text_transform tt)
{
    if (text.empty() || tt == litehtml::text_transform_none) {
        return;
    }
    std::string& str = text;
    switch (tt) {
        case litehtml::text_transform_capitalize:
            if (!str.empty()) {
                str[0] = std::toupper(str[0]);
            }
            break;
        case litehtml::text_transform_uppercase:
            for (char &c : str) {
                c = std::toupper(c);
            }
            break;
        case litehtml::text_transform_lowercase:
            for (char &c : str) {
                c = std::tolower(c);
            }
            break;
        default:
            break;
    }
}

// Import CSS from external resources
void container_qt5::import_css(litehtml::string& text, const litehtml::string& url, litehtml::string& baseurl)
{
    // CRITICAL DIAGNOSTIC STEP: Make this a no-op, similar to qlith-mini.
    // LiteHTML will use its internal master CSS if this doesn't provide one.
    // The 'text' parameter should not be modified if we provide no CSS.
    qDebug() << "container_qt5::import_css called for URL:" << QString::fromStdString(url) 
             << "Base URL:" << QString::fromStdString(baseurl) << "- NO-OP (using LiteHTML internal master CSS)";
    // text = "body { font-family: Cousine; font-size: 16px; } "; // Example of providing minimal CSS
}

// Set clipping rectangle
void container_qt5::set_clip(const litehtml::position& pos, const litehtml::border_radiuses& bdr_radius)
{
    if (!m_painter) return;
    
    // Create clipping region
    QRect clipRect(pos.x, pos.y, pos.width, pos.height);
    
    // If all radii are 0, use rectangular clip
    if (bdr_radius.top_left_x == 0 && bdr_radius.top_left_y == 0 &&
        bdr_radius.top_right_x == 0 && bdr_radius.top_right_y == 0 &&
        bdr_radius.bottom_right_x == 0 && bdr_radius.bottom_right_y == 0 &&
        bdr_radius.bottom_left_x == 0 && bdr_radius.bottom_left_y == 0) {
        m_painter->setClipRect(clipRect, Qt::IntersectClip);
    } else {
        // Use rounded rectangle clip
        QPainterPath path;
        
        // Create rounded rect path with different corner radii
        qreal tl_radius_x = bdr_radius.top_left_x;
        qreal tl_radius_y = bdr_radius.top_left_y;
        qreal tr_radius_x = bdr_radius.top_right_x;
        qreal tr_radius_y = bdr_radius.top_right_y;
        qreal br_radius_x = bdr_radius.bottom_right_x;
        qreal br_radius_y = bdr_radius.bottom_right_y;
        qreal bl_radius_x = bdr_radius.bottom_left_x;
        qreal bl_radius_y = bdr_radius.bottom_left_y;
        
        path.moveTo(clipRect.left() + tl_radius_x, clipRect.top());
        
        // Top edge and top-right corner
        path.lineTo(clipRect.right() - tr_radius_x, clipRect.top());
        if (tr_radius_x > 0 || tr_radius_y > 0) {
            path.arcTo(
                clipRect.right() - tr_radius_x * 2, clipRect.top(),
                tr_radius_x * 2, tr_radius_y * 2,
                90, -90
            );
        }
        
        // Right edge and bottom-right corner
        path.lineTo(clipRect.right(), clipRect.bottom() - br_radius_y);
        if (br_radius_x > 0 || br_radius_y > 0) {
            path.arcTo(
                clipRect.right() - br_radius_x * 2, clipRect.bottom() - br_radius_y * 2,
                br_radius_x * 2, br_radius_y * 2,
                0, -90
            );
        }
        
        // Bottom edge and bottom-left corner
        path.lineTo(clipRect.left() + bl_radius_x, clipRect.bottom());
        if (bl_radius_x > 0 || bl_radius_y > 0) {
            path.arcTo(
                clipRect.left(), clipRect.bottom() - bl_radius_y * 2,
                bl_radius_x * 2, bl_radius_y * 2,
                -90, -90
            );
        }
        
        // Left edge and top-left corner
        path.lineTo(clipRect.left(), clipRect.top() + tl_radius_y);
        if (tl_radius_x > 0 || tl_radius_y > 0) {
            path.arcTo(
                clipRect.left(), clipRect.top(),
                tl_radius_x * 2, tl_radius_y * 2,
                180, -90
            );
        }
        
        path.closeSubpath();
        m_painter->setClipPath(path, Qt::IntersectClip);
    }
    
    qDebug() << "container_qt5::set_clip - Setting clip to" 
             << clipRect.x() << "," << clipRect.y() << "," << clipRect.width() << "," << clipRect.height();
}

// Remove clipping
void container_qt5::del_clip()
{
    // QPainter handles clipping stack via save/restore, so just restore to previous state
    if (m_painter) {
        m_painter->restore();
        m_painter->save();
        qDebug() << "container_qt5::del_clip - Removing clip";
    }
}

// Get viewport position and dimensions
void container_qt5::get_viewport(litehtml::position& viewport) const
{
    qDebug() << "container_qt5::get_viewport - Using default/owner (Diagnostic)";
    if (m_owner) {
        QRect rc = m_owner->rect();
        viewport.x = rc.x();
        viewport.y = rc.y();
        viewport.width = rc.width();
        viewport.height = rc.height();
    } else {
        viewport.x = 0; viewport.y = 0; viewport.width = 800; viewport.height = 600; // Fallback
    }
}

// Get media features for responsive design
void container_qt5::get_media_features(litehtml::media_features& media) const
{
    qDebug() << "container_qt5::get_media_features - Using default/owner (Diagnostic)";
    if (m_owner && QGuiApplication::primaryScreen()) {
        QRect rc = m_owner->rect();
        media.type = litehtml::media_type_screen;
        media.width = rc.width();
        media.height = rc.height();
        media.device_width = QGuiApplication::primaryScreen()->size().width();
        media.device_height = QGuiApplication::primaryScreen()->size().height();
        media.color = QGuiApplication::primaryScreen()->depth(); // More accurate color depth
        media.monochrome = (media.color == 1); // Basic monochrome check
        media.color_index = 256;
        media.resolution = QGuiApplication::primaryScreen()->logicalDotsPerInch();
    } else {
        // Minimal defaults
        media.type = litehtml::media_type_screen;
        media.width = 800; media.height = 600;
        media.device_width = 800; media.device_height = 600;
        media.color = 24; // Common default
        media.monochrome = 0;
        media.color_index = 256;
        media.resolution = 96;
    }
}

// Get language information for the document
void container_qt5::get_language(litehtml::string& language, litehtml::string& culture) const
{
    qDebug() << "container_qt5::get_language - Using default en/US (Diagnostic)";
    language = "en";
    culture = "US";
}

// Create custom elements (not used in basic implementation)
litehtml::element::ptr container_qt5::create_element(const char* tag_name,
                                                    const litehtml::string_map& attributes,
                                                    const std::shared_ptr<litehtml::document>& doc)
{
    // Instead of just logging and returning nullptr, we should let litehtml handle element creation
    // by returning nullptr only when we don't need custom elements
    
    // Add detailed debugging to understand which tags are being requested
    qDebug() << "container_qt5::create_element - Called for tag:" << (tag_name ? tag_name : "nullptr");

    // Ensure tag_name is valid
    if (!tag_name) {
        qWarning() << "container_qt5::create_element - Received null tag_name";
        // Let litehtml handle this case by returning nullptr
        return nullptr;
    }

    // Print attributes for debugging
    if (!attributes.empty()) {
        qDebug() << "Attributes:";
        for (const auto& attr : attributes) {
            qDebug() << "  " << attr.first.c_str() << " = " << attr.second.c_str();
        }
    }

    // Unlike in the old code, we don't return nullptr for all elements
    // Instead, we let litehtml create standard elements by returning nullptr
    // This is how it's implemented in the working qlith-mini version
    return nullptr;
}

// Resolve color names to actual colors
litehtml::string container_qt5::resolve_color(const litehtml::string& color) const
{
    // Enhance color resolution with better logging and safety checks
    qDebug() << "container_qt5::resolve_color - Called with color:" << color.c_str();
    
    // Check for invalid input
    if (color.empty()) {
        qWarning() << "container_qt5::resolve_color - Empty color value provided";
        return color;
    }
    
    // Skip CSS properties that aren't colors
    // These are decoration properties like underline, line-through, initial, etc.
    static const std::set<std::string> non_colors = {
        "underline", "line-through", "initial", "inherit", "none", 
        "normal", "bold", "italic", "oblique"
    };
    
    if (non_colors.find(color) != non_colors.end()) {
        qDebug() << "container_qt5::resolve_color - Ignoring non-color CSS value:" << color.c_str();
        return color;
    }
    
    // If it's already in #RRGGBB format, just return it
    if (color.length() > 0 && color[0] == '#') {
        return color;
    }
    
    // Use custom resolver if available (for testing)
    if (m_customColorResolver) {
        std::string resolved = m_customColorResolver(color);
        qDebug() << "container_qt5::resolve_color - Using custom resolver for" << color.c_str() 
                 << "got" << resolved.c_str();
        return resolved;
    }
    
    // Try to resolve using QColor's named colors
    QString colorName = QString::fromUtf8(color.c_str());
    QColor qcolor(colorName);
    
    if (qcolor.isValid()) {
        QString hexColor = qcolor.name();
        qDebug() << "container_qt5::resolve_color - Resolved" << colorName << "to" << hexColor;
        return hexColor.toStdString();
    }
    
    // If we can't resolve, return the original to avoid creating error cycles
    qDebug() << "container_qt5::resolve_color - Could not resolve color:" << color.c_str();
    return color;
}

// Drawing method
void container_qt5::draw(std::shared_ptr<litehtml::document>& doc, QPainter* painter, int x, int y, const litehtml::position* clip)
{
    // Validate parameters
    if (!doc) {
        qCritical() << "container_qt5::draw called with null document";
        return;
    }

    if (!painter)
    {
        qCritical() << "container_qt5::draw called with null painter";
        return;
    }

    // Store painter to use for drawing operations
    m_painter = painter;

    try
    {
        // Validate clip rectangle or use a safe default
        litehtml::position safeClip;
        if (clip)
        {
            safeClip = *clip;

            // Ensure clip dimensions are positive
            if (safeClip.width <= 0 || safeClip.height <= 0)
            {
                qWarning() << "container_qt5::draw called with invalid clip dimensions. Using safe defaults.";
                // Use a safe default clip size
                safeClip.width = painter->device() ? painter->device()->width() : 800;
                safeClip.height = painter->device() ? painter->device()->height() : 600;
            }
        }
        else
        {
            // Create a default clip based on painter's device
            safeClip.x = 0;
            safeClip.y = 0;
            safeClip.width = painter->device() ? painter->device()->width() : 800;
            safeClip.height = painter->device() ? painter->device()->height() : 600;
        }

        // Set up the painter for document rendering
        painter->save();
        painter->translate(x, y);

        // Draw the document with the safe clip
        try
        {
            // This is the critical call that may be causing the segfault
            // We're adding additional error checking around it
            if (doc.get() == nullptr)
            {
                qCritical() << "container_qt5::draw document pointer is null after validation";
                return;
            }

            doc->draw(reinterpret_cast<litehtml::uint_ptr>(painter), 0, 0, &safeClip);
            qDebug() << "container_qt5::draw completed successfully";
        }
        catch (const std::exception &e)
        {
            qCritical() << "Exception in document->draw:" << e.what();
        }
        catch (...)
        {
            qCritical() << "Unknown exception in document->draw";
        }

        // Clean up
        painter->restore();
    }
    catch (const std::exception &e)
    {
        qCritical() << "Exception in container_qt5::draw:" << e.what();
    }
    catch (...)
    {
        qCritical() << "Unknown exception in container_qt5::draw";
    }

    // Clear the painter reference
    m_painter = nullptr;
}

// Mouse button down handler
void container_qt5::on_lbutton_down(std::shared_ptr<litehtml::document>& doc, int x, int y, int client_x)
{
    if (!doc) {
        return;
    }
    
    setLastMouseCoords(x, y, client_x, y);
    
    litehtml::position::vector redraw_boxes;
    if (doc->on_lbutton_down(x, y, x, y, redraw_boxes)) {
        // TODO: Implement redraw of specific boxes if needed
    }
}

// Mouse button up handler
void container_qt5::on_lbutton_up(std::shared_ptr<litehtml::document>& doc, int x, int y, int client_x)
{
    if (!doc) {
        return;
    }
    
    setLastMouseCoords(x, y, client_x, y);
    
    litehtml::position::vector redraw_boxes;
    if (doc->on_lbutton_up(x, y, x, y, redraw_boxes)) {
        // TODO: Implement redraw of specific boxes if needed
    }
}

// Mouse move handler
void container_qt5::on_mouse_over(std::shared_ptr<litehtml::document>& doc, int x, int y, int client_x)
{
    if (!doc) {
        return;
    }
    
    setLastMouseCoords(x, y, client_x, y);
    
    litehtml::position::vector redraw_boxes;
    if (doc->on_mouse_over(x, y, x, y, redraw_boxes)) {
        // TODO: Implement redraw of specific boxes if needed
    }
}

// Handle image loading callback
void container_qt5::onImageLoaded(const QString& url, const QImage& image)
{
    m_images[url] = image;
    
    if (_doc && m_owner) {
        m_owner->update();
    }
}

