// this_file: qlith/src/container_qpainter.h
#ifndef CONTAINER_QPAINTER_H
#define CONTAINER_QPAINTER_H

#include <QColor>
#include <QImage>
#include <QMap>
#include <QObject>
#include <QPainter>
#include <QString>
#include <QtGlobal>

#include <litehtml.h>

#include "qlith_global.h"

class ContainerQPainterPrivate;

/**
 * @brief The ContainerQPainter class provides a litehtml document container implementation using QPainter.
 * 
 * This class implements the litehtml::document_container interface to render HTML content
 * using Qt's QPainter.
 */
class QLITH_EXPORT ContainerQPainter : public QObject, public litehtml::document_container
{
    Q_OBJECT

public:
    /**
     * @brief Constructs a ContainerQPainter object.
     * @param parent The parent QObject.
     */
    explicit ContainerQPainter(QObject* parent = nullptr);
    
    /**
     * @brief Destroys the ContainerQPainter object.
     */
    ~ContainerQPainter() override;

    /**
     * @brief Set the base URL for resolving relative paths.
     * @param baseUrl The base URL to use.
     */
    void setBaseUrl(const QString& baseUrl);
    
    /**
     * @brief Get the current base URL.
     * @return The base URL.
     */
    QString baseUrl() const;
    
    /**
     * @brief Set the device pixel ratio for high-DPI screens.
     * @param ratio The device pixel ratio.
     */
    void setDevicePixelRatio(qreal ratio);
    
    /**
     * @brief Get the current device pixel ratio.
     * @return The device pixel ratio.
     */
    qreal devicePixelRatio() const;
    
    /**
     * @brief Set the default font name.
     * @param fontName The default font name.
     */
    void setDefaultFontName(const QString& fontName);
    
    /**
     * @brief Get the default font name.
     * @return The default font name.
     */
    QString defaultFontName() const;
    
    /**
     * @brief Set the default font size.
     * @param size The default font size in pixels.
     */
    void setDefaultFontSize(int size);
    
    /**
     * @brief Get the default font size.
     * @return The default font size in pixels.
     */
    int defaultFontSize() const;
    
    /**
     * @brief Start painting on the given QPainter.
     * @param painter The QPainter to use for rendering.
     * @param rect The rectangle to paint in.
     */
    void beginPaint(QPainter* painter, const QRect& rect);
    
    /**
     * @brief Finish painting.
     */
    void endPaint();

    // litehtml::document_container implementation
    litehtml::uint_ptr create_font(const litehtml::tchar_t* faceName, int size, int weight, litehtml::font_style italic, unsigned int decoration, litehtml::font_metrics* fm) override;
    void delete_font(litehtml::uint_ptr hFont) override;
    int text_width(const litehtml::tchar_t* text, litehtml::uint_ptr hFont) override;
    void draw_text(litehtml::uint_ptr hdc, const litehtml::tchar_t* text, litehtml::uint_ptr hFont, litehtml::web_color color, const litehtml::position& pos) override;
    int pt_to_px(int pt) const override;
    int get_default_font_size() const override;
    const litehtml::tchar_t* get_default_font_name() const override;
    void draw_list_marker(litehtml::uint_ptr hdc, const litehtml::list_marker& marker) override;
    void load_image(const litehtml::tchar_t* src, const litehtml::tchar_t* baseurl, bool redraw_on_ready) override;
    void get_image_size(const litehtml::tchar_t* src, const litehtml::tchar_t* baseurl, litehtml::size& sz) override;
    void draw_background(litehtml::uint_ptr hdc, const litehtml::background_paint& bg) override;
    void draw_borders(litehtml::uint_ptr hdc, const litehtml::borders& borders, const litehtml::position& draw_pos, bool root) override;
    
    // Optional overrides for media queries
    void get_client_rect(litehtml::position& client) const override;
    void get_media_features(litehtml::media_features& media) const override;
    
    // URL resolving and resource loading
    void set_caption(const litehtml::tchar_t* caption) override;
    void set_base_url(const litehtml::tchar_t* base_url) override;
    void link(const std::shared_ptr<litehtml::document>& doc, const litehtml::element::ptr& el) override;
    void on_anchor_click(const litehtml::tchar_t* url, const litehtml::element::ptr& el) override;
    void set_cursor(const litehtml::tchar_t* cursor) override;
    void transform_text(litehtml::tstring& text, litehtml::text_transform tt) override;
    void import_css(litehtml::tstring& text, const litehtml::tstring& url, litehtml::tstring& baseurl) override;
    void set_clip(const litehtml::position& pos, const litehtml::border_radiuses& bdr_radius, bool valid_x, bool valid_y) override;
    void del_clip() override;
    
    // Callbacks for resource loading
    virtual void onImageLoaded(const QString& url, const QImage& image);
    virtual void onCssLoaded(const QString& url, const QString& css);

signals:
    void titleChanged(const QString& title);
    void anchorClicked(const QString& url);
    void cursorChanged(const QString& cursor);

private:
    QScopedPointer<ContainerQPainterPrivate> d_ptr;
    Q_DECLARE_PRIVATE(ContainerQPainter)
    
    QPainter* m_painter;
    QRect m_paintRect;
    qreal m_devicePixelRatio;
    QString m_defaultFontName;
    int m_defaultFontSize;
    QString m_baseUrl;
    
    QMap<QString, QImage> m_images;
    QMap<int, font_metrics_t> m_fonts;
    int m_nextFontId;
};

#endif // CONTAINER_QPAINTER_H 