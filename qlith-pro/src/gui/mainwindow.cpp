// -*- coding: utf-8 -*-

# include "qlith/mainwindow.h"
# include "ui_mainwindow.h"
# include "qlith/container_qt5.h"
# include "qlith/litehtmlwidget.h"
# include "qlith/context.h"

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

MainWindow::MainWindow(QWidget *parent) :
QMainWindow(parent),
ui(new Ui::MainWindow)
{
  ui->setupUi(this);

  // Create address bar components
  QLineEdit* urlEdit = new QLineEdit(this);
  urlEdit->setPlaceholderText("Enter URL (file:// or https://)");
  QPushButton* goButton = new QPushButton("Go", this);
  
  // Create address bar layout
  QHBoxLayout* addressBarLayout = new QHBoxLayout();
  addressBarLayout->addWidget(urlEdit);
  addressBarLayout->addWidget(goButton);
  
  // Add address bar to the UI
  ui->verticalLayout_6->insertLayout(0, addressBarLayout);
  
  // Connect the go button and enter key in the URL field
  connect(goButton, &QPushButton::clicked, this, &MainWindow::navigateToUrl);
  connect(urlEdit, &QLineEdit::returnPressed, this, &MainWindow::navigateToUrl);
  
  // Make the existing button load the example page
  connect(ui->pushButton, &QPushButton::clicked, this, &MainWindow::loadExample);
  ui->pushButton->setText("Load Example");

  m_litehtmlWidget = new litehtmlWidget();

  // Set up scroll area
  QScrollArea* scrollArea = new QScrollArea();
  scrollArea->setWidget(m_litehtmlWidget);
  scrollArea->setWidgetResizable(true);
  scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
  scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
  
  // Set scroll area as central widget
  setCentralWidget(scrollArea);
  
  // Connect the linkClicked signal
  connect(m_litehtmlWidget, &litehtmlWidget::linkClicked, this, &MainWindow::loadUrl);

  // Create a litehtml context
  litehtml_context context;

  // Load master CSS
  QFile master_css_fh(":/css/master.css");
  if (!master_css_fh.open(QIODevice::ReadOnly)) {
    qWarning() << "Failed to open master CSS file:" << master_css_fh.errorString();
    // Create an empty CSS if file cannot be opened
    context.load_master_stylesheet("");
  } else {
    QByteArray master_css = master_css_fh.readAll();
    context.load_master_stylesheet(master_css.constData());
    master_css_fh.close();
  }

  // Load the example page by default
  loadExample();

  ui->scrollAreaVerticalLayout->addWidget(m_litehtmlWidget);

  QScrollBar* scrollBar = new QScrollBar(Qt::Vertical);
  scrollBar->setParent(this);
  scrollBar->setFocusPolicy(Qt::StrongFocus);
  scrollBar->setMinimum(0);
  scrollBar->setMaximum(0);
  scrollBar->setValue(0);
  scrollBar->setSingleStep(1);
  scrollBar->setInvertedControls(false);
  connect(scrollBar, &QScrollBar::valueChanged, scrollBar, [this](int val){
    m_litehtmlWidget->getContainer()->setScrollY(-val);
    m_litehtmlWidget->repaint();
  });

  connect(m_litehtmlWidget->getContainer(), &container_qt5::docSizeChanged, m_litehtmlWidget->getContainer(), 
    [this, scrollBar](int w, int h) {
      if (!scrollBar) {
        return;
      }

      const int singleVisiblePageHeight = m_litehtmlWidget->height();
      
      // Subtracts already visible area = widget size
      const int availableScroll = h - singleVisiblePageHeight;
      scrollBar->setMaximum(availableScroll);
      // amount that the value changes on cursor keys
      scrollBar->setSingleStep(singleVisiblePageHeight);
      // amount that the value changes on Page Up and Page Down keys
      scrollBar->setPageStep(singleVisiblePageHeight);
  });

  ui->verticalLayoutBar->addWidget(scrollBar);
}

MainWindow::~MainWindow()
{
  delete ui;
}

void MainWindow::navigateToUrl()
{
  // Get the URL from the line edit
  QLineEdit* urlEdit = findChild<QLineEdit*>();
  if (urlEdit) {
    QString url = urlEdit->text();
    if (!url.isEmpty()) {
      loadUrl(url);
    }
  }
}

