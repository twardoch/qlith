#include "mainwindow.h"
#include "../src/qlithwidget.h"

#include <QApplication>
#include <QToolBar>
#include <QStatusBar>
#include <QLineEdit>
#include <QLabel>
#include <QComboBox>
#include <QFileDialog>
#include <QMessageBox>
#include <QSettings>
#include <QCloseEvent>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QMenuBar>
#include <QMenu>
#include <QPalette>
#include <QPainter>
#include <QSvgGenerator>
#include <QDebug>
#include <QStandardPaths>
#include <QDir>
#include <QDateTime>
#include <QTextStream>
#include <QFile>
#include <QProgressBar>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_htmlWidget(new QlithWidget(this))
    , m_urlEdit(nullptr)
    , m_progressBar(nullptr)
    , m_navigationToolBar(nullptr)
    , m_statusBar(nullptr)
    , m_historyIndex(-1)
    , m_zoomFactor(1.0f)
    , m_renderSize(800, 600) // Default render size
{
    setCentralWidget(m_htmlWidget);
    setMinimumSize(800, 600);
    showMaximized(); // Start with a maximized window

    setupUi();
    setupActions();
    setupMenus();
    setupToolbar();
    setupStatusBar();
    setupConnections();
    
    loadSettings();
    
    // Load default page
    load(QUrl("about:blank"));
}

void MainWindow::onLoadFinished(bool ok)
{
    // Update status bar
    if (m_statusBar) {
        m_statusBar->showMessage(tr("Ready"));
    }
    
    // Update window title
    updateTitle(QString());
    
    qDebug() << "MainWindow::onLoadFinished called with status:" << (ok ? "success" : "failure");
    
    // Additional debug functionality can be re-implemented if needed
    
    // Emit loadFinished signal
    emit loadFinished(ok);
}

MainWindow::~MainWindow()
{
}

void MainWindow::loadUrl()
{
    if (!m_urlEdit)
        return;
        
    QString text = m_urlEdit->text().trimmed();
    if (text.isEmpty())
        return;
        
    QUrl url = QUrl::fromUserInput(text);
    load(url);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    saveSettings();
    event->accept();
}

void MainWindow::updateUrlBar(const QUrl& url)
{
    if (m_urlEdit) {
        m_urlEdit->setText(url.toString());
    }
}

void MainWindow::reload()
{
    if (m_historyIndex >= 0 && m_historyIndex < m_history.size()) {
        QUrl currentUrl = m_history.at(m_historyIndex);
        m_htmlWidget->load(currentUrl);
    }
}

void MainWindow::goBack()
{
    if (m_historyIndex > 0) {
        m_historyIndex--;
        QUrl url = m_history.at(m_historyIndex);
        
        if (m_urlEdit) {
            m_urlEdit->setText(url.toString());
        }
        
        m_htmlWidget->load(url);
        
        // Update UI
        if (m_backAction && m_forwardAction) {
            m_backAction->setEnabled(m_historyIndex > 0);
            m_forwardAction->setEnabled(m_historyIndex < m_history.size() - 1);
        }
        
        updateTitle(QString());
    }
}

void MainWindow::goForward()
{
    if (m_historyIndex < m_history.size() - 1) {
        m_historyIndex++;
        QUrl url = m_history.at(m_historyIndex);
        
        if (m_urlEdit) {
            m_urlEdit->setText(url.toString());
        }
        
        m_htmlWidget->load(url);
        
        // Update UI
        if (m_backAction && m_forwardAction) {
            m_backAction->setEnabled(m_historyIndex > 0);
            m_forwardAction->setEnabled(m_historyIndex < m_history.size() - 1);
        }
        
        updateTitle(QString());
    }
}

void MainWindow::zoomIn()
{
    // Implement zoom functionality
}

void MainWindow::zoomOut()
{
    // Implement zoom functionality
}

void MainWindow::resetZoom()
{
    // Implement zoom functionality
}

