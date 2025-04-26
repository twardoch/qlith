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

// Default maximum width if none is specified
static const int defaultMaxWidth = 1024;

// Private implementation
container_qpainter_private::container_qpainter_private() 
    : painter(nullptr)
    , document(nullptr)
    , defaultFontSize(16)
    , defaultFontFamily("Arial")
{
}

container_qpainter_private::~container_qpainter_private() 
{
}

void container_qpainter_private::load_image(const QString& url)
{
    // We would implement image loading logic here
    // For now, we're keeping it simple
}

// Main implementation
container_qpainter::container_qpainter()
    : d(new container_qpainter_private())
{
}

container_qpainter::~container_qpainter()
{
}

void container_qpainter::setPainter(QPainter* painter)
{
    d->painter = painter;
}

void container_qpainter::setBaseUrl(const QString& base_url)
{
    d->baseUrl = QUrl(base_url);
}

void container_qpainter::setBaseFontSize(int size)
{
    d->defaultFontSize = size;
}

void container_qpainter::setHtml(const QString &html, const QString &baseUrl)
{
    if (!d) {
        qWarning() << "setHtml called with null private data";
        return;
    }

    if (html.isEmpty()) {
        qWarning() << "setHtml called with empty HTML content";
        return;
    }

    try {
        QByteArray html_data = html.toUtf8();
        int len = html_data.size();

        if (len <= 0) {
            qWarning() << "HTML conversion to UTF-8 resulted in empty data";
            return;
        }

        d->current_document_url = baseUrl;
        d->baseUrl = baseUrl;
        
        // Create document using the createFromString static method
        d->document = litehtml::document::createFromString(
            html_data.constData(),
            this,
            litehtml::master_css
        );

        if (!d->document) {
            qWarning() << "Failed to create document from HTML string";
            return;
        }

        // Render the document with appropriate width
        int renderWidth = d->max_width > 0 ? d->max_width : defaultMaxWidth;
        d->document->render(renderWidth);
        
        // Update container size after rendering
        updateSize();
        
        qDebug() << "HTML document created and rendered successfully";
    } catch (const std::exception& e) {
        qWarning() << "Exception in setHtml:" << e.what();
    } catch (...) {
        qWarning() << "Unknown exception in setHtml";
    }
}

std::shared_ptr<litehtml::document> container_qpainter::document() const
{
    return d->document;
}

void container_qpainter::onResourceLoaded(const QUrl &url, const QByteArray &data)
{
    // This would handle resources that are loaded asynchronously
    // For now, we leave it empty
}

