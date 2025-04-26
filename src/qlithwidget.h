// this_file: qlith/src/qlithwidget.h
#ifndef QLITHWIDGET_H
#define QLITHWIDGET_H

#include <QWidget>
#include <QUrl>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QTimer>
#include <QColor>
#include <memory>

#include <litehtml.h>
#include "qlith_global.h"

class ContainerQPainter;
class QlithWidgetPrivate;

/**
 * @brief The QlithWidget class provides a Qt widget for displaying HTML content using litehtml.
 * 
 * QlithWidget renders HTML and CSS using the litehtml library, providing a lightweight HTML
 * rendering capability within Qt applications.
 */
class QLITH_EXPORT QlithWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QUrl baseUrl READ baseUrl WRITE setBaseUrl NOTIFY baseUrlChanged)
    Q_PROPERTY(QColor backgroundColor READ backgroundColor WRITE setBackgroundColor NOTIFY backgroundColorChanged)

public:
    /**
     * @brief Constructs a QlithWidget.
     * @param parent The parent widget.
     */
    explicit QlithWidget(QWidget* parent = nullptr);
    
    /**
     * @brief Destroys the QlithWidget.
     */
    ~QlithWidget() override;

    /**
     * @brief Load HTML content from a string.
     * @param html The HTML content to load.
     */
    void setHtml(const QString& html);
    
    /**
     * @brief Load HTML content from a URL.
     * @param url The URL to load.
     */
    void load(const QUrl& url);
    
    /**
     * @brief Set the base URL for resolving relative URLs.
     * @param url The base URL.
     */
    void setBaseUrl(const QUrl& url);
    
    /**
     * @brief Get the current base URL.
     * @return The base URL.
     */
    QUrl baseUrl() const;
    
    /**
     * @brief Set the background color of the widget.
     * @param color The background color.
     */
    void setBackgroundColor(const QColor& color);
    
    /**
     * @brief Get the current background color.
     * @return The background color.
     */
    QColor backgroundColor() const;
    
    /**
     * @brief Set the default CSS stylesheet to use.
     * @param css The CSS stylesheet.
     */
    void setDefaultStyleSheet(const QString& css);
    
    /**
     * @brief Get the document title.
     * @return The title of the HTML document.
     */
    QString documentTitle() const;
    
    /**
     * @brief Get the litehtml document object.
     * @return A shared pointer to the litehtml document.
     */
    std::shared_ptr<litehtml::document> document() const;

public slots:
    /**
     * @brief Reload the current content.
     */
    void reload();
    
    /**
     * @brief Stop loading the current content.
     */
    void stop();

signals:
    /**
     * @brief Signal emitted when the base URL changes.
     * @param url The new base URL.
     */
    void baseUrlChanged(const QUrl& url);
    
    /**
     * @brief Signal emitted when the background color changes.
     * @param color The new background color.
     */
    void backgroundColorChanged(const QColor& color);
    
    /**
     * @brief Signal emitted when loading of content starts.
     */
    void loadStarted();
    
    /**
     * @brief Signal emitted when loading of content finishes.
     */
    void loadFinished(bool ok);
    
    /**
     * @brief Signal emitted when the document title changes.
     * @param title The new document title.
     */
    void titleChanged(const QString& title);
    
    /**
     * @brief Signal emitted when a link is clicked.
     * @param url The URL of the clicked link.
     */
    void linkClicked(const QUrl& url);

protected:
    // Reimplemented widget events
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;

private:
    QScopedPointer<QlithWidgetPrivate> d_ptr;
    Q_DECLARE_PRIVATE(QlithWidget)
};

#endif // QLITHWIDGET_H 