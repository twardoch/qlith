#include <iostream>
#include <litehtml.h>
#include <memory>
#include <string>
#include <QDebug>
#include <QApplication>
#include <QObject>

// Create a simplified mock container for testing colors
class mock_container : public litehtml::document_container {
private:
    using ColorResolverFunc = std::function<std::string(const std::string&)>;
    ColorResolverFunc m_customColorResolver;

public:
    // Constructor
    mock_container() {
        std::cout << "Mock container created" << std::endl;
    }

    // Set custom color resolver
    void setCustomColorResolver(ColorResolverFunc resolver) {
        m_customColorResolver = resolver;
    }

    // Implement just the minimal set of required functions
    litehtml::uint_ptr create_font(const litehtml::font_description& descr, const litehtml::document* doc, litehtml::font_metrics* fm) override {
        // Return a dummy font handle
        return 1;
    }
    
    void delete_font(litehtml::uint_ptr hFont) override {}
    
    int text_width(const char* text, litehtml::uint_ptr hFont) override {
        return 100; // dummy width
    }
    
    void draw_text(litehtml::uint_ptr hdc, const char* text, litehtml::uint_ptr hFont, litehtml::web_color color, const litehtml::position& pos) override {}
    
    int pt_to_px(int pt) const override {
        return pt * 4 / 3; // standard conversion
    }
    
    int get_default_font_size() const override {
        return 16;
    }
    
    const char* get_default_font_name() const override {
        return "Arial";
    }
    
    void draw_list_marker(litehtml::uint_ptr hdc, const litehtml::list_marker& marker) override {}
    
    void load_image(const char* src, const char* baseurl, bool redraw_on_ready) override {}
    
    void get_image_size(const char* src, const char* baseurl, litehtml::size& sz) override {
        sz.width = 0;
        sz.height = 0;
    }
    
    void draw_image(litehtml::uint_ptr hdc, const litehtml::background_layer& layer, const std::string& url, const std::string& base_url) override {}
    
    void draw_solid_fill(litehtml::uint_ptr hdc, const litehtml::background_layer& layer, const litehtml::web_color& color) override {}
    
    void draw_linear_gradient(litehtml::uint_ptr hdc, const litehtml::background_layer& layer, const litehtml::background_layer::linear_gradient& gradient) override {}
    
    void draw_radial_gradient(litehtml::uint_ptr hdc, const litehtml::background_layer& layer, const litehtml::background_layer::radial_gradient& gradient) override {}
    
    void draw_conic_gradient(litehtml::uint_ptr hdc, const litehtml::background_layer& layer, const litehtml::background_layer::conic_gradient& gradient) override {}
    
    void draw_borders(litehtml::uint_ptr hdc, const litehtml::borders& borders, const litehtml::position& draw_pos, bool root) override {}
    
    void set_caption(const char* caption) override {}
    
    void set_base_url(const char* base_url) override {}
    
    void link(const std::shared_ptr<litehtml::document>& doc, const litehtml::element::ptr& el) override {}
    
    void on_anchor_click(const char* url, const litehtml::element::ptr& el) override {}
    
    void on_mouse_event(const litehtml::element::ptr& el, litehtml::mouse_event event) override {}
    
    void set_cursor(const char* cursor) override {}
    
    void transform_text(litehtml::string& text, litehtml::text_transform tt) override {
        // Very basic text transformation - not all transformations are implemented
        if (tt == litehtml::text_transform_uppercase) {
            std::transform(text.begin(), text.end(), text.begin(), ::toupper);
        } else if (tt == litehtml::text_transform_lowercase) {
            std::transform(text.begin(), text.end(), text.begin(), ::tolower);
        }
    }
    
    void import_css(litehtml::string& text, const litehtml::string& url, litehtml::string& baseurl) override {}
    
    void set_clip(const litehtml::position& pos, const litehtml::border_radiuses& bdr_radius) override {}
    
    void del_clip() override {}
    
    void get_viewport(litehtml::position& viewport) const override {
        viewport.width = 800;
        viewport.height = 600;
    }
    
    void get_media_features(litehtml::media_features& media) const override {
        media.type = litehtml::media_type_screen;
        media.width = 800;
        media.height = 600;
        media.device_width = 800;
        media.device_height = 600;
        media.color = 8;
        media.resolution = 96;
    }
    
    void get_language(litehtml::string& language, litehtml::string& culture) const override {
        language = "en";
        culture = "US";
    }
    
    litehtml::element::ptr create_element(const char* tag_name, 
                                        const litehtml::string_map& attributes,
                                        const std::shared_ptr<litehtml::document>& doc) override {
        return nullptr; // Let litehtml create default element
    }
    
    litehtml::string resolve_color(const litehtml::string& color) const override {
        if (m_customColorResolver) {
            return m_customColorResolver(color);
        }
        return color;
    }
};

// Simple test program to verify color parsing fixes
int main(int argc, char** argv)
{
    QApplication app(argc, argv);
    
    std::cout << "Color parsing test program" << std::endl;
    
    // Create a simple document container
    auto container = std::make_shared<mock_container>();
    
    // Test 1: Basic HTML with simple colors
    const char* simple_html = R"(
        <html>
        <head>
            <style>
                body { color: blue; }
                h1 { color: #ff0000; }
                p { color: rgb(0, 128, 0); }
            </style>
        </head>
        <body>
            <h1>Test Header</h1>
            <p>Test paragraph</p>
        </body>
        </html>
    )";
    
    std::cout << "Testing simple HTML with standard colors..." << std::endl;
    auto document = litehtml::document::createFromString(simple_html, container.get());
    if (document) {
        std::cout << "  Success: Document created" << std::endl;
    } else {
        std::cout << "  Failure: Document creation failed" << std::endl;
        return 1;
    }
    
    // Test 2: Named color resolution
    const char* named_colors_html = R"(
        <html>
        <head>
            <style>
                .c1 { color: aliceblue; }
                .c2 { color: chartreuse; }
                .c3 { color: darkmagenta; }
                .c4 { color: thisDoesNotExist; }
            </style>
        </head>
        <body>
            <div class="c1">Color 1</div>
            <div class="c2">Color 2</div>
            <div class="c3">Color 3</div>
            <div class="c4">Color 4</div>
        </body>
        </html>
    )";
    
    std::cout << "Testing named color resolution..." << std::endl;
    auto document2 = litehtml::document::createFromString(named_colors_html, container.get());
    if (document2) {
        std::cout << "  Success: Document with named colors created" << std::endl;
    } else {
        std::cout << "  Failure: Document with named colors creation failed" << std::endl;
        return 1;
    }
    
    // Test 3: Recursion prevention
    const char* recursive_html = R"(
        <html>
        <head>
            <style>
                body { color: recursiveColor; }
            </style>
        </head>
        <body>
            <p>Test recursion prevention</p>
        </body>
        </html>
    )";
    
    // Override resolve_color to create a recursive scenario
    container->setCustomColorResolver([](const std::string& color) -> std::string {
        if (color == "recursiveColor") {
            return "recursiveColor"; // This would normally cause infinite recursion
        }
        return color;
    });
    
    std::cout << "Testing recursion prevention..." << std::endl;
    auto document3 = litehtml::document::createFromString(recursive_html, container.get());
    if (document3) {
        std::cout << "  Success: Document with potential recursion created safely" << std::endl;
    } else {
        std::cout << "  Failure: Document with potential recursion creation failed" << std::endl;
        return 1;
    }
    
    std::cout << "All tests completed successfully!" << std::endl;
    return 0;
} 