void container_qpainter::draw(QPainter* painter, int x, int y, const litehtml::position* clip)
{
    // Validate painter parameter
    if (!painter) {
        qWarning() << "container_qpainter::draw called with null painter";
        return;
    }
    
    // Validate private data
    if (!d) {
        qWarning() << "container_qpainter::draw called with null private data";
        return;
    }
    
    // Validate document
    if (!d->document) {
        qWarning() << "container_qpainter::draw called with null document";
        return;
    }
    
    // Check if painter is active and has a valid device
    if (!painter->isActive()) {
        qWarning() << "container_qpainter::draw called with inactive painter";
        return;
    }
    
    if (!painter->device()) {
        qWarning() << "container_qpainter::draw called with painter that has no device";
        return;
    }
    
    // Set the painter for use in subsequent operations
    setPainter(painter);
    
    try {
        // If a clip is provided, validate it
        if (clip) {
            // Validate clip rectangle to prevent crashes
            if (clip->width <= 0 || clip->height <= 0) {
                qWarning() << "Invalid clip rectangle in container_qpainter::draw ("
                          << clip->width << "x" << clip->height << ")";
                
                // Create a safe fallback clip based on viewport
                litehtml::position safeClip(0, 0, 
                    qMax(1, painter->viewport().width()),
                    qMax(1, painter->viewport().height()));
                
                qDebug() << "Using fallback clip rectangle:" 
                        << safeClip.x << "," << safeClip.y << " " 
                        << safeClip.width << "x" << safeClip.height;
                
                d->document->draw((litehtml::uint_ptr)painter, x, y, &safeClip);
            } else {
                // Clip is valid, use it
                d->document->draw((litehtml::uint_ptr)painter, x, y, clip);
            }
        } else {
            // No clip provided, create a safe one based on viewport
            litehtml::position pos(0, 0, 
                qMax(1, painter->viewport().width()),
                qMax(1, painter->viewport().height()));
                
            qDebug() << "No clip provided, using viewport:" 
                    << pos.width << "x" << pos.height;
            
            d->document->draw((litehtml::uint_ptr)painter, x, y, &pos);
        }
    } catch (const std::exception& e) {
        qWarning() << "Exception in container_qpainter::draw:" << e.what();
        
        // Try to show error message if painter is still valid
        if (painter->isActive()) {
            painter->save();
            painter->resetTransform();
            painter->setPen(Qt::red);
            painter->drawText(painter->viewport().adjusted(10, 10, -10, -10), 
                            Qt::AlignCenter | Qt::TextWordWrap,
                            QString("Rendering error: %1").arg(e.what()));
            painter->restore();
        }
    } catch (...) {
        qWarning() << "Unknown exception in container_qpainter::draw";
        
        // Try to show generic error message
        if (painter->isActive()) {
            painter->save();
            painter->resetTransform();
            painter->setPen(Qt::red);
            painter->drawText(painter->viewport().adjusted(10, 10, -10, -10), 
                            Qt::AlignCenter | Qt::TextWordWrap,
                            "Unknown rendering error occurred");
            painter->restore();
        }
    }
    
    // Ensure we unset the painter to prevent accidental usage later
    setPainter(nullptr);
}

// Basic implementation of the required litehtml::document_container methods

litehtml::uint_ptr container_qpainter::create_font(const litehtml::font_description& descr, const litehtml::document* doc, litehtml::font_metrics* fm)
{
    QFont* font = new QFont(QString::fromStdString(descr.family));
    font->setPixelSize(descr.size);
    font->setWeight(descr.weight);
    font->setItalic(descr.style == litehtml::font_style_italic);
    font->setUnderline(descr.decoration_line & litehtml::text_decoration_line_underline);
    font->setStrikeOut(descr.decoration_line & litehtml::text_decoration_line_line_through);
    
    if (fm) {
        QFontMetrics metrics(*font);
        fm->height = metrics.height();
        fm->ascent = metrics.ascent();
        fm->descent = metrics.descent();
        fm->x_height = metrics.xHeight();
        fm->draw_spaces = true;
    }
    
    return (litehtml::uint_ptr)font;
}

void container_qpainter::delete_font(litehtml::uint_ptr hFont)
{
    if (hFont) {
        QFont* font = (QFont*)hFont;
        delete font;
    }
}

int container_qpainter::text_width(const char* text, litehtml::uint_ptr hFont)
{
    if (!text || !hFont) {
        return 0;
    }
    
    QFont* font = (QFont*)hFont;
    QFontMetrics fm(*font);
    QString qtext = QString::fromUtf8(text);
    
    return fm.horizontalAdvance(qtext);
}

void container_qpainter::draw_text(litehtml::uint_ptr hdc, const char* text, litehtml::uint_ptr hFont, litehtml::web_color color, const litehtml::position& pos)
{
    if (!hdc || !text || !hFont) {
        return;
    }
    
    QPainter* painter = (QPainter*)hdc;
    QFont* font = (QFont*)hFont;
    
    QColor qcolor(color.red, color.green, color.blue, color.alpha);
    
    painter->save();
    painter->setFont(*font);
    painter->setPen(qcolor);
    painter->drawText(pos.x, pos.y + pos.height, QString::fromUtf8(text));
    painter->restore();
}

