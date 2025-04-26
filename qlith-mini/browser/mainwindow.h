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
    
    // Menus
    QMenu* m_fileMenu;
    QMenu* m_viewMenu;
    QMenu* m_helpMenu;
    
    float m_zoomFactor;
    QUrl m_startUrl;
    QList<QUrl> m_history;
    int m_historyIndex;
};

#endif // MAINWINDOW_H 