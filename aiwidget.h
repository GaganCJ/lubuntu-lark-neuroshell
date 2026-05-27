#ifndef AIWIDGET_H
#define AIWIDGET_H

#include <QDialog>
#include <QLineEdit>
#include <QDBusInterface>
#include <QDBusReply>

// Forward declare QUltralightView
class QUltralightView;

class AIChatWindow : public QDialog {
    Q_OBJECT

public:
    explicit AIChatWindow(QWidget *parent = nullptr);

private slots:
    void sendPrompt();

private:
    void initializeWebCanvas();
    void handleAIResponse(const QString &rawResponse);

    QUltralightView *m_webView;
    QLineEdit *m_inputField;
};

#endif // AIWIDGET_H