#include "qlith/litehtmlwidget.h"
#include "qlith/container_qt5.h"
#include "qlith/context.h"
#include <QPainter>
#include <QMouseEvent>
#include <QResizeEvent>
#include <QWheelEvent>
#include <QApplication>
#include <QDebug>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrl>
#include <QFileInfo>
#include <QScrollBar>
#include <QTimer>
#include <QDesktopServices>
#include <QScrollArea>
#include <QLabel>

/**
 * Constructor
 * @param parent Parent widget
 */
litehtmlWidget::litehtmlWidget(QWidget *parent)
    : QWidget(parent),
      m_container(nullptr),
      m_htmlDocument(nullptr),
      m_vScrollBar(nullptr),
      m_hScrollBar(nullptr),
      m_scrollX(0),
      m_scrollY(0),
      m_backgroundColor(Qt::white),
      m_documentSizeSet(false)
{
    // Set widget attributes for proper display
    setAttribute(Qt::WA_OpaquePaintEvent);

    // Set widget to receive keyboard focus
    setFocusPolicy(Qt::StrongFocus);

    // Set minimum size
    setMinimumSize(200, 150);

    try
    {
        // Use direct allocation like in qlith-mini, ensuring proper construction
        m_container = new container_qt5(this);

        if (!m_container)
        {
            qWarning() << "Failed to create container_qt5 instance";
            return;
        }

        qDebug() << "container_qt5: Initialized with default font:" << m_container->get_default_font_name()
                 << "size:" << m_container->get_default_font_size();

        // Connect container signals to widget slots properly
        connect(m_container, &container_qt5::docSizeChanged, this, &litehtmlWidget::onDocSizeChanged);
        connect(m_container, &container_qt5::documentSizeChanged, this, &litehtmlWidget::updateDocumentSize);
        connect(m_container, &container_qt5::titleChanged, this, &litehtmlWidget::titleChanged);
        connect(m_container, &container_qt5::anchorClicked, this, &litehtmlWidget::anchorClicked);

        // Set up scrollbars
        createScrollBars();
    }
    catch (const std::exception &e)
    {
        qCritical() << "Exception in litehtmlWidget constructor:" << e.what();
    }
    catch (...)
    {
        qCritical() << "Unknown exception in litehtmlWidget constructor";
    }
}

/**
 * Destructor
 */
litehtmlWidget::~litehtmlWidget()
{
    delete m_vScrollBar;
    delete m_hScrollBar;
    delete m_container;
}

/**
 * Get the container object
 * @return Pointer to the container_qt5 instance
 */
container_qt5 *litehtmlWidget::getContainer() const
{
    return m_container;
}

/**
 * Load HTML content
 * @param html The HTML content to load
 * @param baseUrl Base URL for relative paths
 */
