#include "qlitehtmlwidget.h"
#include "container_qpainter.h"
#include <QMouseEvent>
#include <QPainter>
#include <QScrollBar>
#include <QApplication>
#include <QMimeData>
#include <QClipboard>
#include <QDebug>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QFile>
#include <QDir>
#include <QUrl>
#include <QDesktopServices>
#include <QStandardPaths>
#include <QDateTime>
#include <QBuffer>
#include <QTemporaryFile>
#include <zlib.h>

// Function to save debug content to file
void saveDebugContent(const QString &content, const QString &identifier, const QString &extension)
{
    // Check if debugging is enabled
    if (qEnvironmentVariableIsEmpty("QLDW_DEBUG")) {
        return;
    }
    
    // Get the debug directory from environment or use a default
    QString debugDir = qEnvironmentVariable("QLDW_DEBUG_DIR");
    if (debugDir.isEmpty()) {
        debugDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation) + "/qlitehtml_debug";
    }
    
    QDir dir(debugDir);
    if (!dir.exists()) {
        dir.mkpath(".");
    }
    
    // Create a filename based on the current timestamp and identifier
    QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss_zzz");
    QString filename = dir.filePath(QString("%1_%2%3").arg(timestamp).arg(identifier.section('/', -1).section('?', 0, 0)).arg(extension));
    
    // Save content to file
    QFile file(filename);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream stream(&file);
        stream << content;
        file.close();
        qDebug() << "Debug content saved to" << filename;
    } else {
        qWarning() << "Failed to save debug content to" << filename;
    }
}

// Function to save binary data to file
void saveDebugBinaryContent(const QByteArray &data, const QString &identifier, const QString &extension)
{
    // Check if debugging is enabled
    if (qEnvironmentVariableIsEmpty("QLDW_DEBUG")) {
        return;
    }
    
    // Get the debug directory from environment or use a default
    QString debugDir = qEnvironmentVariable("QLDW_DEBUG_DIR");
    if (debugDir.isEmpty()) {
        debugDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation) + "/qlitehtml_debug";
    }
    
    QDir dir(debugDir);
    if (!dir.exists()) {
        dir.mkpath(".");
    }
    
    // Create a filename based on the current timestamp and identifier
    QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss_zzz");
    QString filename = dir.filePath(QString("%1_%2%3").arg(timestamp).arg(identifier.section('/', -1).section('?', 0, 0)).arg(extension));
    
    // Save data to file
    QFile file(filename);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(data);
        file.close();
        qDebug() << "Debug binary content saved to" << filename;
    } else {
        qWarning() << "Failed to save debug binary content to" << filename;
    }
}

// Helper function to test if data is gzipped
bool isGzipped(const QByteArray &data) {
    // Check for gzip magic number (1F 8B)
    if (data.size() < 2) {
        return false;
    }
    
    return ((unsigned char)data[0] == 0x1F && (unsigned char)data[1] == 0x8B);
}

// Helper function to decompress gzipped data
QByteArray decompressGzip(const QByteArray &compressedData) {
    try {
        if (compressedData.isEmpty()) {
            qDebug() << "Cannot decompress empty data";
            return QByteArray();
        }
        
        // First check if the data is actually gzipped by looking for the magic number
        if (!isGzipped(compressedData)) {
            qDebug() << "Data does not appear to be in gzip format (missing magic number)";
            return QByteArray();
        }
        
        qDebug() << "Attempting to decompress gzipped data, size:" << compressedData.size() << "bytes";
        
        // Raw data buffer for output
        QByteArray uncompressedData;
        
        // Initialize zlib stream
        z_stream strm;
        memset(&strm, 0, sizeof(strm));
        
        // Use inflateInit2 with 16+MAX_WBITS for gzip decoding
        if (inflateInit2(&strm, 16+MAX_WBITS) != Z_OK) {
            qDebug() << "Failed to initialize zlib for decompression";
            return QByteArray();
        }
        
        // Set up the input
        strm.avail_in = static_cast<uInt>(compressedData.size());
        strm.next_in = reinterpret_cast<Bytef*>(const_cast<char*>(compressedData.data()));
        
        // Decompress chunk by chunk using a reasonable buffer size
        const int CHUNK = 16384; // 16KB chunks
        char outBuffer[CHUNK];
        int ret;
        
        // Check for null pointers
        if (!strm.next_in) {
            qDebug() << "Error: null input pointer";
            inflateEnd(&strm);
            return QByteArray();
        }
        
        do {
            strm.avail_out = CHUNK;
            strm.next_out = reinterpret_cast<Bytef*>(outBuffer);
            
            ret = inflate(&strm, Z_NO_FLUSH);
            
            // Process error conditions
            if (ret != Z_OK && ret != Z_STREAM_END) {
                inflateEnd(&strm);
                qDebug() << "Decompression error:" << ret;
                if (ret == Z_DATA_ERROR) {
                    qDebug() << "Data is not in gzip format or is corrupted";
                }
                return QByteArray();
            }
            
            // Append the decompressed data
            uncompressedData.append(outBuffer, CHUNK - strm.avail_out);
            
        } while (strm.avail_out == 0);
        
        // Clean up
        inflateEnd(&strm);
        
        qDebug() << "Decompression successful. Original size:" << compressedData.size()
                << "bytes, Decompressed size:" << uncompressedData.size() << "bytes";
        
        return uncompressedData;
    } catch (const std::exception& e) {
        qDebug() << "Exception during decompression:" << e.what();
        return QByteArray();
    } catch (...) {
        qDebug() << "Unknown exception during decompression";
        return QByteArray();
    }
}

