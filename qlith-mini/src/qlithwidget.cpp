#include "qlithwidget.h"
#include "container_qpainter.h"

#include <QMouseEvent>
#include <QPainter>
#include <QResizeEvent>
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
#include <QLocale>
#include <QFileInfo>

// Helper function for debugging
static void saveDebugContent(const QString &content, const QString &identifier, const QString &extension)
{
#ifdef QLITH_DEBUG_DIR
    QDir dir(QLITH_DEBUG_DIR);
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
#endif
}

// Forward declaration
class QlithWidget;

// Private implementation class
class QlithWidgetPrivate : public QObject {
    Q_OBJECT
public:
    QlithWidgetPrivate(QlithWidget *owner);
    ~QlithWidgetPrivate();
    
    // Content loading methods
    void loadHtml(const QString &html, const QUrl &url);
    void loadUrl(const QUrl &url);
    void loadLocalFile(const QString &filePath);
    void loadRemoteUrl(const QUrl &url);
    
    // Layout handling
    void updateWithLayout();

public slots:
    // Handle network reply for assets (images, CSS, etc.)
    void handleNetworkReply(QNetworkReply *reply);

public:
    QlithWidget *q;
    ContainerQPainter *container;
    QNetworkAccessManager *networkManager;
    std::shared_ptr<litehtml::document> document;
    QMap<QUrl, QNetworkReply*> pendingResources;
    
    QString currentHtml;
    QUrl currentUrl;
    QUrl baseUrl;
    QColor backgroundColor;
    QString defaultCss;
    
    bool loading;
    bool needsLayout;
};

// Implementation of QlithWidgetPrivate methods
QlithWidgetPrivate::QlithWidgetPrivate(QlithWidget *owner)
    : QObject(owner)
    , q(owner)
    , container(new ContainerQPainter(owner))
    , networkManager(new QNetworkAccessManager(owner))
    , loading(false)
    , needsLayout(true)
    , currentUrl("about:blank")
    , backgroundColor(Qt::white)
    , defaultCss(
        "html { background-color: white; color: black; font-family: Arial, sans-serif; }"
        "body { margin: 8px; }"
        "a { color: blue; text-decoration: underline; }"
        "img { border: none; }"
    )
{
    // Connect container signals
    QObject::connect(container, &ContainerQPainter::titleChanged, 
                    owner, &QlithWidget::titleChanged);
    
    QObject::connect(container, &ContainerQPainter::anchorClicked, 
                    [this](const QString &url) {
                        if (url.startsWith("http:") || url.startsWith("https:") || url.startsWith("file:")) {
                            emit q->linkClicked(QUrl(url));
                        } else {
                            QUrl resolved = currentUrl.resolved(QUrl(url));
                            emit q->linkClicked(resolved);
                        }
                    });
    
    // Connect network manager signals
    QObject::connect(networkManager, &QNetworkAccessManager::finished,
                    this, &QlithWidgetPrivate::handleNetworkReply);
}

QlithWidgetPrivate::~QlithWidgetPrivate() {
    // Clear any pending requests
    for (QNetworkReply* reply : pendingResources.values()) {
        reply->abort();
        reply->deleteLater();
    }
    pendingResources.clear();
}