void litehtmlWidget::loadHtml(const QString &html, const QString &baseUrl)
{
    qDebug() << "loadHtml: Loading HTML with base URL:" << baseUrl;
    qDebug() << "loadHtml: First 100 chars of HTML:" << html.left(100).replace("\n", "\\n");
    qDebug() << "loadHtml: HTML length:" << html.length();
    qDebug() << "loadHtml: Current widget size:" << size(); 

    if (html.isEmpty())
    {
        qWarning() << "loadHtml: Empty HTML content provided";
        emit documentLoaded(false);
        return;
    }

    if (!m_container)
    {
        qWarning() << "loadHtml: m_container is null. Cannot proceed.";
        try {
            // Attempt to recreate the container as a last resort
            m_container = new container_qt5(this);
            if (!m_container) {
                emit documentLoaded(false);
                return;
            }
            qDebug() << "loadHtml: Successfully recreated container";
            
            // Reconnect signals
            connect(m_container, &container_qt5::docSizeChanged, this, &litehtmlWidget::onDocSizeChanged);
            connect(m_container, &container_qt5::documentSizeChanged, this, &litehtmlWidget::updateDocumentSize);
            connect(m_container, &container_qt5::titleChanged, this, &litehtmlWidget::titleChanged);
            connect(m_container, &container_qt5::anchorClicked, this, &litehtmlWidget::anchorClicked);
        } catch (const std::exception &e) {
            qCritical() << "loadHtml: Failed to recover from null container:" << e.what();
            emit documentLoaded(false);
            return;
        } catch (...) {
            qCritical() << "loadHtml: Failed to recover from null container (unknown error)";
            emit documentLoaded(false);
            return;
        }
    }

    try
    {
        // Clear existing document first 
        if (m_htmlDocument)
        {
            qDebug() << "loadHtml: Releasing existing document";
            m_htmlDocument.reset();
        }

        // Set base URL if provided
        if (!baseUrl.isEmpty())
        {
            qDebug() << "loadHtml: Setting base URL:" << baseUrl;
            // Make sure it's properly terminated with a slash for relative paths
            QString baseUrlWithSlash = baseUrl;
            if (!baseUrlWithSlash.endsWith('/')) {
                baseUrlWithSlash += '/';
            }
            qDebug() << "loadHtml: Normalized base URL:" << baseUrlWithSlash;
            m_container->set_base_url(baseUrlWithSlash.toStdString().c_str());
        }

        // Make a local copy to ensure the string stays in scope
        QString localHtml = html;
        
        // Prepare HTML for parsing - ensure it's properly structured
        if (!localHtml.contains("<!DOCTYPE", Qt::CaseInsensitive)) {
            qDebug() << "loadHtml: Adding DOCTYPE declaration";
            localHtml = "<!DOCTYPE html>\n" + localHtml;
        }

        // Convert to UTF-8
        QByteArray utf8Data = localHtml.toUtf8();
        std::string htmlData(utf8Data.constData(), utf8Data.size());
        
        qDebug() << "loadHtml: Creating litehtml document from string, size:" << htmlData.size();
        
        // Create the document
        try {
            m_htmlDocument = litehtml::document::createFromString(htmlData.c_str(), m_container);
            
            if (!m_htmlDocument) {
                qCritical() << "loadHtml: Failed to create litehtml document. Creating a simple fallback.";
                // Create a super simple document as a last resort
                QString simpleHtml = "<!DOCTYPE html><html><body>Failed to load document</body></html>";
                QByteArray simpleUtf8 = simpleHtml.toUtf8();
                m_htmlDocument = litehtml::document::createFromString(simpleUtf8.constData(), m_container);
                
                if (!m_htmlDocument) {
                    qCritical() << "loadHtml: Even failed to create a simple fallback document";
                    emit documentLoaded(false);
                    return;
                }
            }
            
            qDebug() << "loadHtml: Document created successfully";
            
            // Render the document
            int renderWidth = width() > 0 ? width() : 800;
            qDebug() << "loadHtml: Rendering with width:" << renderWidth;
            
            m_htmlDocument->render(renderWidth);
            
            int docWidth = m_htmlDocument->width();
            int docHeight = m_htmlDocument->height();
            
            qDebug() << "loadHtml: Document dimensions after render:" << docWidth << "x" << docHeight;
            
            if (docWidth <= 0 || docHeight <= 0) {
                qWarning() << "loadHtml: Document has invalid dimensions, using defaults";
                m_documentSize = QSize(renderWidth, 600);
            } else {
                m_documentSize = QSize(docWidth, docHeight);
            }
            
            m_documentSizeSet = true;
            
            // Update scrollbars
            onDocSizeChanged(m_documentSize.width(), m_documentSize.height());
            
            // Force a repaint
            update();
            QApplication::processEvents();
            
            qDebug() << "loadHtml: Document loaded successfully";
            emit documentLoaded(true);
        }
        catch (const std::exception &e) {
            qCritical() << "loadHtml: Exception creating document:" << e.what();
            createFallbackDocument(QString("Exception creating document: %1").arg(e.what()));
        }
    }
    catch (const std::exception &e)
    {
        qCritical() << "loadHtml: Exception during HTML document processing:" << e.what();
        createFallbackDocument(QString("Exception: %1").arg(e.what()));
        emit documentLoaded(false);
    }
    catch (...)
    {
        qCritical() << "loadHtml: Unknown C++ exception during HTML document processing.";
        createFallbackDocument("Unknown C++ exception occurred");
        emit documentLoaded(false);
    }
}

// Helper method to create a fallback document
void litehtmlWidget::createFallbackDocument(const QString& errorMessage)
{
    try {
        qDebug() << "createFallbackDocument: Creating fallback document with error message:" << errorMessage;
        
        QString fallbackHtml = QString("<!DOCTYPE html><html><body><h1>Error</h1><p>%1</p></body></html>")
            .arg(errorMessage);
        std::string fallbackStdString = fallbackHtml.toUtf8().toStdString();
        
        if (!m_container) {
            qCritical() << "createFallbackDocument: m_container is null";
            return;
        }
        
        m_htmlDocument = litehtml::document::createFromString(fallbackStdString.c_str(), m_container);
        
        if (m_htmlDocument) {
            qDebug() << "createFallbackDocument: Fallback document created successfully";
            renderDocument();
        } else {
            qCritical() << "createFallbackDocument: Failed to create fallback document";
        }
    } catch (const std::exception& e) {
        qCritical() << "createFallbackDocument: Exception creating fallback document:" << e.what();
    } catch (...) {
        qCritical() << "createFallbackDocument: Unknown exception creating fallback document";
    }
}

