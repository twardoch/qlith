#ifndef QLITEHTMLWIDGET_H
#define QLITEHTMLWIDGET_H

#include "qlitehtml_global.h"

#include <QAbstractScrollArea>
#include <QUrl>

class QNetworkReply;
class QLiteHtmlWidgetPrivate;

class QLITEHTML_EXPORT QLiteHtmlWidget : public QAbstractScrollArea
{
    Q_OBJECT
public:
    explicit QLiteHtmlWidget(QWidget *parent = nullptr);
    ~QLiteHtmlWidget() override;

    void setHtml(const QString &html, const QUrl &baseUrl = QUrl());
    QString documentTitle() const;
    void load(const QUrl &url);
    QUrl url() const;
    
    void setZoom(qreal zoom);
    qreal zoom() const;
    void zoomIn();
    void zoomOut();
    void resetZoom();
    
    QString selectedText() const;
    void loadFile(const QString &filePath);
    
    // Debug method to save the rendered document as an image
    void saveDebugImage(const QString &filename);

    // New render method for screenshot and export functions
    void render(QPainter *painter);
    
signals:
    void htmlLoaded();
    void zoomChanged(qreal zoom);
    void linkClicked(const QUrl &url);
    void loadFinished();

protected:
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

public slots:
    void resourceNetworkReplyFinished(QNetworkReply *reply);

private:
    QLiteHtmlWidgetPrivate *d;
};

#endif // QLITEHTMLWIDGET_H 