int container_qpainter::pt_to_px(int pt) const
{
    // Convert points to pixels
    QScreen* screen = QApplication::primaryScreen();
    if (screen) {
        return (int)(pt * screen->logicalDotsPerInch() / 72.0);
    }
    return pt;
}

int container_qpainter::get_default_font_size() const
{
    return d->defaultFontSize;
}

const char* container_qpainter::get_default_font_name() const
{
    // Note: This is not the most efficient way, since we're creating a temporary string
    // In a production implementation, we might want to store this permanently
    static QByteArray defaultFontNameUtf8 = d->defaultFontFamily.toUtf8();
    return defaultFontNameUtf8.constData();
}

void container_qpainter::draw_list_marker(litehtml::uint_ptr hdc, const litehtml::list_marker& marker)
{
    // Basic implementation for list markers
    if (!hdc) return;
    
    QPainter* painter = (QPainter*)hdc;
    painter->save();
    
    QColor color(marker.color.red, marker.color.green, marker.color.blue, marker.color.alpha);
    painter->setPen(color);
    painter->setBrush(color);
    
    if (marker.marker_type == litehtml::list_style_type_circle) {
        painter->drawEllipse(marker.pos.x, marker.pos.y, marker.pos.width, marker.pos.height);
    } else if (marker.marker_type == litehtml::list_style_type_disc) {
        painter->drawEllipse(marker.pos.x, marker.pos.y, marker.pos.width, marker.pos.height);
    } else if (marker.marker_type == litehtml::list_style_type_square) {
        painter->drawRect(marker.pos.x, marker.pos.y, marker.pos.width, marker.pos.height);
    }
    
    painter->restore();
}

void container_qpainter::load_image(const char* src, const char* baseurl, bool redraw_on_ready)
{
    if (!src) return;
    
    // Convert to QString
    QString qsrc = QString::fromUtf8(src);
    
    // For now, we don't actually load the image
    // In a real implementation, we'd handle this better
}

void container_qpainter::get_image_size(const char* src, const char* baseurl, litehtml::size& sz)
{
    if (!src) return;
    
    // Convert to QString
    QString qsrc = QString::fromUtf8(src);
    
    // For simplicity, return zero size for all images
    sz.width = 0;
    sz.height = 0;
}

void container_qpainter::draw_image(litehtml::uint_ptr hdc, const litehtml::background_layer& layer, const std::string& url, const std::string& base_url)
{
    // Simplified implementation
    if (!hdc) return;
    
    // For a real implementation, we would load the image and draw it
}

void container_qpainter::draw_solid_fill(litehtml::uint_ptr hdc, const litehtml::background_layer& layer, const litehtml::web_color& color)
{
    if (!hdc) return;
    
    QPainter* painter = (QPainter*)hdc;
    painter->save();
    
    QColor qcolor(color.red, color.green, color.blue, color.alpha);
    painter->fillRect(layer.clip_box.x, layer.clip_box.y, layer.clip_box.width, layer.clip_box.height, qcolor);
    
    painter->restore();
}

void container_qpainter::draw_linear_gradient(litehtml::uint_ptr hdc, const litehtml::background_layer& layer, const litehtml::background_layer::linear_gradient& gradient)
{
    // Simplified implementation
    if (!hdc) return;
    
    // In a real implementation, we would create a QLinearGradient and use it
}

void container_qpainter::draw_radial_gradient(litehtml::uint_ptr hdc, const litehtml::background_layer& layer, const litehtml::background_layer::radial_gradient& gradient)
{
    // Simplified implementation
    if (!hdc) return;
    
    // In a real implementation, we would create a QRadialGradient and use it
}

void container_qpainter::draw_conic_gradient(litehtml::uint_ptr hdc, const litehtml::background_layer& layer, const litehtml::background_layer::conic_gradient& gradient)
{
    // Simplified implementation
    if (!hdc) return;
    
    // In a real implementation, we would create a QConicalGradient and use it
}

