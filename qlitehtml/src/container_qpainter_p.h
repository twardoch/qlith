#ifndef CONTAINER_QPAINTER_P_H
#define CONTAINER_QPAINTER_P_H

#include "container_qpainter.h"
#include <litehtml.h>
#include <QPainter>
#include <QUrl>
#include <QMap>
#include <QPixmap>
#include <QFont>

class container_qpainter_private
{
public:
    container_qpainter_private();
    ~container_qpainter_private();
    
    // Store the document
    std::shared_ptr<litehtml::document> document;
    
    // Store resources
    QMap<QString, QPixmap> images;
    
    // Context
    QPainter* painter = nullptr;
    QUrl baseUrl;
    QString current_document_url;
    
    // Document dimensions
    int max_width = 0;
    int width = 0;
    int height = 0;
    
    // Default font settings
    int defaultFontSize = 16;
    QString defaultFontFamily = "Arial";
    
    // Load resources
    void load_image(const QString& url);
};

#endif // CONTAINER_QPAINTER_P_H
