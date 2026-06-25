#ifndef AIWIDGET_H
#define AIWIDGET_H

#include <QDialog>
#include <QFrame>
#include <QPushButton>
#include <QTextBrowser>
#include <QLineEdit>
#include <QLabel>
#include <QStackedWidget>
#include <QWebEngineView>
#include <QTimer>
#include <QJsonObject>
#include <QJsonArray>

class AIChatWindow : public QDialog {
    Q_OBJECT
public:
    AIChatWindow(QWidget *parent = nullptr);
    ~AIChatWindow();

private slots:
    void switchAIProvider(const QString &provider);
    void toggleViewMode();
    void handleSendPressed();
    void pollResponse();
    void purgeSessionData();
    void loadChatHistory();
    void saveChatHistory();

private:
    void appendChatMessage(const QString &senderName, const QString &messageText, bool isUser);
    void startAutomation(const QString &text);

    QFrame *m_container;
    QPushButton *m_geminiBtn;
    QPushButton *m_copilotBtn;
    QPushButton *m_toggleModeBtn;
    
    // View stack:
    // Index 0: Native Chat Layout
    // Index 1: Gemini Web View
    // Index 2: Copilot Web View
    QStackedWidget *m_viewStack;
    QWidget *m_nativeWidget;
    
    // Native Chat UI components:
    QTextBrowser *m_chatDisplay;
    QLineEdit *m_inputField;
    QPushButton *m_sendBtn;
    QLabel *m_statusLabel;
    
    // Background WebEngine views:
    QWebEngineView *m_geminiView;
    QWebEngineView *m_copilotView;
    
    // Automation state:
    QString m_activeProvider;
    bool m_isWebMode;
    QTimer *m_pollTimer;
    int m_pollAttempts;
    QString m_lastExtractedResponse;
};

#endif // AIWIDGET_H