class QLiteHtmlWidgetPrivate {
public:
    QLiteHtmlWidgetPrivate(QLiteHtmlWidget *owner)
        : q(owner)
        , container(nullptr)
        , networkManager(new QNetworkAccessManager(owner))
        , loaded(false)
        , needsLayout(true)
        , htmlWidth(0)
        , htmlHeight(0)
        , zoom(1.0)
        , currentUrl("about:blank")
        , selectionMode(false)
        , selectedElement(nullptr)
        , safeMode(false)
    {
    }

    ~QLiteHtmlWidgetPrivate()
    {
        delete container;
    }

    void init()
    {
        // Create the container which manages litehtml context
        container = new container_qpainter();
        
        // Create network connections
        QObject::connect(networkManager, SIGNAL(finished(QNetworkReply*)),
                        q, SLOT(resourceNetworkReplyFinished(QNetworkReply*)));
    }

    void updateDocumentSize()
    {
        if (!container) {
            qWarning() << "updateDocumentSize: Container is null";
            return;
        }

        if (!q || !q->viewport()) {
            qWarning() << "updateDocumentSize: Owner widget or viewport is null";
            return;
        }

        // First, check if we have a valid document
        if (!container->document()) {
            qWarning() << "updateDocumentSize: Document is null";
            return;
        }

        // Update the document size based on widget size, with minimum values
        // Default to a reasonable minimum size if viewport has invalid dimensions
        int viewportWidth = q->viewport()->width();
        int viewportHeight = q->viewport()->height();
        
        if (viewportWidth <= 0) viewportWidth = 640;
        if (viewportHeight <= 0) viewportHeight = 480;
        
        htmlWidth = qMax(1, viewportWidth) / zoom;
        htmlHeight = qMax(1, viewportHeight) / zoom;
        
        // Extra safety check
        if (htmlWidth <= 0 || htmlHeight <= 0) {
            qWarning() << "updateDocumentSize: Invalid dimensions after calculations:" 
                       << htmlWidth << "x" << htmlHeight;
            htmlWidth = qMax(1, htmlWidth);
            htmlHeight = qMax(1, htmlHeight);
        }

        // Make sure we have a valid container before rendering
        try {
            // Render again with new dimensions
            container->document()->render(htmlWidth);
            
            // Get the actual document height (which could be different after rendering)
            htmlHeight = container->document()->height();
            
            // Extra validation - ensure we have a positive height
            if (htmlHeight <= 0) {
                qWarning() << "Document rendered with zero or negative height, using minimum height";
                htmlHeight = 1;
            }
            
            // Set viewport sizes for scrolling - use safe calculations to avoid division by zero
            int verticalMaximum = (int)(htmlHeight * zoom) - viewportHeight;
            int horizontalMaximum = (int)(htmlWidth * zoom) - viewportWidth;
            
            q->verticalScrollBar()->setRange(0, qMax(0, verticalMaximum));
            q->verticalScrollBar()->setPageStep(viewportHeight);
            q->horizontalScrollBar()->setRange(0, qMax(0, horizontalMaximum));
            q->horizontalScrollBar()->setPageStep(viewportWidth);

            // Redraw
            q->viewport()->update();
        } 
        catch (const std::exception& e) {
            qWarning() << "Exception in updateDocumentSize:" << e.what();
            
            // Set default scrollbar settings to avoid crashes
            q->verticalScrollBar()->setRange(0, 0);
            q->horizontalScrollBar()->setRange(0, 0);
        } 
        catch (...) {
            qWarning() << "Unknown exception in updateDocumentSize";
            
            // Set default scrollbar settings to avoid crashes
            q->verticalScrollBar()->setRange(0, 0);
            q->horizontalScrollBar()->setRange(0, 0);
        }
    }
    
