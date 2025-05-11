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

MainWindow::MainWindow(bool debugMode, QWidget *parent) : QMainWindow(parent),
                                                          ui(new Ui::MainWindow),
                                                          m_litehtmlWidget(nullptr),
                                                          m_debugMode(debugMode)
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

  // Create a new litehtmlWidget for normal mode
  if (m_litehtmlWidget)
  {
    delete m_litehtmlWidget;
  } // Should be null here, but good practice
  m_litehtmlWidget = new litehtmlWidget();
  // m_litehtmlWidget->setContext(&m_context); // No longer setting context from here

  QScrollArea* scrollArea = new QScrollArea();
  scrollArea->setWidget(m_litehtmlWidget);
  scrollArea->setWidgetResizable(true);
  scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
  scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
  setCentralWidget(scrollArea); // This replaces the central widget set by ui->setupUi(this)

  // Connect to the correct signal for link clicks
  connect(m_litehtmlWidget, &litehtmlWidget::anchorClicked, this, &MainWindow::loadUrl);
  // connect(m_litehtmlWidget, &litehtmlWidget::documentLoaded, this, &MainWindow::onDocumentLoaded); // Example new signal

  setupScrollbar(); // This needs to be adapted if central widget is replaced or m_litehtmlWidget changes
  // ui->scrollAreaVerticalLayout->addWidget(m_litehtmlWidget); // This line might be problematic if setCentralWidget is used.
  // It implies adding to a specific layout in the .ui file.
  // If centralWidget is the scrollArea, then this is not needed.

  // Initial content loading (if not in debug mode)
  QStringList args = QCoreApplication::arguments();
  if (args.size() > 1 && !args.at(1).isEmpty())
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

  // Connect the widget's documentLoaded signal to our onDocumentLoaded slot
  if (m_litehtmlWidget) {
    connect(m_litehtmlWidget, &litehtmlWidget::documentLoaded, this, &MainWindow::onDocumentLoaded);
  }
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
  if (!m_litehtmlWidget)
  {
    qWarning() << "loadUrl: litehtmlWidget is null";
    return;
  }

  qDebug() << "loadUrl: Loading URL:" << url;
  m_litehtmlWidget->loadUrl(url);
}

void MainWindow::loadExample()
{
  if (!m_litehtmlWidget)
  {
    qWarning() << "loadExample: litehtmlWidget is null";
    return;
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
  
  // Check if the file exists
  QFileInfo fileInfo(filePath);
  if (!fileInfo.exists() || !fileInfo.isReadable()) {
    qWarning() << "File does not exist or is not readable:" << filePath;
    emit documentLoaded(false);
    return;
  }
  
  // Read the file content
  QFile file(filePath);
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    qWarning() << "Failed to open file:" << filePath;
    emit documentLoaded(false);
    return;
  }
  
  // Read the HTML content
  QByteArray htmlData = file.readAll();
  file.close();
  
  // Convert to string
  QString htmlContent = QString::fromUtf8(htmlData);
  
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
      // Load the HTML with the file's directory as the base URL for relative paths
      QString baseUrl = QUrl::fromLocalFile(fileInfo.absolutePath() + "/").toString();
      m_litehtmlWidget->loadHtml(htmlContent, baseUrl);
      
      // The documentLoaded signal will be emitted by the onDocumentLoaded slot
      // which is connected to the widget's documentLoaded signal
    } else {
      qWarning() << "LiteHTML widget is not initialized";
      emit documentLoaded(false);
    }
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
  generator.setSize(m_litehtmlWidget->size());
  generator.setViewBox(m_litehtmlWidget->rect());
  generator.setTitle(windowTitle());
  generator.setDescription("Generated by Qlith Pro");
  
  // Render to SVG
  QPainter painter;
  if (!painter.begin(&generator)) {
    qWarning() << "Failed to start painting to SVG file:" << filePath;
    return false;
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
  
  // Ensure the target directory exists
  QFileInfo fileInfo(filePath);
  QDir dir = fileInfo.dir();
  if (!dir.exists()) {
    if (!dir.mkpath(".")) {
      qWarning() << "Failed to create directory for PNG export:" << dir.path();
      return false;
    }
  }
  
  // Create a pixmap with the widget size
  QPixmap pixmap(m_litehtmlWidget->size());
  pixmap.fill(Qt::white);
  
  // Create a painter for the pixmap
  QPainter painter(&pixmap);
  
  // Let the widget render into the painter
  m_litehtmlWidget->render(&painter);
  
  // Save to PNG
  bool result = pixmap.save(filePath, "PNG");
  if (result) {
    qInfo() << "Exported PNG to:" << filePath;
  } else {
    qWarning() << "Failed to save PNG to:" << filePath;
  }
  
  return result;
}

void MainWindow::onDocumentLoaded(bool success)
{
  qDebug() << "Document loaded:" << (success ? "success" : "failure");
  emit documentLoaded(success);
}
