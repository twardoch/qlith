// -*- coding: utf-8 -*-

# include "qlith/mainwindow.h"
#include "ui_mainwindow.h"
#include "qlith/litehtmlwidget.h"
#include "qlith/container_qt5.h"

#include <QDebug>
#include <QFile>
#include <QScrollArea>
#include <QScrollBar>
#include <QSlider>
#include <QDesktopServices>
#include <QUrl>
#include <QLineEdit>
#include <QPushButton>
#include <QHBoxLayout>
#include <QDir>
#include <QCoreApplication>
#include <QFileInfo>
#include <QLabel>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QMenuBar>
#include <QStatusBar>
#include <QComboBox>
#include <QCheckBox>
#include <QMessageBox>
#include <QPainter>
#include <QSvgGenerator>
#include <QPlainTextEdit>
#include <QApplication>
#include <QEventLoop>
#include <QTimer>
#include <QThread>
#include <QToolBar> // Added for history actions

MainWindow::MainWindow(bool debugMode, QWidget *parent) : QMainWindow(parent),
                                                          ui(new Ui::MainWindow),
                                                          m_litehtmlWidget(nullptr),
                                                          m_htmlEditor(nullptr),
                                                          m_debugMode(debugMode),
                                                          m_renderSize(800, 600), // Default render size
                                                          m_historyIndex(-1)     // Initialize history index
{
  ui->setupUi(this);

  if (m_debugMode)
  {
    // Minimal UI for debug mode - This part was already designed to be simple
    // and to isolate litehtml initialization, so we keep its structure.
    QWidget *centralWidget = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(centralWidget);
    QHBoxLayout *controlLayout = new QHBoxLayout();
    QTextEdit *htmlEditor = new QTextEdit(centralWidget);
    htmlEditor->setPlainText("<html><head><title>Test</title></head><body><h1>Test Page</h1><p>This is a simple test page.</p></body></html>");
    QPushButton *renderButtonLiteHTML = new QPushButton("Render with LiteHTML", centralWidget);
    QLabel *statusLabel = new QLabel("Status: Ready for LiteHTML test", centralWidget);

    controlLayout->addWidget(renderButtonLiteHTML);
    controlLayout->addWidget(statusLabel);
    layout->addLayout(controlLayout);
    layout->addWidget(htmlEditor, 1);

    // Placeholder for where litehtmlWidget would go, or just use a simple label for now
    QLabel *contentPlaceholder = new QLabel("LiteHTML content will appear here or in a new window.", centralWidget);
    contentPlaceholder->setAlignment(Qt::AlignCenter);
    contentPlaceholder->setFrameStyle(QFrame::Box | QFrame::Sunken);
    layout->addWidget(contentPlaceholder, 2);

    centralWidget->setLayout(layout);
    setCentralWidget(centralWidget);

    connect(renderButtonLiteHTML, &QPushButton::clicked, [this, htmlEditor, statusLabel]()
            {
      try
      {
        statusLabel->setText("Status: Creating LiteHTML widget...");
        if (this->m_litehtmlWidget)
        {
          delete this->m_litehtmlWidget;
          this->m_litehtmlWidget = nullptr;
        }
        this->m_litehtmlWidget = new litehtmlWidget();

        statusLabel->setText("Status: Loading HTML into LiteHTML widget...");
        this->m_litehtmlWidget->loadHtml(htmlEditor->toPlainText());

        statusLabel->setText("Status: Displaying LiteHTML widget...");
        QScrollArea *scrollArea = new QScrollArea(this);
        scrollArea->setWidget(this->m_litehtmlWidget);
        scrollArea->setWidgetResizable(true);
        scrollArea->resize(600, 400);
        scrollArea->setWindowTitle("LiteHTML Render Test");
        scrollArea->show();

        statusLabel->setText("Status: LiteHTML rendering attempted.");
      }
      catch (const std::exception &e)
      {
        statusLabel->setText(QString("LiteHTML Error: %1").arg(e.what()));
        qCritical() << "LiteHTML rendering exception:" << e.what();
      }
      catch (...)
      {
        statusLabel->setText("LiteHTML Error: Unknown exception.");
        qCritical() << "LiteHTML rendering unknown exception.";
      } });

    setWindowTitle("qlith-pro - Debug Mode");
    return; // Important: Return to not execute normal mode UI setup
  }

  // == Normal Mode UI Setup (Not Debug) ==
  qDebug() << "MainWindow: Setting up normal mode UI.";

  // Create address bar components
  QLineEdit* urlEdit = new QLineEdit(this);
  urlEdit->setPlaceholderText("Enter URL (file:// or https://)");
  QPushButton *goButton = new QPushButton("Go", this);
  QHBoxLayout* addressBarLayout = new QHBoxLayout();
  addressBarLayout->addWidget(urlEdit);
  addressBarLayout->addWidget(goButton);

  // Setup actions for navigation
  setupNavigationActions(); // New method to create actions

  QToolBar* navigationToolBar = addToolBar(tr("Navigation"));
  navigationToolBar->addAction(m_backAction);
  navigationToolBar->addAction(m_forwardAction);
  navigationToolBar->addAction(m_reloadAction);
  navigationToolBar->addAction(m_stopAction);
  // Add the address bar layout to the toolbar
  QWidget* addressBarWidget = new QWidget();
  addressBarWidget->setLayout(addressBarLayout);
  navigationToolBar->addWidget(addressBarWidget);


  // Instead of adding to verticalLayout_6, we make the toolbar part of the MainWindow
  // ui->verticalLayout_6->insertLayout(0, addressBarLayout); // Remove this line

  connect(goButton, &QPushButton::clicked, this, &MainWindow::navigateToUrl);
  connect(urlEdit, &QLineEdit::returnPressed, this, &MainWindow::navigateToUrl);
  connect(ui->pushButton, &QPushButton::clicked, this, &MainWindow::loadExample); // Assuming ui->pushButton is "Load Example"
  ui->pushButton->setText("Load Example");


  // Create a new litehtmlWidget for normal mode - ALWAYS initialize
  initializeLitehtmlWidget();

  // Connect new actions
  if (m_backAction) connect(m_backAction, &QAction::triggered, this, &MainWindow::goBack);
  if (m_forwardAction) connect(m_forwardAction, &QAction::triggered, this, &MainWindow::goForward);
  if (m_reloadAction) connect(m_reloadAction, &QAction::triggered, this, &MainWindow::reload);
  if (m_stopAction) connect(m_stopAction, &QAction::triggered, this, &MainWindow::stop);


  // Initial content loading (if not in debug mode)
  QStringList args = QCoreApplication::arguments();
  if (args.size() > 1 && !args.at(1).isEmpty() && !args.at(1).startsWith("--"))
  {
    QString filePath = args.at(1);
    qDebug() << "MainWindow: Loading file from command line:" << filePath;

    // Handle relative paths more robustly
    QFileInfo fileInfo(filePath);
    if (fileInfo.exists() && fileInfo.isReadable()) {
      loadFile(filePath);
    } else if (fileInfo.isRelative()) {
      // Try to resolve relative path
      QString absPath = QDir::current().absoluteFilePath(filePath);
      if (QFileInfo(absPath).exists() && QFileInfo(absPath).isReadable()) {
        qDebug() << "MainWindow: Resolved relative path to:" << absPath;
        loadFile(absPath);
      } else {
        qWarning() << "MainWindow: File not found:" << filePath;
        loadExample(); // Fallback to example
      }
    } else {
      qWarning() << "MainWindow: Invalid or nonexistent file:" << filePath;
      loadExample(); // Fallback to example
    }
    
    // Update URL field if it exists
    QLineEdit* urlEdit = findChild<QLineEdit*>();
    if (urlEdit) {
      urlEdit->setText(filePath);
    }
  }
  else
  {
    qDebug() << "MainWindow: No file from command line, loading example.";
    loadExample();
  }

  setWindowTitle("qlith-pro");

  // Start with a maximized window
  showMaximized();
}

