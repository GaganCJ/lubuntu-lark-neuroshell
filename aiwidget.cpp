#include "aiwidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QJsonDocument>
#include <QJsonObject>

// Assuming Ultralight SDK include path
#include <QUltralightView>

AIChatWindow::AIChatWindow(QWidget *parent) : QDialog(parent) {
    setWindowFlags(Qt::Popup | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    resize(420, 560);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(8, 8, 8, 8);

    QFrame *container = new QFrame(this);
    container->setObjectName("CopilotContainer");
    container->setStyleSheet("QFrame#CopilotContainer { background-color: #0d1117; border: 1px solid #30363d; border-radius: 12px; }");
    QVBoxLayout *containerLayout = new QVBoxLayout(container);

    QHBoxLayout *header = new QHBoxLayout();
    QLabel *modelBadge = new QLabel("✨ gemma3:1b-it-qat", this);
    modelBadge->setStyleSheet("QLabel { color: #58a6ff; background-color: #161b22; border: 1px solid #30363d; border-radius: 10px; padding: 2px 10px; font-size: 11px; font-family: monospace; font-weight: bold; }");

    header->addWidget(modelBadge);
    header->addStretch();
    
    QPushButton *closeBtn = new QPushButton("✕", this);
    closeBtn->setStyleSheet("color: #8b949e; background: transparent; border: none; font-size: 14px;");
    connect(closeBtn, &QPushButton::clicked, this, &QWidget::hide);
    header->addWidget(closeBtn);
    containerLayout->addLayout(header);

    m_webView = new QUltralightView(this);
    containerLayout->addWidget(m_webView);
    initializeWebCanvas();

    m_inputField = new QLineEdit(this);
    m_inputField->setPlaceholderText("Ask NeuroShell or type '/' for tasks...");
    m_inputField->setStyleSheet("QLineEdit { color: #f0f6fc; background-color: #161b22; border: 1px solid #30363d; border-radius: 20px; padding: 10px 16px; font-size: 13px; } QLineEdit:focus { border: 1px solid #58a6ff; }");

    containerLayout->addWidget(m_inputField);

    mainLayout->addWidget(container);
    connect(m_inputField, &QLineEdit::returnPressed, this, &AIChatWindow::sendPrompt);
}

void AIChatWindow::initializeWebCanvas() {
    QString htmlTemplate = R"(
    <!DOCTYPE html><html><head>
    <link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/prism/1.29.0/themes/prism-tomorrow.min.css">
    <script src="https://cdnjs.cloudflare.com/ajax/libs/prism/1.29.0/prism.min.js"></script>
    <script src="https://cdnjs.cloudflare.com/ajax/libs/mermaid/10.2.4/mermaid.min.js"></script>
    <script>mermaid.initialize({startOnLoad:false, theme:'dark'});</script>
    <style>
        body { font-family: sans-serif; background-color: #0d1117; color: #c9d1d9; margin: 0; padding: 8px; font-size: 13px; }
        .message { margin-bottom: 16px; animation: fadeIn 0.2s ease-out; }
        .user-header { color: #ff7b72; font-weight: bold; font-size: 11px; }
        .ai-header { color: #58a6ff; font-weight: bold; font-size: 11px; }
        pre { background-color: #161b22 !important; border: 1px solid #30363d; border-radius: 6px; padding: 10px; overflow-x: auto; }
        table { border-collapse: collapse; width: 100%; margin: 10px 0; background: #161b22; }
        th, td { padding: 8px; border: 1px solid #30363d; text-align: left; }
        th { background-color: #21262d; }
        @keyframes fadeIn { from { opacity: 0; transform: translateY(2px); } to { opacity: 1; transform: translateY(0); } }
    </style></head><body><div id="canvas"></div><script>
        const cv = document.getElementById('canvas');
        function addMsg(user, text) {
            const cls = user ? 'user-header' : 'ai-header';
            const icon = user ? '👤 User' : '🤖 NeuroShell';
            cv.innerHTML += `<div class="message"><div class="${cls}">${icon}</div><div>${text}</div></div>`;
            Prism.highlightAll();
            mermaid.init(undefined, document.querySelectorAll('.mermaid'));
            window.scrollTo(0, document.body.scrollHeight);
        }
    </script></body></html>)";

    m_webView->loadHTML(htmlTemplate);
}

void AIChatWindow::sendPrompt() {
    QString text = m_inputField->text().trimmed();
    if (text.isEmpty()) return;

    QJsonObject uObj; uObj["t"] = text;
    QString uJs = QString("addMsg(true, %1.t);").arg(QJsonDocument(uObj).toJson(QJsonDocument::Compact));
    m_webView->evaluateJavaScript(uJs);
    m_inputField->clear();

    QDBusInterface neuroBus("org.lxqt.neuroshell", "/org/lxqt/neuroshell", "org.lxqt.neuroshell.Interface", QDBusConnection::sessionBus());
    if (neuroBus.isValid()) {
        QDBusReply<QString> reply = neuroBus.call("ProcessIntent", text);
        if (reply.isValid()) { handleAIResponse(reply.value()); }
        else { handleAIResponse("<span style='color:#f85149;'>D-Bus Payload Sync Failure.</span>"); }
    } else {
        handleAIResponse("<span style='color:#f85149;'>NeuroShell System Service is completely offline.</span>");
    }
}

void AIChatWindow::handleAIResponse(const QString &rawResponse) {
    QJsonObject aObj; aObj["t"] = rawResponse;
    QString aJs = QString("addMsg(false, %1.t);").arg(QJsonDocument(aObj).toJson(QJsonDocument::Compact));
    m_webView->evaluateJavaScript(aJs);
}