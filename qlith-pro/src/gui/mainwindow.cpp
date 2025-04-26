// -*- coding: utf-8 -*-

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "container_qt5.h"

#include <QDebug>
#include <QFile>
#include <QScrollArea>
#include <QScrollBar>
#include <QSlider>

MainWindow::MainWindow(QWidget *parent) :
QMainWindow(parent),
ui(new Ui::MainWindow)
{
  ui->setupUi(this);

  m_litehtmlWidget = new litehtmlWidget();

  //QFile master_css_fh("/home/snoopy/projects/litehtml-qt/include/master.css");
  QFile master_css_fh("://res/css/master.css");
  master_css_fh.open(QIODevice::ReadOnly);
  QByteArray master_css = master_css_fh.readAll();
  //qDebug() << master_css;
  ctxt.load_master_stylesheet(master_css.constData());

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

  <style>
  #root {
      width: 500px;
      display: inline;
  }
  .first {
      background-color: #ddd;
      padding: 10px;
      color: black;
      margin-bottom: 5px;
      border-bottom: 1px solid red;
  }
  .first div {
      background-color: white;
      border: 1px solid black;
      height: 50px;
      line-height: 100%;
      text-align: center;
      vertical-align: middle;
  }
  #root.second .first div {
      border-color: red;
      background-color: #aaa;
      color: white;
  }
  .first .bunt {
      border-radius: 25px;
      border-top: 5px solid silver;
      border-right: 5px solid blue;
      border-bottom: 5px solid black;
      border-left: 5px solid red;
  }
  .first .bunt2 {
      border-radius: 5px;
      border-top: 5px solid red;
      border-right: 1px solid blue;
      border-left: 1px solid blue;
      border-bottom: 1px solid blue;
  }
  </style>

  <div id="root">
    <div class="first">
      <div style="border-image: url(:/res/img/test2.png); border-width: 30px; ">border-image: url(:/res/img/test2.png); border-width: 30px; </div>
    </div>
    <div class="first">
      <div style="border-image: linear-gradient(to left, #61c69d 0%, #2d72bc 100%); border-image-slice: 1; border-width: 3px;">border-image: linear-gradient(to left, #61c69d 0%, #2d72bc 100%); border-image-slice: 1; border-width: 3px;</div>
    </div>
    <div class="first">
      <div style="border: 5px solid gold;border-style: inset;padding: 5px;">border-style: inset; padding: 5px;</div>
    </div>
    <div class="first">
      <div style="border: 5px solid gold;border-style: outset;padding: 5px;">border-style: outset; padding: 5px;</div>
    </div>
    <div class="first">
      <div style="border-style: dashed;padding: 5px;">border-style: dashed; padding: 5px;</div>
    </div>
    <div class="first">
      <div style="border-style: double;padding: 5px;">border-style: double; padding: 5px;</div>
    </div>
    <div class="first">
      <div style="border: 5px solid gold; border-style: double;padding: 5px;">border: 5px solid gold;border-style: double; padding: 5px;</div>
    </div>
    <div class="first">
      <div style="border: 15px solid rgba(255,0,0,0.0);background:gold;">border: 15px solid rgba(255,0,0,0.0);</div>
    </div>
    <div class="first">
      <div style="border: 15px solid rgba(255,0,0,0.5);background:gold;">border: 15px solid rgba(255,0,0,0.5);</div>
    </div>
    <div class="first">
      <div style="border: 15px solid rgba(255,0,0,1.0);background:gold;">border: 15px solid rgba(255,0,0,1.0);</div>
    </div>
    <div class="first">
      <div style="border: 5px solid pink; border-left-width: 15px;">border: 5px solid pink; border-left-width: 15px;</div>
    </div>
    <div class="first">
      <div style="border: 5px solid green; background: orange; border-left-width: 15px; border-radius: 55px;">border: 5px solid green; background: orange; border-left-width: 15px; border-radius: 55px;</div>
    </div>
    <div class="first">
      <div style="">1px width / 1px radius</div>
    </div>
    <div class="first">
      <div style="border-width: 5px">5px width / 0px radius</div>
    </div>
    <div class="first">
      <div style="border-radius: 1px;">1px width / 1px radius</div>
    </div>
    <div class="first">
      <div style="border-width: 5px; border-radius: 5px;">5px width / 5px radius</div>
    </div>
    <div class="first">
      <div style="border-radius: 5px;">1px width / 5px radius</div>
    </div>
    <div class="first">
      <div style="border-width: 5px; border-radius: 15px;">5px width / 15px radius</div>
    </div>
    <div class="first">
      <div style="border-width: 5px; border-left-width: 15px; border-right-width: 15px; border-radius: 15px;">5px 15px width / 15px radius</div>
    </div>
    <div class="first">
      <div style="border-width: 5px; border-left-width: 15px; border-right-width: 15px; border-radius: 25px;">5px 15px width / 25px radius</div>
    </div>
    <div class="first">
      <div class="bunt">Bunt</div>
    </div>
  </div>

<style>
.container1 {
  margin: 40px auto;
  width: 80%;
}

.element1 {
  border-width: 55px;
  border-image: url(:/res/img/test3.png) 50 round round;
  /* fill keyword makes sure the center part is preserved, so u can see how it is affected
  by the different values of the border-image-repeat property */
  border-image-repeat: round round;
  border-image-width: 50px;
}
</style>

<div class="container1">
  <div class="element1">
    <p>Lorem ipsum dolor sit amet, consectetur adipisicing elit. Veniam, optio, repellendus, quod, incidunt nulla iusto quam sunt sequi porro repudiandae consectetur veritatis consequuntur nam repellat quae temporibus laboriosam officia modi.</p>
  </div>
  <p><strong>You should read more about how the different border image properties work together in the <a href="http://tympanus.net/codrops/css_reference/border-image"><code>border-image</code></a> shorthand property entry</strong>.</p>
</div>

  <img src=':/res/img/test.png' alt='' width="200" height="200" style="width:200px;height:200px">
  <img src=':/res/img/test2.png' alt='' width="200px" height="200px" style="width:200px;height:200px">

  <div style="background-image: url(':/res/img/test2.png');background-repeat: repeat-y;width:400px;height:400px;">
    background-repeat: repeat-y;
  </div>

  <div style="background-image: url(':/res/img/test2.png');background-repeat: repeat;width:400px;height:400px;">
    background-repeat: repeat;
  </div>

  <div id="root" class="second">
    <div class="first">
      <div style="">1px width / 1px radius</div>
    </div>
    <div class="first">
      <div style="border-width: 5px">5px width / 0px radius</div>
    </div>
    <div class="first">
      <div style="border-radius: 1px;">1px width / 1px radius</div>
    </div>
    <div class="first">
      <div style="border-width: 5px; border-radius: 5px;">5px width / 5px radius</div>
    </div>
    <div class="first">
      <div style="border-radius: 5px;">1px width / 5px radius</div>
    </div>
    <div class="first">
      <div style="border-width: 5px; border-radius: 15px;">5px width / 15px radius</div>
    </div>
    <div class="first">
      <div style="border-width: 5px; border-left-width: 15px; border-right-width: 15px; border-radius: 15px;">5px 15px width / 15px radius</div>
    </div>
    <div class="first">
      <div style="border-width: 5px; border-left-width: 15px; border-right-width: 15px; border-radius: 25px;">5px 15px width / 25px radius</div>
    </div>
    <div class="first">
      <div class="bunt2">Bunt 2</div>
    </div>
  </div>

  <style>
  #home {
    background: no-repeat url(':/res/img/test2.png') 0px 0px;
    overflow:hidden;
    margin:15px;
    font: "Droid Sans";
    border-left:1px solid red;
    width:50px;
    height:50px
    background-size:     cover;                      /* <------ */
    background-repeat:   no-repeat;
    background-position: center center;              /* optional, center the image */

  }

  #home:hover         { background: no-repeat url(':/res/img/test2.png') -29px 0px; overflow:hidden; }
  #home:active        { background: no-repeat url(':/res/img/test2.png') -58px 0px; overflow:hidden; }

  #hoverme {
    background: red;
    overflow:hidden;
    margin:15px;
    font: "Droid Sans";
    border-left:1px solid red;
    width:50px;
    height:50px
  }

  #hoverme:hover         { background: green; overflow:hidden;cursor:pointer; }
  #hoverme:active        { background: orange; overflow:hidden;cursor:pointer; }
  </style>

  Lorem Ipsum<input width="50px" height="50px" type="checkbox" name="nameOfChoice" value="1" style="width:50px;height:50px;background:darkgrey;">

  <customtag>customtag</customtag>

  <div id='home'>dddd</div>

  <div id='hoverme'>hoverme</div>

  <ul><li>One� І� °� ї� І� °� ї</li><li>Zwei\u6211\u662F\u4E2D\u6587</li><li>Trois</li></ul>
  <bold>123SFSDFDFSDF</bold>
  <strong>emphasized text</strong>
  <span style='font-weight: bold;'>bold text</span>
  <p>Line1.1 Line1.2<br />Line2</p><ul><li>One</li><li>Zwei</li><li>Trois</li></ul>
  <table><tr><th>H1</th><th>H2</th></tr><tr><td>C1.1</td><td>C1.2</td></tr><tr><td>C2.1</td><td>C2.2</td></tr></table>


  Lorem Ipsum<input width="50px" height="50px" type="checkbox" name="nameOfChoice" value="1" style="width:50px;height:50px;background:darkgrey;">

  <customtag>customtag</customtag>
  <link rel="stylesheet" href="://res/css/reset.css">
  <link rel="stylesheet" href="://res/css/bootstrap.css">
  <link rel="stylesheet" href="://res/css/test1.css">

  <div id='home'>dddd</div>

  <div style='margin:15px;font: "Droid Sans";background:#CCC;border-bottom:6px solid yellow;width:30px;height:30px'>ТЕКСТ1 УСtext1</div>
  <div style='font: "Arial Unicode MS";background:cyan;border-right:6px solid yellow;width:30px;height:30px'>ТЕКСТ2</div>
  <div style='font: "Cousine Regular";background:#EEE;border:3px solid lightblue;width:30px;height:30px'>ТЕКСТ3</div>
  <div style='font: "Font Awesome 5 Free";background:pink;border-left:16px solid grey;width:30px;height:30px'>
  )raw";

  htmlStr += "\uf15c ? ";
  htmlStr += u8"\uf15c";
  htmlStr += u8"\uf118";
  htmlStr += u8"\uf118 ? \uf118";


  htmlStr += R"raw(\uf118 ? \uf118</div>

  <ul><li>One� І� °� ї� І� °� ї</li><li>Zwei\u6211\u662F\u4E2D\u6587</li><li>Trois</li></ul>
  <bold>123SFSDFDFSDF</bold>
  <strong>emphasized text</strong>
  <span style='font-weight: bold;'>bold text</span>
  <p>Line1.1 Line1.2<br />Line2</p><ul><li>One</li><li>Zwei</li><li>Trois</li></ul>
  <table><tr><th>H1</th><th>H2</th></tr><tr><td>C1.1</td><td>C1.2</td></tr><tr><td>C2.1</td><td>C2.2</td></tr></table>
  <img src=':/res/img/test.png' alt=''>
  <img src=':/res/img/test2.png' alt=''>