void MainWindow::initializeLitehtmlWidget()
{
  // Clean up existing widget if it exists
  if (m_litehtmlWidget)
  {
    delete m_litehtmlWidget;
    m_litehtmlWidget = nullptr;
  }

  // Create a new litehtmlWidget
  m_litehtmlWidget = new litehtmlWidget();
  if (!m_litehtmlWidget) {
    qCritical() << "Failed to create litehtmlWidget instance!";
    return;
  }

  // Set specific size for the widget to match our render size
  if (m_renderSize.isValid()) {
    m_litehtmlWidget->setMinimumSize(m_renderSize.width(), m_renderSize.height());
    m_litehtmlWidget->resize(m_renderSize);
  }

  // Create a scroll area and set the litehtmlWidget as its child
  QScrollArea* scrollArea = new QScrollArea();
  scrollArea->setWidget(m_litehtmlWidget);
  scrollArea->setWidgetResizable(true);
  scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
  scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
  
  // Set the scroll area as the central widget
  setCentralWidget(scrollArea);

  // Connect to the correct signal for link clicks and document loading
  connect(m_litehtmlWidget, &litehtmlWidget::anchorClicked, this, &MainWindow::loadUrl);
  connect(m_litehtmlWidget, &litehtmlWidget::documentLoaded, this, &MainWindow::onDocumentLoaded);
  
  qDebug() << "litehtmlWidget initialized successfully";
}

