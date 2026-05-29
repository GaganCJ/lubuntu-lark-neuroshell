#include "aiwidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QJsonDocument>
#include <QJsonObject>

// Use native Qt widgets for the UI
#include <QTextBrowser>

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

    m_textBrowser = new QTextBrowser(this);
    m_textBrowser->setOpenExternalLinks(true);
    m_textBrowser->setStyleSheet("QTextBrowser { background-color: #0d1117; border: none; }");
    m_textBrowser->document()->setDefaultStyleSheet(R"(
        body { font-family: sans-serif; color: #c9d1d9; font-size: 13px; }
        pre {
            background-color: #161b22;
            border: 1px solid #30363d;
            border-radius: 6px;
            padding: 10px;
            font-family: monospace;
            white-space: pre-wrap;
        }
        table { border-collapse: collapse; width: 100%; margin: 10px 0; background: #161b22; }
        th, td { padding: 8px; border: 1px solid #30363d; text-align: left; }
        th { background-color: #21262d; }
    )");
    containerLayout->addWidget(m_textBrowser);

    m_inputField = new QLineEdit(this);
    m_inputField->setPlaceholderText("Ask NeuroShell or type '/' for tasks...");
    m_inputField->setStyleSheet("QLineEdit { color: #f0f6fc; background-color: #161b22; border: 1px solid #30363d; border-radius: 20px; padding: 10px 16px; font-size: 13px; } QLineEdit:focus { border: 1px solid #58a6ff; }");

    containerLayout->addWidget(m_inputField);

    mainLayout->addWidget(container);
    connect(m_inputField, &QLineEdit::returnPressed, this, &AIChatWindow::sendPrompt);
}

void AIChatWindow::sendPrompt() {
    QString text = m_inputField->text().trimmed();
    if (text.isEmpty()) return;

    QString userHtml = QString(
        R"(<div style="color: #ff7b72; font-weight: bold; font-size: 11px; margin-top: 8px;">👤 User</div>)"
        R"(<div style="margin-bottom: 16px;">%1</div>)"
    ).arg(text.toHtmlEscaped());

    m_textBrowser->append(userHtml);
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
    QString aiHtml = QString(
        R"(<div style="color: #58a6ff; font-weight: bold; font-size: 11px;">🤖 NeuroShell</div>)"
        R"(<div style="margin-bottom: 16px;">%1</div>)"
    ).arg(rawResponse); // rawResponse is expected to be HTML from the daemon
    m_textBrowser->append(aiHtml);
}