#include "qlith/litehtmlwidget.h"
#include "qlith/container_qt5.h"
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

/**
 * Constructor for litehtmlWidget
 * @param parent Parent widget
 */
litehtmlWidget::litehtmlWidget(QWidget* parent)
    : QWidget(parent)
    , m_container(new container_qt5(this))
    , m_htmlDocument(nullptr)
    , m_vScrollBar(new QScrollBar(Qt::Vertical, this))
    , m_hScrollBar(new QScrollBar(Qt::Horizontal, this))
    , m_scrollX(0)
    , m_scrollY(0)
    , m_documentSizeSet(false)
    , m_documentSize(0, 0)
    , m_needUpdate(false)
{
    // Setup scrollbars
    m_vScrollBar->hide();
    m_hScrollBar->hide();
    
    m_vScrollBar->setRange(0, 0);
    m_hScrollBar->setRange(0, 0);
    
    connect(m_vScrollBar, &QScrollBar::valueChanged, [this](int value) {
        m_scrollY = value;
        update();
    });
    
    connect(m_hScrollBar, &QScrollBar::valueChanged, [this](int value) {
        m_scrollX = value;
        update();
    });
    
    // Connect container signals
    connect(m_container, &container_qt5::documentSizeChanged, this, 
        [this](int width, int height) {
            m_documentSize = QSize(width, height);
            m_documentSizeSet = true;
            
            // Update scrollbar ranges
            m_vScrollBar->setRange(0, std::max(0, height - this->height()));
            m_vScrollBar->setPageStep(this->height());
            
            m_hScrollBar->setRange(0, std::max(0, width - this->width()));
            m_hScrollBar->setPageStep(this->width());
            
            // Show scrollbars if needed
            m_vScrollBar->setVisible(height > this->height());
            m_hScrollBar->setVisible(width > this->width());
            
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
        });
    
    connect(m_container, &container_qt5::anchorClicked, this, &litehtmlWidget::anchorClicked);
    
    // Setup timer for deferred updates
    m_updateTimer.setSingleShot(true);
    connect(&m_updateTimer, &QTimer::timeout, this, [this]() {
        if (m_needUpdate) {
            update();
            m_needUpdate = false;
        }
    });
    
    setMouseTracking(true);
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
container_qt5* litehtmlWidget::getContainer() const
{
    return m_container;
}

/**
 * Load HTML content
 * @param html The HTML content to load
 * @param baseUrl Base URL for relative paths
 */
void litehtmlWidget::loadHtml(const QString& html, const QString& baseUrl)
{
    m_container->set_base_url(baseUrl.toStdString().c_str());
    m_htmlDocument = litehtml::document::createFromString(html.toStdString().c_str(), m_container);
    
    if (m_htmlDocument) {
        // Reset scroll position
        m_scrollX = 0;
        m_scrollY = 0;
        m_vScrollBar->setValue(0);
        m_hScrollBar->setValue(0);
        
        // Force re-layout
        m_documentSizeSet = false;
        
        // Render document
        if (width() > 0) {
            m_htmlDocument->render(width());
        }
        
        update();
    }
}

/**
 * Load HTML content from a URL
 * @param url The URL to load the HTML content from
 */
void litehtmlWidget::loadUrl(const QString& url)
{
    QNetworkAccessManager* manager = new QNetworkAccessManager(this);
    QNetworkRequest request(url);
    
    QNetworkReply* reply = manager->get(request);
    
    connect(reply, &QNetworkReply::finished, this, [=]() {
        if (reply->error() == QNetworkReply::NoError) {
            QString content = QString::fromUtf8(reply->readAll());
            loadHtml(content, url);
        } else {
            qWarning() << "Failed to load URL:" << url << "Error:" << reply->errorString();
        }
        
        reply->deleteLater();
        manager->deleteLater();
    });
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
void litehtmlWidget::setScrollPosition(const QPoint& pos)
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
    return scrollPosition();
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
void litehtmlWidget::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);
    
    if (!m_htmlDocument) {
        return;
    }
    
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::TextAntialiasing, true);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
    
    // Set white background
    painter.fillRect(rect(), Qt::white);
    
    // Apply scroll position
    painter.translate(-m_scrollX, -m_scrollY);
    
    // Draw the HTML document
    litehtml::position clip(0, 0, width() + m_scrollX, height() + m_scrollY);
    m_container->draw(m_htmlDocument, &painter, 0, 0, &clip);
}

/**
 * Resize event handler
 * @param event Resize event
 */
void litehtmlWidget::resizeEvent(QResizeEvent* event)
{
    Q_UNUSED(event);
    
    if (m_htmlDocument) {
        // Re-render the document with the new width
        m_htmlDocument->render(width());
        
        // Update scrollbar positions
        if (m_documentSizeSet) {
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
    }
}

/**
 * Mouse press event handler
 * @param event Mouse event
 */
void litehtmlWidget::mousePressEvent(QMouseEvent* event)
{
    if (m_htmlDocument) {
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
void litehtmlWidget::mouseReleaseEvent(QMouseEvent* event)
{
    if (m_htmlDocument) {
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
void litehtmlWidget::mouseMoveEvent(QMouseEvent* event)
{
    if (m_htmlDocument) {
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
void litehtmlWidget::wheelEvent(QWheelEvent* event)
{
    if (m_vScrollBar->isVisible()) {
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

/**
 * Handler for link clicks
 * @param url The clicked link URL
 */
void litehtmlWidget::linkClicked(const QString& url)
{
    // Handle link click
    QDesktopServices::openUrl(QUrl(url));
}

/**
 * Handler for anchor clicks
 * @param url The clicked anchor URL
 */
void litehtmlWidget::anchorClicked(const QString& url)
{
    // Emit the linkClicked signal
    emit linkClicked(url);
} 