void MainWindow::setupScrollbar()
{
  // Disabled in this version
  // This method would need significant rework if used outside debug mode
  // given the changes to central widget management.
  // For normal mode, the QScrollArea now handles scrollbars for m_litehtmlWidget.
}

MainWindow::~MainWindow()
{
  delete ui;
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
  QMainWindow::resizeEvent(event);

  // Update litehtmlWidget size if needed
  if (m_litehtmlWidget)
  {
    m_litehtmlWidget->setMinimumWidth(width() - 30);
  }
}

void MainWindow::navigateToUrl()
{
  QLineEdit* urlEdit = findChild<QLineEdit*>();
  if (urlEdit && !urlEdit->text().isEmpty())
  {
    loadUrl(urlEdit->text());
  }
}

void MainWindow::loadUrl(const QString &url)
{
  // Ensure widget is initialized
  if (!m_litehtmlWidget) {
    qWarning() << "loadUrl: litehtmlWidget is null, initializing...";
    initializeLitehtmlWidget();
    if (!m_litehtmlWidget) {
      qCritical() << "loadUrl: Failed to initialize litehtmlWidget";
      return;
    }
  }

  qDebug() << "loadUrl: Loading URL:" << url;
  
  // Try to convert to a QUrl with proper handling of relative paths
  QUrl qUrl;
  if (QFileInfo(url).isRelative() && !url.startsWith("http://") && !url.startsWith("https://")) {
    // Handle relative file paths
    QString absolutePath = QDir::current().absoluteFilePath(url);
    qUrl = QUrl::fromLocalFile(absolutePath);
    qDebug() << "loadUrl: Converted relative path to absolute URL:" << qUrl.toString();
  } else {
    qUrl = QUrl::fromUserInput(url);
  }
  
  // If it's a valid file URL, load directly with the file method
  if (qUrl.isValid() && qUrl.isLocalFile()) {
    loadFile(qUrl.toLocalFile());
    return;
  }
  
  // Otherwise use the standard URL loading
  m_litehtmlWidget->loadUrl(qUrl.toString());
  updateHistory(qUrl);
}

