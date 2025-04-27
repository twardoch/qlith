// this_file: qlith-pro/src/gui/litehtmlwidget.h
#pragma once

#include <QWidget>
#include <QPoint>
#include <QSize>

class container_qt5;

/**
 * @brief Widget for rendering HTML content using litehtml
 * 
 * The litehtmlWidget provides a QWidget-based component for rendering HTML 
 * using the litehtml library. It handles all necessary events and provides
 * methods for loading HTML content and controlling the view.
 */
class litehtmlWidget : public QWidget
{
    Q_OBJECT

public:
    /**
     * @brief Construct a new litehtmlWidget
     * @param parent The parent widget
     */
    explicit litehtmlWidget(QWidget* parent = nullptr);
    
    /**
     * @brief Destroy the litehtmlWidget
     */
    virtual ~litehtmlWidget();
    
    /**
     * @brief Get the litehtml container object
     * @return Pointer to the container_qt5 instance
     */
    container_qt5* getContainer() const;
    
    /**
     * @brief Load HTML content into the widget
     * @param html The HTML content to load
     * @param baseUrl The base URL for resolving relative links
     */
    void loadHtml(const QString& html, const QString& baseUrl = QString());
    
    /**
     * @brief Set the scroll position of the view
     * @param pos The new scroll position
     */
    void setScrollPosition(const QPoint& pos);
    
    /**
     * @brief Get the current scroll position
     * @return Current scroll position
     */
    QPoint scrollPosition() const;
    
    /**
     * @brief Get the size of the document
     * @return Size of the document
     */
    QSize documentSize() const;

protected:
    /**
     * @brief Handle paint events
     * @param event The paint event
     */
    void paintEvent(QPaintEvent* event) override;
    
    /**
     * @brief Handle resize events
     * @param event The resize event
     */
    void resizeEvent(QResizeEvent* event) override;
    
    /**
     * @brief Handle mouse press events
     * @param event The mouse event
     */
    void mousePressEvent(QMouseEvent* event) override;
    
    /**
     * @brief Handle mouse release events
     * @param event The mouse event
     */
    void mouseReleaseEvent(QMouseEvent* event) override;
    
    /**
     * @brief Handle mouse move events
     * @param event The mouse event
     */
    void mouseMoveEvent(QMouseEvent* event) override;
    
    /**
     * @brief Handle mouse wheel events
     * @param event The wheel event
     */
    void wheelEvent(QWheelEvent* event) override;

private slots:
    /**
     * @brief Handle document size changes
     * @param width The new document width
     * @param height The new document height
     */
    void onDocSizeChanged(int width, int height);

private:
    container_qt5* m_container;    ///< HTML container implementation
    bool m_needsLayout;            ///< Flag indicating if layout is needed
    QPoint m_scrollPos;            ///< Current scroll position
    QSize m_docSize;               ///< Size of the document
    bool m_isLoading;              ///< Flag indicating if content is loading
}; 