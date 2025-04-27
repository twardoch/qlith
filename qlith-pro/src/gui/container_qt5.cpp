#include "container_qt5.h"
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
        _doc->draw(0, getScroll().x(), getScroll().y(), &clipPos);
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
    return static_cast<int>(pt * dpr * (96.0 / 72.0));
}

// Get default font size
int container_qt5::get_default_font_size() const
{
    return m_defaultFontSize;
}

// Get default font name
const char* container_qt5::get_default_font_name() const
{
    static QByteArray fontName;
    fontName = m_defaultFontName.toUtf8();
    return fontName.constData();
}

// Draw a list marker (bullet or number)
void container_qt5::draw_list_marker(litehtml::uint_ptr hdc, const litehtml::list_marker& marker)
{
    if (!m_painter) {
        qWarning() << "draw_list_marker: Painter is null";
        return;
    }
    
    m_painter->save();
    
    // Handle image markers
    if (!marker.image.empty()) {
        QString imageUrl = QString::fromUtf8(marker.image.c_str());
        auto it = m_images.find(imageUrl);
        if (it != m_images.end()) {
            m_painter->drawImage(QRect(marker.pos.x, marker.pos.y, marker.pos.width, marker.pos.height), it.value());
        }
    } else {
        // Handle text markers (bullets, numbers)
        switch (marker.marker_type) {
            case litehtml::list_style_type_circle:
                m_painter->setPen(webColorToQColor(marker.color));
                m_painter->setBrush(Qt::NoBrush);
                m_painter->drawEllipse(marker.pos.x, marker.pos.y, marker.pos.width, marker.pos.height);
                break;
                
            case litehtml::list_style_type_disc:
                m_painter->setPen(Qt::NoPen);
                m_painter->setBrush(webColorToQColor(marker.color));
                m_painter->drawEllipse(marker.pos.x, marker.pos.y, marker.pos.width, marker.pos.height);
                break;
                
            case litehtml::list_style_type_square:
                m_painter->setPen(Qt::NoPen);
                m_painter->setBrush(webColorToQColor(marker.color));
                m_painter->drawRect(marker.pos.x, marker.pos.y, marker.pos.width, marker.pos.height);
                break;
                
            default:
                // For decimal, etc., we draw the text
                if (marker.font != 0) {
                    QString text = QString::number(marker.index) + ".";
                    draw_text(hdc, text.toUtf8().constData(), marker.font, marker.color, marker.pos);
                }
                break;
        }
    }
    
    m_painter->restore();
}

// Load an image
void container_qt5::load_image(const char* src, const char* baseurl, bool redraw_on_ready)
{
    QString sourceUrl = QString::fromUtf8(src);
    QString baseUrl = QString::fromUtf8(baseurl);
    
    // Check if we already have this image
    if (m_images.contains(sourceUrl)) {
        return;
    }
    
    // Resolve URL
    QUrl url;
    if (QUrl(sourceUrl).isRelative()) {
        url = QUrl(baseUrl).resolved(QUrl(sourceUrl));
    } else {
        url = QUrl(sourceUrl);
    }
    
    if (url.isLocalFile()) {
        // Load local image
        QImage image(url.toLocalFile());
        if (!image.isNull()) {
            m_images[sourceUrl] = image;
            if (redraw_on_ready && _doc) {
                m_owner->update();
            }
        }
    } else {
        // For network images, we would need to use QNetworkAccessManager
        // (Not implemented here for simplicity)
        qDebug() << "Network image loading not implemented:" << url.toString();
    }
}

// Get image dimensions
void container_qt5::get_image_size(const char* src, const char* baseurl, litehtml::size& sz)
{
    QString sourceUrl = QString::fromUtf8(src);
    
    auto it = m_images.find(sourceUrl);
    if (it != m_images.end()) {
        sz.width = it.value().width();
        sz.height = it.value().height();
    } else {
        // Default size for missing images
        sz.width = 0;
        sz.height = 0;
    }
}