// Load HTML content
void QlithWidgetPrivate::loadHtml(const QString &html, const QUrl &url) {
    currentHtml = html;
    currentUrl = url;
    
    // Set base URL for the container
    container->setBaseUrl(url.toString());
    
    // Create and render document
    if (document) {
        document.reset();
    }
    
    // Prepare master CSS
    QString masterCss = defaultCss;
    
    // Create context for litehtml
    // Implementation depends on the litehtml version, here's a simplified approach
    QByteArray htmlData = html.toUtf8();
    
    try {
        qDebug() << "QlithWidget: Creating litehtml document from string, length:" << htmlData.size();
        
        // Create litehtml document
        document = litehtml::document::createFromString(
            htmlData.constData(), 
            container
        );
        
        if (document) {
            // Get device pixel ratio for proper scaling
            qreal devicePixelRatio = container->devicePixelRatio();
            int scaledWidth = qRound(q->width() / devicePixelRatio);
            
            qDebug() << "QlithWidget: Document created successfully, rendering with device pixel ratio:" 
                     << devicePixelRatio << ", widget width:" << q->width() 
                     << ", scaled width for rendering:" << scaledWidth;
                     
            // Render using scaled width to account for device pixel ratio
            document->render(scaledWidth);
            needsLayout = false;
            q->update();
            
            qDebug() << "QlithWidget: Document rendered, emitting loadFinished(true)";
            loading = false;
            emit q->loadFinished(true);
        } else {
            qWarning() << "QlithWidget: Failed to create litehtml document";
            loading = false;
            emit q->loadFinished(false);
        }
    } catch (const std::exception &e) {
        qWarning() << "QlithWidget: Exception creating document:" << e.what();
        loading = false;
        emit q->loadFinished(false);
    } catch (...) {
        qWarning() << "QlithWidget: Unknown exception creating document";
        loading = false;
        emit q->loadFinished(false);
    }
}

// Load URL
void QlithWidgetPrivate::loadUrl(const QUrl &url) {
    // Skip if same URL and already loaded
    if (url == currentUrl && !needsLayout) {
        return;
    }
    
    currentUrl = url;
    loading = true;
    needsLayout = true;
    
    // Debug output
    qDebug() << "QlithWidget::loadUrl - Loading URL:" << url.toString() << ", scheme:" << url.scheme();
    
    emit q->loadStarted();
    
    // Handle different URL schemes
    if (url.scheme() == "file") {
        qDebug() << "QlithWidget::loadUrl - Loading local file:" << url.toLocalFile();
        loadLocalFile(url.toLocalFile());
    } else if (url.scheme() == "http" || url.scheme() == "https") {
        loadRemoteUrl(url);
    } else if (url.scheme() == "about" && url.path() == "blank") {
        loadHtml("<html><body></body></html>", url);
    } else {
        qWarning() << "Unsupported URL scheme:" << url.scheme();
        emit q->loadFinished(false);
    }
}

// Load local file
void QlithWidgetPrivate::loadLocalFile(const QString &filePath) {
    // Debug output
    qDebug() << "QlithWidget::loadLocalFile - Attempting to load file:" << filePath;
    QFileInfo fileInfo(filePath);
    qDebug() << "  - Absolute file path:" << fileInfo.absoluteFilePath();
    qDebug() << "  - File exists:" << fileInfo.exists();
    
    QFile file(filePath);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QString html = QString::fromUtf8(file.readAll());
        loadHtml(html, QUrl::fromLocalFile(filePath));
    } else {
        qWarning() << "Failed to open file:" << filePath << " - " << file.errorString();
        emit q->loadFinished(false);
    }
}

// Load remote URL
void QlithWidgetPrivate::loadRemoteUrl(const QUrl &url) {
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::UserAgentHeader, "Qlith/1.0");
    
    QNetworkReply *reply = networkManager->get(request);
    
    QObject::connect(reply, &QNetworkReply::finished, [this, reply, url]() {
        if (reply->error() == QNetworkReply::NoError) {
            QString contentType = reply->header(QNetworkRequest::ContentTypeHeader).toString();
            QByteArray data = reply->readAll();
            
            if (contentType.contains("text/html", Qt::CaseInsensitive) || 
                url.path().endsWith(".html", Qt::CaseInsensitive) ||
                url.path().endsWith(".htm", Qt::CaseInsensitive)) {
                
                QString html = QString::fromUtf8(data);
                loadHtml(html, url);
            } else {
                qWarning() << "URL did not return HTML content:" << url;
                emit q->loadFinished(false);
            }
        } else {
            qWarning() << "Failed to load URL:" << url << " - " << reply->errorString();
            emit q->loadFinished(false);
        }
        
        reply->deleteLater();
    });
}

