#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QUrl>
#include <QActionGroup>

class QLiteHtmlWidget;
class QLineEdit;
class QLabel;
class QComboBox;
class QNetworkAccessManager;
class QNetworkReply;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

    void loadUrl(const QUrl &url);
    void loadFile(const QString &filePath);
    
    // Export methods
    void saveAsPng(const QString &filename);
    void saveAsSvg(const QString &filename);

protected:
    void closeEvent(QCloseEvent *event) override;

signals:
    void loadFinished();

private slots:
    void onLoadFinished();
    void reloadPage();
    void goBack();
    void goForward();
    void openFile();
    void zoomIn();
    void zoomOut();
    void resetZoom();
    void updateZoomLabel(qreal zoom);
    void showAbout();
    void toggleDarkMode();
    void setDarkMode(bool enabled);
    void linkClicked(const QUrl &url);
    void updateWindowTitle();
    void urlEntered();

private:
    void createActions();
    void createToolBar();
    void createStatusBar();
    void readSettings();
    void writeSettings();
    void updateNavigationActions();

    QLiteHtmlWidget *m_htmlWidget;
    QNetworkAccessManager *m_networkManager;
    QLineEdit *m_urlEdit;
    QLabel *m_statusLabel;
    QLabel *m_zoomLabel;
    QComboBox *m_zoomCombo;
    QActionGroup *m_zoomActions;
    
    QAction *m_backAction;
    QAction *m_forwardAction;
    QAction *m_reloadAction;
    QAction *m_openAction;
    QAction *m_exitAction;
    QAction *m_zoomInAction;
    QAction *m_zoomOutAction;
    QAction *m_resetZoomAction;
    QAction *m_darkModeAction;
    QAction *m_aboutAction;
    
    bool m_darkModeEnabled;
    QList<QUrl> m_history;
    int m_currentHistoryIndex;
    int m_maxHistorySize;
};

#endif // MAINWINDOW_H 