// Draw an image as a background
void container_qt5::draw_image(litehtml::uint_ptr hdc, const litehtml::background_layer& layer, const std::string& url, const std::string& base_url)
{
    if (!m_painter) {
        qWarning() << "draw_image: Painter is null";
        return;
    }
    
    QString sourceUrl = QString::fromStdString(url);
    auto it = m_images.find(sourceUrl);
    if (it != m_images.end()) {
        QRect rc = positionToRect(layer.clip_box);
        
        switch (layer.repeat) {
            case litehtml::background_repeat_no_repeat:
                // Draw single image at specified position
                m_painter->drawImage(rc, it.value());
                break;
                
            case litehtml::background_repeat_repeat_x:
                // Repeat horizontally
                for (int x = rc.left(); x < rc.right(); x += it.value().width()) {
                    m_painter->drawImage(QRect(x, rc.top(), it.value().width(), rc.height()), it.value());
                }
                break;
                
            case litehtml::background_repeat_repeat_y:
                // Repeat vertically
                for (int y = rc.top(); y < rc.bottom(); y += it.value().height()) {
                    m_painter->drawImage(QRect(rc.left(), y, rc.width(), it.value().height()), it.value());
                }
                break;
                
            case litehtml::background_repeat_repeat:
                // Repeat in both directions
                m_painter->drawTiledPixmap(rc, QPixmap::fromImage(it.value()));
                break;
        }
    }
}

// Draw a solid color background
void container_qt5::draw_solid_fill(litehtml::uint_ptr hdc, const litehtml::background_layer& layer, const litehtml::web_color& color)
{
    if (!m_painter) {
        qWarning() << "draw_solid_fill: Painter is null";
        return;
    }
    
    QRect rc = positionToRect(layer.clip_box);
    m_painter->fillRect(rc, webColorToQColor(color));
}

// Draw a linear gradient
void container_qt5::draw_linear_gradient(litehtml::uint_ptr hdc, const litehtml::background_layer& layer, const litehtml::background_layer::linear_gradient& gradient)
{
    if (!m_painter) {
        qWarning() << "draw_linear_gradient: Painter is null";
        return;
    }
    
    m_painter->save();
    
    QRect rc = positionToRect(layer.clip_box);
    m_painter->setClipRect(rc);
    
    // Create a linear gradient using the start and end points directly from the gradient object
    QLinearGradient qGradient;
    qGradient.setStart(gradient.start.x, gradient.start.y);
    qGradient.setFinalStop(gradient.end.x, gradient.end.y);
    
    // Add color stops
    for (const auto& stop : gradient.color_points) {
        qGradient.setColorAt(stop.offset, webColorToQColor(stop.color));
    }
    
    // Draw the gradient
    m_painter->fillRect(rc, qGradient);
    
    m_painter->restore();
}

// Draw a radial gradient
void container_qt5::draw_radial_gradient(litehtml::uint_ptr hdc, const litehtml::background_layer& layer, const litehtml::background_layer::radial_gradient& gradient)
{
    if (!m_painter) {
        qWarning() << "draw_radial_gradient: Painter is null";
        return;
    }
    
    m_painter->save();
    
    QRect rc = positionToRect(layer.clip_box);
    m_painter->setClipRect(rc);
    
    // Create a radial gradient using the position and radius directly from the gradient object
    QRadialGradient qGradient(gradient.position.x, gradient.position.y, gradient.radius.x);
    
    // Add color stops
    for (const auto& stop : gradient.color_points) {
        qGradient.setColorAt(stop.offset, webColorToQColor(stop.color));
    }
    
    // Draw the gradient
    m_painter->fillRect(rc, qGradient);
    
    m_painter->restore();
}

// Draw a conic gradient
void container_qt5::draw_conic_gradient(litehtml::uint_ptr hdc, const litehtml::background_layer& layer, const litehtml::background_layer::conic_gradient& gradient)
{
    if (!m_painter) {
        qWarning() << "draw_conic_gradient: Painter is null";
        return;
    }
    
    m_painter->save();
    
    QRect rc = positionToRect(layer.clip_box);
    m_painter->setClipRect(rc);
    
    // Qt uses QConicalGradient for conic gradients
    QConicalGradient qGradient(gradient.position.x, gradient.position.y, gradient.angle);
    
    // Add color stops
    for (const auto& stop : gradient.color_points) {
        qGradient.setColorAt(stop.offset, webColorToQColor(stop.color));
    }
    
    // Draw the gradient
    m_painter->fillRect(rc, qGradient);
    
    m_painter->restore();
}

