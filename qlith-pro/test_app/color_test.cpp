#include <iostream>
#include <litehtml.h>
#include <memory>
#include <string>
#include <QDebug>
#include "qlith/container_qt5.h"

// Simple test program to verify color parsing fixes
int main(int argc, char** argv)
{
    std::cout << "Color parsing test program" << std::endl;
    
    // Create a simple document container
    auto container = std::make_shared<container_qt5>(nullptr);
    
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