void container_qpainter::draw_borders(litehtml::uint_ptr hdc, const litehtml::borders& borders, const litehtml::position& draw_pos, bool root)
{
    if (!hdc) return;
    
    QPainter* painter = (QPainter*)hdc;
    painter->save();
    
    // Draw top border
    if (borders.top.width > 0) {
        QColor color(borders.top.color.red, borders.top.color.green, borders.top.color.blue, borders.top.color.alpha);
        painter->setPen(QPen(color, borders.top.width));
        painter->drawLine(draw_pos.x, draw_pos.y, draw_pos.x + draw_pos.width, draw_pos.y);
    }
    
    // Draw right border
    if (borders.right.width > 0) {
        QColor color(borders.right.color.red, borders.right.color.green, borders.right.color.blue, borders.right.color.alpha);
        painter->setPen(QPen(color, borders.right.width));
        painter->drawLine(draw_pos.x + draw_pos.width, draw_pos.y, draw_pos.x + draw_pos.width, draw_pos.y + draw_pos.height);
    }
    
    // Draw bottom border
    if (borders.bottom.width > 0) {
        QColor color(borders.bottom.color.red, borders.bottom.color.green, borders.bottom.color.blue, borders.bottom.color.alpha);
        painter->setPen(QPen(color, borders.bottom.width));
        painter->drawLine(draw_pos.x, draw_pos.y + draw_pos.height, draw_pos.x + draw_pos.width, draw_pos.y + draw_pos.height);
    }
    
    // Draw left border
    if (borders.left.width > 0) {
        QColor color(borders.left.color.red, borders.left.color.green, borders.left.color.blue, borders.left.color.alpha);
        painter->setPen(QPen(color, borders.left.width));
        painter->drawLine(draw_pos.x, draw_pos.y, draw_pos.x, draw_pos.y + draw_pos.height);
    }
    
    painter->restore();
}

void container_qpainter::set_caption(const char* caption)
{
    // For now, we don't need to implement this
}

void container_qpainter::set_base_url(const char* base_url)
{
    if (base_url) {
        d->baseUrl = QUrl(QString::fromUtf8(base_url));
    }
}

void container_qpainter::link(const std::shared_ptr<litehtml::document>& doc, const litehtml::element::ptr& el)
{
    // For now, we don't need to implement this
}

void container_qpainter::on_anchor_click(const char* url, const litehtml::element::ptr& el)
{
    // For now, we don't need to implement this
}

bool container_qpainter::on_element_click(const litehtml::element::ptr& el)
{
    // For now, we don't need to implement this
    return false;
}

void container_qpainter::on_mouse_event(const litehtml::element::ptr& el, litehtml::mouse_event event)
{
    // For now, we don't need to implement this
}

void container_qpainter::set_cursor(const char* cursor)
{
    // For now, we don't need to implement this
}