// Handle network reply for assets (images, CSS, etc.)
void QlithWidgetPrivate::handleNetworkReply(QNetworkReply *reply) {
    if (!reply) return;
    
    QUrl url = reply->url();
    QString urlStr = url.toString();
    
    if (reply->error() == QNetworkReply::NoError) {
        QByteArray data = reply->readAll();
        
        // Handle based on content type
        QString contentType = reply->header(QNetworkRequest::ContentTypeHeader).toString();
        
        if (contentType.startsWith("image/", Qt::CaseInsensitive) ||
            url.path().endsWith(".png", Qt::CaseInsensitive) ||
            url.path().endsWith(".jpg", Qt::CaseInsensitive) ||
            url.path().endsWith(".jpeg", Qt::CaseInsensitive) ||
            url.path().endsWith(".gif", Qt::CaseInsensitive)) {
            
            // Handle image
            QImage image;
            if (image.loadFromData(data)) {
                container->onImageLoaded(urlStr, image);
                q->update();
            }
        } else if (contentType.contains("text/css", Qt::CaseInsensitive) ||
                  url.path().endsWith(".css", Qt::CaseInsensitive)) {
            
            // Handle CSS
            QString css = QString::fromUtf8(data);
            container->onCssLoaded(urlStr, css);
            q->update();
        }
    }
    
    // Remove reply from pending list
    pendingResources.remove(url);
    reply->deleteLater();
}

// Redraw with layout if needed
void QlithWidgetPrivate::updateWithLayout() {
    if (needsLayout && document) {
        // Get device pixel ratio for proper scaling
        qreal devicePixelRatio = container->devicePixelRatio();
        int scaledWidth = qRound(q->width() / devicePixelRatio);
        
        qDebug() << "QlithWidget::updateWithLayout - Rendering with device pixel ratio:" 
                 << devicePixelRatio << ", widget width:" << q->width() 
                 << ", scaled width for rendering:" << scaledWidth;
                 
        // Render using scaled width to account for device pixel ratio
        document->render(scaledWidth);
        needsLayout = false;
    }
    q->update();
}

//
// QlithWidget implementation
//

QlithWidget::QlithWidget(QWidget* parent)
    : QWidget(parent)
    , d_ptr(new QlithWidgetPrivate(this))
{
    // Set widget attributes
    setAttribute(Qt::WA_OpaquePaintEvent);
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);
}

QlithWidget::~QlithWidget()
{
}

void QlithWidget::setHtml(const QString& html)
{
    Q_D(QlithWidget);
    emit loadStarted();
    d->loadHtml(html, d->baseUrl.isEmpty() ? QUrl("about:blank") : d->baseUrl);
}

void QlithWidget::load(const QUrl& url)
{
    Q_D(QlithWidget);
    d->loadUrl(url);
}

void QlithWidget::setBaseUrl(const QUrl& url)
{
    Q_D(QlithWidget);
    if (d->baseUrl != url) {
        d->baseUrl = url;
        d->container->setBaseUrl(url.toString());
        emit baseUrlChanged(url);
    }
}

QUrl QlithWidget::baseUrl() const
{
    Q_D(const QlithWidget);
    return d->baseUrl;
}

void QlithWidget::setBackgroundColor(const QColor& color)
{
    Q_D(QlithWidget);
    if (d->backgroundColor != color) {
        d->backgroundColor = color;
        update();
        emit backgroundColorChanged(color);
    }
}

QColor QlithWidget::backgroundColor() const
{
    Q_D(const QlithWidget);
    return d->backgroundColor;
}

void QlithWidget::setDefaultStyleSheet(const QString& css)
{
    Q_D(QlithWidget);
    d->defaultCss = css;
    // Reload if we have a document
    if (d->document) {
        setHtml(d->currentHtml);
    }
}

QString QlithWidget::documentTitle() const
{
    Q_D(const QlithWidget);
    // This would require accessing the document title
    // We would need to implement this with litehtml
    return QString();
}

std::shared_ptr<litehtml::document> QlithWidget::document() const
{
    Q_D(const QlithWidget);
    return d->document;
}