<style>
* {
  box-sizing: border-box;
}

.header h1 {
  font-size: 50px;
}

/* Style the top navigation bar */
.topnav {
  overflow: hidden;
  background-color: #333;
}

/* Style the topnav links */
.topnav a {
  float: left;
  display: block;
  color: #f2f2f2;
  text-align: center;
  padding: 14px 16px;
  text-decoration: none;
}

/* Change color on hover */
.topnav a:hover {
  background-color: #ddd;
  color: black;
}

/* Create two unequal columns that floats next to each other */
/* Left column */
.leftcolumn {
  float: left;
  width: 75%;
}

/* Right column */
.rightcolumn {
  float: left;
  width: 25%;
  background-color: #f1f1f1;
  padding-left: 20px;
}

/* Fake image */
.fakeimg {
  background-color: #aaa;
  width: 100%;
  padding: 20px;
}

/* Add a card effect for articles */
.card {
  background-color: white;
  padding: 20px;
  margin-top: 20px;
}

/* Clear floats after the columns */
.row:after {
  content: "";
  display: table;
  clear: both;
}

/* Footer */
.footer {
  padding: 20px;
  text-align: center;
  background: #ddd;
  margin-top: 20px;
}

/* Responsive layout - when the screen is less than 800px wide, make the two columns stack on top of each other instead of next to each other */
@media screen and (max-width: 800px) {
  .leftcolumn, .rightcolumn {
    width: 100%;
    padding: 0;
  }
}

