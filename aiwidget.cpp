#include "aiwidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QSettings>
#include <QWebEngineProfile>
#include <QWebEngineCookieStore>
#include <QWebEnginePage>
#include <QDir>
#include <QScrollBar>
#include <QDateTime>
#include <QJsonDocument>

AIChatWindow::AIChatWindow(QWidget *parent) : QDialog(parent), 
    m_activeProvider("gemini"),
    m_isWebMode(false),
    m_pollAttempts(0) {

    // 1. FORCE CHROMIUM ENGINE OPTIMIZATIONS BEFORE SYSTEM INITIALIZATION
    qputenv("QTWEBENGINE_CHROMIUM_FLAGS", 
        "--single-process "
        "--disable-gpu "
        "--mute-audio "
        "--disable-extensions "
        "--disable-notifications"
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
    
    m_toggleModeBtn = new QPushButton("🌐 Web View", this);
    m_toggleModeBtn->setStyleSheet(
        "QPushButton { color: #8b949e; background: #161b22; border: 1px solid #30363d; border-radius: 6px; padding: 4px 12px; font-size: 11px; font-weight: bold; }"
        "QPushButton:hover { color: #58a6ff; background: #21262d; }"
    );
    connect(m_toggleModeBtn, &QPushButton::clicked, this, &AIChatWindow::toggleViewMode);
    header->addWidget(m_toggleModeBtn);

    QPushButton *clearAuthBtn = new QPushButton("⚙ Sign Out", this);
    clearAuthBtn->setStyleSheet("QPushButton { color: #8b949e; background: transparent; border: none; font-size: 11px; } QPushButton:hover { color: #f85149; }");
    connect(clearAuthBtn, &QPushButton::clicked, this, &AIChatWindow::purgeSessionData);
    header->addWidget(clearAuthBtn);

    QPushButton *closeBtn = new QPushButton("✕", this);
    closeBtn->setStyleSheet("color: #8b949e; background: transparent; border: none; font-size: 13px; font-weight: bold;");
    connect(closeBtn, &QPushButton::clicked, this, &QWidget::hide);
    header->addWidget(closeBtn);
    
    containerLayout->addLayout(header);

    // 3. STACKED WORKSPACE
    m_viewStack = new QStackedWidget(this);

    // --- Index 0: Native Chat Layout ---
    m_nativeWidget = new QWidget(this);
    QVBoxLayout *nativeLayout = new QVBoxLayout(m_nativeWidget);
    nativeLayout->setContentsMargins(0, 4, 0, 0);

    m_chatDisplay = new QTextBrowser(this);
    m_chatDisplay->setOpenExternalLinks(true);
    m_chatDisplay->setStyleSheet(
        "QTextBrowser {"
        "  background-color: #0d1117;"
        "  border: none;"
        "  color: #c9d1d9;"
        "}"
    );
    m_chatDisplay->document()->setDefaultStyleSheet(
        "body { font-family: sans-serif; background-color: #0d1117; color: #c9d1d9; }"
        "code, pre { font-family: monospace; background-color: #161b22; border-radius: 4px; padding: 2px; }"
        "pre { padding: 8px; border: 1px solid #30363d; overflow-x: auto; }"
    );
    nativeLayout->addWidget(m_chatDisplay);

    m_statusLabel = new QLabel("Idle", this);
    m_statusLabel->setStyleSheet(
        "QLabel {"
        "  color: #8b949e;"
        "  font-size: 11px;"
        "  padding: 4px 6px;"
        "}"
    );
    nativeLayout->addWidget(m_statusLabel);

    QHBoxLayout *inputLayout = new QHBoxLayout();
    m_inputField = new QLineEdit(this);
    m_inputField->setPlaceholderText("Ask a question...");
    m_inputField->setStyleSheet(
        "QLineEdit {"
        "  color: #f0f6fc;"
        "  background-color: #161b22;"
        "  border: 1px solid #30363d;"
        "  border-radius: 18px;"
        "  padding: 8px 14px;"
        "  font-size: 13px;"
        "}"
        "QLineEdit:focus {"
        "  border: 1px solid #58a6ff;"
        "}"
    );
    connect(m_inputField, &QLineEdit::returnPressed, this, &AIChatWindow::handleSendPressed);
    inputLayout->addWidget(m_inputField);

    m_sendBtn = new QPushButton("Send", this);
    m_sendBtn->setStyleSheet(
        "QPushButton {"
        "  color: #c9d1d9;"
        "  background-color: #21262d;"
        "  border: 1px solid #30363d;"
        "  border-radius: 18px;"
        "  padding: 8px 16px;"
        "  font-weight: bold;"
        "  font-size: 12px;"
        "}"
        "QPushButton:hover {"
        "  background-color: #30363d;"
        "}"
    );
    connect(m_sendBtn, &QPushButton::clicked, this, &AIChatWindow::handleSendPressed);
    inputLayout->addWidget(m_sendBtn);
    nativeLayout->addLayout(inputLayout);

    m_viewStack->addWidget(m_nativeWidget);

    // --- Setup persistent profiles and cache path ---
    QString profileStoragePath = QDir::homePath() + "/.config/lark/ai_profile";
    QDir().mkpath(profileStoragePath);

    QWebEngineProfile *profile = new QWebEngineProfile("LarkAIProfile", this);
    profile->setPersistentCookiesPolicy(QWebEngineProfile::ForcePersistentCookies);
    profile->setPersistentStoragePath(profileStoragePath);
    profile->setCachePath(profileStoragePath + "/cache");

    // --- Index 1: Gemini Background View ---
    m_geminiView = new QWebEngineView(this);
    m_geminiView->setPage(new QWebEnginePage(profile, this));
    m_geminiView->setStyleSheet("background-color: #0d1117;");
    m_viewStack->addWidget(m_geminiView);

    // --- Index 2: Copilot Background View ---
    m_copilotView = new QWebEngineView(this);
    m_copilotView->setPage(new QWebEnginePage(profile, this));
    m_copilotView->setStyleSheet("background-color: #0d1117;");
    m_viewStack->addWidget(m_copilotView);

    containerLayout->addWidget(m_viewStack);
    mainLayout->addWidget(m_container);

    // --- Load History & Config ---
    loadChatHistory();

    // Start background loading of endpoints
    m_geminiView->setUrl(QUrl("https://gemini.google.com/app"));
    m_copilotView->setUrl(QUrl("https://copilot.microsoft.com"));

    // Set up polling timer
    m_pollTimer = new QTimer(this);
    m_pollTimer->setInterval(1000);
    connect(m_pollTimer, &QTimer::timeout, this, &AIChatWindow::pollResponse);

    // Restore provider state
    QSettings settings("LarkOS", "NeuroShell");
    QString savedProvider = settings.value("active_ai_provider", "gemini").toString();
    switchAIProvider(savedProvider);
}

AIChatWindow::~AIChatWindow() {
    saveChatHistory();
}

void AIChatWindow::switchAIProvider(const QString &provider) {
    m_activeProvider = provider;
    m_geminiBtn->setChecked(provider == "gemini");
    m_copilotBtn->setChecked(provider == "copilot");

    QSettings settings("LarkOS", "NeuroShell");
    settings.setValue("active_ai_provider", provider);

    if (m_isWebMode) {
        m_viewStack->setCurrentIndex(provider == "gemini" ? 1 : 2);
    } else {
        m_viewStack->setCurrentIndex(0);
    }

    // Refresh chat history display for active provider
    m_chatDisplay->clear();
    for (const auto &msgVal : m_chatHistory) {
        QJsonObject msg = msgVal.toObject();
        if (msg.value("provider").toString() == provider) {
            appendChatMessage(
                msg.value("isUser").toBool() ? "You" : (provider == "gemini" ? "Gemini" : "Copilot"),
                msg.value("text").toString(),
                msg.value("isUser").toBool()
            );
        }
    }
}

void AIChatWindow::toggleViewMode() {
    m_isWebMode = !m_isWebMode;
    m_toggleModeBtn->setText(m_isWebMode ? "✨ Native View" : "🌐 Web View");
    
    if (m_isWebMode) {
        m_viewStack->setCurrentIndex(m_activeProvider == "gemini" ? 1 : 2);
    } else {
        m_viewStack->setCurrentIndex(0);
        
        // Re-sync display on return
        m_chatDisplay->clear();
        for (const auto &msgVal : m_chatHistory) {
            QJsonObject msg = msgVal.toObject();
            if (msg.value("provider").toString() == m_activeProvider) {
                appendChatMessage(
                    msg.value("isUser").toBool() ? "You" : (m_activeProvider == "gemini" ? "Gemini" : "Copilot"),
                    msg.value("text").toString(),
                    msg.value("isUser").toBool()
                );
            }
        }
    }
}

void AIChatWindow::handleSendPressed() {
    QString text = m_inputField->text().trimmed();
    if (text.isEmpty()) return;

    m_inputField->clear();

    // 1. Log and display user message natively
    QJsonObject userMsg;
    userMsg["provider"] = m_activeProvider;
    userMsg["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    userMsg["isUser"] = true;
    userMsg["text"] = text;
    m_chatHistory.append(userMsg);
    saveChatHistory();

    appendChatMessage("You", text, true);

    // 2. Start Automation State
    m_statusLabel->setText("⚡ Injecting query to backend...");
    m_pollAttempts = 0;
    m_lastExtractedResponse = "";
    
    startAutomation(text);
    m_pollTimer->start();
}

void AIChatWindow::startAutomation(const QString &text) {
    QWebEngineView *activeView = (m_activeProvider == "gemini") ? m_geminiView : m_copilotView;
    if (!activeView) return;

    QString escaped = text;
    escaped.replace("\\", "\\\\");
    escaped.replace("\"", "\\\"");
    escaped.replace("\n", "\\n");
    escaped.replace("\r", "\\r");

    QString jsCode;
    if (m_activeProvider == "gemini") {
        jsCode = QString(
            "(function() {"
            "  var input = document.querySelector('rich-textarea div[contenteditable=\"true\"]');"
            "  if (input) {"
            "    input.focus();"
            "    input.innerText = \"%1\";"
            "    input.dispatchEvent(new Event('input', { bubbles: true }));"
            "    input.dispatchEvent(new Event('change', { bubbles: true }));"
            "    setTimeout(function() {"
            "      var btn = document.querySelector('button[aria-label=\"Send message\"]') || document.querySelector('.send-button');"
            "      if (btn) btn.click();"
            "    }, 150);"
            "    return true;"
            "  }"
            "  return false;"
            "})();"
        ).arg(escaped);
    } else {
        jsCode = QString(
            "(function() {"
            "  var input = document.querySelector('textarea') || document.querySelector('[contenteditable=\"true\"]') || document.querySelector('#searchbox');"
            "  if (input) {"
            "    input.focus();"
            "    if (input.tagName === 'TEXTAREA' || input.tagName === 'INPUT') {"
            "      input.value = \"%1\";"
            "    } else {"
            "      input.innerText = \"%1\";"
            "    }"
            "    input.dispatchEvent(new Event('input', { bubbles: true }));"
            "    input.dispatchEvent(new Event('change', { bubbles: true }));"
            "    setTimeout(function() {"
            "      var btn = document.querySelector('button[aria-label=\"Submit\"]') || document.querySelector('button[aria-label=\"Send message\"]') || document.querySelector('.send-button') || document.querySelector('button.submit');"
            "      if (btn) btn.click();"
            "    }, 150);"
            "    return true;"
            "  }"
            "  return false;"
            "})();"
        ).arg(escaped);
    }

    activeView->page()->runJavaScript(jsCode);
}

void AIChatWindow::pollResponse() {
    QWebEngineView *activeView = (m_activeProvider == "gemini") ? m_geminiView : m_copilotView;
    if (!activeView) return;

    m_pollAttempts++;

    QString jsCode;
    if (m_activeProvider == "gemini") {
        jsCode = 
            "(function() {"
            "  var responses = document.querySelectorAll('message-content');"
            "  if (responses.length > 0) {"
            "    var last = responses[responses.length - 1];"
            "    var html = last.innerHTML || last.innerText;"
            "    var sendBtn = document.querySelector('button[aria-label=\"Send message\"]') || document.querySelector('.send-button');"
            "    var isFinished = sendBtn && !sendBtn.disabled && !document.querySelector('mat-progress-bar');"
            "    return JSON.stringify({text: html, finished: !!isFinished});"
            "  }"
            "  return JSON.stringify({text: '', finished: false});"
            "})();";
    } else {
        jsCode = 
            "(function() {"
            "  var responses = document.querySelectorAll('.message') || document.querySelectorAll('[role=\"presentation\"]') || document.querySelectorAll('.chat-message');"
            "  if (responses.length > 0) {"
            "    var last = responses[responses.length - 1];"
            "    var html = last.innerHTML || last.innerText;"
            "    var sendBtn = document.querySelector('button[aria-label=\"Submit\"]') || document.querySelector('button[aria-label=\"Send message\"]') || document.querySelector('.send-button');"
            "    var isFinished = sendBtn && !sendBtn.disabled;"
            "    return JSON.stringify({text: html, finished: !!isFinished});"
            "  }"
            "  return JSON.stringify({text: '', finished: false});"
            "})();";
    }

    activeView->page()->runJavaScript(jsCode, [this](const QVariant &res) {
        QString jsonStr = res.toString();
        QJsonDocument doc = QJsonDocument::fromJson(jsonStr.toUtf8());
        if (!doc.isNull() && doc.isObject()) {
            QJsonObject obj = doc.object();
            QString text = obj.value("text").toString().trimmed();
            bool finished = obj.value("finished").toBool();

            if (!text.isEmpty()) {
                m_statusLabel->setText(finished ? "✅ Response complete" : "⏳ Response streaming...");
                
                if (text != m_lastExtractedResponse) {
                    m_lastExtractedResponse = text;
                    
                    // Update display by drawing history + active stream
                    m_chatDisplay->clear();
                    for (const auto &msgVal : m_chatHistory) {
                        QJsonObject msg = msgVal.toObject();
                        if (msg.value("provider").toString() == m_activeProvider) {
                            appendChatMessage(
                                msg.value("isUser").toBool() ? "You" : (m_activeProvider == "gemini" ? "Gemini" : "Copilot"),
                                msg.value("text").toString(),
                                msg.value("isUser").toBool()
                            );
                        }
                    }
                    appendChatMessage(
                        m_activeProvider == "gemini" ? "Gemini" : "Copilot",
                        text,
                        false
                    );
                }
            }

            if (finished || m_pollAttempts > 120) { // Timeout after 120 seconds
                m_pollTimer->stop();
                m_statusLabel->setText("Idle");
                
                // Commit finalized message to history
                if (!m_lastExtractedResponse.isEmpty()) {
                    QJsonObject newMsg;
                    newMsg["provider"] = m_activeProvider;
                    newMsg["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);
                    newMsg["isUser"] = false;
                    newMsg["text"] = m_lastExtractedResponse;
                    m_chatHistory.append(newMsg);
                    saveChatHistory();
                }
            }
        }
    });
}

void AIChatWindow::purgeSessionData() {
    m_chatHistory.clear();
    saveChatHistory();
    m_chatDisplay->clear();

    QWebEngineCookieStore *cookieStore = m_geminiView->page()->profile()->cookieStore();
    cookieStore->deleteAllCookies(); 
    m_geminiView->page()->profile()->clearHttpCache();
    
    m_geminiView->reload();
    m_copilotView->reload();
    
    m_statusLabel->setText("Session cookies and chat history successfully purged.");
}

void AIChatWindow::loadChatHistory() {
    QString path = QDir::homePath() + "/.config/lark/ai_history.json";
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) return;

    QByteArray data = file.readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isNull() && doc.isArray()) {
        QJsonArray arr = doc.array();
        m_chatHistory.clear();
        for (const auto &val : arr) {
            m_chatHistory.append(val.toObject());
        }
    }
}

void AIChatWindow::saveChatHistory() {
    QString path = QDir::homePath() + "/.config/lark/ai_history.json";
    QDir().mkpath(QFileInfo(path).absolutePath());
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly)) return;

    QJsonArray arr;
    for (const auto &obj : m_chatHistory) {
        arr.append(obj);
    }
    QJsonDocument doc(arr);
    file.write(doc.toJson());
}

