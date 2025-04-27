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

// Default font settings
static const int DEFAULT_FONT_SIZE = 16;
static const QString DEFAULT_FONT_FAMILY = "Arial";

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
int container_qt5::m_defaultFontSize = DEFAULT_FONT_SIZE;

// Constructor
container_qt5::container_qt5(QWidget* parent)
    : QObject(parent)
    , litehtml::document_container()
    , m_owner(parent)
    , m_painter(nullptr)
    , m_nextFontId(1)
    , m_defaultFontName(DEFAULT_FONT_FAMILY)
{
    Q_ASSERT(m_owner != nullptr);
    
    // Try to get device pixel ratio from screen
    qreal devicePixelRatio = 1.0;
    if (QGuiApplication::primaryScreen()) {
        devicePixelRatio = QGuiApplication::primaryScreen()->devicePixelRatio();
    }
}

// Destructor
container_qt5::~container_qt5()
{
    // Clean up resources
    m_images.clear();
    m_fonts.clear();
}

// Set the document to render
void container_qt5::set_document(std::shared_ptr<litehtml::document> doc)
{
    _doc = doc;
}

// Get the default font size
int container_qt5::getDefaultFontSize() {
    return m_defaultFontSize;
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
    
    // Get widget size
    QRect rc = m_owner->rect();
    
    // Render document at appropriate width
    _doc->render(std::max(rc.width(), 1));
    
    // Set up clipping rectangle
    litehtml::position clipPos;
    clipPos.width = rc.width();
    clipPos.height = rc.height();
    clipPos.x = rc.x();
    clipPos.y = rc.y();
    
    // Draw the document
    try {
        _doc->draw(reinterpret_cast<litehtml::uint_ptr>(&painter), getScroll().x(), getScroll().y(), &clipPos);
    }
    catch (std::exception& e) {
        qWarning() << "Exception in container_qt5::repaint:" << e.what();
    }
    catch (...) {
        qWarning() << "Unknown exception in container_qt5::repaint";
    }
    
    // Clean up
    setPainter(nullptr);

    // Notify if document size has changed
    if (_doc->width() != m_lastDocWidth || _doc->height() != m_lastDocHeight) {
        emit docSizeChanged(_doc->width(), _doc->height());
        emit documentSizeChanged(_doc->width(), _doc->height());
        m_lastDocWidth = _doc->width();
        m_lastDocHeight = _doc->height();
    }
}

// litehtml::document_container implementation

// Font creation
litehtml::uint_ptr container_qt5::create_font(const litehtml::font_description& descr, const litehtml::document* doc, litehtml::font_metrics* fm)
{
    // Create QFont from font description
    QFont font;
    font.setFamily(QString::fromUtf8(descr.family.c_str()));
    font.setPixelSize(descr.size);
    font.setWeight(descr.weight);
    font.setItalic(descr.style == litehtml::font_style_italic);
    font.setUnderline(descr.decoration_line & litehtml::text_decoration_line_underline);
    font.setStrikeOut(descr.decoration_line & litehtml::text_decoration_line_line_through);
    
    // Create metrics object
    font_metrics_t metrics(font);
    
    // Fill in font metrics if requested
    if (fm) {
        QFontMetrics qfm(font);
        fm->height = qfm.height();
        fm->ascent = qfm.ascent();
        fm->descent = qfm.descent();
        fm->x_height = qfm.boundingRect('x').height();
    }
    
    // Store the font with a unique ID
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
        qWarning() << "draw_text: Invalid font ID:" << fontId;
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
    // Standard conversion: 1 point = 1/72 inch
    qreal dpr = 1.0;
    if (QGuiApplication::primaryScreen()) {
        dpr = QGuiApplication::primaryScreen()->devicePixelRatio();
    }
    
    // 96 DPI is the standard screen resolution
    return (pt * 96) / 72;
}

// Get default font size
int container_qt5::get_default_font_size() const
{
    return m_defaultFontSize;
}

// Get default font name
const char* container_qt5::get_default_font_name() const
{
    return m_defaultFontName.toUtf8().constData();
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
    
    switch (marker.type) {
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
            // For other types (like numbers or letters), we draw the text
            if (!marker.text.empty()) {
                m_painter->setFont(m_fonts[1].font);  // Use default font
                m_painter->drawText(rect, Qt::AlignLeft | Qt::AlignVCenter, QString::fromUtf8(marker.text.c_str()));
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
    
    // Calculate the start and end points
    QPointF startPoint(rect.x() + gradient.start_x * rect.width(), rect.y() + gradient.start_y * rect.height());
    QPointF endPoint(rect.x() + gradient.end_x * rect.width(), rect.y() + gradient.end_y * rect.height());
    
    // Create the gradient
    QLinearGradient linearGradient(startPoint, endPoint);
    
    // Add color stops
    for (const auto& stop : gradient.color_stops) {
        linearGradient.setColorAt(stop.first, webColorToQColor(stop.second));
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
    QPointF centerPoint(rect.x() + gradient.center_x * rect.width(), rect.y() + gradient.center_y * rect.height());
    qreal radius = gradient.radius_x * std::min(rect.width(), rect.height());
    
    // Create the gradient
    QRadialGradient radialGradient(centerPoint, radius);
    
    // Add color stops
    for (const auto& stop : gradient.color_stops) {
        radialGradient.setColorAt(stop.first, webColorToQColor(stop.second));
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
    QPointF centerPoint(rect.x() + gradient.center_x * rect.width(), rect.y() + gradient.center_y * rect.height());
    
    // Qt doesn't have a direct conic gradient, so we'll use a QConicalGradient
    QConicalGradient conicGradient(centerPoint, gradient.angle);
    
    // Add color stops
    for (const auto& stop : gradient.color_stops) {
        conicGradient.setColorAt(stop.first, webColorToQColor(stop.second));
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
    // This is already used in the code but not implemented
    // Store base URL if needed
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
    // Try to load the CSS file from resources first
    QString cssUrl = QString::fromStdString(url);
    
    // Convert to resource path if needed
    if (cssUrl.startsWith("://")) {
        QFile file(cssUrl);
        if (file.open(QIODevice::ReadOnly)) {
            QByteArray cssData = file.readAll();
            text = cssData.constData();
            file.close();
            return;
        }
    }
    
    // For external CSS, we would need to implement a network request
    // For simplicity in this example, we're just handling resource files
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
    if (!doc) {
        qWarning() << "container_qt5::draw called with null document";
        return;
    }

    // Set up painter
    setPainter(painter);
    
    try {
        // Draw the document at the specified position with clipping
        doc->draw(reinterpret_cast<litehtml::uint_ptr>(painter), x, y, clip);
    }
    catch (std::exception& e) {
        qWarning() << "Exception in container_qt5::draw:" << e.what();
    }
    catch (...) {
        qWarning() << "Unknown exception in container_qt5::draw";
    }
    
    // Clean up
    setPainter(nullptr);
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