    QLiteHtmlWidget* owner() const { return q; }

    QLiteHtmlWidget *q;
    container_qpainter *container;
    QNetworkAccessManager *networkManager;
    QMap<QUrl, QNetworkReply*> pendingResources;
    
    bool loaded;
    bool needsLayout;
    int htmlWidth;
    int htmlHeight;
    qreal zoom;
    QString html;
    QUrl currentUrl;
    QUrl baseUrl;
    
    // Selection
    bool selectionMode;
    litehtml::element::ptr selectedElement;
    
    // Safe mode
    bool safeMode;
};

QLiteHtmlWidget::QLiteHtmlWidget(QWidget *parent)
    : QAbstractScrollArea(parent)
    , d(new QLiteHtmlWidgetPrivate(this))
{
    d->init();
    setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    viewport()->setMouseTracking(true);
    
    // Check if we're in safe mode
    if (qEnvironmentVariableIsSet("QLITEHTML_SAFE_MODE")) {
        qDebug() << "QLiteHtmlWidget: Safe mode enabled";
        d->safeMode = true;
        
        // Reduce default zoom in safe mode to avoid rendering issues
        d->zoom = 0.8;
    }
}

QLiteHtmlWidget::~QLiteHtmlWidget()
{
    delete d;
}

void QLiteHtmlWidget::setHtml(const QString &html, const QUrl &baseUrl)
{
    // Save HTML for debugging
    saveDebugContent(html, baseUrl.toString(), ".html");
    
    d->html = html;
    d->baseUrl = baseUrl;
    d->currentUrl = baseUrl;
    d->loaded = false;
    d->needsLayout = true;
    
    // Reset scroll positions
    verticalScrollBar()->setValue(0);
    horizontalScrollBar()->setValue(0);
    
    // Reset selection
    d->selectedElement = nullptr;
    d->selectionMode = false;
    
    // Load the document
    if (d->container) {
        d->container->setHtml(html, baseUrl.toString());
        d->loaded = true;
        d->updateDocumentSize();
        emit htmlLoaded();
    }
}