void AIChatWindow::appendChatMessage(const QString &senderName, const QString &messageText, bool isUser) {
    QString bubbleHtml;
    if (isUser) {
        bubbleHtml = QString(
            "<table width=\"100%\"><tr><td align=\"right\">"
            "  <div style=\"background-color: #1f6feb; color: white; padding: 10px 14px; border-radius: 12px; max-width: 80%; text-align: left; margin: 4px 0;\">"
            "    <span style=\"font-size: 10px; opacity: 0.8; font-weight: bold;\">%1</span><br>"
            "    <span style=\"font-size: 13px;\">%2</span>"
            "  </div>"
            "</td></tr></table>"
        ).arg(senderName, messageText.toHtmlEscaped());
    } else {
        // Scraped HTML from websites (Gemini/Copilot) contains formatting tags which we preserve
        bubbleHtml = QString(
            "<table width=\"100%\"><tr><td align=\"left\">"
            "  <div style=\"background-color: #21262d; border: 1px solid #30363d; color: #c9d1d9; padding: 10px 14px; border-radius: 12px; max-width: 80%; text-align: left; margin: 4px 0;\">"
            "    <span style=\"font-size: 10px; color: #8b949e; font-weight: bold;\">%1</span><br>"
            "    <span style=\"font-size: 13px;\">%2</span>"
            "  </div>"
            "</td></tr></table>"
        ).arg(senderName, messageText);
    }
    
    m_chatDisplay->append(bubbleHtml);
    m_chatDisplay->verticalScrollBar()->setValue(m_chatDisplay->verticalScrollBar()->maximum());
}