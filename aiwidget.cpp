#include "aiwidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QSettings>
#include <QWebEngineProfile>
#include <QWebEngineCookieStore>
#include <QDir>

AIChatWindow::AIChatWindow(QWidget *parent) : QDialog(parent) {
    // 1. FORCE CHROMIUM ENGINE OPTIMIZATIONS BEFORE SYSTEM INITIALIZATION
    qputenv("QTWEBENGINE_CHROMIUM_FLAGS", 
        "--disable-gpu "
        "--disable-extensions "
        "--mute-audio "
        "--disable-notifications "
        "--single-process"
    );

    // Treat the widget as a sleek, frameless system tray dropdown panel
    setWindowFlags(Qt::Popup | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    resize(440, 600);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(6, 6, 6, 6);

    m_container = new QFrame(this);
    m_container->setObjectName("LarkAIContainer");
    m_container->setStyleSheet(
        "QFrame#LarkAIContainer {"
        "  background-color: #0d1117;" 
        "  border: 1px solid #30363d;"
        "  border-radius: 12px;"
        "}"
    );
    QVBoxLayout *containerLayout = new QVBoxLayout(m_container);

    // 2. DUAL-PROVIDER COMPACT NAVIGATION TOOLBAR
    QHBoxLayout *header = new QHBoxLayout();
    
    m_geminiBtn = new QPushButton("✨ Gemini", this);
    m_geminiBtn->setCheckable(true);
    m_geminiBtn->setStyleSheet(
        "QPushButton { color: #8b949e; background: #161b22; border: 1px solid #30363d; border-radius: 6px; padding: 4px 12px; font-size: 11px; font-weight: bold; }"
        "QPushButton:checked { color: #58a6ff; background: #21262d; border: 1px solid #58a6ff; }"
    );

    m_copilotBtn = new QPushButton("🚀 Copilot", this);
    m_copilotBtn->setCheckable(true);
    m_copilotBtn->setStyleSheet(
        "QPushButton { color: #8b949e; background: #161b22; border: 1px solid #30363d; border-radius: 6px; padding: 4px 12px; font-size: 11px; font-weight: bold; }"
        "QPushButton:checked { color: #238636; background: #21262d; border: 1px solid #238636; }"
    );

    connect(m_geminiBtn, &QPushButton::clicked, [this]() { switchAIProvider("gemini"); });
    connect(m_copilotBtn, &QPushButton::clicked, [this]() { switchAIProvider("copilot"); });

    header->addWidget(m_geminiBtn);
    header->addWidget(m_copilotBtn);
    header->addStretch();
    
    QPushButton *clearAuthBtn = new QPushButton("⚙ Sign Out", this);
    clearAuthBtn->setStyleSheet("QPushButton { color: #8b949e; background: transparent; border: none; font-size: 11px; } QPushButton:hover { color: #f85149; }");
    connect(clearAuthBtn, &QPushButton::clicked, this, &AIChatWindow::purgeSessionData);
    header->addWidget(clearAuthBtn);

    QPushButton *closeBtn = new QPushButton("✕", this);
    closeBtn->setStyleSheet("color: #8b949e; background: transparent; border: none; font-size: 13px; font-weight: bold;");
    connect(closeBtn, &QPushButton::clicked, this, &QWidget::hide);
    header->addWidget(closeBtn);
    
    containerLayout->addLayout(header);

    // 3. CHROMIUM BROWSER DESKTOP VIEWPORT
    m_webView = new QWebEngineView(this);
    m_webView->setStyleSheet("background-color: #0d1117;");
    containerLayout->addWidget(m_webView);

    // Persistent profile setup for background cookie management
    QWebEngineProfile *profile = m_webView->page()->profile();
    profile->setPersistentCookiesPolicy(QWebEngineProfile::ForcePersistentCookies);
    
    QString profileStoragePath = QDir::homePath() + "/.config/lark/ai_profile";
    profile->setPersistentStoragePath(profileStoragePath);
    profile->setCachePath(profileStoragePath + "/cache");

    mainLayout->addWidget(m_container);

    // Read initialization settings
    QSettings settings("LarkOS", "NeuroShell");
    QString activeProvider = settings.value("active_ai_provider", "gemini").toString();
    switchAIProvider(activeProvider);
}

void AIChatWindow::switchAIProvider(const QString &provider) {
    QSettings settings("LarkOS", "NeuroShell");
    settings.setValue("active_ai_provider", provider);

    m_geminiBtn->setChecked(provider == "gemini");
    m_copilotBtn->setChecked(provider == "copilot");

    if (provider == "gemini") {
        m_webView->setUrl(QUrl("https://gemini.google.com/app"));
    } else if (provider == "copilot") {
        m_webView->setUrl(QUrl("https://copilot.microsoft.com"));
    }
}

void AIChatWindow::purgeSessionData() {
    QWebEngineCookieStore *cookieStore = m_webView->page()->profile()->cookieStore();
    cookieStore->deleteAllCookies(); 
    m_webView->page()->profile()->clearHttpCache();
    m_webView->reload();
}