// Helper method to render the loaded document
void litehtmlWidget::renderDocument()
{
    // Calculate render width
    int renderWidth = width();
    if (renderWidth <= 0 || renderWidth > 10000) {
        renderWidth = 800;
    }
    if (renderWidth < 200) renderWidth = 800;
    
    qDebug() << "renderDocument: Rendering document with width:" << renderWidth;
    
    try {
        if (!m_htmlDocument) {
            qCritical() << "renderDocument: m_htmlDocument is null";
            return;
        }
        
        m_htmlDocument->render(renderWidth);
        qDebug() << "renderDocument: Document rendered with dimensions:" << m_htmlDocument->width() << "x" << m_htmlDocument->height();
        
        // Get document size
        m_documentSize = QSize(m_htmlDocument->width(), m_htmlDocument->height());
        if (m_documentSize.width() <= 0 || m_documentSize.height() <= 0) {
            qWarning() << "renderDocument: Document has invalid dimensions - using defaults";
            m_documentSize = QSize(renderWidth, 600);
        }
        
        m_documentSizeSet = true;
        qDebug() << "renderDocument: Document size set to:" << m_documentSize;
        
        // Update scrollbars
        onDocSizeChanged(m_documentSize.width(), m_documentSize.height());
        
        // Force a repaint
        update();
        QApplication::processEvents();
        
        qDebug() << "renderDocument: Document rendered successfully";
        emit documentLoaded(true);
    } catch (const std::exception& e) {
        qCritical() << "renderDocument: Exception rendering document:" << e.what();
        m_documentSize = QSize(renderWidth, 600);
        m_documentSizeSet = true;
        update();
        emit documentLoaded(false);
    } catch (...) {
        qCritical() << "renderDocument: Unknown exception rendering document";
        m_documentSize = QSize(renderWidth, 600);
        m_documentSizeSet = true;
        update();
        emit documentLoaded(false);
    }
}

/**
 * Load HTML content from a URL
 * @param url The URL to load the HTML content from
 */
void litehtmlWidget::loadUrl(const QString &url)
{
    qDebug() << "Loading URL:" << url;

    // Handle file:// URLs directly
    if (url.startsWith("file://", Qt::CaseInsensitive))
    {
        QString localPath = QUrl(url).toLocalFile();
        QFile file(localPath);
        if (file.open(QIODevice::ReadOnly))
        {
            QString content = QString::fromUtf8(file.readAll());
            loadHtml(content, url);
            file.close();
        }
        else
        {
            QString errorHtml = QString("<html><body><h1>Error</h1><p>Failed to open file: %1</p><p>Error: %2</p></body></html>")
                                    .arg(localPath)
                                    .arg(file.errorString());
            loadHtml(errorHtml);
        }
        return;
    }

    // For HTTP/HTTPS URLs, use QNetworkAccessManager
    QNetworkAccessManager *manager = new QNetworkAccessManager(this);
    QNetworkRequest request(url);

    QNetworkReply *reply = manager->get(request);

    connect(reply, &QNetworkReply::finished, this, [=]()
            {
        if (reply->error() == QNetworkReply::NoError) {
            QString content = QString::fromUtf8(reply->readAll());
            loadHtml(content, url);
        } else {
            QString errorHtml = QString("<html><body><h1>Error</h1><p>Failed to load URL: %1</p><p>Error: %2</p></body></html>")
                                    .arg(url)
                                    .arg(reply->errorString());
            loadHtml(errorHtml);
        }
        
        reply->deleteLater();
        manager->deleteLater(); });
}

/**
 * Set the scroll position
 * @param x New horizontal scroll position
 * @param y New vertical scroll position
 */
void litehtmlWidget::setScrollPos(int x, int y)
{
    m_scrollX = x;
    m_scrollY = y;

    m_hScrollBar->setValue(x);
    m_vScrollBar->setValue(y);

    update();
}

/**
 * Set the scroll position
 * @param pos New scroll position
 */
void litehtmlWidget::setScrollPosition(const QPoint &pos)
{
    setScrollPos(pos.x(), pos.y());
}

/**
 * Get the current scroll position
 * @return Current scroll position
 */
QPoint litehtmlWidget::scrollPosition() const
{
    return QPoint(m_scrollX, m_scrollY);
}

/**
 * Get the current scroll position
 * @return Current scroll position
 */