void QLiteHtmlWidget::load(const QUrl &url)
{
    // Handle special "about:blank" protocol
    if (url.scheme() == "about" && url.path() == "blank") {
        QString blankHtml = "<html><body></body></html>";
        setHtml(blankHtml, url);
        emit loadFinished();
        return;
    }
    
    bool debugMode = !qEnvironmentVariableIsEmpty("QLDW_DEBUG");
    if (debugMode) {
        qDebug() << "Loading URL:" << url.toString();
    }
    
    QNetworkRequest request(url);
    request.setRawHeader("Accept-Encoding", "gzip, deflate");
    
    // Add more headers in debug mode
    if (debugMode) {
        request.setRawHeader("User-Agent", "QLiteHtmlWidget/1.0");
        request.setRawHeader("Accept", "text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8");
        request.setRawHeader("Accept-Language", "en-US,en;q=0.5");
        
        QString headerInfo = "Request Headers for " + url.toString() + ":\n";
        const QList<QByteArray> rawHeaderList = request.rawHeaderList();
        for (const QByteArray &header : rawHeaderList) {
            headerInfo += header + ": " + request.rawHeader(header) + "\n";
        }
        saveDebugContent(headerInfo, url.toString(), ".request_headers");
    }
    
    QNetworkReply *reply = d->networkManager->get(request);
    
    if (debugMode) {
        // Monitor download progress in debug mode
        connect(reply, &QNetworkReply::downloadProgress, [url](qint64 bytesReceived, qint64 bytesTotal) {
            qDebug() << "Download progress for" << url.toString() 
                     << ":" << bytesReceived << "of" << bytesTotal << "bytes";
        });
    }
    
    connect(reply, &QNetworkReply::finished, [this, reply, url, debugMode]() {
        reply->deleteLater();
        
        if (reply->error() == QNetworkReply::NoError) {
            QByteArray responseData = reply->readAll();
            
            if (debugMode) {
                qDebug() << "Network request completed for:" << url.toString();
                qDebug() << "Raw response size:" << responseData.size() << "bytes";
                
                // Save response metadata in debug mode
                QString metadata = "URL: " + url.toString() + "\n"
                                  + "Status Code: " + QString::number(reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt()) + "\n"
                                  + "Content Type: " + reply->header(QNetworkRequest::ContentTypeHeader).toString() + "\n"
                                  + "Content Length: " + QString::number(responseData.size()) + " bytes\n";
                saveDebugContent(metadata, url.toString(), ".metadata");
            }
            
            // Save raw response data for debugging
            saveDebugBinaryContent(responseData, url.toString(), ".raw");
            
            // Save response headers for debugging
            QString headerInfo = "URL: " + url.toString() + "\n\nHeaders:\n";
            const QList<QByteArray> headerList = reply->rawHeaderList();
            for (const QByteArray &header : headerList) {
                headerInfo += header + ": " + reply->rawHeader(header) + "\n";
            }
            saveDebugContent(headerInfo, url.toString(), ".headers");
            
            // Check if content is compressed
            QString contentEncoding;
            if (reply->hasRawHeader("Content-Encoding")) {
                contentEncoding = reply->rawHeader("Content-Encoding").toLower();
                if (debugMode) {
                    qDebug() << "Content-Encoding:" << contentEncoding;
                }
            }
            
            QByteArray contentToProcess = responseData;
            
            // Handle gzipped content
            if (!responseData.isEmpty() && contentEncoding.contains("gzip")) {
                if (debugMode) {
                    qDebug() << "Content-Encoding header indicates gzip, size:" << responseData.size() << "bytes";
                    qDebug() << "DISABLED: Skipping gzip decompression for debugging";
                }
                
                // DISABLED FOR DEBUGGING
                /*
                try {
                    bool actuallyGzipped = isGzipped(responseData);
                    if (debugMode) {
                        qDebug() << "Magic number check indicates content is" << (actuallyGzipped ? "gzipped" : "not gzipped");
                    }
                    
                    // Try our manual gzip decompression only if it's actually gzipped
                    QByteArray decompressed;
                    
                    if (actuallyGzipped) {
                        decompressed = decompressGzip(responseData);
                        
                        if (!decompressed.isEmpty()) {
                            if (debugMode) {
                                qDebug() << "Manual gzip decompression successful. Decompressed size:" << decompressed.size() << "bytes";
                            }
                            contentToProcess = decompressed;
                            
                            // Save the decompressed content for debugging
                            saveDebugBinaryContent(decompressed, url.toString(), ".decompressed");
                        } else {
                            if (debugMode) {
                                qDebug() << "Manual gzip decompression failed despite magic number. Using raw data.";
                            }
                            // We'll fall back to using the original data if decompression failed
                            contentToProcess = responseData;
                        }
                    } else {
                        if (debugMode) {
                            qDebug() << "Content-Encoding says gzip but magic number not found. Server may have already decompressed.";
                        }
                        // Server may have already decompressed the content despite the Content-Encoding header
                        contentToProcess = responseData;
                    }
                } catch (const std::exception& e) {
                    qDebug() << "Exception during gzip handling:" << e.what();
                    contentToProcess = responseData; // Use the original data in case of error
                } catch (...) {
                    qDebug() << "Unknown exception during gzip handling";
                    contentToProcess = responseData; // Use the original data in case of error
                }
                */
                
                // Just use the raw data as-is for now
                contentToProcess = responseData;
                
            } else if (responseData.isEmpty()) {
                qWarning() << "Warning: Received empty response data";
            }
            
            // Convert to string with encoding detection
            // First try UTF-8
            QString html = QString::fromUtf8(contentToProcess);
            
            // If it looks like garbage, try Latin1
            if (html.contains(QChar(0xFFFD)) && html.size() < contentToProcess.size() / 2) {
                if (debugMode) {
                    qDebug() << "UTF-8 decoding produced replacement characters, trying Latin1";
                }
                html = QString::fromLatin1(contentToProcess);
            }
            
            // Save the processed HTML
            saveDebugContent(html, url.toString(), ".html");
            
            // Log some stats about the HTML
            if (debugMode) {
                qDebug() << "Final HTML size:" << html.size() << "characters";
                qDebug() << "First 100 chars:" << html.left(100);
            }
            
            // Set HTML content
            setHtml(html, url);
            emit loadFinished();
        } else {
            qWarning() << "Failed to load URL:" << url << "Error:" << reply->errorString();
            QString errorHtml = QString("<html><body><h1>Failed to load %1</h1><p>%2</p></body></html>")
                                .arg(url.toString())
                                .arg(reply->errorString());
            setHtml(errorHtml, QUrl("about:blank"));
            emit loadFinished();
        }
    });
}