/* Responsive layout - when the screen is less than 400px wide, make the navigation links stack on top of each other instead of next to each other */
@media screen and (max-width: 400px) {
  .topnav a {
    float: none;
    width: 100%;
  }
}

div.polaroid {
  width: 250px;
  height: 250px;
  text-align: center;
}

div.container {
  padding: 10px;
}
#borderimg {
  border: 10px solid transparent;
  padding: 15px;
 border: 5px solid grey;
 border-image: url(':/res/img/test2.png');
}
</style>

<h2>Polaroid Images / Cards</h2>

<div class="polaroid">
  <img src=':/res/img/test2.png' alt="Norway" style="width:100%">
  <div class="container">
    <p>Hardanger, Norway</p>
  </div>
</div>

<div class="header">
  <h1>My Website</h1>
  <p>Resize the browser window to see the effect.</p>
</div>

<div class="topnav">
  <a href="#">Link</a>
  <a href="#">Link</a>
  <a href="#">Link</a>
  <a href="#" style="float:right">Link</a>
</div>

<div class="row">
  <div class="leftcolumn">
    <div class="card">
      <h2>TITLE HEADING</h2>
      <h5>Title description, Dec 7, 2017</h5>
      <div class="fakeimg" style="height:200px;">Image</div>
      <p>Some text..</p>
      <p>Sunt in culpa qui officia deserunt mollit anim id est laborum consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco.</p>
    </div>
    <div class="card">
      <h2>TITLE HEADING</h2>
      <h5>Title description, Sep 2, 2017</h5>
      <div class="fakeimg" style="height:200px;">Image</div>
      <p>Some text..</p>
      <p>Sunt in culpa qui officia deserunt mollit anim id est laborum consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco.</p>
    </div>
  </div>
  <div class="rightcolumn">
    <div class="card">
      <h2>About Me</h2>
      <div class="fakeimg" style="height:100px;">Image</div>
      <p>Some text about me in culpa qui officia deserunt mollit anim..</p>
    </div>
    <div class="card">
      <h3>Popular Post</h3>
      <div class="fakeimg"><p>Image</p></div>
      <div class="fakeimg"><p>Image</p></div>
      <div class="fakeimg"><p>Image</p></div>
    </div>
    <div class="card">
      <h3>Follow Me</h3>
      <p>Some text..</p>
    </div>
  </div>
