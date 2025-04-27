// this_file: qlith-pro/src/gui/litehtmlwidget.h
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
    container_qt5 m_container;    ///< HTML container implementation
    litehtml::document::ptr m_htmlDocument;
    QScrollBar* m_vScrollBar;       ///< Vertical scroll bar
    QScrollBar* m_hScrollBar;       ///< Horizontal scroll bar
    int m_scrollX;                  ///< X scroll position
    int m_scrollY;                  ///< Y scroll position
    bool m_documentSizeSet;          ///< Flag indicating if document size is set
    QSize m_documentSize;           ///< Size of the document
    QTimer m_updateTimer;           ///< Timer for updating the widget
    bool m_needUpdate;              ///< Flag indicating if an update is needed
}; 