QPoint litehtmlWidget::scrollPos() const
{
    return QPoint(m_scrollX, m_scrollY);
}

/**
 * Get the size of the HTML document
 * @return Document size
 */
QSize litehtmlWidget::documentSize() const
{
    return m_documentSize;
}

/**
 * Paint event handler
 * @param event Paint event
 */
void litehtmlWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    qDebug() << "litehtmlWidget::paintEvent - Widget size:" << size() << ", Document size:" << documentSize();

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::TextAntialiasing, true);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);

    painter.fillRect(rect(), Qt::white); // Always fill background

    if (!m_htmlDocument)
    {
        painter.setPen(Qt::darkGray);
        painter.drawText(rect(), Qt::AlignCenter, "No HTML document loaded or document is null.");
        qDebug() << "paintEvent: No HTML document to draw.";
        return;
    }

    if (!m_container)
    {
        painter.setPen(Qt::red);
        painter.drawText(rect(), Qt::AlignCenter, "Error: Document container is null.");
        qCritical() << "paintEvent: m_container is null. Cannot draw document.";
        return;
    }

    try
    {
        // Safety check for document dimensions before drawing
        if (m_htmlDocument->width() <= 0 || m_htmlDocument->height() <= 0)
        {
            painter.setPen(Qt::red);
            painter.drawText(rect(), Qt::AlignCenter, "Error: Document has invalid dimensions.");
            qWarning() << "paintEvent: Document has invalid dimensions:"
                       << m_htmlDocument->width() << "x" << m_htmlDocument->height();
            return;
        }

        // Translate by the scroll position and set up clipping
        painter.translate(-m_scrollX, -m_scrollY);

        // Create a clipping rectangle
        litehtml::position clip(0, 0, width() + m_scrollX, height() + m_scrollY);

        // Check if the clip rect is valid
        if (clip.width <= 0 || clip.height <= 0)
        {
            // Reset the painter and show error
            painter.resetTransform();
            painter.setPen(Qt::red);
            painter.drawText(rect(), Qt::AlignCenter, "Error: Invalid clip rectangle.");
            qWarning() << "paintEvent: Invalid clip rect:" << clip.x << clip.y << clip.width << clip.height;
            return;
        }

        qDebug() << "paintEvent: Drawing document with clip:" << clip.x << clip.y << clip.width << clip.height;
        
        // Draw the document with the validated clip rect - this is the core functionality
        m_container->draw(m_htmlDocument, &painter, 0, 0, &clip);
        
        qDebug() << "paintEvent: Document drawing completed successfully";
    }
    catch (const std::exception &e)
    {
        qCritical() << "paintEvent: Exception while drawing document:" << e.what();
        painter.resetTransform(); // Reset transform before drawing error text
        painter.setPen(Qt::red);
        painter.drawText(rect(), Qt::AlignCenter, QString("Error rendering HTML: %1").arg(e.what()));
    }
    catch (...)
    {
        qCritical() << "paintEvent: Unknown exception while drawing document.";
        painter.resetTransform();
        painter.setPen(Qt::red);
        painter.drawText(rect(), Qt::AlignCenter, "Unknown error rendering HTML.");
    }
}

/**
 * Resize event handler
 * @param event Resize event
 */
void litehtmlWidget::resizeEvent(QResizeEvent *event)
{
    QSize oldSize = event->oldSize();
    QSize newSize = event->size();
    
    qDebug() << "litehtmlWidget::resizeEvent - Old size:" << oldSize << ", New size:" << newSize;
    
    // Only rerender if width has changed significantly (helps avoid expensive re-renders)
    int widthDelta = qAbs(oldSize.width() - newSize.width());
    if (m_htmlDocument && (oldSize.width() <= 1 || widthDelta > 5))
    {
        qDebug() << "litehtmlWidget::resizeEvent - Re-rendering document with new width:" << newSize.width();
        try {
            m_htmlDocument->render(newSize.width());
        }
        catch (const std::exception &e) {
            qCritical() << "resizeEvent: Exception during document re-render:" << e.what();
        }
        catch (...) {
            qCritical() << "resizeEvent: Unknown exception during document re-render";
        }
    }

    // Update scrollbar positions
    if (m_documentSizeSet)
    {
        m_vScrollBar->setRange(0, std::max(0, m_documentSize.height() - height()));
        m_vScrollBar->setPageStep(height());

        m_hScrollBar->setRange(0, std::max(0, m_documentSize.width() - width()));
        m_hScrollBar->setPageStep(width());

        // Show scrollbars if needed
        m_vScrollBar->setVisible(m_documentSize.height() > height());
        m_hScrollBar->setVisible(m_documentSize.width() > width());

        // Position scrollbars
        m_vScrollBar->setGeometry(width() - m_vScrollBar->sizeHint().width(),
                               0,
                               m_vScrollBar->sizeHint().width(),
                               height() - (m_hScrollBar->isVisible() ? m_hScrollBar->sizeHint().height() : 0));

        m_hScrollBar->setGeometry(0,
                               height() - m_hScrollBar->sizeHint().height(),
                               width() - (m_vScrollBar->isVisible() ? m_vScrollBar->sizeHint().width() : 0),
                               m_hScrollBar->sizeHint().height());
    }
    
    QWidget::resizeEvent(event);
}

