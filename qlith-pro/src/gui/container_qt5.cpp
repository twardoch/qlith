#include "qlith/container_qt5.h"
#include <QDebug>
#include <QFont>
#include <QFontMetrics>
#include <QPainter>
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
    int fontId = static_cast<int>(hFont);
    m_fonts.remove(fontId);
}

// Measure text width
int container_qt5::text_width(const char* text, litehtml::uint_ptr hFont)
{
    int fontId = static_cast<int>(hFont);
    if (!m_fonts.contains(fontId)) {
        qWarning() << "text_width: Invalid font ID:" << fontId;
        return 0;
    }
    
    const font_metrics_t& metrics = m_fonts[fontId];
    return metrics.metrics.horizontalAdvance(QString::fromUtf8(text));
}

// Draw text
void container_qt5::draw_text(litehtml::uint_ptr hdc, const char* text, litehtml::uint_ptr hFont, litehtml::web_color color, const litehtml::position& pos)
{
    if (!m_painter) {
        qWarning() << "draw_text: Painter is null";
        return;
    }
    
    int fontId = static_cast<int>(hFont);
    if (!m_fonts.contains(fontId)) {
        qWarning() << "draw_text: Invalid font ID:" << fontId << " - Using fallback font";

        // Create a fallback font
        QFont fallbackFont;
        fallbackFont.setFamily(m_defaultFontName);
        fallbackFont.setPixelSize(m_defaultFontSize);

        // Draw with fallback font
        m_painter->save();
        m_painter->setPen(webColorToQColor(color));
        m_painter->setFont(fallbackFont);
        QFontMetrics metrics(fallbackFont);
        m_painter->drawText(pos.x, pos.y + metrics.ascent(), QString::fromUtf8(text));
        m_painter->restore();
        return;
    }
    
    // Set up painter for text rendering
    m_painter->save();
    m_painter->setPen(webColorToQColor(color));
    m_painter->setFont(m_fonts[fontId].font);
    
    // Draw the text
    const QFontMetrics& metrics = m_fonts[fontId].metrics;
    m_painter->drawText(pos.x, pos.y + metrics.ascent(), QString::fromUtf8(text));
    
    m_painter->restore();
}

// Convert points to pixels
int container_qt5::pt_to_px(int pt) const
{
    qreal dpr = 1.0;
    if (QGuiApplication::primaryScreen()) {
        dpr = QGuiApplication::primaryScreen()->devicePixelRatio();
    }
    return (pt * 96) / 72; // Standard conversion, adjust dpr if needed for your logic
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
    if (!m_painter) {
        return;
    }
    
    m_painter->save();
    
    QRect rect(marker.pos.x, marker.pos.y, marker.pos.width, marker.pos.height);
    
    m_painter->setPen(webColorToQColor(marker.color));
    
    switch (marker.marker_type) {
        case litehtml::list_style_type_circle:
            m_painter->setBrush(Qt::NoBrush);
            m_painter->drawEllipse(rect);
            break;
        case litehtml::list_style_type_disc:
            m_painter->setBrush(webColorToQColor(marker.color));
            m_painter->drawEllipse(rect);
            break;
        case litehtml::list_style_type_square:
            m_painter->setBrush(webColorToQColor(marker.color));
            m_painter->drawRect(rect);
            break;
        default:
            // For other types (like numbers or letters), draw numbered markers
            if (marker.index >= 0) {
                m_painter->setFont(m_fonts[static_cast<int>(marker.font)].font);
                // Just draw the index as a string - in a more complete implementation
                // we would format this according to the list style
                QString indexText = QString::number(marker.index) + ".";
                m_painter->drawText(rect, Qt::AlignLeft | Qt::AlignVCenter, indexText);
            }
            break;
    }
    
    m_painter->restore();
}

