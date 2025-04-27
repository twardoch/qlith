// this_file: qlith-pro/include/qlith/litehtmlwidget.h
#pragma once

#include <QWidget>
#include <QString>
#include <QPoint>
#include <QSize>
#include <QScrollBar>
#include <QTimer>
#include <memory>

// Forward declarations
class container_qt5;
namespace litehtml {
    class document;
    typedef std::shared_ptr<document> document_ptr;
}

/**
 * A Qt widget that renders HTML using the litehtml library
 */
class litehtmlWidget : public QWidget
{
    Q_OBJECT

public:
    /**
     * Constructor
     * @param parent Parent widget
     */
    explicit litehtmlWidget(QWidget* parent = nullptr);

    /**
     * Destructor
     */
    ~litehtmlWidget();

    /**
     * Get the container object
     * @return Pointer to the container_qt5 instance
     */
    container_qt5* getContainer() const;

    /**
     * Load HTML content
     * @param html HTML content to load
     * @param baseUrl Base URL for resolving relative paths
     */
    void loadHtml(const QString& html, const QString& baseUrl = QString());

    /**
     * Load HTML content from a URL
     * @param url The URL to load the HTML content from
     */
    void loadUrl(const QString& url);

    /**
     * Set the scroll position
     * @param x New horizontal scroll position
     * @param y New vertical scroll position
     */
    void setScrollPos(int x, int y);

    /**
     * Set the scroll position
     * @param pos New scroll position
     */
    void setScrollPosition(const QPoint& pos);

    /**
     * Get the current scroll position
     * @return Current scroll position
     */
    QPoint scrollPosition() const;

    /**
     * Get the current scroll position
     * @return Current scroll position
     */
    QPoint scrollPos() const;

    /**
     * Get the size of the HTML document
     * @return Document size
     */
    QSize documentSize() const;

signals:
    /**
     * Emitted when a link is clicked
     * @param url URL of the clicked link
     */
    void linkClicked(const QString& url);

    /**
     * Emitted when the title changes
     * @param title New title
     */
    void titleChanged(const QString& title);

protected:
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;

private slots:
    void onDocSizeChanged(int w, int h);
    void anchorClicked(const QString& url);

private:
    container_qt5* m_container;
    litehtml::document_ptr m_htmlDocument;
    QScrollBar* m_vScrollBar;
    QScrollBar* m_hScrollBar;
    int m_scrollX;
    int m_scrollY;
    bool m_documentSizeSet;
    QSize m_documentSize;
    QTimer m_updateTimer;
    bool m_needUpdate;
}; 