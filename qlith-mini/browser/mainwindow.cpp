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

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_htmlWidget(new QlithWidget(this))
    , m_urlEdit(nullptr)
    , m_progressBar(nullptr)
    , m_navigationToolBar(nullptr)
    , m_statusBar(nullptr)
    , m_historyIndex(-1)
    , m_zoomFactor(1.0f)
{
    setCentralWidget(m_htmlWidget);
    setMinimumSize(800, 600);

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
    
    // Additional debug functionality can be re-implemented if needed
    
    // No need to emit loadFinished as we're responding to it
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

void MainWindow::createActions()
{
    // File menu
    QMenu *fileMenu = menuBar()->addMenu(tr("&File"));
    
    m_openAction = fileMenu->addAction(tr("&Open..."), this, &MainWindow::openFile);
    m_openAction->setShortcut(QKeySequence::Open);
    
    fileMenu->addSeparator();
    
    m_exitAction = fileMenu->addAction(tr("E&xit"), this, &QWidget::close);
    m_exitAction->setShortcut(QKeySequence::Quit);
    
    // View menu
    QMenu *viewMenu = menuBar()->addMenu(tr("&View"));
    
    m_zoomInAction = viewMenu->addAction(tr("Zoom &In"), this, &MainWindow::zoomIn);
    m_zoomInAction->setShortcut(QKeySequence::ZoomIn);
    
    m_zoomOutAction = viewMenu->addAction(tr("Zoom &Out"), this, &MainWindow::zoomOut);
    m_zoomOutAction->setShortcut(QKeySequence::ZoomOut);
    
    m_resetZoomAction = viewMenu->addAction(tr("&Reset Zoom"), this, &MainWindow::resetZoom);
    m_resetZoomAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_0));
    
    viewMenu->addSeparator();
    
    m_darkModeAction = viewMenu->addAction(tr("&Dark Mode"));
    m_darkModeAction->setCheckable(true);
    connect(m_darkModeAction, &QAction::triggered, this, &MainWindow::toggleDarkMode);
    
    // Navigation actions (for toolbar)
    m_backAction = new QAction(tr("Back"), this);
    m_backAction->setShortcut(QKeySequence::Back);
    connect(m_backAction, &QAction::triggered, this, &MainWindow::goBack);
    
    m_forwardAction = new QAction(tr("Forward"), this);
    m_forwardAction->setShortcut(QKeySequence::Forward);
    connect(m_forwardAction, &QAction::triggered, this, &MainWindow::goForward);
    
    m_reloadAction = new QAction(tr("Reload"), this);
    m_reloadAction->setShortcut(QKeySequence::Refresh);
    connect(m_reloadAction, &QAction::triggered, this, &MainWindow::reload);
    
    // Help menu
    QMenu *helpMenu = menuBar()->addMenu(tr("&Help"));
    
    m_aboutAction = helpMenu->addAction(tr("&About"), this, &MainWindow::about);
}

void MainWindow::createToolBar()
{
    QToolBar *navigationBar = addToolBar(tr("Navigation"));
    navigationBar->setMovable(false);
    
    navigationBar->addAction(m_backAction);
    navigationBar->addAction(m_forwardAction);
    navigationBar->addAction(m_reloadAction);
    
    navigationBar->addSeparator();
    
    m_urlEdit = new QLineEdit(this);
    m_urlEdit->setClearButtonEnabled(true);
    connect(m_urlEdit, &QLineEdit::returnPressed, this, &MainWindow::updateUrlBar);
    navigationBar->addWidget(m_urlEdit);
}

void MainWindow::createStatusBar()
{
    m_statusLabel = new QLabel(tr("Ready"), this);
    statusBar()->addWidget(m_statusLabel, 1);
    
    m_zoomLabel = new QLabel(tr("Zoom: 100%"), this);
    statusBar()->addPermanentWidget(m_zoomLabel);
}

void MainWindow::readSettings()
{
    QSettings settings;
    
    restoreGeometry(settings.value("geometry").toByteArray());
    restoreState(settings.value("windowState").toByteArray());
    
    bool darkMode = settings.value("darkMode", false).toBool();
    setDarkMode(darkMode);
    
    qreal zoom = settings.value("zoom", 1.0).toDouble();
    m_htmlWidget->setZoom(zoom);
}

void MainWindow::writeSettings()
{
    QSettings settings;
    
    settings.setValue("geometry", saveGeometry());
    settings.setValue("windowState", saveState());
    settings.setValue("darkMode", m_darkModeEnabled);
    settings.setValue("zoom", m_htmlWidget->zoom());
}

void MainWindow::updateNavigationActions()
{
    m_backAction->setEnabled(m_historyIndex > 0);
    m_forwardAction->setEnabled(m_historyIndex < m_history.size() - 1);
}