void container_qpainter::transform_text(litehtml::string& text, litehtml::text_transform tt)
{
    if (text.empty() || tt == litehtml::text_transform_none) {
        return;
    }
    
    QString qtext = QString::fromUtf8(text.c_str());
    
    switch (tt) {
        case litehtml::text_transform_capitalize:
            qtext = qtext.at(0).toUpper() + qtext.mid(1);
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
    
    text = qtext.toUtf8().constData();
}

void container_qpainter::import_css(litehtml::string& text, const litehtml::string& url, litehtml::string& baseurl)
{
    // For now, we don't need to implement this
    // In a real implementation, we would load and process CSS
}

void container_qpainter::set_clip(const litehtml::position& pos, const litehtml::border_radiuses& bdr_radius)
{
    if (!d->painter) return;
    
    d->painter->setClipRect(pos.x, pos.y, pos.width, pos.height);
}

void container_qpainter::del_clip()
{
    if (!d->painter) return;
    
    d->painter->setClipping(false);
}

void container_qpainter::get_viewport(litehtml::position& viewport) const
{
    // Initialize with safe default values
    viewport.x = 0;
    viewport.y = 0;
    viewport.width = 800;  // Default reasonable width
    viewport.height = 600; // Default reasonable height

    // Check if both the private data and painter exist
    if (!d) {
        qWarning() << "get_viewport called with null private data";
        return;
    }
    
    if (!d->painter) {
        qWarning() << "get_viewport called with null painter";
        return;
    }
    
    try {
        // Check if painter has a valid device
        if (!d->painter->device()) {
            qWarning() << "Painter has no valid device in get_viewport";
            return;
        }
        
        QRect view = d->painter->viewport();
        
        // Only override defaults if valid
        if (view.width() > 0 && view.height() > 0) {
            viewport.width = view.width();
            viewport.height = view.height();
        } else {
            // If viewport dimensions are invalid, try device dimensions
            int deviceWidth = d->painter->device()->width();
            int deviceHeight = d->painter->device()->height();
            
            if (deviceWidth > 0 && deviceHeight > 0) {
                viewport.width = deviceWidth;
                viewport.height = deviceHeight;
                qDebug() << "Using device dimensions:" << deviceWidth << "x" << deviceHeight;
            } else {
                qWarning() << "Invalid viewport and device dimensions, using defaults";
            }
        }
    } catch (const std::exception& e) {
        qWarning() << "Exception in get_viewport:" << e.what();
    } catch (...) {
        qWarning() << "Unknown exception in get_viewport";
    }
}

litehtml::element::ptr container_qpainter::create_element(const char* tag_name, const litehtml::string_map& attributes, const std::shared_ptr<litehtml::document>& doc)
{
    // Let litehtml handle element creation
    return nullptr;
}

void container_qpainter::get_media_features(litehtml::media_features& media) const
{
    media.type = litehtml::media_type_screen;
    
    if (d->painter && d->painter->device()) {
        media.width = d->painter->device()->width();
        media.height = d->painter->device()->height();
        
        QScreen* screen = QApplication::primaryScreen();
        if (screen) {
            media.device_width = screen->size().width();
            media.device_height = screen->size().height();
            media.resolution = screen->logicalDotsPerInch() / 96.0f;
        }
    }
    
    media.color = 8;
    media.monochrome = 0;
    media.color_index = 256;
}

void container_qpainter::get_language(litehtml::string& language, litehtml::string& culture) const
{
    language = "en";
    culture = "US";
}

void container_qpainter::updateSize()
{
    // Verify we have valid private data
    if (!d) {
        qWarning() << "updateSize called with null private data";
        return;
    }

    if (!d->document) {
        qWarning() << "updateSize called with null document";
        return;
    }
    
    try {
        // Update dimensions after rendering
        int newWidth = d->document->width();
        int newHeight = d->document->height();
        
        // Validate dimensions - protect against unreasonable values
        if (newWidth <= 0 || newWidth > 16384) {
            qWarning() << "Invalid document width:" << newWidth << "- using fallback";
            newWidth = qBound(1, newWidth, 16384);
        }
        
        if (newHeight <= 0 || newHeight > 16384) {
            qWarning() << "Invalid document height:" << newHeight << "- using fallback";
            newHeight = qBound(1, newHeight, 16384);
        }
        
        d->width = newWidth;
        d->height = newHeight;
        
        // Debug logging if size changed significantly
        static const double SIGNIFICANT_CHANGE_FACTOR = 2.0;
        if (d->width > 0 && d->height > 0 && 
            (newWidth > d->width * SIGNIFICANT_CHANGE_FACTOR || 
             newHeight > d->height * SIGNIFICANT_CHANGE_FACTOR)) {
            qDebug() << "Document size changed significantly:" 
                    << "previous =" << d->width << "x" << d->height
                    << ", new =" << newWidth << "x" << newHeight;
        }
    } catch (const std::exception& e) {
        qWarning() << "Exception in updateSize:" << e.what();
    } catch (...) {
        qWarning() << "Unknown exception in updateSize";
    }
} 