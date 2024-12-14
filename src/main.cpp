/**
 * @file main.cpp
 * @brief AetherHUD - An FFXIV-styled WebEngine-based overlay system
 * 
 * This application creates frameless, click-through windows that can display web content
 * as overlays for Final Fantasy XIV. It includes game-specific features like hotkey support,
 * smart transparency, and FFXIV-styled UI elements.
 * 
 * Copyright (c) 2024 Your Company Name
 * License: MIT
 */

#include <QApplication>
#include <QWebEngineView>
#include <QWindow>
#include <QScreen>
#include <QWebChannel>
#include <QWebEngineProfile>
#include <QWebEngineSettings>
#include <QWebEnginePage>
#include <QShortcut>
#include <QTimer>
#include <QSettings>
#include <QWindow>
#include <QKeySequence>
#include <QJsonObject>
#include <QJsonDocument>

class AetherHUDPage : public QWebEnginePage {
    Q_OBJECT

public:
    explicit AetherHUDPage(QObject* parent = nullptr) : QWebEnginePage(parent) {}

protected:
    bool acceptNavigationRequest(const QUrl &url, NavigationType type, bool isMainFrame) override {
        if (url.scheme() == "https" || url.scheme() == "http") {
            connect(this, &QWebEnginePage::certificateError,
                    this, [this](QWebEngineCertificateError certificateError) {
                showErrorPage("Certificate Error: " + certificateError.description(), 
                            certificateError.url().toString());
                return false;
            });
        }
        return QWebEnginePage::acceptNavigationRequest(url, type, isMainFrame);
    }

public slots:
    void showErrorPage(const QString& errorMessage, const QString& failingUrl) {
        // FFXIV-styled error page with aetheric theme
        QString errorHtml = QString(R"(
            <html>
            <head>
                <style>
                    @font-face {
                        font-family: 'Cinzel';
                        src: url(':/aetherhud/fonts/Cinzel-Regular.ttf') format('truetype');
                        font-weight: normal;
                        font-style: normal;
                    }
                    body { 
                        background: rgba(0,0,0,0.85);
                        margin: 0;
                        padding: 20px;
                        font-family: 'Cinzel', serif;
                        border: 1px solid rgba(255,206,84,0.3);
                        box-shadow: 0 0 15px rgba(255,206,84,0.2);
                    }
                    h1 { 
                        color: #ffce54;
                        text-shadow: 0 0 10px rgba(255,206,84,0.5);
                        margin-bottom: 10px;
                        font-size: 24px;
                        text-transform: uppercase;
                        letter-spacing: 2px;
                    }
                    .url {
                        color: #8b8b8b;
                        font-size: 14px;
                        word-break: break-all;
                        margin-top: 10px;
                        padding: 10px;
                        background: rgba(0,0,0,0.3);
                        border-left: 3px solid #ffce54;
                    }
                </style>
            </head>
            <body>
                <h1>%1</h1>
                <div class="url">%2</div>
            </body>
            </html>
        )").arg(errorMessage.toHtmlEscaped(),
                failingUrl.toHtmlEscaped());

        setHtml(errorHtml, QUrl(failingUrl));
    }
};

class AetherHUDWindow : public QWebEngineView {
    Q_OBJECT

private:
    QString lastError;
    QWebEngineView* inspector;
    bool inspectorAttached;
    bool isLocked;
    QString currentJob;
    QSettings settings;
    QShortcut* toggleShortcut;
    QShortcut* lockShortcut;

public:
    AetherHUDWindow(const QUrl& url) 
        : QWebEngineView()
        , inspector(nullptr)
        , inspectorAttached(false)
        , isLocked(false)
        , settings("AetherHUD", "Overlay")
    {
        // Configure window for transparency and click-through
        setAttribute(Qt::WA_TranslucentBackground);
        setStyleSheet("background: transparent;");
        
        // Set window flags for overlay behavior
        setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Tool);
        
        // Initialize custom page
        AetherHUDPage* customPage = new AetherHUDPage(this);
        setPage(customPage);
        page()->setBackgroundColor(Qt::transparent);

