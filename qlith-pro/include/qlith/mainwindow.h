// this_file: qlith-pro/include/qlith/mainwindow.h
// -*- coding: utf-8 -*-

#pragma once

#include <QMainWindow>

#include "qlith/container_qt5.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
Q_OBJECT

public:
  explicit MainWindow(QWidget *parent = nullptr);
  ~MainWindow();

private:
  Ui::MainWindow *ui;
  litehtmlWidget* m_litehtmlWidget;
  litehtml::context ctxt;
};
