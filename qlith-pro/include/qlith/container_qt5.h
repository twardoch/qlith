// this_file: qlith-pro/include/qlith/container_qt5.h
#pragma once

#include <litehtml.h>
#include <litehtml/document.h>
#include <litehtml/document_container.h>
#include <QObject>
#include <QWidget>
#include <QHash>
#include <QImage>
#include <QFont>
#include <QPainter>
#include <QPoint>
#include <QMap>
#include <QFontMetrics>

// Forward declarations
class litehtmlWidget;

/**
 * Implementation of the litehtml document_container for Qt5
 */
class container_qt5 : public QObject, public litehtml::document_container
{
  Q_OBJECT

signals:
  void docSizeChanged(int w, int h);
  void documentSizeChanged(int w, int h);
  void titleChanged(const QString& title);
  void anchorClicked(const QString& url);
  void cursorChanged(const QString& cursor);

public:
    std::shared_ptr<litehtml::document> _doc;

private:
    QHash<QString, QByteArray> m_loaded_css;
    QHash<QString, QImage> m_images;
    int lastCursorX = 0;
    int lastCursorY = 0;
    int lastCursorClientX = 0;
    int lastCursorClientY = 0;
    int offsetX = 0;
    int offsetY = 0;
    QPoint m_Scroll{0,0};
    int m_lastDocWidth = 0, m_lastDocHeight = 0;
    QString m_defaultFontName; // Default font name (e.g., "Arial")
    int m_defaultFontSize;     // Default font size in pixels
    QString m_baseUrl;         // Base URL for resolving relative paths
    QWidget* m_owner;
    QPainter* m_painter = nullptr;
    
    // Font storage
    struct font_metrics_t {
        QFont font;
        QFontMetrics metrics;
        
        font_metrics_t() : metrics(font) {}
        explicit font_metrics_t(const QFont& f) : font(f), metrics(f) {}
    };
    
    QMap<int, font_metrics_t> m_fonts;
    int m_nextFontId = 1;

  public:
    /**
     * Default constructor
     */
    explicit container_qt5(QWidget *parent = nullptr);

    /**
     * Destructor
     */
    ~container_qt5();

    void setLastMouseCoords(int x, int y, int xClient, int yClient);

    std::shared_ptr<litehtml::document> getDocument()
    {
      return _doc;
    }

  void set_document(std::shared_ptr<litehtml::document> doc);

  // Additional methods for mouse interaction
  void on_lbutton_down(std::shared_ptr<litehtml::document>& doc, int x, int y, int client_x);
  void on_lbutton_up(std::shared_ptr<litehtml::document>& doc, int x, int y, int client_x);
  void on_mouse_over(std::shared_ptr<litehtml::document>& doc, int x, int y, int client_x);

  // Drawing method
  void draw(std::shared_ptr<litehtml::document>& doc, QPainter* painter, int x, int y, const litehtml::position* clip);

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
  void on_mouse_event(const litehtml::element::ptr& el, litehtml::mouse_event event) override;
  void set_cursor(const char* cursor) override;
  void transform_text(litehtml::string& text, litehtml::text_transform tt) override;
  void import_css(litehtml::string& text, const litehtml::string& url, litehtml::string& baseurl) override;
  void set_clip(const litehtml::position& pos, const litehtml::border_radiuses& bdr_radius) override;
  void del_clip() override;
  void get_viewport(litehtml::position& viewport) const override;
  void get_media_features(litehtml::media_features& media) const override;
  void get_language(litehtml::string& language, litehtml::string& culture) const override;
  litehtml::element::ptr create_element(const char* tag_name, 
                                       const litehtml::string_map& attributes,
                                       const std::shared_ptr<litehtml::document>& doc) override;
  litehtml::string resolve_color(const litehtml::string& color) const override;

  // Helper methods
  void repaint(QPainter& painter);
  QPoint getScroll() const;
  void setScroll(const QPoint &val);
  void setScrollX(const int &val);
  void setScrollY(const int &val);
  void setPainter(QPainter* painter);
  void onImageLoaded(const QString& url, const QImage& image);

  // Convert litehtml web_color to QColor
  static QColor webColorToQColor(const litehtml::web_color &color)
  {
    return QColor(color.red, color.green, color.blue, color.alpha);
  }
};

Q_DECLARE_METATYPE(container_qt5*)