        // Set up hotkeys
        setupHotkeys();
        
        // Initialize window position from settings
        loadWindowState();
        
        // Set up error handling
        connect(page(), &QWebEnginePage::loadFinished,
                this, &AetherHUDWindow::handleLoadFinished);
        
        setUrl(url);
    }

    ~AetherHUDWindow() {
        saveWindowState();
    }

protected:
    void mousePressEvent(QMouseEvent* event) override {
        if (!isLocked) {
            QWebEngineView::mousePressEvent(event);
        }
    }

    void mouseMoveEvent(QMouseEvent* event) override {
        if (!isLocked) {
            QWebEngineView::mouseMoveEvent(event);
        }
    }

private slots:
    void setupHotkeys() {
        // Toggle visibility shortcut (Ctrl+Shift+H by default)
        toggleShortcut = new QShortcut(QKeySequence("Ctrl+Shift+H"), this);
        connect(toggleShortcut, &QShortcut::activated,
                this, &AetherHUDWindow::toggleVisibility);

        // Toggle lock shortcut (Ctrl+Shift+L by default)
        lockShortcut = new QShortcut(QKeySequence("Ctrl+Shift+L"), this);
        connect(lockShortcut, &QShortcut::activated,
                this, &AetherHUDWindow::toggleLock);
    }

    void toggleVisibility() {
        setVisible(!isVisible());
    }

    void toggleLock() {
        isLocked = !isLocked;
        // Update cursor and border style based on lock state
        if (isLocked) {
            setCursor(Qt::ArrowCursor);
            setStyleSheet("background: transparent; border: none;");
        } else {
            setCursor(Qt::SizeAllCursor);
            setStyleSheet("background: transparent; border: 1px solid rgba(255,206,84,0.3);");
        }
    }

    void saveWindowState() {
        QJsonObject state;
        state["pos"] = QJsonObject{
            {"x", x()},
            {"y", y()}
        };
        state["size"] = QJsonObject{
            {"width", width()},
            {"height", height()}
        };
        state["visible"] = isVisible();
        state["locked"] = isLocked;

        settings.setValue(currentJob.isEmpty() ? "defaultState" : currentJob,
                        QJsonDocument(state).toJson(QJsonDocument::Compact));
    }

    void loadWindowState() {
        QString stateKey = currentJob.isEmpty() ? "defaultState" : currentJob;
        QByteArray savedState = settings.value(stateKey).toByteArray();
        
        if (!savedState.isEmpty()) {
            QJsonObject state = QJsonDocument::fromJson(savedState).object();
            QJsonObject pos = state["pos"].toObject();
            QJsonObject size = state["size"].toObject();
            
            move(pos["x"].toInt(), pos["y"].toInt());
            resize(size["width"].toInt(), size["height"].toInt());
            setVisible(state["visible"].toBool());
            isLocked = state["locked"].toBool();
        }
    }

    void handleLoadFinished(bool ok) {
        if (!ok && lastError.isEmpty()) {
            static_cast<AetherHUDPage*>(page())->showErrorPage(
                tr("Failed to load overlay"),
                url().toString()
            );
        }
    }

public slots:
    void setJob(const QString& job) {
        if (currentJob != job) {
            saveWindowState();  // Save state for current job
            currentJob = job;
            loadWindowState();  // Load state for new job
        }
    }
};

int main(int argc, char *argv[]) {
    // Set attributes before creating QApplication
    QCoreApplication::setAttribute(Qt::AA_UseSoftwareOpenGL);
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    
    // Now create the application
    QApplication app(argc, argv);
    
    // Process command line arguments
    QUrl url("http://localhost:3000");
    if (argc > 1) {
        url = QUrl::fromUserInput(argv[1]);
        if (!url.isValid()) {
            qDebug() << "Invalid URL provided:" << argv[1];
            return 1;
        }
    }
    
    // Create and show main window
    AetherHUDWindow* window = new AetherHUDWindow(url);
    window->resize(800, 600);
    window->show();
    
    return app.exec();
}

#include "main.moc"