void MainWindow::updateHistory(const QUrl& url)
{
    if (url.isEmpty() || url.toString() == "about:blank") // Don't add blank pages or current page if already at top
    {
        if (m_history.isEmpty() || m_history.last() != url) {
            // allow one about:blank at the start
        } else {
            return;
        }
    }

    if (m_historyIndex >= 0 && m_historyIndex < m_history.size() - 1) {
        m_history.erase(m_history.begin() + m_historyIndex + 1, m_history.end());
    }

    // Avoid adding consecutive duplicates
    if (m_history.isEmpty() || m_history.last() != url) {
        m_history.append(url);
    }

    while (m_history.size() > 100) { // Limit history size
        m_history.removeFirst();
    }
    m_historyIndex = m_history.size() - 1;

    if (m_backAction) m_backAction->setEnabled(m_historyIndex > 0);
    if (m_forwardAction) m_forwardAction->setEnabled(m_historyIndex < m_history.size() - 1);
}


void MainWindow::goBack()
{
    if (m_historyIndex > 0) {
        m_historyIndex--;
        QUrl url = m_history.at(m_historyIndex);
        QLineEdit* urlEdit = findChild<QLineEdit*>();
        if (urlEdit) {
            urlEdit->setText(url.toString());
        }
        m_litehtmlWidget->loadUrl(url.toString()); // Do not call updateHistory here, loadUrl will do it via signal or direct call if needed
        if (m_backAction) m_backAction->setEnabled(m_historyIndex > 0);
        if (m_forwardAction) m_forwardAction->setEnabled(m_historyIndex < m_history.size() - 1);
    }
}

void MainWindow::goForward()
{
    if (m_historyIndex < m_history.size() - 1) {
        m_historyIndex++;
        QUrl url = m_history.at(m_historyIndex);
        QLineEdit* urlEdit = findChild<QLineEdit*>();
        if (urlEdit) {
            urlEdit->setText(url.toString());
        }
        m_litehtmlWidget->loadUrl(url.toString()); // Do not call updateHistory here
        if (m_backAction) m_backAction->setEnabled(m_historyIndex > 0);
        if (m_forwardAction) m_forwardAction->setEnabled(m_historyIndex < m_history.size() - 1);
    }
}

void MainWindow::reload()
{
    if (m_historyIndex >= 0 && m_historyIndex < m_history.size()) {
        QUrl currentUrl = m_history.at(m_historyIndex);
        // To ensure it's treated as a fresh load, create a new QUrl instance
        m_litehtmlWidget->loadUrl(QUrl(currentUrl.toString()).toString());
    } else if (m_litehtmlWidget) {
        // If history is empty, try reloading whatever is current in the widget
        // This might require getting current URL from widget or container if available
        // For now, let's assume loadExample or an initial page might be reloaded by re-calling loadExample
        loadExample(); // Or some other default reload action
    }
}

void MainWindow::stop()
{
    // m_litehtmlWidget should have a stop() method if it supports stopping network requests.
    // For now, this is a placeholder.
    qDebug() << "MainWindow::stop() called - (Placeholder: m_litehtmlWidget->stop() if implemented)";
    // if (m_litehtmlWidget) { m_litehtmlWidget->stop(); }
}

void MainWindow::setupNavigationActions()
{
    m_backAction = new QAction(tr("Back"), this);
    m_backAction->setIcon(QIcon::fromTheme("go-previous"));
    m_backAction->setShortcut(QKeySequence::Back);
    m_backAction->setEnabled(false);

    m_forwardAction = new QAction(tr("Forward"), this);
    m_forwardAction->setIcon(QIcon::fromTheme("go-next"));
    m_forwardAction->setShortcut(QKeySequence::Forward);
    m_forwardAction->setEnabled(false);

    m_reloadAction = new QAction(tr("Reload"), this);
    m_reloadAction->setIcon(QIcon::fromTheme("view-refresh"));
    m_reloadAction->setShortcut(QKeySequence::Refresh);

    m_stopAction = new QAction(tr("Stop"), this);
    m_stopAction->setIcon(QIcon::fromTheme("process-stop"));
    m_stopAction->setShortcut(Qt::Key_Escape);
}


