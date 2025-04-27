// this_file: qlith-pro/include/qlith/mainwindow.h
// -*- coding: utf-8 -*-

#pragma once

#include <QMainWindow>
#include <QResizeEvent>
#include <QString>

namespace Ui {
class MainWindow;
}

class litehtmlWidget;

class MainWindow : public QMainWindow
{
Q_OBJECT

public:
  explicit MainWindow(QWidget *parent = nullptr);
  ~MainWindow();

protected:
  /**
   * @brief Handle window resize events
   * @param event The resize event
   */
  void resizeEvent(QResizeEvent* event) override;

private slots:
  /**
   * @brief Navigate to the URL entered in the address bar
   */
  void navigateToUrl();
  
  /**
   * @brief Handle URL navigation
   * @param url The URL to navigate to
   */
  void loadUrl(const QString& url);

private:
  /**
   * @brief Load example HTML content
   */
  void loadExample();

  Ui::MainWindow *ui;
  litehtmlWidget *m_litehtmlWidget;
};