// Draw borders around an element
void container_qt5::draw_borders(litehtml::uint_ptr hdc, const litehtml::borders& borders, const litehtml::position& draw_pos, bool root)
{
    if (!m_painter) {
        qWarning() << "draw_borders: Painter is null";
        return;
    }
    
    m_painter->save();
    
    // Draw the borders
    QRect borderRect = positionToRect(draw_pos);
    
    // Top border
    if (borders.top.width > 0) {
        m_painter->setPen(Qt::NoPen);
        m_painter->setBrush(webColorToQColor(borders.top.color));
        m_painter->drawRect(
            borderRect.left(),
            borderRect.top(),
            borderRect.width(),
            borders.top.width
        );
    }
    
    // Right border
    if (borders.right.width > 0) {
        m_painter->setPen(Qt::NoPen);
        m_painter->setBrush(webColorToQColor(borders.right.color));
        m_painter->drawRect(
            borderRect.right() - borders.right.width,
            borderRect.top() + borders.top.width,
            borders.right.width,
            borderRect.height() - borders.top.width - borders.bottom.width
        );
    }
    
    // Bottom border
    if (borders.bottom.width > 0) {
        m_painter->setPen(Qt::NoPen);
        m_painter->setBrush(webColorToQColor(borders.bottom.color));
        m_painter->drawRect(
            borderRect.left(),
            borderRect.bottom() - borders.bottom.width,
            borderRect.width(),
            borders.bottom.width
        );
    }
    
    // Left border
    if (borders.left.width > 0) {
        m_painter->setPen(Qt::NoPen);
        m_painter->setBrush(webColorToQColor(borders.left.color));
        m_painter->drawRect(
            borderRect.left(),
            borderRect.top() + borders.top.width,
            borders.left.width,
            borderRect.height() - borders.top.width - borders.bottom.width
        );
    }
    
    m_painter->restore();
}

// Set a document caption (title)
void container_qt5::set_caption(const char* caption)
{
    emit titleChanged(QString::fromUtf8(caption));
}

// Set the base URL for resolving relative paths
void container_qt5::set_base_url(const char* base_url)
{
    // Store base URL
    QString baseUrl = QString::fromUtf8(base_url);
    // TODO: Update base URL in actual implementation
}

// Handle links between documents
void container_qt5::link(const std::shared_ptr<litehtml::document>& doc, const litehtml::element::ptr& el)
{
    // No implementation needed for basic functionality
}

// Handle clicks on anchor elements
void container_qt5::on_anchor_click(const char* url, const litehtml::element::ptr& el)
{
    QString linkUrl = QString::fromUtf8(url);
    emit anchorClicked(linkUrl);
}

// Handle mouse events on elements
void container_qt5::on_mouse_event(const litehtml::element::ptr& el, litehtml::mouse_event event)
{
    // Handle mouse hover events
}

// Set the cursor for the current mouse position
void container_qt5::set_cursor(const char* cursor)
{
    QString cursorName = QString::fromUtf8(cursor);
    emit cursorChanged(cursorName);
}

// Transform text based on CSS text-transform property
void container_qt5::transform_text(litehtml::string& text, litehtml::text_transform tt)
{
    if (text.empty()) {
        return;
    }

    QString qText = QString::fromUtf8(text.c_str());

    switch (tt) {
        case litehtml::text_transform_capitalize:
            qText = qText.at(0).toUpper() + qText.mid(1);
            break;
        case litehtml::text_transform_uppercase:
            qText = qText.toUpper();
            break;
        case litehtml::text_transform_lowercase:
            qText = qText.toLower();
            break;
        default:
            break;
    }

    text = qText.toStdString();
}

// Import CSS for the document
void container_qt5::import_css(litehtml::string& text, const litehtml::string& url, litehtml::string& baseurl)
{
    // TODO: Implement CSS loading from external files
    QString cssUrl = QString::fromStdString(url);
    QFile file(cssUrl);
    if (file.open(QFile::ReadOnly)) {
        QByteArray data = file.readAll();
        text = std::string(data.constData(), data.size());
    }
}

