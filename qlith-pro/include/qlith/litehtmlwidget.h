// this_file: qlith-pro/include/qlith/litehtmlwidget.h
#pragma once

#include <QWidget>
#include <QPoint>
#include <QSize>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QResizeEvent>
#include <QWheelEvent>
#include <QScrollBar>
#include <QTimer>
#include <QColor>

#include "litehtml.h"
#include "qlith/container_qt5.h"

/**
 * @brief Widget for displaying HTML content using litehtml
 */
class litehtmlWidget : public QWidget
{
    Q_OBJECT

public:
    /**
     * @brief Constructor
     * @param parent Parent widget
     */
    explicit litehtmlWidget(QWidget* parent = nullptr);
    
    /**
     * @brief Destructor
     */
    ~litehtmlWidget();
    
    /**
     * @brief Get the container object
     * @return Pointer to the container_qt5 instance
     */
    container_qt5* getContainer() const;
    
    /**
     * @brief Load HTML content
     * @param html HTML content
     * @param baseUrl Base URL for resolving relative paths
     */
    void loadHtml(const QString& html, const QString& baseUrl = QString());
    
    /**
     * @brief Load URL content
     * @param url URL of the content
     */
    void loadUrl(const QString& url);
    
    /**
     * @brief Set the scroll position
     * @param x X position
     * @param y Y position
     */
    void setScrollPos(int x, int y);
    
    /**
     * @brief Set the scroll position
     * @param pos New scroll position
     */
    void setScrollPosition(const QPoint& pos);
    
    /**
     * @brief Get current scroll position
     * @return Current scroll position
     */
    QPoint scrollPosition() const;
    
    /**
     * @brief Get current scroll position
     * @return Current scroll position
     */
    QPoint scrollPos() const;
    
    /**
     * @brief Get document size
     * @return Size of the document
     */
    QSize documentSize() const;

signals:
    /**
     * @brief Signal emitted when the title changes
     * @param title New title
     */
    void titleChanged(const QString& title);
    
    /**
     * @brief Signal emitted when an anchor is clicked
     * @param url URL of the clicked anchor
     */
    void anchorClicked(const QString& url);
    
    /**
     * @brief Signal emitted when document is loaded
     * @param success True if loading was successful
     */
    void documentLoaded(bool success);

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

private:
    /**
     * @brief Create scrollbars for the widget
     */
    void createScrollBars();
    
    /**
     * @brief Update document size from container
     */
    void updateDocumentSize(int w, int h);
    
    /**
     * @brief Handle document size changes
     */
    void onDocSizeChanged(int w, int h);

private:
    container_qt5* m_container;     ///< HTML container implementation
    litehtml::document::ptr m_htmlDocument;
    QScrollBar* m_vScrollBar;       ///< Vertical scroll bar
    QScrollBar* m_hScrollBar;       ///< Horizontal scroll bar
    int m_scrollX;                  ///< X scroll position
    int m_scrollY;                  ///< Y scroll position
    QColor m_backgroundColor;       ///< Background color
    bool m_documentSizeSet;         ///< Flag indicating if document size is set
    QSize m_documentSize;           ///< Size of the document
}; 