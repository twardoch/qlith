// this_file: qlith-pro/include/qlith/mainwindow.h
// -*- coding: utf-8 -*-

#pragma once

#include <QMainWindow>
#include <QResizeEvent>

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

private:
  /**
   * @brief Load example HTML content
   */
  void loadExample();

  Ui::MainWindow *ui;
  litehtmlWidget *m_litehtmlWidget;
};