// Draw an image
void container_qt5::draw_image(litehtml::uint_ptr hdc, const litehtml::background_layer& layer, const std::string& url, const std::string& base_url)
{
    if (!m_painter) {
        return;
    }
    
    QString imageUrl = QString::fromStdString(url);
    
    if (m_images.contains(imageUrl)) {
        QRect rect(layer.origin_box.x, layer.origin_box.y, layer.origin_box.width, layer.origin_box.height);
        m_painter->drawImage(rect, m_images[imageUrl]);
    }
}

// Get image dimensions
void container_qt5::get_image_size(const char* src, const char* baseurl, litehtml::size& sz)
{
    QString imageUrl = QString::fromUtf8(src);
    
    if (m_images.contains(imageUrl)) {
        QImage& img = m_images[imageUrl];
        sz.width = img.width();
        sz.height = img.height();
    } else {
        // Load the image if it's not loaded yet
        load_image(src, baseurl, false);
        
        // Set default size
        sz.width = 0;
        sz.height = 0;
    }
}

// Load an image from a URL
void container_qt5::load_image(const char* src, const char* baseurl, bool redraw_on_ready)
{
    if (!src || !*src) {
        return;
    }
    
    QString imageUrl = QString::fromUtf8(src);
    
    // Check if the image is already loaded
    if (m_images.contains(imageUrl)) {
        return;
    }
    
    // Check if it's a resource path
    if (imageUrl.startsWith("://")) {
        QImage img(imageUrl);
        if (!img.isNull()) {
            m_images[imageUrl] = img;
            
            if (redraw_on_ready && _doc && m_owner) {
                m_owner->update();
            }
        }
    }
    
    // For external images, we would need to implement a network request
    // For simplicity in this example, we're just handling resource files
}

// Draw solid color background
void container_qt5::draw_solid_fill(litehtml::uint_ptr hdc, const litehtml::background_layer& layer, const litehtml::web_color& color)
{
    if (!m_painter) {
        return;
    }
    
    QColor qcolor = webColorToQColor(color);
    QRect rect(layer.origin_box.x, layer.origin_box.y, layer.origin_box.width, layer.origin_box.height);
    
    m_painter->save();
    m_painter->setPen(Qt::NoPen);
    m_painter->setBrush(qcolor);
    m_painter->drawRect(rect);
    m_painter->restore();
}

// Draw linear gradient
void container_qt5::draw_linear_gradient(litehtml::uint_ptr hdc, const litehtml::background_layer& layer, const litehtml::background_layer::linear_gradient& gradient)
{
    if (!m_painter) {
        return;
    }
    
    QRect rect(layer.origin_box.x, layer.origin_box.y, layer.origin_box.width, layer.origin_box.height);
    
    // Calculate the start and end points using the gradient points
    QPointF startPoint(rect.x() + gradient.start.x * rect.width(), rect.y() + gradient.start.y * rect.height());
    QPointF endPoint(rect.x() + gradient.end.x * rect.width(), rect.y() + gradient.end.y * rect.height());
    
    // Create the gradient
    QLinearGradient linearGradient(startPoint, endPoint);
    
    // Add color stops
    for (const auto& stop : gradient.color_points) {
        linearGradient.setColorAt(stop.offset, webColorToQColor(stop.color));
    }
    
    // Draw the gradient
    m_painter->save();
    m_painter->setPen(Qt::NoPen);
    m_painter->setBrush(linearGradient);
    m_painter->drawRect(rect);
    m_painter->restore();
}

