#ifndef CONTAINER_QPAINTER_H
#define CONTAINER_QPAINTER_H

#include "qlitehtml_global.h"
#include <litehtml.h>
#include <QPainter>
#include <QUrl>
#include <memory>

class container_qpainter_private;

class QLITEHTML_EXPORT container_qpainter : public litehtml::document_container
{
public:
    container_qpainter();
    ~container_qpainter() override;

    // Set the painter to draw with
    void setPainter(QPainter* painter);
    
    // Set the base URL for resource loading
    void setBaseUrl(const QString& base_url);
    
    // Set the base font size
    void setBaseFontSize(int size);
    
    // Load HTML content with base URL
    void setHtml(const QString &html, const QString &baseUrl);
    
    // Get document
    std::shared_ptr<litehtml::document> document() const;
    
    // Handle loaded resources
    void onResourceLoaded(const QUrl &url, const QByteArray &data);
    
    // Drawing method used by QLiteHtmlWidget
    void draw(QPainter* painter, int x, int y, const litehtml::position* clip);
    
    // Update document size after rendering
    void updateSize();

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
    void set_caption(const char* caption) override;
    void set_base_url(const char* base_url) override;
    void link(const std::shared_ptr<litehtml::document>& doc, const litehtml::element::ptr& el) override;
    void on_anchor_click(const char* url, const litehtml::element::ptr& el) override;
    bool on_element_click(const litehtml::element::ptr& el) override;
    void on_mouse_event(const litehtml::element::ptr& el, litehtml::mouse_event event) override;
    void set_cursor(const char* cursor) override;
    void transform_text(litehtml::string& text, litehtml::text_transform tt) override;
    void import_css(litehtml::string& text, const litehtml::string& url, litehtml::string& baseurl) override;
    void set_clip(const litehtml::position& pos, const litehtml::border_radiuses& bdr_radius) override;
    void del_clip() override;
    void get_viewport(litehtml::position& viewport) const override;
    litehtml::element::ptr create_element(const char* tag_name, const litehtml::string_map& attributes, const std::shared_ptr<litehtml::document>& doc) override;
    void get_media_features(litehtml::media_features& media) const override;
    void get_language(litehtml::string& language, litehtml::string& culture) const override;
    
private:
    std::unique_ptr<container_qpainter_private> d;
};

#endif // CONTAINER_QPAINTER_H
