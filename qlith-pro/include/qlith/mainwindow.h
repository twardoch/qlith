// this_file: qlith-pro/include/qlith/mainwindow.h
// -*- coding: utf-8 -*-

#pragma once

#include <QMainWindow>
#include <QResizeEvent>
#include <QString>
#include <QTextEdit>
#include <QSize>

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
  
  /**
   * @brief Load HTML file
   * @param filePath Path to the HTML file
   */
  void loadFile(const QString& filePath);
  
  /**
   * @brief Export the current view to an SVG file
   * @param filePath Path where to save the SVG file
   * @return True if export was successful, false otherwise
   */
  bool exportToSvg(const QString& filePath);
  
  /**
   * @brief Export the current view to a PNG file
   * @param filePath Path where to save the PNG file
   * @return True if export was successful, false otherwise
   */
  bool exportToPng(const QString& filePath);

  /**
   * @brief Set the rendering size for export operations.
   * @param size The size to use for rendering.
   */
  void setRenderSize(const QSize& size);

signals:
  /**
   * @brief Signal emitted when a document is loaded
   * @param success True if document was loaded successfully
   */
  void documentLoaded(bool success);

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
  
  /**
   * @brief Handle document loaded event from litehtmlWidget
   * @param success True if loading was successful
   */
  void onDocumentLoaded(bool success);

private:
  /**
   * @brief Initialize or reinitialize the litehtmlWidget
   */
  void initializeLitehtmlWidget();

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
  QTextEdit *m_htmlEditor;          // HTML editor for debug mode
  // litehtml_context m_context; // No longer needed
  bool m_debugMode; // Debug mode flag
  QSize m_renderSize; // Rendering size for export operations
};