void MainWindow::loadExample()
{
  // Ensure widget is initialized
  if (!m_litehtmlWidget) {
    qWarning() << "loadExample: litehtmlWidget is null, initializing...";
    initializeLitehtmlWidget();
    if (!m_litehtmlWidget) {
      qCritical() << "loadExample: Failed to initialize litehtmlWidget";
      return;
    }
  }

  // Simple example HTML content
  QString exampleHtml = R"(
  <!DOCTYPE html>
  <html>
  <head>
    <title>Example Page</title>
    <style>
      body { font-family: Arial, sans-serif; margin: 20px; }
      h1 { color: #333; }
      p { line-height: 1.5; }
    </style>
  </head>
  <body>
    <h1>Welcome to qlith-pro</h1>
    <p>This is a simple HTML example page rendered with litehtml.</p>
    <p>The rendering engine supports basic HTML, CSS, and images.</p>
  </body>
  </html>
  )";

  m_litehtmlWidget->loadHtml(exampleHtml);
}

void MainWindow::loadFile(const QString& filePath)
{
  qDebug() << "Loading file:" << filePath;
  
  // Ensure widget is initialized first
  if (!m_litehtmlWidget) {
    qWarning() << "loadFile: litehtmlWidget is null, initializing...";
    initializeLitehtmlWidget();
    if (!m_litehtmlWidget) {
      qCritical() << "loadFile: Failed to initialize litehtmlWidget";
      emit documentLoaded(false);
      return;
    }
  }

  // Convert path to absolute if it's relative
  QString absoluteFilePath = filePath;
  QFileInfo fileInfo(filePath);
  if (!fileInfo.isAbsolute()) {
    absoluteFilePath = QDir::current().absoluteFilePath(filePath);
    fileInfo = QFileInfo(absoluteFilePath);
    qDebug() << "Converted relative path to absolute:" << absoluteFilePath;
  }
  
  // Check if the file exists and is readable
  if (!fileInfo.exists()) {
    qWarning() << "File does not exist:" << absoluteFilePath;
    QString errorHtml = QString("<!DOCTYPE html><html><body><h1>Error</h1><p>File does not exist: %1</p></body></html>")
      .arg(absoluteFilePath);
    m_litehtmlWidget->loadHtml(errorHtml);
    emit documentLoaded(false);
    return;
  }
  
  if (!fileInfo.isReadable()) {
    qWarning() << "File is not readable:" << absoluteFilePath;
    QString errorHtml = QString("<!DOCTYPE html><html><body><h1>Error</h1><p>File is not readable: %1</p></body></html>")
      .arg(absoluteFilePath);
    m_litehtmlWidget->loadHtml(errorHtml);
    emit documentLoaded(false);
    return;
  }
  
  // Read the file content
  QFile file(absoluteFilePath);
  if (!file.open(QIODevice::ReadOnly)) {
    qWarning() << "Failed to open file:" << absoluteFilePath << "Error:" << file.errorString();
    QString errorHtml = QString("<!DOCTYPE html><html><body><h1>Error</h1><p>Failed to open file: %1</p><p>Error: %2</p></body></html>")
      .arg(absoluteFilePath)
      .arg(file.errorString());
    m_litehtmlWidget->loadHtml(errorHtml);
    emit documentLoaded(false);
    return;
  }
  
  // Read the content - use binary mode to avoid text conversion issues
  QByteArray fileData = file.readAll();
  file.close();
  
  if (fileData.isEmpty()) {
    qWarning() << "File is empty:" << absoluteFilePath;
    QString errorHtml = QString("<!DOCTYPE html><html><body><h1>Error</h1><p>File is empty: %1</p></body></html>")
      .arg(absoluteFilePath);
    m_litehtmlWidget->loadHtml(errorHtml);
    emit documentLoaded(false);
    return;
  }
  
  // Convert to string with proper UTF-8 handling
  QString htmlContent = QString::fromUtf8(fileData);

  // Log the content for debugging
  qDebug() << "HTML content size:" << htmlContent.size() << "bytes";
  qDebug() << "First 100 chars of content:" << htmlContent.left(100).replace('\n', "\\n");
  
  // Get the base URL for relative paths (the directory containing the HTML file)
  QString baseUrl = QUrl::fromLocalFile(fileInfo.absolutePath()).toString();
  qDebug() << "Base URL for relative paths:" << baseUrl;
  
  // In debug mode, update the text editor
  if (m_debugMode) {
    QPlainTextEdit* htmlEditor = findChild<QPlainTextEdit*>();
    if (htmlEditor) {
      htmlEditor->setPlainText(htmlContent);
    }
  }
  
  // Load the HTML content with base URL for proper relative path resolution
  m_litehtmlWidget->loadHtml(htmlContent, baseUrl);
  updateHistory(QUrl::fromLocalFile(absoluteFilePath)); // Add to history
}