void MainWindow::loadUrl(const QString& url)
{
  QUrl qurl(url);
  
  // Update the address bar with the current URL
  QLineEdit* urlEdit = findChild<QLineEdit*>();
  if (urlEdit) {
    urlEdit->setText(url);
  }
  
  // Handle different URL schemes
  if (qurl.scheme() == "file") {
    // Load local file
    QFile file(qurl.toLocalFile());
    if (file.open(QIODevice::ReadOnly)) {
      QString html = QString::fromUtf8(file.readAll());
      m_litehtmlWidget->loadHtml(html, url);
      file.close();
    } else {
      // Show error message if file can't be opened
      m_litehtmlWidget->loadHtml("<html><body><h1>Error</h1><p>Could not open file: " + qurl.toLocalFile() + "</p></body></html>");
    }
  } else if (qurl.scheme() == "http" || qurl.scheme() == "https") {
    // Load remote URL
    m_litehtmlWidget->loadUrl(url);
  } else {
    // For other schemes and invalid URLs, just try to load it directly
    m_litehtmlWidget->loadUrl(url);
  }
}

void MainWindow::loadExample()
{
  // Example HTML content
  QString html = R"(
  <html>
  <head>
    <link rel="stylesheet" href=":/css/reset.css">
    <link rel="stylesheet" href=":/css/bootstrap.css">
    <style>
      body {
        font-family: Arial, sans-serif;
        margin: 20px;
        line-height: 1.5;
      }
      h1 {
        color: #2c3e50;
        text-align: center;
        border-bottom: 2px solid #3498db;
        padding-bottom: 10px;
      }
      p {
        margin-bottom: 15px;
      }
      a {
        color: #3498db;
        text-decoration: none;
      }
      a:hover {
        text-decoration: underline;
      }
      .container {
        max-width: 800px;
        margin: 0 auto;
        background-color: #f9f9f9;
        padding: 20px;
        border-radius: 5px;
        box-shadow: 0 2px 10px rgba(0,0,0,0.1);
      }
      .highlight {
        background-color: #ffffcc;
        padding: 5px;
        border-left: 3px solid #f39c12;
      }
      button {
        background-color: #3498db;
        color: white;
        border: none;
        padding: 10px 15px;
        border-radius: 4px;
        cursor: pointer;
        font-size: 14px;
        margin: 5px;
      }
      button:hover {
        background-color: #2980b9;
      }
      img {
        max-width: 100%;
        height: auto;
        display: block;
        margin: 20px auto;
      }
    </style>
  </head>
  <body>
    <div class="container">
      <h1>Qlith HTML Rendering Demo</h1>
      
      <p>This is a demonstration of the <span class="highlight">qlith-pro</span> HTML rendering capabilities using the litehtml library.</p>
      
      <p>Features demonstrated:</p>
      <ul>
        <li>Text formatting and CSS styling</li>
        <li>Links: <a href="https://github.com/litehtml/litehtml">litehtml on GitHub</a></li>
        <li>Basic layout with containers and margins</li>
        <li>Buttons and interactive elements</li>
      </ul>
      
      <div style="text-align: center; margin: 20px 0;">
        <button type="button">Primary Button</button>
        <button type="button" style="background-color: #27ae60;">Success Button</button>
        <button type="button" style="background-color: #e74c3c;">Danger Button</button>
      </div>
      
      <p>Lorem ipsum dolor sit amet, consectetur adipiscing elit. Nullam in dui mauris. Vivamus hendrerit arcu sed erat molestie vehicula. Sed auctor neque eu tellus rhoncus ut eleifend nibh porttitor. Ut in nulla enim. Phasellus molestie magna non est bibendum non venenatis nisl tempor.</p>
      
      <p style="text-align: center;"><a href="https://example.com">Visit Example Website</a></p>
      
      <p style="text-align: center;"><img src=":/img/test.png" alt="Test Image"></p>
    </div>
  </body>
  </html>
  )";
  
  // Load the HTML into the widget
  m_litehtmlWidget->loadHtml(html);
}

// Handle window resize
void MainWindow::resizeEvent(QResizeEvent* event)
{
  QMainWindow::resizeEvent(event);
  
  // Update litehtmlWidget size if needed
  if (m_litehtmlWidget) {
    m_litehtmlWidget->setMinimumWidth(width() - 30);
  }
}