QUrl QLiteHtmlWidget::url() const
{
    return d->currentUrl;
}

void QLiteHtmlWidget::setZoom(qreal zoom)
{
    if (qFuzzyCompare(d->zoom, zoom))
        return;

    d->zoom = zoom;
    if (d->loaded) {
        d->updateDocumentSize();
        emit zoomChanged(zoom);
    }
}

qreal QLiteHtmlWidget::zoom() const
{
    return d->zoom;
}

void QLiteHtmlWidget::paintEvent(QPaintEvent *event)
{
    // If we don't have valid document data, just use standard paint handling
    if (!d->container || !d->loaded) {
        QAbstractScrollArea::paintEvent(event);
        return;
    }
    
    try {
        QPainter p(viewport());
        
        if (!p.isActive()) {
            qWarning() << "QLiteHtmlWidget::paintEvent: Failed to initialize painter";
            QAbstractScrollArea::paintEvent(event);
            return;
        }
        
        // Apply the scroll position
        p.translate(-horizontalScrollBar()->value(), -verticalScrollBar()->value());
        
        // Setup the HTML container and render
        d->container->setPainter(&p);
        
        if (d->needsLayout) {
            // Safely update document size
            try {
                d->updateDocumentSize();
            } catch (const std::exception& e) {
                qWarning() << "Exception during document size update:" << e.what();
                d->needsLayout = false;
            } catch (...) {
                qWarning() << "Unknown exception during document size update";
                d->needsLayout = false;
            }
        }
        
        // Set up clip rectangle based on the viewport and scroll position
        QRect clipRect = viewport()->rect();
        clipRect.translate(horizontalScrollBar()->value(), verticalScrollBar()->value());
        p.setClipRect(clipRect);
        
        // Draw the document with safety checks
        try {
            // Convert QRect to litehtml::position for the draw call
            litehtml::position litehtmlClip(
                clipRect.x(), 
                clipRect.y(), 
                clipRect.width(), 
                clipRect.height()
            );
            
            d->container->draw(&p, 0, 0, &litehtmlClip);
        } catch (const std::exception& e) {
            qWarning() << "Exception during document draw:" << e.what();
        } catch (...) {
            qWarning() << "Unknown exception during document draw";
        }
        
        d->container->setPainter(nullptr);
    } catch (const std::exception& e) {
        qWarning() << "Exception in paintEvent:" << e.what();
    } catch (...) {
        qWarning() << "Unknown exception in paintEvent";
    }
}

void QLiteHtmlWidget::resizeEvent(QResizeEvent *event)
{
    QAbstractScrollArea::resizeEvent(event);
    d->needsLayout = true;
}

void QLiteHtmlWidget::mousePressEvent(QMouseEvent *event)
{
    if (!d->container || !d->container->document() || !d->loaded) {
        return;
    }

    if (event->button() == Qt::LeftButton) {
        int x = event->x() / d->zoom + horizontalScrollBar()->value() / d->zoom;
        int y = event->y() / d->zoom + verticalScrollBar()->value() / d->zoom;
        
        // Handle selection or link click logic
        // For a simplified implementation, we'll handle basic selection
        d->selectionMode = true;
        
        // In a full implementation, we would use document's find_element_by_point
        // and handle it more robustly
        viewport()->update();
    }
    
    QAbstractScrollArea::mousePressEvent(event);
}

void QLiteHtmlWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if (!d->container || !d->container->document() || !d->loaded) {
        return;
    }

    if (event->button() == Qt::LeftButton) {
        if (d->selectionMode) {
            d->selectionMode = false;
            
            // Handle clicking on links
            int x = event->x() / d->zoom + horizontalScrollBar()->value() / d->zoom;
            int y = event->y() / d->zoom + verticalScrollBar()->value() / d->zoom;
            
            // In a full implementation, we would check if it's a link
            // and emit linkClicked signal
            
            // For now, let's use a simple check if it's an anchor element
            if (d->selectedElement && d->selectedElement->get_tagName() == std::string("a")) {
                // Get the href attribute
                const char* href = d->selectedElement->get_attr("href");
                if (href) {
                    QString qhref = QString::fromUtf8(href);
                    QUrl url;
                    
                    if (qhref.startsWith("http://") || qhref.startsWith("https://")) {
                        url = QUrl(qhref);
                    } else {
                        // Relative URL
                        url = d->baseUrl.resolved(QUrl(qhref));
                    }
                    
                    emit linkClicked(url);
                }
            }
        }
    }
    
    QAbstractScrollArea::mouseReleaseEvent(event);
}

void QLiteHtmlWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (!d->container || !d->container->document() || !d->loaded) {
        return;
    }
    
    int x = event->x() / d->zoom + horizontalScrollBar()->value() / d->zoom;
    int y = event->y() / d->zoom + verticalScrollBar()->value() / d->zoom;
    
    // Handle cursor changes based on what's under the mouse
    // In a full implementation, we would use the appropriate methods
    // from litehtml to handle this
    
    QAbstractScrollArea::mouseMoveEvent(event);
}

void QLiteHtmlWidget::keyPressEvent(QKeyEvent *event)
{
    // Handle key navigation
    if (event->key() == Qt::Key_Home) {
        verticalScrollBar()->setValue(0);
    } else if (event->key() == Qt::Key_End) {
        verticalScrollBar()->setValue(verticalScrollBar()->maximum());
    } else if (event->key() == Qt::Key_Up) {
        verticalScrollBar()->setValue(verticalScrollBar()->value() - 20);
    } else if (event->key() == Qt::Key_Down) {
        verticalScrollBar()->setValue(verticalScrollBar()->value() + 20);
    } else if (event->key() == Qt::Key_Left) {
        horizontalScrollBar()->setValue(horizontalScrollBar()->value() - 20);
    } else if (event->key() == Qt::Key_Right) {
        horizontalScrollBar()->setValue(horizontalScrollBar()->value() + 20);
    } else if (event->key() == Qt::Key_PageUp) {
        verticalScrollBar()->setValue(verticalScrollBar()->value() - viewport()->height());
    } else if (event->key() == Qt::Key_PageDown) {
        verticalScrollBar()->setValue(verticalScrollBar()->value() + viewport()->height());
    } else {
        QAbstractScrollArea::keyPressEvent(event);
    }
}

void QLiteHtmlWidget::resourceNetworkReplyFinished(QNetworkReply *reply)
{
    if (reply->error() == QNetworkReply::NoError) {
        QUrl url = reply->url();
        QByteArray data = reply->readAll();
        
        // Notify the container about the loaded resource
        if (d->container) {
            d->container->onResourceLoaded(url, data);
        }
    }
    
    reply->deleteLater();
}

QString QLiteHtmlWidget::documentTitle() const
{
    // Not available in the simplified implementation
    return QString();
}

QString QLiteHtmlWidget::selectedText() const
{
    // Not available in the simplified implementation
    return QString();
}

void QLiteHtmlWidget::zoomIn()
{
    setZoom(d->zoom * 1.2);
}

void QLiteHtmlWidget::zoomOut()
{
    setZoom(d->zoom / 1.2);
}

void QLiteHtmlWidget::resetZoom()
{
    setZoom(1.0);
}

void QLiteHtmlWidget::loadFile(const QString &filePath)
{
    QFile file(filePath);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QString html = QString::fromUtf8(file.readAll());
        QUrl fileUrl = QUrl::fromLocalFile(filePath);
        setHtml(html, fileUrl);
        emit loadFinished();
    } else {
        QString errorHtml = QString("<html><body><h1>Failed to load file</h1><p>Could not open %1: %2</p></body></html>")
                          .arg(filePath)
                          .arg(file.errorString());
        setHtml(errorHtml, QUrl("about:blank"));
        emit loadFinished();
    }
}