void MainWindow::setRenderSize(const QSize& size)
{
  qDebug() << "Setting render size to:" << size;
  m_renderSize = size;
  
  // Update the widget size if it exists
  if (m_litehtmlWidget) {
    m_litehtmlWidget->setMinimumSize(size.width() / 2, size.height() / 2);
    m_litehtmlWidget->resize(size);
  }
}

bool MainWindow::exportToSvg(const QString& filePath)
{
  if (!m_litehtmlWidget) {
    qWarning() << "Cannot export to SVG: litehtmlWidget is null";
    return false;
  }
  
  // Ensure the target directory exists
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
  
  // Get actual document size from the widget
  QSize documentSize = m_litehtmlWidget->documentSize();
  QSize widgetSize = m_litehtmlWidget->size();
  
  // Use custom render size if set, or document size, or widget size as fallback
  QSize exportSize;
  if (m_renderSize.isValid()) {
    exportSize = m_renderSize;
  } else if (documentSize.isValid() && documentSize.width() > 0 && documentSize.height() > 0) {
    exportSize = documentSize;
  } else {
    exportSize = widgetSize;
  }
  
  qDebug() << "SVG Export - Document size:" << documentSize << "Widget size:" << widgetSize << "Export size:" << exportSize;
  
  generator.setSize(exportSize);
  generator.setViewBox(QRect(0, 0, exportSize.width(), exportSize.height()));
  generator.setTitle(windowTitle());
  generator.setDescription("Generated by Qlith Pro");
  
  // Render to SVG
  QPainter painter;
  if (!painter.begin(&generator)) {
    qWarning() << "Failed to start painting to SVG file:" << filePath;
    return false;
  }
  
  // Apply appropriate scaling
  if (m_renderSize.isValid() && documentSize.isValid() && documentSize.width() > 0 && documentSize.height() > 0) {
    // Scale based on document size while maintaining aspect ratio
    double scaleX = (double)exportSize.width() / documentSize.width();
    double scaleY = (double)exportSize.height() / documentSize.height();
    
    // Option 1: Use the minimum scale factor to fit while preserving aspect ratio
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
  }
  
  // Let the widget render into the painter
  m_litehtmlWidget->render(&painter);
  
  painter.end();
  
  qInfo() << "Exported SVG to:" << filePath;
  return true;
}