void MainWindow::about()
{
    QMessageBox::about(this, tr("About Qlith Browser"),
                       tr("A simple HTML browser demo using the Qlith library."));
}

void MainWindow::handleLinkClick(const QUrl &url)
{
    // Handle clicked links - navigate to them
    load(url);
}

void MainWindow::updateTitle(const QString &title)
{
    QString windowTitle;
    
    if (!title.isEmpty()) {
        windowTitle = title;
    } else if (m_historyIndex >= 0 && m_historyIndex < m_history.size()) {
        windowTitle = m_history.at(m_historyIndex).toString();
    } else {
        windowTitle = tr("Qlith Browser");
    }
    
    // Add browser name if not already there
    if (!windowTitle.contains("Qlith Browser")) {
        windowTitle = tr("%1 - Qlith Browser").arg(windowTitle);
    }
    
    setWindowTitle(windowTitle);
}

void MainWindow::onLoadStarted()
{
    // Update status bar
    if (m_statusBar) {
        m_statusBar->showMessage(tr("Loading..."));
    }
}

void MainWindow::setupConnections()
{
    // Connect QlithWidget signals
    connect(m_htmlWidget, &QlithWidget::loadStarted, this, &MainWindow::onLoadStarted);
    connect(m_htmlWidget, &QlithWidget::loadFinished, this, &MainWindow::onLoadFinished);
    connect(m_htmlWidget, &QlithWidget::titleChanged, this, &MainWindow::updateTitle);
    connect(m_htmlWidget, &QlithWidget::linkClicked, this, &MainWindow::handleLinkClick);
    
    // Connect URL bar
    if (m_urlEdit) {
        connect(m_urlEdit, &QLineEdit::returnPressed, this, &MainWindow::loadUrl);
    }
    
    // Connect actions
    if (m_backAction) {
        connect(m_backAction, &QAction::triggered, this, &MainWindow::goBack);
    }
    
    if (m_forwardAction) {
        connect(m_forwardAction, &QAction::triggered, this, &MainWindow::goForward);
    }
    
    if (m_reloadAction) {
        connect(m_reloadAction, &QAction::triggered, this, &MainWindow::reload);
    }
    
    if (m_stopAction) {
        connect(m_stopAction, &QAction::triggered, this, &MainWindow::stop);
    }
}

void MainWindow::load(const QUrl& url)
{
    if (!url.isValid())
        return;
        
    if (m_urlEdit) {
        m_urlEdit->setText(url.toString());
    }
    
    m_htmlWidget->load(url);
    
    // Add to history
    if (m_historyIndex >= 0 && m_historyIndex < m_history.size() - 1) {
        // Remove forward history if we navigate from a point in history
        m_history.erase(m_history.begin() + m_historyIndex + 1, m_history.end());
    }
    
    m_history.append(url);
    // Limit history size
    while (m_history.size() > 100) {
        m_history.removeFirst();
    }
    m_historyIndex = m_history.size() - 1;
    
    // Update UI
    if (m_backAction && m_forwardAction) {
        m_backAction->setEnabled(m_historyIndex > 0);
        m_forwardAction->setEnabled(m_historyIndex < m_history.size() - 1);
    }
}

void MainWindow::stop()
{
    if (m_htmlWidget) {
        m_htmlWidget->stop();
    }
    
    // Update status bar
    if (m_statusBar) {
        m_statusBar->showMessage(tr("Stopped"));
    }
}

void MainWindow::setupUi()
{
    // Main widget is already set in constructor
    // Additional UI setup if needed
}

