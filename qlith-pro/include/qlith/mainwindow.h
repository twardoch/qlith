// this_file: qlith-pro/include/qlith/mainwindow.h
// -*- coding: utf-8 -*-

#pragma once

#include <QMainWindow>
#include <QResizeEvent>
#include <QString>

// Forward declarations only
// class litehtml_context; // No longer needed
class litehtmlWidget;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
Q_OBJECT

public:
  explicit MainWindow(bool debugMode = false, QWidget *parent = nullptr);
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

  // /**
  //  * @brief Initialize the HTML context and load master CSS - No longer needed
  //  */
  // void initializeContext();

  /**
   * @brief Set up the scrollbar for the HTML content
   */
  void setupScrollbar();

  Ui::MainWindow *ui;
  litehtmlWidget *m_litehtmlWidget; // Keep this for managing the widget instance
  // litehtml_context m_context; // No longer needed
  bool m_debugMode; // Debug mode flag
};
