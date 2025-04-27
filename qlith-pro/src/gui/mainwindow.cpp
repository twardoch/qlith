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

MainWindow::MainWindow(QWidget *parent) :
QMainWindow(parent),
ui(new Ui::MainWindow)
{
  ui->setupUi(this);

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
  connect(m_litehtmlWidget, &litehtmlWidget::linkClicked, this, [](const QString& url) {
    QDesktopServices::openUrl(QUrl(url));
  });

  // Create a litehtml context
  litehtml_context context;

  // Load master CSS
  QFile master_css_fh("://res/css/master.css");
  master_css_fh.open(QIODevice::ReadOnly);
  QByteArray master_css = master_css_fh.readAll();
  context.load_master_stylesheet(master_css.constData());

  std::string htmlStr = u8"";

  htmlStr += R"raw(
  <html>
  <head>
  <link rel="stylesheet" href="://res/css/reset.css">
  <link rel="stylesheet" href="://res/css/test1.css">
  </head>
  <body>

  <style>
    h1{
      color: green;
      text-align: center;
    }
    div.one{
      margin-top: 40px;
      text-align: center;
    }
    button{
      margin-top: 10px;
    }
  </style>

  <div class="container">
    <h1>Geek</h1>

jkljkljkl

    <div style="box-shadow: -4px -10px 5px 0px rgba(0,0,0,0.75) , 4px 10px 5px 0px rgba(110,0,0,0.75) , 18px 10px 5px 0px rgba(10,220,0,0.75);">
      <div> box-shadow </div>
    </div>

jkljkljkljkl

    <!-- Bootstrap Button Classes -->
    <div class="one">
      <button type="button" class="btn btn-primary">Primary</button>
      <button type="button" class="btn btn-secondary">Secondary</button>
      <button type="button" class="btn btn-success">Success</button>
      <button type="button" class="btn btn-danger">Danger</button>
      <button type="button" class="btn btn-warning">Warning</button>
      <button type="button" class="btn btn-info">Info</button>
      <button type="button" class="btn btn-light">Light</button>
      <button type="button" class="btn btn-dark">Dark</button>
      <button type="button" class="btn btn-link">Link</button>
    </div>

  </div>

  </body>
  </html>)raw";

  const char* html = htmlStr.c_str();
  
  // Create document
  auto doc = litehtml::document::createFromString(html, m_litehtmlWidget->getContainer(), context.get_master_css());
  m_litehtmlWidget->getContainer()->set_document(doc);

  m_litehtmlWidget->show();

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

void MainWindow::loadExample()
{
  // Example HTML content
  QString html = R"(
  <html>
  <head>
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