// Set clipping region
void container_qt5::set_clip(const litehtml::position& pos, const litehtml::border_radiuses& bdr_radius)
{
    if (!m_painter) {
        qWarning() << "set_clip: Painter is null";
        return;
    }
    
    // Store the clip
    g_clips.push_back(pos);
    
    // Apply the clip to the painter
    m_painter->setClipRect(QRect(pos.x, pos.y, pos.width, pos.height), Qt::IntersectClip);
}

// Remove the current clipping region
void container_qt5::del_clip()
{
    if (!m_painter) {
        qWarning() << "del_clip: Painter is null";
        return;
    }
    
    // Remove the last clip
    if (!g_clips.empty()) {
        g_clips.pop_back();
    }
    
    // Reset the clip path
    m_painter->setClipping(false);
    
    // Reapply remaining clips
    for (const auto& clip : g_clips) {
        m_painter->setClipRect(QRect(clip.x, clip.y, clip.width, clip.height), Qt::IntersectClip);
    }
}

// Get the current viewport dimensions
void container_qt5::get_viewport(litehtml::position& viewport) const
{
    QRect rc = m_owner->rect();
    viewport.x = rc.left();
    viewport.y = rc.top();
    viewport.width = rc.width();
    viewport.height = rc.height();
}

// Get media features for CSS media queries
void container_qt5::get_media_features(litehtml::media_features& media) const
{
    QRect rc = m_owner->rect();
    
    // Set up media features
    media.width = rc.width();
    media.height = rc.height();
    media.device_width = rc.width();
    media.device_height = rc.height();
    media.color = 8; // 8 bits per color component
    media.monochrome = 0;
    media.resolution = 96; // 96 dpi is standard
    
    // Device pixel ratio for high DPI screens
    qreal dpr = 1.0;
    if (QGuiApplication::primaryScreen()) {
        dpr = QGuiApplication::primaryScreen()->devicePixelRatio();
        media.resolution = static_cast<int>(96.0 * dpr);
    }
}

// Get language and culture for the document
void container_qt5::get_language(litehtml::string& language, litehtml::string& culture) const
{
    language = "en";
    culture = "";
}

// Create a custom element
litehtml::element::ptr container_qt5::create_element(const char* tag_name, 
                                                    const litehtml::string_map& attributes,
                                                    const std::shared_ptr<litehtml::document>& doc)
{
    // Let litehtml create default element
    return nullptr;
}

// Handle image loading callback
void container_qt5::onImageLoaded(const QString& url, const QImage& image)
{
    m_images[url] = image;
    
    if (_doc) {
        m_owner->update();
    }
}

//
// litehtmlWidget implementation
//

litehtmlWidget::litehtmlWidget(QWidget* parent)
    : QWidget(parent)
    , m_container(new container_qt5(this))
    , m_needsLayout(false)
    , m_scrollPos(0, 0)
    , m_docSize(0, 0)
    , m_isLoading(false)
{
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);
    
    connect(m_container, &container_qt5::docSizeChanged, this, &litehtmlWidget::onDocSizeChanged);
    connect(m_container, &container_qt5::anchorClicked, this, &litehtmlWidget::linkClicked);
    connect(m_container, &container_qt5::cursorChanged, [this](const QString& cursor) {
        // Set appropriate cursor
        if (cursor == "pointer") {
            setCursor(Qt::PointingHandCursor);
        } else if (cursor == "text") {
            setCursor(Qt::IBeamCursor);
        } else {
            setCursor(Qt::ArrowCursor);
        }
    });
}

litehtmlWidget::~litehtmlWidget()
{
    delete m_container;
}

container_qt5* litehtmlWidget::getContainer() const
{
    return m_container;
}

void litehtmlWidget::loadHtml(const QString& html, const QString& baseUrl)
{
    m_isLoading = true;
    
    // Create document without using the context class
    m_container->_doc = litehtml::document::createFromString(html.toUtf8().constData(), m_container);
    
    // Set base URL
    if (!baseUrl.isEmpty()) {
        m_container->set_base_url(baseUrl.toUtf8().constData());
    }
    
    // Reset scroll position
    setScrollPosition(QPoint(0, 0));
    
    m_needsLayout = true;
    m_isLoading = false;
    update();
}