// Draw radial gradient
void container_qt5::draw_radial_gradient(litehtml::uint_ptr hdc, const litehtml::background_layer& layer, const litehtml::background_layer::radial_gradient& gradient)
{
    if (!m_painter) {
        return;
    }
    
    QRect rect(layer.origin_box.x, layer.origin_box.y, layer.origin_box.width, layer.origin_box.height);
    
    // Calculate the center point and radius
    QPointF centerPoint(rect.x() + gradient.position.x * rect.width(), rect.y() + gradient.position.y * rect.height());
    qreal radiusX = gradient.radius.x * rect.width();
    qreal radiusY = gradient.radius.y * rect.height();
    qreal radius = std::max(radiusX, radiusY);
    
    // Create the gradient
    QRadialGradient radialGradient(centerPoint, radius);
    
    // Add color stops
    for (const auto& stop : gradient.color_points) {
        radialGradient.setColorAt(stop.offset, webColorToQColor(stop.color));
    }
    
    // Draw the gradient
    m_painter->save();
    m_painter->setPen(Qt::NoPen);
    m_painter->setBrush(radialGradient);
    m_painter->drawRect(rect);
    m_painter->restore();
}

// Draw conic gradient
void container_qt5::draw_conic_gradient(litehtml::uint_ptr hdc, const litehtml::background_layer& layer, const litehtml::background_layer::conic_gradient& gradient)
{
    if (!m_painter) {
        return;
    }
    
    QRect rect(layer.origin_box.x, layer.origin_box.y, layer.origin_box.width, layer.origin_box.height);
    
    // Calculate the center point
    QPointF centerPoint(rect.x() + gradient.position.x * rect.width(), rect.y() + gradient.position.y * rect.height());
    
    // Qt doesn't have a direct conic gradient, so we'll use a QConicalGradient
    QConicalGradient conicGradient(centerPoint, gradient.angle);
    
    // Add color stops
    for (const auto& stop : gradient.color_points) {
        conicGradient.setColorAt(stop.offset, webColorToQColor(stop.color));
    }
    
    // Draw the gradient
    m_painter->save();
    m_painter->setPen(Qt::NoPen);
    m_painter->setBrush(conicGradient);
    m_painter->drawRect(rect);
    m_painter->restore();
}

// Draw borders
void container_qt5::draw_borders(litehtml::uint_ptr hdc, const litehtml::borders& borders, const litehtml::position& draw_pos, bool root)
{
    if (!m_painter) {
        return;
    }
    
    m_painter->save();
    
    // Draw top border
    if (borders.top.width > 0) {
        m_painter->setPen(Qt::NoPen);
        m_painter->setBrush(webColorToQColor(borders.top.color));
        m_painter->drawRect(
            draw_pos.x,
            draw_pos.y,
            draw_pos.width,
            borders.top.width
        );
    }
    
    // Draw right border
    if (borders.right.width > 0) {
        m_painter->setPen(Qt::NoPen);
        m_painter->setBrush(webColorToQColor(borders.right.color));
        m_painter->drawRect(
            draw_pos.x + draw_pos.width - borders.right.width,
            draw_pos.y,
            borders.right.width,
            draw_pos.height
        );
    }
    
    // Draw bottom border
    if (borders.bottom.width > 0) {
        m_painter->setPen(Qt::NoPen);
        m_painter->setBrush(webColorToQColor(borders.bottom.color));
        m_painter->drawRect(
            draw_pos.x,
            draw_pos.y + draw_pos.height - borders.bottom.width,
            draw_pos.width,
            borders.bottom.width
        );
    }
    
    // Draw left border
    if (borders.left.width > 0) {
        m_painter->setPen(Qt::NoPen);
        m_painter->setBrush(webColorToQColor(borders.left.color));
        m_painter->drawRect(
            draw_pos.x,
            draw_pos.y,
            borders.left.width,
            draw_pos.height
        );
    }
    
    m_painter->restore();
}

// Set caption (title)
void container_qt5::set_caption(const char* caption)
{
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
    // No implementation needed
}

// Anchor click handling
void container_qt5::on_anchor_click(const char* url, const litehtml::element::ptr& el)
{
    QString linkUrl = QString::fromUtf8(url);
    emit anchorClicked(linkUrl);
}

// Mouse event handling
void container_qt5::on_mouse_event(const litehtml::element::ptr& el, litehtml::mouse_event event)
{
    // No implementation needed
}

