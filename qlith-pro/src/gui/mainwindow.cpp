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

MainWindow::MainWindow(bool debugMode, QWidget *parent) : QMainWindow(parent),
                                                          ui(new Ui::MainWindow),
                                                          m_litehtmlWidget(nullptr),
                                                          m_htmlEditor(nullptr),
                                                          m_debugMode(debugMode),
                                                          m_renderSize(800, 600) // Default render size
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
  ui->verticalLayout_6->insertLayout(0, addressBarLayout);

  connect(goButton, &QPushButton::clicked, this, &MainWindow::navigateToUrl);
  connect(urlEdit, &QLineEdit::returnPressed, this, &MainWindow::navigateToUrl);
  connect(ui->pushButton, &QPushButton::clicked, this, &MainWindow::loadExample);
  ui->pushButton->setText("Load Example");

  // Create a new litehtmlWidget for normal mode - ALWAYS initialize
  initializeLitehtmlWidget();

  // Initial content loading (if not in debug mode)
  QStringList args = QCoreApplication::arguments();
  if (args.size() > 1 && !args.at(1).isEmpty() && !args.at(1).startsWith("--"))
  {
    QString filePath = args.at(1);
    qDebug() << "MainWindow: Loading file from command line:" << filePath;
    // Simplified URL/File loading logic
    QUrl urlToLoad = QUrl::fromUserInput(filePath); // Handles local files and web URLs
    if (urlToLoad.isValid())
    {
      loadUrl(urlToLoad.toString());
      if (urlEdit)
      {
        urlEdit->setText(urlToLoad.toString());
      }
    }
    else
    {
      qWarning() << "MainWindow: Invalid URL or file path from command line:" << filePath;
      loadExample(); // Fallback to example
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
  m_litehtmlWidget->loadUrl(url);
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
  
  // Check if the file exists
  QFileInfo fileInfo(filePath);
  if (!fileInfo.exists() || !fileInfo.isReadable()) {
    qWarning() << "File does not exist or is not readable:" << filePath;
    // Create error document instead of failing
    QString errorHtml = QString("<!DOCTYPE html><html><body><h1>Error</h1><p>File does not exist or is not readable: %1</p></body></html>")
      .arg(filePath);
    m_litehtmlWidget->loadHtml(errorHtml);
    return;
  }
  
  // Read the file content
  QFile file(filePath);
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    qWarning() << "Failed to open file:" << filePath;
    // Create error document for file open failure
    QString errorHtml = QString("<!DOCTYPE html><html><body><h1>Error</h1><p>Failed to open file: %1</p><p>Error: %2</p></body></html>")
      .arg(filePath)
      .arg(file.errorString());
    m_litehtmlWidget->loadHtml(errorHtml);
    return;
  }
  
  // Read the HTML content
  QByteArray htmlData = file.readAll();
  file.close();
  
  // Convert to string
  QString htmlContent = QString::fromUtf8(htmlData);

  // Log the content size
  qDebug() << "HTML content size:" << htmlContent.size() << "bytes";
  qDebug() << "First 100 chars of content:" << htmlContent.left(100);
  
  // Check if we're in debug mode
  if (m_debugMode) {
    // In debug mode, set the HTML content to the text editor
    QPlainTextEdit* htmlEditor = findChild<QPlainTextEdit*>();
    if (htmlEditor) {
      htmlEditor->setPlainText(htmlContent);
      // Don't automatically render - let the user click the render button
    }
    emit documentLoaded(true);
  } else {
    // In normal mode, load the HTML into the widget
    if (m_litehtmlWidget) {
      // Reset the widget size to ensure proper rendering
      if (m_renderSize.isValid()) {
        qDebug() << "Setting widget size to:" << m_renderSize;
        m_litehtmlWidget->setMinimumSize(m_renderSize.width(), m_renderSize.height());
        m_litehtmlWidget->resize(m_renderSize);
      }
      
      // Make sure the widget is visible
      m_litehtmlWidget->setVisible(true);
      
      // Process events to ensure widget is laid out
      QApplication::processEvents();
      
      // Load the HTML with the file's directory as the base URL for relative paths
      QString baseUrl = QUrl::fromLocalFile(fileInfo.absolutePath() + "/").toString();
      qDebug() << "Loading HTML with base URL:" << baseUrl;
      
      // Create a local connection to handle document loading
      QEventLoop loadWaitLoop;
      bool loadSuccess = false;
      
      // Use a direct connection to ensure we get the signal
      QMetaObject::Connection conn = connect(m_litehtmlWidget, &litehtmlWidget::documentLoaded, 
        [&loadWaitLoop, &loadSuccess](bool success) {
          qDebug() << "MainWindow: Document loaded signal received with status:" << success;
          loadSuccess = success;
          loadWaitLoop.quit();
        }, Qt::DirectConnection);
      
      // Set a timeout to prevent hanging
      QTimer::singleShot(3000, &loadWaitLoop, [&loadWaitLoop]() {
        qWarning() << "MainWindow: Document load timeout occurred";
        loadWaitLoop.quit();
      });
      
      // Load the HTML content
      m_litehtmlWidget->loadHtml(htmlContent, baseUrl);
      
      // Wait for loading to complete or timeout
      qDebug() << "MainWindow: Waiting for document to load...";
      loadWaitLoop.exec();
      
      // Disconnect the temporary connection
      disconnect(conn);
      
      // Emit our own documentLoaded signal
      emit documentLoaded(loadSuccess);
      
      // Force a repaint of the window to ensure UI updates
      update();
      QApplication::processEvents();
    } else {
      qWarning() << "LiteHTML widget is not initialized";
      emit documentLoaded(false);
    }
  }
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
    
    qDebug() << "Applied scale:" << scale << "with centering";
  }
  
  // Let the widget render into the painter
  qDebug() << "Rendering widget to pixmap";
  m_litehtmlWidget->render(&painter);
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

void MainWindow::onDocumentLoaded(bool success)
{
  qDebug() << "MainWindow::onDocumentLoaded - Success:" << success;
  
  // Additional document loading logic could go here
  
  // Forward the signal
  emit documentLoaded(success);
}