void MainWindow::setupActions()
{
    m_backAction = new QAction(tr("Back"), this);
    m_backAction->setIcon(QIcon::fromTheme("go-previous"));
    m_backAction->setShortcut(QKeySequence::Back);
    m_backAction->setToolTip(tr("Go back to previous page"));
    
    m_forwardAction = new QAction(tr("Forward"), this);
    m_forwardAction->setIcon(QIcon::fromTheme("go-next"));
    m_forwardAction->setShortcut(QKeySequence::Forward);
    m_forwardAction->setToolTip(tr("Go forward to next page"));
    
    m_reloadAction = new QAction(tr("Reload"), this);
    m_reloadAction->setIcon(QIcon::fromTheme("view-refresh"));
    m_reloadAction->setShortcut(QKeySequence::Refresh);
    m_reloadAction->setToolTip(tr("Reload current page"));
    
    m_stopAction = new QAction(tr("Stop"), this);
    m_stopAction->setIcon(QIcon::fromTheme("process-stop"));
    m_stopAction->setShortcut(Qt::Key_Escape);
    m_stopAction->setToolTip(tr("Stop loading page"));
    
    m_viewSourceAction = new QAction(tr("View Source"), this);
    m_viewSourceAction->setToolTip(tr("View page source"));
    
    m_zoomInAction = new QAction(tr("Zoom In"), this);
    m_zoomInAction->setIcon(QIcon::fromTheme("zoom-in"));
    m_zoomInAction->setShortcut(QKeySequence::ZoomIn);
    
    m_zoomOutAction = new QAction(tr("Zoom Out"), this);
    m_zoomOutAction->setIcon(QIcon::fromTheme("zoom-out"));
    m_zoomOutAction->setShortcut(QKeySequence::ZoomOut);
    
    m_resetZoomAction = new QAction(tr("Reset Zoom"), this);
    m_resetZoomAction->setIcon(QIcon::fromTheme("zoom-original"));
    m_resetZoomAction->setShortcut(QKeySequence(tr("Ctrl+0")));
    
    m_exitAction = new QAction(tr("Exit"), this);
    m_exitAction->setShortcut(QKeySequence::Quit);
    connect(m_exitAction, &QAction::triggered, this, &QWidget::close);
    
    m_aboutAction = new QAction(tr("About"), this);
    connect(m_aboutAction, &QAction::triggered, this, &MainWindow::about);
    
    // Update initial state
    m_backAction->setEnabled(false);
    m_forwardAction->setEnabled(false);
}

void MainWindow::setupMenus()
{
    m_fileMenu = menuBar()->addMenu(tr("&File"));
    m_fileMenu->addAction(m_exitAction);
    
    m_viewMenu = menuBar()->addMenu(tr("&View"));
    m_viewMenu->addAction(m_backAction);
    m_viewMenu->addAction(m_forwardAction);
    m_viewMenu->addAction(m_reloadAction);
    m_viewMenu->addAction(m_stopAction);
    m_viewMenu->addSeparator();
    m_viewMenu->addAction(m_zoomInAction);
    m_viewMenu->addAction(m_zoomOutAction);
    m_viewMenu->addAction(m_resetZoomAction);
    m_viewMenu->addSeparator();
    m_viewMenu->addAction(m_viewSourceAction);
    
    m_helpMenu = menuBar()->addMenu(tr("&Help"));
    m_helpMenu->addAction(m_aboutAction);
}

void MainWindow::setupToolbar()
{
    m_navigationToolBar = addToolBar(tr("Navigation"));
    m_navigationToolBar->addAction(m_backAction);
    m_navigationToolBar->addAction(m_forwardAction);
    m_navigationToolBar->addAction(m_reloadAction);
    m_navigationToolBar->addAction(m_stopAction);
    
    // Add URL bar
    m_urlEdit = new QLineEdit(this);
    m_urlEdit->setClearButtonEnabled(true);
    m_urlEdit->setPlaceholderText(tr("Enter URL..."));
    m_navigationToolBar->addWidget(m_urlEdit);
}