bool MainWindow::exportToPng(const QString& filePath)
{
  if (!m_litehtmlWidget) {
    qWarning() << "Cannot export to PNG: litehtmlWidget is null";
    return false;
  }
  
  qDebug() << "exportToPng: Starting PNG export to" << filePath;
  
  // Get sizes
  QSize documentSize = m_litehtmlWidget->documentSize();
  QSize widgetSize = m_litehtmlWidget->size();
  
  qDebug() << "Widget size:" << widgetSize;
  qDebug() << "Document size:" << documentSize;
  qDebug() << "Requested render size:" << m_renderSize;
  
  // Ensure the target directory exists
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
  
  qDebug() << "Using export size:" << exportSize;
  
  // Store original widget size and state if possible
  QSize originalWidgetSize = m_litehtmlWidget->size();
  bool widgetWasVisible = m_litehtmlWidget->isVisible();
  // m_litehtmlWidget->setVisible(true); // Ensure widget is visible for rendering if it wasn't

  // Determine the target rendering size for the widget
  QSize targetRenderWidgetSize = exportSize; // Use the calculated exportSize for resizing

  qDebug() << "PNG Export - Temporarily resizing widget to:" << targetRenderWidgetSize;
  m_litehtmlWidget->resize(targetRenderWidgetSize);

  // Force a relayout and process events to ensure it takes effect
  // This might involve reloading content or just re-rendering with new dimensions
  // For litehtmlWidget, calling loadHtml again or ensuring its internal document
  // re-renders at the new width is key. Since loadHtml is complex,
  // we will try to make the widget re-evaluate its layout.
  if (m_litehtmlWidget->getContainer() && m_litehtmlWidget->getContainer()->getDocument()) {
      m_litehtmlWidget->getContainer()->getDocument()->render(targetRenderWidgetSize.width());
      // Update documentSize within the widget as render might change it
      m_litehtmlWidget->updateDocumentSize(m_litehtmlWidget->getContainer()->getDocument()->width(), m_litehtmlWidget->getContainer()->getDocument()->height());
  }
  QApplication::processEvents(); // Process events to allow GUI updates

  // Get the document size again after resize, as it may have changed
  documentSize = m_litehtmlWidget->documentSize(); // Update documentSize
  qDebug() << "PNG Export - Document size after resize:" << documentSize;

  // Ensure exportSize is still appropriate (e.g., if document grew larger than m_renderSize)
  exportSize = QSize(
      qMax(exportSize.width(), documentSize.width()),
      qMax(exportSize.height(), documentSize.height())
  );
  qDebug() << "PNG Export - Final export pixmap size:" << exportSize;

  // Create a pixmap with the final export size
  QPixmap pixmap(exportSize);
  pixmap.fill(Qt::white); // Fill with white background
  
  // Create a painter for the pixmap
  QPainter painter(&pixmap);
  
  // No scaling needed here if widget is already at exportSize and document rendered to fit
  // Scaling logic from SVG can be removed or adapted if necessary, but direct render is preferred.
  // If exportSize is different from targetRenderWidgetSize (e.g. document was larger),
  // we might need to adjust. For now, assume render at targetRenderWidgetSize is what we capture.

  // Let the widget render into the painter
  qDebug() << "Rendering widget (now resized) to pixmap";
  m_litehtmlWidget->render(&painter); // Render the (potentially resized) widget content
  painter.end();
  
  // Save to PNG
  qDebug() << "Saving pixmap to PNG file:" << filePath;
  bool result = pixmap.save(filePath, "PNG");

  // Restore original widget size and visibility
  qDebug() << "PNG Export - Restoring widget to original size:" << originalWidgetSize;
  m_litehtmlWidget->resize(originalWidgetSize);
  // m_litehtmlWidget->setVisible(widgetWasVisible);
  // Reload or re-render content at original size if necessary
  if (m_litehtmlWidget->getContainer() && m_litehtmlWidget->getContainer()->getDocument()) {
      m_litehtmlWidget->getContainer()->getDocument()->render(originalWidgetSize.width());
      m_litehtmlWidget->updateDocumentSize(m_litehtmlWidget->getContainer()->getDocument()->width(), m_litehtmlWidget->getContainer()->getDocument()->height());
  }
  QApplication::processEvents();
  if (result) {
    qInfo() << "Successfully exported PNG to:" << filePath;
  } else {
    qWarning() << "Failed to save PNG to:" << filePath;
  }
  
  return result;
}

void MainWindow::onDocumentLoaded(bool success)
{
  qDebug() << "MainWindow::onDocumentLoaded - Success:" << success;
  
  // Additional document loading logic could go here
  
  // Forward the signal
  emit documentLoaded(success);
}
