#include <QApplication>
#include <QCommandLineParser>
#include <QFileInfo>
#include <QSvgGenerator>
#include <QPainter>
#include <QImage>
#include <QDebug>
#include <QUrl>
#include <QSize>
#include <QTimer>
#include <iostream>

int main(int argc, char *argv[])
{
    try {
        // Enable high DPI support
        QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
        QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
        
        QApplication app(argc, argv);
        
        // Parse command line arguments
        QCommandLineParser parser;
        parser.setApplicationDescription("Minimal HTML export tool (fallback mode)");
        parser.addHelpOption();
        parser.addVersionOption();
        
        QCommandLineOption urlOption(QStringList() << "u" << "url", "The URL or file to open", "url");
        QCommandLineOption pngOption(QStringList() << "png", "Save rendered page as PNG", "filename");
        QCommandLineOption svgOption(QStringList() << "svg", "Save rendered page as SVG", "filename");
        QCommandLineOption widthOption(QStringList() << "w" << "width", "Width of the output", "width", "800");
        QCommandLineOption heightOption(QStringList() << "ht" << "height", "Height of the output", "height", "600");
        
        parser.addOption(urlOption);
        parser.addOption(pngOption);
        parser.addOption(svgOption);
        parser.addOption(widthOption);
        parser.addOption(heightOption);
        parser.addPositionalArgument("url", "The URL or file to open", "[url]");
        
        parser.process(app);
        
        // Get dimensions
        int width = parser.value(widthOption).toInt();
        int height = parser.value(heightOption).toInt();
        
        // Get URL
        QString urlStr;
        if (parser.isSet(urlOption)) {
            urlStr = parser.value(urlOption);
        } else {
            const QStringList args = parser.positionalArguments();
            if (!args.isEmpty()) {
                urlStr = args.at(0);
            }
        }
        
        // Get export options
        bool saveAsPng = parser.isSet(pngOption);
        bool saveAsSvg = parser.isSet(svgOption);
        QString pngFilename = parser.value(pngOption);
        QString svgFilename = parser.value(svgOption);
        
        // If no URL or no export options, show help
        if (urlStr.isEmpty() || (!saveAsPng && !saveAsSvg)) {
            std::cout << "Minimal export tool (fallback mode)\n"
                      << "Usage: minimal_main --url <url> --png <filename> [--svg <filename>]\n";
            return 1;
        }
        
        // Create a simple fallback image instead of rendering HTML
        if (saveAsPng) {
            QImage image(width, height, QImage::Format_ARGB32_Premultiplied);
            image.fill(Qt::white);
            
            QPainter painter(&image);
            painter.setPen(Qt::black);
            painter.setFont(QFont("Arial", 14));
            
            // Draw a frame
            painter.drawRect(10, 10, width - 20, height - 20);
            
            // Draw a title
            painter.drawText(20, 40, QString("Fallback Rendering: %1").arg(urlStr));
            
            // Draw some explanatory text
            painter.drawText(20, 80, "HTML rendering failed - this is a fallback image.");
            painter.drawText(20, 110, "Please try again with --safe-mode or without export options.");
            
            // Save the image
            if (image.save(pngFilename, "PNG")) {
                std::cout << "Saved fallback PNG to " << pngFilename.toStdString() << std::endl;
            } else {
                std::cerr << "Failed to save fallback PNG" << std::endl;
            }
        }
        
        // Create a simple fallback SVG
        if (saveAsSvg) {
            QSvgGenerator generator;
            generator.setFileName(svgFilename);
            generator.setSize(QSize(width, height));
            generator.setViewBox(QRect(0, 0, width, height));
            generator.setTitle("Fallback Rendering");
            
            QPainter painter;
            if (painter.begin(&generator)) {
                // Fill with white
                painter.fillRect(0, 0, width, height, Qt::white);
                
                // Set up painter
                painter.setPen(Qt::black);
                painter.setFont(QFont("Arial", 14));
                
                // Draw a frame
                painter.drawRect(10, 10, width - 20, height - 20);
                
                // Draw a title
                painter.drawText(20, 40, QString("Fallback Rendering: %1").arg(urlStr));
                
                // Draw some explanatory text
                painter.drawText(20, 80, "HTML rendering failed - this is a fallback SVG.");
                painter.drawText(20, 110, "Please try again with --safe-mode or without export options.");
                
                painter.end();
                std::cout << "Saved fallback SVG to " << svgFilename.toStdString() << std::endl;
            } else {
                std::cerr << "Failed to create fallback SVG" << std::endl;
            }
        }
        
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Fatal exception: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "Unknown fatal exception" << std::endl;
        return 1;
    }
} 