// Set cursor
void container_qt5::set_cursor(const char* cursor)
{
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
            for (auto& c : str) {
                c = std::toupper(c);
            }
            break;
        case litehtml::text_transform_lowercase:
            for (auto& c : str) {
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
    // Log the import request
    qDebug() << "container_qt5::import_css called for URL:" << QString::fromStdString(url) << "Base URL:" << QString::fromStdString(baseurl);
    
    QString cssUrl = QString::fromStdString(url);
    
    // Check if we've already loaded this CSS
    if (m_loaded_css.contains(cssUrl)) {
        text = m_loaded_css[cssUrl].constData();
        qDebug() << "container_qt5::import_css - Using cached CSS for" << cssUrl;
        return;
    }
    
    // Basic implementation - handle file:// URLs
    if (cssUrl.startsWith("file://", Qt::CaseInsensitive)) {
        QString localPath = QUrl(cssUrl).toLocalFile();
        QFile file(localPath);
        
        if (file.open(QIODevice::ReadOnly)) {
            QByteArray cssData = file.readAll();
            text = cssData.constData();
            m_loaded_css[cssUrl] = cssData;
            qDebug() << "container_qt5::import_css - Loaded CSS from file:" << localPath;
            file.close();
        } else {
            qWarning() << "container_qt5::import_css - Failed to open CSS file:" << localPath;
        }
        return;
    }
    
    // For other URLs, we provide a minimal default CSS
    // In a production app, we would implement network requests here
    static const char* defaultCSS = 
        "html, body { margin: 0; padding: 0; font-family: Arial, sans-serif; }\n"
        "h1 { font-size: 2em; margin: 0.67em 0; }\n"
        "h2 { font-size: 1.5em; margin: 0.75em 0; }\n"
        "h3 { font-size: 1.17em; margin: 0.83em 0; }\n"
        "p { margin: 1em 0; }\n";
    
    text = defaultCSS;
    m_loaded_css[cssUrl] = QByteArray(defaultCSS);
    qDebug() << "container_qt5::import_css - Using default CSS for" << cssUrl;
}

// Set clipping rectangle
void container_qt5::set_clip(const litehtml::position& pos, const litehtml::border_radiuses& bdr_radius)
{
    if (m_painter) {
        m_painter->save();
        m_painter->setClipRect(QRect(pos.x, pos.y, pos.width, pos.height));
    }
}

// Remove clipping
void container_qt5::del_clip()
{
    if (m_painter) {
        m_painter->restore();
    }
}

// Get viewport position and dimensions
void container_qt5::get_viewport(litehtml::position& viewport) const
{
    if (m_owner) {
        QRect rc = m_owner->rect();
        viewport.x = rc.x();
        viewport.y = rc.y();
        viewport.width = rc.width();
        viewport.height = rc.height();
    } else {
        viewport.x = 0;
        viewport.y = 0;
        viewport.width = 1024;
        viewport.height = 768;
    }
}

// Get media features for responsive design
void container_qt5::get_media_features(litehtml::media_features& media) const
{
    if (m_owner) {
        QRect rc = m_owner->rect();
        
        media.type = litehtml::media_type_screen;
        media.width = rc.width();
        media.height = rc.height();
        media.device_width = QGuiApplication::primaryScreen()->size().width();
        media.device_height = QGuiApplication::primaryScreen()->size().height();
        media.color = 8;
        media.monochrome = 0;
        media.color_index = 256;
        media.resolution = 96;
    }
}

// Get language information for the document
void container_qt5::get_language(litehtml::string& language, litehtml::string& culture) const
{
    language = "en";
    culture = "US";
}

// Create custom elements (not used in basic implementation)
litehtml::element::ptr container_qt5::create_element(const char* tag_name, 
                                                    const litehtml::string_map& attributes,
                                                    const std::shared_ptr<litehtml::document>& doc)
{
    return nullptr;
}

// Resolve color names to actual colors
litehtml::string container_qt5::resolve_color(const litehtml::string& color) const
{
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