void MainWindow::saveAsPng(const QString &filename)
{
    if (!m_htmlWidget) {
        qWarning() << "Cannot save PNG: HTML widget is null";
        return;
    }
    
    // Use a safer size calculation
    QSize size = m_htmlWidget->size();
    if (size.width() <= 0 || size.height() <= 0) {
        size = QSize(800, 600); // fallback to a reasonable size
    }
    
    try {
        // Create an offscreen QImage instead of QPixmap for better compatibility
        QImage image(size, QImage::Format_ARGB32_Premultiplied);
        image.fill(Qt::white);
        
        QPainter painter(&image);
        
        // Use safer rendering settings
        painter.setRenderHint(QPainter::Antialiasing, false);
        painter.setRenderHint(QPainter::TextAntialiasing, false);
        painter.setRenderHint(QPainter::SmoothPixmapTransform, false);
        
        // Safely check HTML widget state before rendering
        if (!m_htmlWidget->isVisible()) {
            // If widget isn't visible, make sure it has valid dimensions
            m_htmlWidget->resize(size);
        }
        
        // Safer rendering call with error handling
        try {
            m_htmlWidget->render(&painter);
        } catch (const std::exception& e) {
            qWarning() << "Exception during rendering:" << e.what();
            painter.setPen(Qt::red);
            painter.drawText(QRect(0, 0, size.width(), size.height()), Qt::AlignCenter,
                            "Error during rendering: " + QString(e.what()));
        } catch (...) {
            qWarning() << "Unknown exception during rendering";
            painter.setPen(Qt::red);
            painter.drawText(QRect(0, 0, size.width(), size.height()), Qt::AlignCenter,
                            "Unknown error during rendering");
        }
        
        painter.end();
        
        // Save the image
        if (image.save(filename, "PNG")) {
            qDebug() << "Successfully saved PNG to:" << filename;
        } else {
            qWarning() << "Failed to save PNG to:" << filename;
        }
    } catch (const std::exception& e) {
        qWarning() << "Exception while saving PNG:" << e.what();
    } catch (...) {
        qWarning() << "Unknown exception while saving PNG";
    }
}

void MainWindow::saveAsSvg(const QString &filename)
{
    if (!m_htmlWidget) {
        qWarning() << "Cannot save SVG: HTML widget is null";
        return;
    }
    
    // Use a safer size calculation
    QSize size = m_htmlWidget->size();
    if (size.width() <= 0 || size.height() <= 0) {
        size = QSize(800, 600); // fallback to a reasonable size
    }
    
    try {
        QSvgGenerator generator;
        generator.setFileName(filename);
        generator.setSize(size);
        generator.setViewBox(QRect(0, 0, size.width(), size.height()));
        generator.setTitle(m_htmlWidget->documentTitle().isEmpty() ? "Generated Document" : m_htmlWidget->documentTitle());
        generator.setDescription("Generated by QLiteHTML Browser");
        
        // Safely check HTML widget state before rendering
        if (!m_htmlWidget->isVisible()) {
            // If widget isn't visible, make sure it has valid dimensions
            m_htmlWidget->resize(size);
        }
        
        QPainter painter;
        if (painter.begin(&generator)) {
            // Fill with white background first
            painter.fillRect(QRect(0, 0, size.width(), size.height()), Qt::white);
            
            // Use low-quality rendering to avoid unsupported paint engine issues
            painter.setRenderHint(QPainter::Antialiasing, false);
            painter.setRenderHint(QPainter::TextAntialiasing, false);
            painter.setRenderHint(QPainter::SmoothPixmapTransform, false);
            
            // Safer rendering call with error handling
            try {
                m_htmlWidget->render(&painter);
            } catch (const std::exception& e) {
                qWarning() << "Exception during SVG rendering:" << e.what();
                painter.setPen(Qt::red);
                painter.drawText(QRect(0, 0, size.width(), size.height()), Qt::AlignCenter,
                                "Error during rendering: " + QString(e.what()));
            } catch (...) {
                qWarning() << "Unknown exception during SVG rendering";
                painter.setPen(Qt::red);
                painter.drawText(QRect(0, 0, size.width(), size.height()), Qt::AlignCenter,
                                "Unknown error during rendering");
            }
            
            painter.end();
            qDebug() << "Successfully saved SVG to:" << filename;
        } else {
            qWarning() << "Failed to begin painter for SVG generation";
        }
    } catch (const std::exception& e) {
        qWarning() << "Exception while saving SVG:" << e.what();
    } catch (...) {
        qWarning() << "Unknown exception while saving SVG";
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
        connect(m_urlEdit, &QLineEdit::returnPressed, this, &MainWindow::updateUrlBar);
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

void MainWindow::loadFile(const QString &filePath)
{
    loadUrl(QUrl::fromLocalFile(filePath));
}

void MainWindow::urlEntered()
{
    QString text = m_urlEdit->text().trimmed();
    if (text.isEmpty())
        return;
        
    QUrl url = QUrl::fromUserInput(text);
    loadUrl(url);
}

void MainWindow::linkClicked(const QUrl &url)
{
    loadUrl(url);
} 