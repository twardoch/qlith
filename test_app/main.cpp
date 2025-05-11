#include <QApplication>
#include <QMainWindow>
#include <QPushButton>
#include <QVBoxLayout>
#include <QLabel>
#include <QFile>
#include <QDir>
#include <QDebug>
#include <QFontDatabase>
#include <QResource>
#include <QMessageBox>

// Simple class to test font loading
class FontTester : public QMainWindow
{
    Q_OBJECT
public:
    FontTester()
    {
        setWindowTitle("Font Test App");
        resize(600, 400);

        QWidget *centralWidget = new QWidget(this);
        QVBoxLayout *layout = new QVBoxLayout(centralWidget);

        // Create UI components
        QLabel *statusLabel = new QLabel("Status: Ready", centralWidget);
        QPushButton *testButton = new QPushButton("Test Font Loading", centralWidget);
        QLabel *resultLabel = new QLabel(centralWidget);
        resultLabel->setWordWrap(true);
        resultLabel->setMinimumHeight(300);

        // Add to layout
        layout->addWidget(statusLabel);
        layout->addWidget(testButton);
        layout->addWidget(resultLabel);

        setCentralWidget(centralWidget);

        // Connect button
        connect(testButton, &QPushButton::clicked, [=]()
                {
            statusLabel->setText("Status: Testing...");
            
            QString result = "Font Test Results:\n\n";
            
            // Print resource directories
            result += "Current directory: " + QDir::currentPath() + "\n";
            
            // Check if we can find resources
            QDir resourceDir(":/");
            result += "Resource directory exists: " + QString(resourceDir.exists() ? "Yes" : "No") + "\n";
            
            if (resourceDir.exists()) {
                result += "Resource directory contents:\n";
                foreach (QString entry, resourceDir.entryList()) {
                    result += "- " + entry + "\n";
                }
            }
            
            // Try to check for font file
            QFile fontFile(":/font/Cousine-Regular.ttf");
            result += "Font file exists: " + QString(fontFile.exists() ? "Yes" : "No") + "\n";
            
            if (fontFile.exists()) {
                if (fontFile.open(QIODevice::ReadOnly)) {
                    QByteArray fontData = fontFile.readAll();
                    fontFile.close();
                    
                    result += "Font file size: " + QString::number(fontData.size()) + " bytes\n";
                    
                    // Try to load the font
                    int fontId = QFontDatabase::addApplicationFontFromData(fontData);
                    if (fontId != -1) {
                        QStringList families = QFontDatabase::applicationFontFamilies(fontId);
                        result += "Font loaded successfully. Font families:\n";
                        foreach (QString family, families) {
                            result += "- " + family + "\n";
                        }
                    } else {
                        result += "Failed to load font from data\n";
                    }
                } else {
                    result += "Failed to open font file\n";
                }
            } else {
                // Try alternative path
                QFile altFontFile(":/res/font/Cousine-Regular.ttf");
                result += "Alternate font file exists: " + QString(altFontFile.exists() ? "Yes" : "No") + "\n";
                
                if (altFontFile.exists()) {
                    // Same test for alternate path
                    if (altFontFile.open(QIODevice::ReadOnly)) {
                        // Process font
                        QByteArray fontData = altFontFile.readAll();
                        altFontFile.close();
                        
                        result += "Alternate font file size: " + QString::number(fontData.size()) + " bytes\n";
                        
                        // Try to load the font
                        int fontId = QFontDatabase::addApplicationFontFromData(fontData);
                        if (fontId != -1) {
                            QStringList families = QFontDatabase::applicationFontFamilies(fontId);
                            result += "Alternate font loaded successfully. Font families:\n";
                            foreach (QString family, families) {
                                result += "- " + family + "\n";
                            }
                        } else {
                            result += "Failed to load alternate font from data\n";
                        }
                    } else {
                        result += "Failed to open alternate font file\n";
                    }
                }
            }
            
            // Show results
            resultLabel->setText(result);
            statusLabel->setText("Status: Test complete"); });
    }
};

#include "main.moc"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // Enable resource debugging
    QResource::registerResource("../resources/res.qrc");
    qDebug() << "Resource registration result:"
             << QResource::registerResource("../resources/res.qrc");

    FontTester tester;
    tester.show();

    return app.exec();
}