void MainWindow::setupStatusBar()
{
    m_statusBar = statusBar();
    m_statusBar->showMessage(tr("Ready"));
    
    // Add progress bar
    m_progressBar = new QProgressBar(this);
    m_progressBar->setMaximumWidth(150);
    m_progressBar->setMaximumHeight(16);
    m_progressBar->setVisible(false);
    m_statusBar->addPermanentWidget(m_progressBar);
}

void MainWindow::loadSettings()
{
    QSettings settings;
    
    // Window geometry
    const QByteArray geometry = settings.value("geometry", QByteArray()).toByteArray();
    if (!geometry.isEmpty()) {
        restoreGeometry(geometry);
    }
    
    // Start URL
    m_startUrl = settings.value("startUrl", QUrl("about:blank")).toUrl();
    if (m_startUrl.isValid()) {
        load(m_startUrl);
    }
}

void MainWindow::saveSettings()
{
    QSettings settings;
    
    // Window geometry
    settings.setValue("geometry", saveGeometry());
    
    // Current URL
    if (m_historyIndex >= 0 && m_historyIndex < m_history.size()) {
        settings.setValue("startUrl", m_history.at(m_historyIndex));
    }
}

void MainWindow::viewSource()
{
    // Simple implementation to view the HTML source
    if (m_historyIndex >= 0 && m_historyIndex < m_history.size()) {
        // Get the HTML from the current page
        // This would require access to the raw HTML content
        QMessageBox::information(this, tr("View Source"), 
                              tr("View source not implemented yet."));
    }
}

void MainWindow::setRenderSize(const QSize& size)
{
    qDebug() << "Setting render size to:" << size;
    m_renderSize = size;
}

bool MainWindow::exportToSvg(const QString& filePath)
{
    if (!m_htmlWidget) {
        return false;
    }
    
    // Create directory if it doesn't exist
    QFileInfo fileInfo(filePath);
    QDir dir = fileInfo.dir();
    if (!dir.exists()) {
        if (!dir.mkpath(".")) {
            qWarning() << "Failed to create directory for SVG export:" << dir.path();
            return false;
        }
    }
    
    // Create SVG generator
    QSvgGenerator generator;
    generator.setFileName(filePath);
    
    // Get sizes - in qlith-mini we need to get document size directly from the document
    QSize widgetSize = m_htmlWidget->size();
    QSize documentSize;
    
    // Get document size from litehtml document if available
    auto doc = m_htmlWidget->document();
    if (doc) {
        documentSize = QSize(doc->width(), doc->height());
    }
    
    qDebug() << "SVG Export - Widget size:" << widgetSize;
    qDebug() << "SVG Export - Document size:" << documentSize;
    qDebug() << "SVG Export - Requested render size:" << m_renderSize;
    
    // Use custom render size if set, or document size, or widget size as fallback
    QSize exportSize;
    if (m_renderSize.isValid()) {
        exportSize = m_renderSize;
    } else if (documentSize.isValid() && documentSize.width() > 0 && documentSize.height() > 0) {
        exportSize = documentSize;
    } else {
        exportSize = widgetSize;
    }
    
    qDebug() << "SVG Export - Using export size:" << exportSize;
    
    generator.setSize(exportSize);
    generator.setViewBox(QRect(0, 0, exportSize.width(), exportSize.height()));
    generator.setTitle(windowTitle());
    generator.setDescription(tr("Generated by Qlith Browser"));
    
    // Render to SVG
    QPainter painter;
    if (!painter.begin(&generator)) {
        qWarning() << "Failed to start painting to SVG file:" << filePath;
        return false;
    }
    
    // Apply appropriate scaling
    if (documentSize.isValid() && documentSize.width() > 0 && documentSize.height() > 0) {
        // Scale based on document size while maintaining aspect ratio
        double scaleX = (double)exportSize.width() / documentSize.width();
        double scaleY = (double)exportSize.height() / documentSize.height();
        
        // Use the minimum scale factor to fit while preserving aspect ratio
        double scale = qMin(scaleX, scaleY);
        painter.scale(scale, scale);
        
        // Center the content
        if (scaleX > scaleY) {
            double offsetX = (exportSize.width() - documentSize.width() * scale) / (2 * scale);
            painter.translate(offsetX, 0);
        } else {
            double offsetY = (exportSize.height() - documentSize.height() * scale) / (2 * scale);
            painter.translate(0, offsetY);
        }
        
        qDebug() << "SVG Export - Applied scale:" << scale << "with centering";
    }
    
    // Let the widget render into the painter
    m_htmlWidget->render(&painter);
    
    painter.end();
    
    qInfo() << "Exported SVG to:" << filePath;
    return true;
}