QSize QlithWidget::documentSize() const
{
    Q_D(const QlithWidget);
    if (d->document) {
        return QSize(d->document->width(), d->document->height());
    }
    return QSize();
}

bool QlithWidget::needsVerticalScrolling() const
{
    Q_D(const QlithWidget);
    if (d->document) {
        return d->document->height() > height();
    }
    return false;
}

void QlithWidget::reload()
{
    Q_D(QlithWidget);
    if (d->currentUrl.isValid()) {
        load(d->currentUrl);
    } else if (!d->currentHtml.isEmpty()) {
        setHtml(d->currentHtml);
    }
}

void QlithWidget::stop()
{
    Q_D(QlithWidget);
    // Abort any ongoing network requests
    for (QNetworkReply* reply : d->pendingResources.values()) {
        reply->abort();
        reply->deleteLater();
    }
    d->pendingResources.clear();
    d->loading = false;
}

// Widget events

void QlithWidget::paintEvent(QPaintEvent* event)
{
    Q_D(QlithWidget);
    QPainter painter(this);
    
    // Fill background
    painter.fillRect(rect(), d->backgroundColor);
    
    // Render document if available
    if (d->document) {
        d->container->beginPaint(&painter, rect());
        
        // Create clip position that accommodates the full document height
        int docWidth = d->document->width();
        int docHeight = d->document->height();
        
        // Use the maximum of widget height and document height to ensure all content is rendered
        litehtml::position clip(0, 0, width(), qMax(height(), docHeight));
        
        qDebug() << "QlithWidget::paintEvent - Document size:" << docWidth << "x" << docHeight
                 << ", Widget size:" << width() << "x" << height()
                 << ", Using clip size:" << clip.width << "x" << clip.height;
        
        d->document->draw((litehtml::uint_ptr)&painter, 0, 0, &clip);
        
        d->container->endPaint();
    }
}

void QlithWidget::resizeEvent(QResizeEvent* event)
{
    Q_D(QlithWidget);
    if (d->document) {
        // Mark for relayout
        d->needsLayout = true;
        // Layout immediately
        d->updateWithLayout();
    }
    QWidget::resizeEvent(event);
}

void QlithWidget::mousePressEvent(QMouseEvent* event)
{
    Q_D(QlithWidget);
    if (d->document) {
        litehtml::position::vector redraw_boxes;
        int x = event->pos().x();
        int y = event->pos().y();
        
        if (event->button() == Qt::LeftButton) {
            d->document->on_lbutton_down(x, y, x, y, redraw_boxes);
            if (!redraw_boxes.empty()) {
                update();
            }
        }
    }
    QWidget::mousePressEvent(event);
}

void QlithWidget::mouseReleaseEvent(QMouseEvent* event)
{
    Q_D(QlithWidget);
    if (d->document) {
        litehtml::position::vector redraw_boxes;
        int x = event->pos().x();
        int y = event->pos().y();
        
        if (event->button() == Qt::LeftButton) {
            d->document->on_lbutton_up(x, y, x, y, redraw_boxes);
            if (!redraw_boxes.empty()) {
                update();
            }
        }
    }
    QWidget::mouseReleaseEvent(event);
}

void QlithWidget::mouseMoveEvent(QMouseEvent* event)
{
    Q_D(QlithWidget);
    if (d->document) {
        litehtml::position::vector redraw_boxes;
        int x = event->pos().x();
        int y = event->pos().y();
        
        d->document->on_mouse_over(x, y, x, y, redraw_boxes);
        if (!redraw_boxes.empty()) {
            update();
        }
    }
    QWidget::mouseMoveEvent(event);
}

void QlithWidget::wheelEvent(QWheelEvent* event)
{
    // We're not implementing scrolling in this basic widget
    // A scrollable version would handle this
    QWidget::wheelEvent(event);
}

void QlithWidget::keyPressEvent(QKeyEvent* event)
{
    // Handle key events if needed
    QWidget::keyPressEvent(event);
}

#include "qlithwidget.moc" 