/**
 * Mouse press event handler
 * @param event Mouse event
 */
void litehtmlWidget::mousePressEvent(QMouseEvent *event)
{
    if (m_htmlDocument)
    {
        int x = event->pos().x() + m_scrollX;
        int y = event->pos().y() + m_scrollY;

        m_container->on_lbutton_down(m_htmlDocument, x, y, 0);
    }

    QWidget::mousePressEvent(event);
}

/**
 * Mouse release event handler
 * @param event Mouse event
 */
void litehtmlWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if (m_htmlDocument)
    {
        int x = event->pos().x() + m_scrollX;
        int y = event->pos().y() + m_scrollY;

        m_container->on_lbutton_up(m_htmlDocument, x, y, 0);
    }

    QWidget::mouseReleaseEvent(event);
}

/**
 * Mouse move event handler
 * @param event Mouse event
 */
void litehtmlWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (m_htmlDocument)
    {
        int x = event->pos().x() + m_scrollX;
        int y = event->pos().y() + m_scrollY;

        m_container->on_mouse_over(m_htmlDocument, x, y, 0);
    }

    QWidget::mouseMoveEvent(event);
}

/**
 * Wheel event handler for scrolling
 * @param event Wheel event
 */
void litehtmlWidget::wheelEvent(QWheelEvent *event)
{
    if (m_vScrollBar->isVisible())
    {
        // Calculate scroll amount (adjust the divisor for sensitivity)
        int delta = event->angleDelta().y() / 8;

        // Update vertical scrollbar
        m_vScrollBar->setValue(m_vScrollBar->value() - delta);
    }

    event->accept();
}

/**
 * Handler for document size changes
 * @param w New width
 * @param h New height
 */
void litehtmlWidget::onDocSizeChanged(int w, int h)
{
    m_documentSize = QSize(w, h);
    m_documentSizeSet = true;

    // Update scrollbar ranges
    m_vScrollBar->setRange(0, std::max(0, h - this->height()));
    m_vScrollBar->setPageStep(this->height());

    m_hScrollBar->setRange(0, std::max(0, w - this->width()));
    m_hScrollBar->setPageStep(this->width());

    // Show scrollbars if needed
    m_vScrollBar->setVisible(h > this->height());
    m_hScrollBar->setVisible(w > this->width());

    // Position scrollbars
    m_vScrollBar->setGeometry(this->width() - m_vScrollBar->sizeHint().width(),
                              0,
                              m_vScrollBar->sizeHint().width(),
                              this->height() - (m_hScrollBar->isVisible() ? m_hScrollBar->sizeHint().height() : 0));

    m_hScrollBar->setGeometry(0,
                              this->height() - m_hScrollBar->sizeHint().height(),
                              this->width() - (m_vScrollBar->isVisible() ? m_vScrollBar->sizeHint().width() : 0),
                              m_hScrollBar->sizeHint().height());

    update();
}

void litehtmlWidget::updateDocumentSize(int w, int h)
{
    // Delegates to the existing onDocSizeChanged method
    onDocSizeChanged(w, h);
}

/**
 * Create scrollbars for the widget
 */
void litehtmlWidget::createScrollBars()
{
    // Create vertical scrollbar
    m_vScrollBar = new QScrollBar(Qt::Vertical, this);
    m_vScrollBar->setRange(0, 0);
    m_vScrollBar->hide();
    connect(m_vScrollBar, &QScrollBar::valueChanged, [this](int value)
            {
        m_scrollY = value;
        update(); });

    // Create horizontal scrollbar
    m_hScrollBar = new QScrollBar(Qt::Horizontal, this);
    m_hScrollBar->setRange(0, 0);
    m_hScrollBar->hide();
    connect(m_hScrollBar, &QScrollBar::valueChanged, [this](int value)
            {
        m_scrollX = value;
        update(); });
}

// Removed linkClicked and titleChanged implementations - they are signals and shouldn't be implemented