bool MainWindow::exportToPng(const QString& filePath)
{
    if (!m_htmlWidget) {
        qWarning() << "exportToPng: HTML widget is null";
        return false;
    }
    
    qDebug() << "exportToPng: Starting PNG export to" << filePath;
    
    // Get sizes - in qlith-mini we need to get document size directly from the document
    QSize widgetSize = m_htmlWidget->size();
    QSize documentSize;
    
    // Get document size from litehtml document if available
    auto doc = m_htmlWidget->document();
    if (doc) {
        documentSize = QSize(doc->width(), doc->height());
    }
    
    qDebug() << "PNG Export - Widget size:" << widgetSize;
    qDebug() << "PNG Export - Document size:" << documentSize;
    qDebug() << "PNG Export - Requested render size:" << m_renderSize;
    
    // Create directory if it doesn't exist
    QFileInfo fileInfo(filePath);
    QDir dir = fileInfo.dir();
    if (!dir.exists()) {
        if (!dir.mkpath(".")) {
            qWarning() << "Failed to create directory for PNG export:" << dir.path();
            return false;
        }
    }
    
    // Determine the export size
    QSize exportSize;
    if (m_renderSize.isValid()) {
        exportSize = m_renderSize;
    } else if (documentSize.isValid() && documentSize.width() > 0 && documentSize.height() > 0) {
        exportSize = documentSize;
    } else if (widgetSize.isValid() && widgetSize.width() > 0 && widgetSize.height() > 0) {
        exportSize = widgetSize;
    } else {
        exportSize = QSize(800, 600); // Default size if all else fails
    }
    
    qDebug() << "PNG Export - Using export size:" << exportSize;
    
    // Create a pixmap with the export size
    QPixmap pixmap(exportSize);
    pixmap.fill(Qt::white);
    
    // Create a painter for the pixmap
    QPainter painter(&pixmap);
    
    // Apply appropriate scaling
    if (documentSize.isValid() && documentSize.width() > 0 && documentSize.height() > 0) {
        // Scale based on document size while maintaining aspect ratio
        double scaleX = (double)exportSize.width() / documentSize.width();
        double scaleY = (double)exportSize.height() / documentSize.height();
        
        // Use the minimum scale factor to fit while preserving aspect ratio
        double scale = qMin(scaleX, scaleY);
        painter.scale(scale, scale);
        
        // Center the content
        if (scaleX > scaleY) {
            double offsetX = (exportSize.width() - documentSize.width() * scale) / (2 * scale);
            painter.translate(offsetX, 0);
        } else {
            double offsetY = (exportSize.height() - documentSize.height() * scale) / (2 * scale);
            painter.translate(0, offsetY);
        }
        
        qDebug() << "PNG Export - Applied scale:" << scale << "with centering";
    }
    
    // Let the widget render into the painter
    qDebug() << "Rendering widget to pixmap";
    m_htmlWidget->render(&painter);
    painter.end();
    
    // Save to PNG
    qDebug() << "Saving pixmap to PNG file:" << filePath;
    bool result = pixmap.save(filePath, "PNG");
    if (result) {
        qInfo() << "Successfully exported PNG to:" << filePath;
    } else {
        qWarning() << "Failed to save PNG to:" << filePath;
    }
    
    return result;
} 