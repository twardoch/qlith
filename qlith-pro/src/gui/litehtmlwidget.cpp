#include "litehtmlwidget.h"
#include <QPainter>
#include <QMouseEvent>
#include <QResizeEvent>
#include <QWheelEvent>
#include <QApplication>
#include <QDebug>

/**
 * Constructor for litehtmlWidget
 * @param parent Parent widget
 */
litehtmlWidget::litehtmlWidget(QWidget* parent)
    : QWidget(parent)
    , m_container(new container_qt5(this))
    , m_needsLayout(false)
    , m_scrollPos(0, 0)
    , m_docSize(0, 0)
    , m_isLoading(false)
{
    // Set attributes
    setAttribute(Qt::WA_OpaquePaintEvent);
    setFocusPolicy(Qt::StrongFocus);
    setMouseTracking(true);
    
    // Connect signals
    connect(m_container, &container_qt5::docSizeChanged, this, &litehtmlWidget::onDocSizeChanged);
    connect(m_container, &container_qt5::anchorClicked, this, &litehtmlWidget::linkClicked);
    connect(m_container, &container_qt5::cursorChanged, this, [this](const QString& cursor) {
        if (cursor == "pointer") {
            setCursor(Qt::PointingHandCursor);
        } else if (cursor == "text") {
            setCursor(Qt::IBeamCursor);
        } else {
            setCursor(Qt::ArrowCursor);
        }
    });
}

/**
 * Destructor
 */
litehtmlWidget::~litehtmlWidget()
{
    delete m_container;
}

/**
 * Get the container object used for rendering
 * @return The container_qt5 instance
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
    m_isLoading = true;
    
    // Create document context
    litehtml::context ctx;
    
    // Create new document with the HTML content
    m_container->_doc = litehtml::document::createFromString(
        html.toUtf8().constData(),
        m_container,
        &ctx
    );
    
    // Set base URL if provided
    if (!baseUrl.isEmpty()) {
        m_container->set_base_url(baseUrl.toUtf8().constData());
    }
    
    // Reset scroll position
    setScrollPosition(QPoint(0, 0));
    
    // Mark for layout
    m_needsLayout = true;
    m_isLoading = false;
    update();
}

/**
 * Set the scroll position
 * @param pos The new scroll position
 */
void litehtmlWidget::setScrollPosition(const QPoint& pos)
{
    // Ensure we don't scroll past document boundaries
    QPoint newPos = pos;
    
    // Don't allow negative scrolling
    if (newPos.x() < 0) newPos.setX(0);
    if (newPos.y() < 0) newPos.setY(0);
    
    // Don't scroll past document edges
    if (m_container->_doc) {
        int maxX = qMax(0, m_docSize.width() - width());
        int maxY = qMax(0, m_docSize.height() - height());
        
        if (newPos.x() > maxX) newPos.setX(maxX);
        if (newPos.y() > maxY) newPos.setY(maxY);
    }
    
    // Update scroll position if changed
    if (m_scrollPos != newPos) {
        m_scrollPos = newPos;
        m_container->setScroll(m_scrollPos);
        update();
    }
}

/**
 * Get the current scroll position
 * @return Current scroll position
 */
QPoint litehtmlWidget::scrollPosition() const
{
    return m_scrollPos;
}

/**
 * Get the size of the document
 * @return Document size
 */
QSize litehtmlWidget::documentSize() const
{
    return m_docSize;
}

/**
 * Paint event handler
 * @param event Paint event
 */
void litehtmlWidget::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);
    
    // Fill background
    painter.fillRect(rect(), Qt::white);
    
    // If no document, just return
    if (!m_container->_doc) {
        return;
    }
    
    // Perform layout if needed
    if (m_needsLayout) {
        m_container->_doc->render(width());
        m_needsLayout = false;
    }
    
    // Render the document
    m_container->repaint(painter);
}

/**
 * Mouse move event handler
 * @param event Mouse event
 */
void litehtmlWidget::mouseMoveEvent(QMouseEvent* event)
{
    if (m_container->_doc) {
        // Update mouse position
        m_container->setLastMouseCoords(
            event->x() + m_scrollPos.x(),
            event->y() + m_scrollPos.y(),
            event->x(),
            event->y()
        );
        
        // Trigger hover event
        m_container->_doc->on_mouse_over(
            event->x() + m_scrollPos.x(),
            event->y() + m_scrollPos.y(),
            event->x(),
            event->y(),
            m_container->elementUnderCursor()
        );
        
        // Update the view
        update();
    }
}

/**
 * Mouse press event handler
 * @param event Mouse event
 */
void litehtmlWidget::mousePressEvent(QMouseEvent* event)
{
    if (m_container->_doc && event->button() == Qt::LeftButton) {
        // Update mouse position
        m_container->setLastMouseCoords(
            event->x() + m_scrollPos.x(),
            event->y() + m_scrollPos.y(),
            event->x(),
            event->y()
        );
        
        // Trigger click event
        m_container->_doc->on_lbutton_down(
            event->x() + m_scrollPos.x(),
            event->y() + m_scrollPos.y(),
            event->x(),
            event->y(),
            m_container->elementUnderCursor()
        );
        
        // Update the view
        update();
    }
}

/**
 * Mouse release event handler
 * @param event Mouse event
 */
void litehtmlWidget::mouseReleaseEvent(QMouseEvent* event)
{
    if (m_container->_doc && event->button() == Qt::LeftButton) {
        // Update mouse position
        m_container->setLastMouseCoords(
            event->x() + m_scrollPos.x(),
            event->y() + m_scrollPos.y(),
            event->x(),
            event->y()
        );
        
        // Trigger click event
        m_container->_doc->on_lbutton_up(
            event->x() + m_scrollPos.x(),
            event->y() + m_scrollPos.y(),
            event->x(),
            event->y(),
            m_container->elementUnderCursor()
        );
        
        // Update the view
        update();
    }
}

/**
 * Resize event handler
 * @param event Resize event
 */
void litehtmlWidget::resizeEvent(QResizeEvent* event)
{
    if (m_container->_doc) {
        // Mark layout as needing update
        m_needsLayout = true;
        
        // Adjust scroll position if needed
        setScrollPosition(m_scrollPos);
        
        update();
    }
}

/**
 * Wheel event handler for scrolling
 * @param event Wheel event
 */
void litehtmlWidget::wheelEvent(QWheelEvent* event)
{
    if (m_container->_doc) {
        // Calculate scroll delta based on wheel delta
        int delta = event->angleDelta().y() / 8;
        
        // Create new scroll position
        QPoint newPos = m_scrollPos;
        
        if (event->modifiers() & Qt::ShiftModifier) {
            // Horizontal scroll if Shift is pressed
            newPos.setX(newPos.x() - delta);
        } else {
            // Vertical scroll by default
            newPos.setY(newPos.y() - delta);
        }
        
        // Apply scroll position
        setScrollPosition(newPos);
    }
}

/**
 * Handler for document size changes
 * @param w New width
 * @param h New height
 */
void litehtmlWidget::onDocSizeChanged(int w, int h)
{
    m_docSize = QSize(w, h);
    
    // Adjust scrollbars if widget is in a scroll area
    updateGeometry();
    
    // Ensure current scroll position is still valid
    setScrollPosition(m_scrollPos);
} 