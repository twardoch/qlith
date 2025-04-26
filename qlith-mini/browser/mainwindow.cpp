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
    , m_htmlWidget(new QLiteHtmlWidget(this))
    , m_networkManager(new QNetworkAccessManager(this))
    , m_darkModeEnabled(false)
    , m_currentHistoryIndex(-1)
    , m_maxHistorySize(100)
{
    setCentralWidget(m_htmlWidget);
    setMinimumSize(800, 600);

    createActions();
    createToolBar();
    createStatusBar();
    
    connect(m_htmlWidget, &QLiteHtmlWidget::loadFinished, this, &MainWindow::onLoadFinished);
    connect(m_htmlWidget, &QLiteHtmlWidget::zoomChanged, this, &MainWindow::updateZoomLabel);
    connect(m_htmlWidget, &QLiteHtmlWidget::linkClicked, this, &MainWindow::linkClicked);
    
    readSettings();
    updateNavigationActions();
    
    // Load default page
    loadUrl(QUrl("about:blank"));
}

void MainWindow::onLoadFinished()
{
    m_statusLabel->setText(tr("Ready"));
    updateWindowTitle();
    
    // Save debug image when loading is finished
    QString debugDir = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/qlitehtml_debug";
    QDir dir;
    if (!dir.exists(debugDir)) {
        dir.mkpath(debugDir);
    }
    
    // Create filename with timestamp and URL
    QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss");
    QString urlPart;
    if (m_currentHistoryIndex >= 0 && m_currentHistoryIndex < m_history.size()) {
        QUrl currentUrl = m_history.at(m_currentHistoryIndex);
        if (currentUrl.isValid() && !currentUrl.host().isEmpty()) {
            urlPart = "_" + currentUrl.host().replace(".", "_");
        }
    }
    
    // Save debug rendering image
    QString debugImagePath = debugDir + "/" + timestamp + urlPart + "_render.png";
    m_htmlWidget->saveDebugImage(debugImagePath);
    
    // Also save document structure for debugging
    QString domDebugPath = debugDir + "/" + timestamp + urlPart + "_dom.txt";
    QFile domFile(domDebugPath);
    if (domFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream stream(&domFile);
        stream << "URL: " << (m_currentHistoryIndex >= 0 && m_currentHistoryIndex < m_history.size() 
                             ? m_history.at(m_currentHistoryIndex).toString() 
                             : "unknown") << "\n";
        stream << "Document Title: " << m_htmlWidget->documentTitle() << "\n";
        stream << "Window Size: " << width() << "x" << height() << "\n";
        stream << "Viewport Size: " << m_htmlWidget->viewport()->width() << "x" << m_htmlWidget->viewport()->height() << "\n";
        stream << "Zoom: " << m_htmlWidget->zoom() << "\n";
        domFile.close();
        qDebug() << "DOM debug info saved to:" << domDebugPath;
    }
    
    emit loadFinished();
}

MainWindow::~MainWindow()
{
}

void MainWindow::loadUrl(const QUrl &url)
{
    if (!url.isValid())
        return;
        
    m_urlEdit->setText(url.toString());
    
    if (url.scheme() == "file") {
        m_htmlWidget->loadFile(url.toLocalFile());
    } else {
        m_htmlWidget->load(url);
    }
    
    // Add to history
    if (m_currentHistoryIndex >= 0 && m_currentHistoryIndex < m_history.size() - 1) {
        // Remove forward history if we navigate from a point in history
        m_history.erase(m_history.begin() + m_currentHistoryIndex + 1, m_history.end());
    }
    
    m_history.append(url);
    if (m_history.size() > m_maxHistorySize) {
        m_history.removeFirst();
    }
    m_currentHistoryIndex = m_history.size() - 1;
    
    updateNavigationActions();
    updateWindowTitle();
}

void MainWindow::loadFile(const QString &filePath)
{
    loadUrl(QUrl::fromLocalFile(filePath));
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    writeSettings();
    event->accept();
}

void MainWindow::urlEntered()
{
    QString text = m_urlEdit->text().trimmed();
    if (text.isEmpty())
        return;
        
    QUrl url = QUrl::fromUserInput(text);
    loadUrl(url);
}

void MainWindow::reloadPage()
{
    if (m_currentHistoryIndex >= 0 && m_currentHistoryIndex < m_history.size()) {
        QUrl currentUrl = m_history.at(m_currentHistoryIndex);
        m_htmlWidget->load(currentUrl);
    }
}