void litehtmlWidget::setScrollPosition(const QPoint& pos)
{
    m_scrollPos = pos;
    m_container->setScroll(m_scrollPos);
    update();
}

QPoint litehtmlWidget::scrollPosition() const
{
    return m_scrollPos;
}

QSize litehtmlWidget::documentSize() const
{
    return m_docSize;
}

void litehtmlWidget::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);
    
    // Draw background
    painter.fillRect(rect(), Qt::white);
    
    // Draw content
    if (m_container->_doc) {
        m_container->repaint(painter);
    }
}

void litehtmlWidget::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    
    // Trigger re-layout
    if (m_container->_doc) {
        m_needsLayout = true;
        update();
    }
}

void litehtmlWidget::mousePressEvent(QMouseEvent* event)
{
    if (m_container->_doc) {
        // Adjust mouse coordinates for scrolling
        int x = event->x() + m_scrollPos.x();
        int y = event->y() + m_scrollPos.y();
        
        // Track coordinates
        m_container->setLastMouseCoords(x, y, event->x(), event->y());
        
        // Get the element under the cursor
        litehtml::element::ptr el = m_container->elementUnderCursor();
        
        // Handle click
        if (el) {
            litehtml::position::vector redraw_boxes;
            m_container->_doc->on_lbutton_down(x, y, event->x(), event->y(), redraw_boxes);
            update();
        }
    }
}

void litehtmlWidget::mouseReleaseEvent(QMouseEvent* event)
{
    if (m_container->_doc) {
        // Adjust mouse coordinates for scrolling
        int x = event->x() + m_scrollPos.x();
        int y = event->y() + m_scrollPos.y();
        
        // Track coordinates
        m_container->setLastMouseCoords(x, y, event->x(), event->y());
        
        // Get the element under the cursor
        litehtml::element::ptr el = m_container->elementUnderCursor();
        
        // Handle release
        if (el) {
            litehtml::position::vector redraw_boxes;
            m_container->_doc->on_lbutton_up(x, y, event->x(), event->y(), redraw_boxes);
            update();
        }
    }
}

void litehtmlWidget::mouseMoveEvent(QMouseEvent* event)
{
    if (m_container->_doc) {
        // Adjust mouse coordinates for scrolling
        int x = event->x() + m_scrollPos.x();
        int y = event->y() + m_scrollPos.y();
        
        // Track coordinates
        m_container->setLastMouseCoords(x, y, event->x(), event->y());
        
        // Get the element under the cursor
        litehtml::element::ptr el = m_container->elementUnderCursor();
        
        // Handle hover
        if (el) {
            litehtml::position::vector redraw_boxes;
            m_container->_doc->on_mouse_over(x, y, event->x(), event->y(), redraw_boxes);
            update();
        }
    }
}

void litehtmlWidget::wheelEvent(QWheelEvent* event)
{
    // Adjust scroll position based on wheel delta
    QPoint delta = event->angleDelta() / 8;
    if (!delta.isNull()) {
        QPoint newPos = m_scrollPos - QPoint(0, delta.y());
        
        // Clamp to document bounds
        newPos.setX(qBound(0, newPos.x(), qMax(0, m_docSize.width() - width())));
        newPos.setY(qBound(0, newPos.y(), qMax(0, m_docSize.height() - height())));
        
        setScrollPosition(newPos);
    }
    
    event->accept();
}

void litehtmlWidget::onDocSizeChanged(int w, int h)
{
    m_docSize = QSize(w, h);
    
    // Adjust scroll position if out of bounds
    QPoint newPos = m_scrollPos;
    newPos.setX(qBound(0, newPos.x(), qMax(0, m_docSize.width() - width())));
    newPos.setY(qBound(0, newPos.y(), qMax(0, m_docSize.height() - height())));
    
    if (newPos != m_scrollPos) {
        setScrollPosition(newPos);
    }
}
