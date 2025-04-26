// this_file: qlith/src/container_qpainter.h
#ifndef CONTAINER_QPAINTER_H
#define CONTAINER_QPAINTER_H

#include <QColor>
#include <QFont>
#include <QFontMetrics>
#include <QImage>
#include <QMap>
#include <QObject>
#include <QPainter>
#include <QRect>
#include <QUrl>

#include <litehtml/document.h>
#include <litehtml/document_container.h>

class ContainerQPainter : public QObject, public litehtml::document_container
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
    litehtml::uint_ptr create_font(const litehtml::font_description& descr, const litehtml::document* doc, litehtml::font_metrics* fm) override;
    void delete_font(litehtml::uint_ptr hFont) override;
    int text_width(const char* text, litehtml::uint_ptr hFont) override;
    void draw_text(litehtml::uint_ptr hdc, const char* text, litehtml::uint_ptr hFont, litehtml::web_color color, const litehtml::position& pos) override;
    int pt_to_px(int pt) const override;
    int get_default_font_size() const override;
    const char* get_default_font_name() const override;
    void draw_list_marker(litehtml::uint_ptr hdc, const litehtml::list_marker& marker) override;
    void load_image(const char* src, const char* baseurl, bool redraw_on_ready) override;
    void get_image_size(const char* src, const char* baseurl, litehtml::size& sz) override;
    void draw_image(litehtml::uint_ptr hdc, const litehtml::background_layer& layer, const std::string& url, const std::string& base_url) override;
    void draw_solid_fill(litehtml::uint_ptr hdc, const litehtml::background_layer& layer, const litehtml::web_color& color) override;
    void draw_linear_gradient(litehtml::uint_ptr hdc, const litehtml::background_layer& layer, const litehtml::background_layer::linear_gradient& gradient) override;
    void draw_radial_gradient(litehtml::uint_ptr hdc, const litehtml::background_layer& layer, const litehtml::background_layer::radial_gradient& gradient) override;
    void draw_conic_gradient(litehtml::uint_ptr hdc, const litehtml::background_layer& layer, const litehtml::background_layer::conic_gradient& gradient) override;
    void draw_borders(litehtml::uint_ptr hdc, const litehtml::borders& borders, const litehtml::position& draw_pos, bool root) override;
    void get_viewport(litehtml::position& viewport) const override;
    void get_media_features(litehtml::media_features& media) const override;
    void set_caption(const char* caption) override;
    void set_base_url(const char* base_url) override;
    void link(const std::shared_ptr<litehtml::document>& doc, const litehtml::element::ptr& el) override;
    void on_anchor_click(const char* url, const litehtml::element::ptr& el) override;
    void on_mouse_event(const litehtml::element::ptr& el, litehtml::mouse_event event) override;
    void set_cursor(const char* cursor) override;
    void transform_text(litehtml::string& text, litehtml::text_transform tt) override;
    void import_css(litehtml::string& text, const litehtml::string& url, litehtml::string& baseurl) override;
    void set_clip(const litehtml::position& pos, const litehtml::border_radiuses& bdr_radius) override;
    void del_clip() override;
    void get_language(litehtml::string& language, litehtml::string& culture) const override;
    litehtml::element::ptr create_element(const char* tag_name, 
                                          const litehtml::string_map& attributes,
                                          const std::shared_ptr<litehtml::document>& doc) override;

    // URL resolving and resource loading
    void onImageLoaded(const QString& url, const QImage& image);
    void onCssLoaded(const QString& url, const QString& css);

signals:
    void titleChanged(const QString& title);
    void anchorClicked(const QString& url);
    void cursorChanged(const QString& cursor);

private:
    QPainter* m_painter;
    QRect m_paintRect;
    QString m_baseUrl;
    qreal m_devicePixelRatio;
    QString m_defaultFontName;
    int m_defaultFontSize;
    
    QMap<QString, QImage> m_images;
    QMap<int, struct font_metrics_t> m_fonts;
    int m_nextFontId;
};

#endif // CONTAINER_QPAINTER_H 