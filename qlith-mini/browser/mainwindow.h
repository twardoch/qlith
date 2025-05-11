// this_file: qlith/browser/mainwindow.h
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QUrl>

class QLineEdit;
class QProgressBar;
class QlithWidget;
class QAction;
class QToolBar;
class QStatusBar;
class QMenu;

/**
 * @brief The MainWindow class provides the main browser window for the qlith browser.
 * 
 * This class represents the main window of the browser application, containing
 * a QlithWidget for rendering HTML content, navigation controls, and other browser
 * functionality.
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    /**
     * @brief Constructs a MainWindow.
     * @param parent The parent widget.
     */
    explicit MainWindow(QWidget* parent = nullptr);
    
    /**
     * @brief Destroys the MainWindow.
     */
    ~MainWindow() override;
    
    /**
     * @brief Load content from a URL.
     * @param url The URL to load.
     */
    void load(const QUrl& url);
    
    /**
     * @brief Export the current view to an SVG file.
     * @param filePath The path where to save the SVG file.
     * @return True if export was successful, false otherwise.
     */
    bool exportToSvg(const QString& filePath);
    
    /**
     * @brief Export the current view to a PNG file.
     * @param filePath The path where to save the PNG file.
     * @return True if export was successful, false otherwise.
     */
    bool exportToPng(const QString& filePath);
    
    /**
     * @brief Set the rendering size for export operations.
     * @param size The size to use for rendering.
     */
    void setRenderSize(const QSize& size);

signals:
    /**
     * @brief Signal emitted when page loading is finished.
     * @param ok True if loading was successful, false otherwise.
     */
    void loadFinished(bool ok);

protected:
    void closeEvent(QCloseEvent* event) override;

private slots:
    void loadUrl();
    void updateUrlBar(const QUrl& url);
    void updateTitle(const QString& title);
    void handleLinkClick(const QUrl& url);
    void onLoadStarted();
    void onLoadFinished(bool ok);
    
    // Actions
    void goBack();
    void goForward();
    void reload();
    void stop();
    void zoomIn();
    void zoomOut();
    void resetZoom();
    void viewSource();
    void about();

private:
    void setupUi();
    void setupActions();
    void setupMenus();
    void setupToolbar();
    void setupStatusBar();
    void setupConnections();
    void loadSettings();
    void saveSettings();
    
    QlithWidget* m_htmlWidget;
    QLineEdit* m_urlEdit;
    QProgressBar* m_progressBar;
    QToolBar* m_navigationToolBar;
    QStatusBar* m_statusBar;
    
    // Menus
    QMenu* m_fileMenu;
    QMenu* m_viewMenu;
    QMenu* m_helpMenu;
    
    // Actions
    QAction* m_backAction;
    QAction* m_forwardAction;
    QAction* m_reloadAction;
    QAction* m_stopAction;
    QAction* m_zoomInAction;
    QAction* m_zoomOutAction;
    QAction* m_resetZoomAction;
    QAction* m_viewSourceAction;
    QAction* m_exitAction;
    QAction* m_aboutAction;
    
    // History management
    QList<QUrl> m_history;
    int m_historyIndex;
    QUrl m_startUrl;
    
    // Zoom
    float m_zoomFactor;
    
    // Rendering size for export operations
    QSize m_renderSize;
};

#endif // MAINWINDOW_H 