void MainWindow::goBack()
{
    if (m_currentHistoryIndex > 0) {
        m_currentHistoryIndex--;
        QUrl url = m_history.at(m_currentHistoryIndex);
        m_urlEdit->setText(url.toString());
        
        if (url.scheme() == "file") {
            m_htmlWidget->loadFile(url.toLocalFile());
        } else {
            m_htmlWidget->load(url);
        }
        
        updateNavigationActions();
        updateWindowTitle();
    }
}

void MainWindow::goForward()
{
    if (m_currentHistoryIndex < m_history.size() - 1) {
        m_currentHistoryIndex++;
        QUrl url = m_history.at(m_currentHistoryIndex);
        m_urlEdit->setText(url.toString());
        
        if (url.scheme() == "file") {
            m_htmlWidget->loadFile(url.toLocalFile());
        } else {
            m_htmlWidget->load(url);
        }
        
        updateNavigationActions();
        updateWindowTitle();
    }
}

void MainWindow::openFile()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open HTML File"),
                                                   QString(),
                                                   tr("HTML Files (*.html *.htm);;All Files (*)"));
    if (!fileName.isEmpty()) {
        loadFile(fileName);
    }
}

void MainWindow::zoomIn()
{
    m_htmlWidget->zoomIn();
}

void MainWindow::zoomOut()
{
    m_htmlWidget->zoomOut();
}

void MainWindow::resetZoom()
{
    m_htmlWidget->resetZoom();
}

void MainWindow::updateZoomLabel(qreal zoom)
{
    int percent = qRound(zoom * 100);
    m_zoomLabel->setText(tr("Zoom: %1%").arg(percent));
}

void MainWindow::showAbout()
{
    QMessageBox::about(this, tr("About QLiteHTML Browser"),
                      tr("<h2>QLiteHTML Browser</h2>"
                         "<p>A lightweight HTML browser based on litehtml and Qt.</p>"
                         "<p>Version 1.0</p>"));
}

void MainWindow::toggleDarkMode()
{
    setDarkMode(!m_darkModeEnabled);
}

void MainWindow::setDarkMode(bool enabled)
{
    m_darkModeEnabled = enabled;
    m_darkModeAction->setChecked(enabled);
    
    if (enabled) {
        QPalette darkPalette;
        darkPalette.setColor(QPalette::Window, QColor(53, 53, 53));
        darkPalette.setColor(QPalette::WindowText, Qt::white);
        darkPalette.setColor(QPalette::Base, QColor(25, 25, 25));
        darkPalette.setColor(QPalette::AlternateBase, QColor(53, 53, 53));
        darkPalette.setColor(QPalette::ToolTipBase, Qt::white);
        darkPalette.setColor(QPalette::ToolTipText, Qt::white);
        darkPalette.setColor(QPalette::Text, Qt::white);
        darkPalette.setColor(QPalette::Button, QColor(53, 53, 53));
        darkPalette.setColor(QPalette::ButtonText, Qt::white);
        darkPalette.setColor(QPalette::BrightText, Qt::red);
        darkPalette.setColor(QPalette::Link, QColor(42, 130, 218));
        darkPalette.setColor(QPalette::Highlight, QColor(42, 130, 218));
        darkPalette.setColor(QPalette::HighlightedText, Qt::black);
        qApp->setPalette(darkPalette);
    } else {
        qApp->setPalette(QApplication::style()->standardPalette());
    }
}

void MainWindow::linkClicked(const QUrl &url)
{
    loadUrl(url);
}

void MainWindow::updateWindowTitle()
{
    QString title = m_htmlWidget->documentTitle();
    if (title.isEmpty()) {
        if (m_currentHistoryIndex >= 0 && m_currentHistoryIndex < m_history.size()) {
            title = m_history.at(m_currentHistoryIndex).toString();
        } else {
            title = tr("QLiteHTML Browser");
        }
    } else {
        title = tr("%1 - QLiteHTML Browser").arg(title);
    }
    setWindowTitle(title);
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
    connect(m_reloadAction, &QAction::triggered, this, &MainWindow::reloadPage);
    
    // Help menu
    QMenu *helpMenu = menuBar()->addMenu(tr("&Help"));
    
    m_aboutAction = helpMenu->addAction(tr("&About"), this, &MainWindow::showAbout);
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
    connect(m_urlEdit, &QLineEdit::returnPressed, this, &MainWindow::urlEntered);
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
    m_backAction->setEnabled(m_currentHistoryIndex > 0);
    m_forwardAction->setEnabled(m_currentHistoryIndex < m_history.size() - 1);
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