void QLiteHtmlWidget::saveDebugImage(const QString &filename)
{
    if (!d->loaded) {
        qDebug() << "Cannot save debug image - document not loaded";
        return;
    }
    
    int width = d->htmlWidth > 0 ? d->htmlWidth : viewport()->width();
    int height = d->htmlHeight > 0 ? d->htmlHeight : viewport()->height();
    
    qDebug() << "Debug image dimensions: " << width << "x" << height;
    qDebug() << "Viewport dimensions: " << viewport()->width() << "x" << viewport()->height();
    qDebug() << "Document loaded: " << d->loaded;
    qDebug() << "Has container: " << (d->container != nullptr);
    qDebug() << "Has document: " << (d->container && d->container->document() ? "yes" : "no");
    qDebug() << "HTML size: " << d->html.size() << " characters";
    
    // Create a pixmap at least the size of the viewport
    width = qMax(width, viewport()->width());
    height = qMax(height, viewport()->height());
    
    QPixmap pixmap(width, height);
    pixmap.fill(Qt::white);
    
    QPainter painter(&pixmap);
    
    if (d->container && d->container->document()) {
        // Render the document
        litehtml::position clip(0, 0, width, height);
        try {
            d->container->draw(&painter, 0, 0, &clip);
        } catch (const std::exception& e) {
            qDebug() << "Exception during rendering:" << e.what();
        } catch (...) {
            qDebug() << "Unknown exception during rendering";
        }
    } else {
        // Just draw some text to indicate there's no document
        painter.setPen(Qt::red);
        painter.drawText(QRect(0, 0, width, height), Qt::AlignCenter, 
                         "No document available to render");
    }
    
    // Save the image
    if (pixmap.save(filename)) {
        qDebug() << "Saved debug image to:" << filename;
    } else {
        qDebug() << "Failed to save debug image to:" << filename;
    }
}

void QLiteHtmlWidget::render(QPainter *painter)
{
    if (!painter) {
        qWarning() << "Cannot render: painter is null";
        return;
    }

    if (!d) {
        qWarning() << "Cannot render: private data is null";
        return;
    }

    if (!d->container) {
        qWarning() << "Cannot render: container is null";
        return;
    }

    // Check if the document is actually loaded and valid
    bool hasValidDocument = d->loaded && d->container->document();
    
    if (!hasValidDocument) {
        qWarning() << "Cannot render: document is not loaded or invalid";
        // Draw error message instead of returning
        painter->save();
        painter->setPen(Qt::red);
        painter->drawText(painter->viewport(), Qt::AlignCenter, 
                         "No valid HTML document loaded");
        painter->restore();
        return;
    }
    
    // Save the painter state
    painter->save();
    
    try {
        // Set up the painter for rendering
        d->container->setPainter(painter);
        
        // Apply zoom
        painter->scale(d->zoom, d->zoom);
        
        // Use the full widget size for rendering
        int viewWidth = qMax(1, width()) / d->zoom;
        int viewHeight = qMax(1, height()) / d->zoom;
        
        // Create a valid clip rectangle
        litehtml::position clip(0, 0, viewWidth, viewHeight);
        
        // Check for zero dimensions which can cause crashes
        if (viewWidth <= 0 || viewHeight <= 0) {
            qWarning() << "Invalid rendering dimensions:" << viewWidth << "x" << viewHeight;
            throw std::runtime_error("Invalid rendering dimensions");
        }
        
        // Draw at the origin with the clip rectangle
        d->container->draw(painter, 0, 0, &clip);
    }
    catch (const std::exception& e) {
        qWarning() << "Exception during rendering:" << e.what();
        
        // Draw error message
        painter->restore(); // Restore original state
        painter->save();    // Save again for our error message
        painter->setPen(Qt::red);
        painter->drawText(painter->viewport(), Qt::AlignCenter,
                         QString("Rendering error: %1").arg(e.what()));
    }
    catch (...) {
        qWarning() << "Unknown exception during rendering";
        
        // Draw error message
        painter->restore(); // Restore original state
        painter->save();    // Save again for our error message
        painter->setPen(Qt::red);
        painter->drawText(painter->viewport(), Qt::AlignCenter,
                         "Unknown rendering error occurred");
    }
    
    // Restore the painter state
    painter->restore();
} 