</div>

<div class="footer">
  <h2>Footer</h2>
</div>

<h1>The border-image Property</h1>

<p>Here, the middle sections of the image are repeated to create the border:</p>
<p id="borderimg">border-image;</p>

<p>Here is the original image:</p><img src=':/res/img/test2.png'>
<p><strong>Note:</strong> Internet Explorer 10, and earlier versions, do not support the border-image property.</p>


  </body>
  </html>)raw";

  const char* html = htmlStr.c_str();

  //QString html2 = QString::fromUtf8(qPrintable(u8"F?llungRaupeStepС‹� І� °С‹� І� °text"));//tr("F?llungRaupeStepС‹� І� °С‹� І� °text");


  //auto doc = litehtml::document::createFromUTF8("<html><body><ul><li>One</li><li>Zwei</li><li>Trois</li></ul></body></html>", &c, &ctxt);
  //auto doc = litehtml::document::createFromUTF8("<html><body><p>Line1.1 Line1.2<br />Line2</p><ul><li>One</li><li>Zwei</li><li>Trois</li></ul></body></html>", &c, &ctxt);
  //auto doc = litehtml::document::createFromUTF8("<html><body><a href=\"http://linuxfr.org/\">DLFP</a></body></html>", &c, &ctxt);
  //auto doc = litehtml::document::createFromUTF8("<html><body><table><tr><th>H1</th><th>H2</th></tr><tr><td>C1.1</td><td>C1.2</td></tr><tr><td>C2.1</td><td>C2.2</td></tr></table></body></html>", c, &ctxt);
  auto doc = litehtml::document::createFromUTF8(qPrintable(html), m_litehtmlWidget->getContainer(), &ctxt);
  m_litehtmlWidget->getContainer()->set_document(doc);

  m_litehtmlWidget->show();

  ui->scrollAreaVerticalLayout->addWidget(m_litehtmlWidget);
  //c->setGeometry(0, 0, 500, 1200);

  /*//ui->scrollArea->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  //ui->scrollArea->setGeometry(0,0,400, 400);
  ui->scrollArea->verticalScrollBar()->setMinimum(0);
  ui->scrollArea->verticalScrollBar()->setMaximum(1500);//maximum();
  ui->scrollArea->verticalScrollBar()->setValue(50);//maximum();
  ui->scrollArea->verticalScrollBar()->setRange(0, 1500);
  //ui->scrollArea->verticalScrollBar()->setFixedSize(10, 10);
  ui->scrollArea->setEnabled(true);
  ui->scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
  ui->scrollArea->resize(500,500);
  ui->scrollArea->setWidgetResizable(true);*/

  /*auto slider = new QSlider(Qt::Vertical);
  slider->setFocusPolicy(Qt::StrongFocus);
  slider->setTickPosition(QSlider::TicksBothSides);
  slider->setTickInterval(10);
  slider->setSingleStep(1);
  connect(slider, &QSlider::valueChanged, slider, [](int val){
    qDebug() << val;
  });

  ui->scrollAreaVerticalLayout->addWidget(slider);*/

  QScrollBar* scrollBar = new QScrollBar(Qt::Vertical);
  scrollBar->setParent(this);
  scrollBar->setFocusPolicy(Qt::StrongFocus);
  scrollBar->setMinimum(0);
  scrollBar->setMaximum(0);
  scrollBar->setValue(0);
  scrollBar->setSingleStep(1);
  scrollBar->setInvertedControls(false);
  connect(scrollBar, &QScrollBar::valueChanged, scrollBar, [this](int val){
    //qDebug() << "QScrollBar::valueChanged " << val;
    m_litehtmlWidget->getContainer()->setScrollY(-val);
    m_litehtmlWidget->repaint();
  });

  connect(m_litehtmlWidget->getContainer(), &container_qt5::docSizeChanged, m_litehtmlWidget->getContainer(), [this, scrollBar](int w, int h){
    //qDebug() << "docSizeChanged " << w << h;
    if (!scrollBar) {
      return;
    }

    const int singleVisiblePageHeight = m_litehtmlWidget->height();

    ///\note substracts already visible area = widget size
    const int alailibleScroll = h - singleVisiblePageHeight;
    scrollBar->setMaximum(alailibleScroll);
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
